#include "serialbridge.h"
#include "serialportmanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>
#include <QUuid>
#include <QSerialPortInfo>

SerialBridge::SerialBridge(QObject *parent)
    : QObject(parent)
    , m_server(new QWebSocketServer(QStringLiteral("SerialBridge"), 
                                     QWebSocketServer::NonSecureMode, this))
    , m_serialManager(nullptr)
    , m_port(8765)
    , m_running(false)
{
    connect(m_server, &QWebSocketServer::newConnection, this, &SerialBridge::onNewConnection);
}

SerialBridge::~SerialBridge()
{
    stop();
}

bool SerialBridge::start(quint16 port)
{
    if (m_running) {
        return false;
    }
    
    m_port = port;
    
    if (m_server->listen(QHostAddress::LocalHost, m_port)) {
        m_running = true;
        qDebug() << "SerialBridge listening on port" << m_port;
        return true;
    }
    
    qWarning() << "Failed to start SerialBridge:" << m_server->errorString();
    return false;
}

void SerialBridge::stop()
{
    if (!m_running) {
        return;
    }
    
    // Close all client connections
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        it.key()->close();
    }
    m_clients.clear();
    
    m_server->close();
    m_running = false;
    
    qDebug() << "SerialBridge stopped";
}

bool SerialBridge::isRunning() const
{
    return m_running;
}

void SerialBridge::setSerialManager(SerialPortManager *manager)
{
    m_serialManager = manager;
}

void SerialBridge::onNewConnection()
{
    QWebSocket *socket = m_server->nextPendingConnection();
    
    QString clientId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_clients[socket] = clientId;
    
    connect(socket, &QWebSocket::textMessageReceived, 
            this, &SerialBridge::onTextMessageReceived);
    connect(socket, &QWebSocket::disconnected, 
            this, &SerialBridge::onSocketDisconnected);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, &SerialBridge::onError);
    
    qDebug() << "Client connected:" << clientId;
    emit clientConnected(clientId);
    
    // Send welcome message
    QJsonObject welcome;
    welcome["jsonrpc"] = "2.0";
    welcome["method"] = "welcome";
    welcome["params"] = QJsonObject{{"clientId", clientId}, {"version", "1.0"}};
    
    socket->sendTextMessage(QString::fromUtf8(QJsonDocument(welcome).toJson()));
}

void SerialBridge::onTextMessageReceived(const QString &message)
{
    QWebSocket *socket = qobject_cast<QWebSocket*>(sender());
    if (!socket || !m_clients.contains(socket)) {
        return;
    }
    
    QString clientId = m_clients[socket];
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject errorMsg = createError(-1, -32700, "Parse error: " + error.errorString());
        socket->sendTextMessage(QString::fromUtf8(QJsonDocument(errorMsg).toJson()));
        return;
    }
    
    QJsonObject request = doc.object();
    processJsonRpcRequest(socket, request);
}

void SerialBridge::onSocketDisconnected()
{
    QWebSocket *socket = qobject_cast<QWebSocket*>(sender());
    if (!socket) {
        return;
    }
    
    if (m_clients.contains(socket)) {
        QString clientId = m_clients.take(socket);
        qDebug() << "Client disconnected:" << clientId;
        emit clientDisconnected(clientId);
    }
    
    socket->deleteLater();
}

void SerialBridge::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    QWebSocket *socket = qobject_cast<QWebSocket*>(sender());
    if (socket) {
        qWarning() << "WebSocket error:" << socket->errorString();
    }
}

void SerialBridge::processJsonRpcRequest(QWebSocket *client, const QJsonObject &request)
{
    // Validate JSON-RPC 2.0 format
    if (request["jsonrpc"].toString() != "2.0") {
        QJsonObject errorMsg = createError(request["id"].toInt(), -32600, "Invalid Request");
        client->sendTextMessage(QString::fromUtf8(QJsonDocument(errorMsg).toJson()));
        return;
    }
    
    QString method = request["method"].toString();
    QJsonObject params = request["params"].toObject();
    int id = request["id"].toInt();
    
    QJsonObject response;
    
    if (method == "connect") {
        response = handleConnect(params);
    } else if (method == "disconnect") {
        response = handleDisconnect(params);
    } else if (method == "write") {
        response = handleWrite(params);
    } else if (method == "read") {
        response = handleRead(params);
    } else if (method == "listPorts") {
        response = handleListPorts(params);
    } else if (method == "subscribe") {
        response = handleSubscribe(params);
    } else {
        response = createError(id, -32601, "Method not found: " + method);
    }
    
    response["id"] = id;
    client->sendTextMessage(QString::fromUtf8(QJsonDocument(response).toJson()));
}

QJsonObject SerialBridge::createResponse(int id, const QVariant &result)
{
    QJsonObject response;
    response["jsonrpc"] = "2.0";
    response["id"] = id;
    response["result"] = QJsonValue::fromVariant(result);
    return response;
}

QJsonObject SerialBridge::createError(int id, int code, const QString &message)
{
    QJsonObject response;
    response["jsonrpc"] = "2.0";
    response["id"] = id;
    response["error"] = QJsonObject{
        {"code", code},
        {"message", message}
    };
    return response;
}

QJsonObject SerialBridge::handleConnect(const QJsonObject &params)
{
    if (!m_serialManager) {
        return createError(-1, -32000, "Serial manager not available");
    }
    
    QString portName = params["port"].toString();
    int baudRate = params["baudRate"].toInt(115200);
    
    // Convert to Qt enums (simplified)
    QSerialPort::BaudRate br = static_cast<QSerialPort::BaudRate>(baudRate);
    
    bool success = m_serialManager->connect(portName, br, 
                                             QSerialPort::Data8,
                                             QSerialPort::NoParity,
                                             QSerialPort::OneStop,
                                             QSerialPort::NoFlowControl);
    
    if (success) {
        return createResponse(-1, true);
    } else {
        return createError(-1, -32001, m_serialManager->errorString());
    }
}

QJsonObject SerialBridge::handleDisconnect(const QJsonObject &params)
{
    Q_UNUSED(params);
    
    if (!m_serialManager) {
        return createError(-1, -32000, "Serial manager not available");
    }
    
    bool success = m_serialManager->disconnect();
    return createResponse(-1, success);
}

QJsonObject SerialBridge::handleWrite(const QJsonObject &params)
{
    if (!m_serialManager) {
        return createError(-1, -32000, "Serial manager not available");
    }
    
    QByteArray data = QByteArray::fromBase64(params["data"].toString().toUtf8());
    qint64 written = m_serialManager->write(data);
    
    return createResponse(-1, written >= 0);
}

QJsonObject SerialBridge::handleRead(const QJsonObject &params)
{
    Q_UNUSED(params);
    
    // Note: Actual implementation would need async handling
    return createResponse(-1, QByteArray());
}

QJsonObject SerialBridge::handleListPorts(const QJsonObject &params)
{
    Q_UNUSED(params);
    
    QJsonArray ports;
    QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();
    
    for (const QSerialPortInfo &port : availablePorts) {
        QJsonObject portInfo;
        portInfo["portName"] = port.portName();
        portInfo["description"] = port.description();
        portInfo["manufacturer"] = port.manufacturer();
        portInfo["serialNumber"] = port.serialNumber();
        ports.append(portInfo);
    }
    
    return createResponse(-1, ports);
}

QJsonObject SerialBridge::handleSubscribe(const QJsonObject &params)
{
    QString event = params["event"].toString();
    Q_UNUSED(event);
    
    // Note: Would implement event subscription logic
    return createResponse(-1, true);
}
