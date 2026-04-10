#ifndef SERIALBRIDGE_H
#define SERIALBRIDGE_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QMap>
#include <QJsonDocument>

class SerialPortManager;

// JSON-RPC 2.0 WebSocket bridge for serial communication
class SerialBridge : public QObject
{
    Q_OBJECT

public:
    explicit SerialBridge(QObject *parent = nullptr);
    ~SerialBridge();

    bool start(quint16 port = 8765);
    void stop();
    bool isRunning() const;
    
    void setSerialManager(SerialPortManager *manager);

signals:
    void clientConnected(const QString &clientId);
    void clientDisconnected(const QString &clientId);
    void messageReceived(const QString &clientId, const QJsonObject &message);

private slots:
    void onNewConnection();
    void onTextMessageReceived(const QString &message);
    void onSocketDisconnected();
    void onError(QAbstractSocket::SocketError error);

private:
    void processJsonRpcRequest(QWebSocket *client, const QJsonObject &request);
    QJsonObject createResponse(int id, const QVariant &result);
    QJsonObject createError(int id, int code, const QString &message);
    
    // JSON-RPC methods
    QJsonObject handleConnect(const QJsonObject &params);
    QJsonObject handleDisconnect(const QJsonObject &params);
    QJsonObject handleWrite(const QJsonObject &params);
    QJsonObject handleRead(const QJsonObject &params);
    QJsonObject handleListPorts(const QJsonObject &params);
    QJsonObject handleSubscribe(const QJsonObject &params);

    QWebSocketServer *m_server;
    QMap<QWebSocket*, QString> m_clients;
    SerialPortManager *m_serialManager;
    
    quint16 m_port;
    bool m_running;
};

#endif // SERIALBRIDGE_H
