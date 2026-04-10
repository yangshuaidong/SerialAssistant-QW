#include "serialportmanager.h"
#include <QDebug>
#include <QThread>

SerialPortManager::SerialPortManager(QObject *parent)
    : QObject(parent)
    , m_serialPort(new QSerialPort(this))
    , m_hotplugTimer(new QTimer(this))
    , m_reconnectTimer(new QTimer(this))
    , m_autoReconnectEnabled(true)
    , m_maxReconnectAttempts(5)
    , m_reconnectInterval(1000)
    , m_reconnectCount(0)
    , m_writing(false)
{
    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialPortManager::onReadyRead);
    connect(m_serialPort, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::error), 
            this, &SerialPortManager::onErrorOccurred);
    
    connect(m_hotplugTimer, &QTimer::timeout, this, &SerialPortManager::checkHotplug);
    connect(m_reconnectTimer, &QTimer::timeout, this, &SerialPortManager::attemptReconnect);
    m_reconnectTimer->setSingleShot(true);
    
    startHotplugMonitor();
}

SerialPortManager::~SerialPortManager()
{
    stopHotplugMonitor();
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }
}

bool SerialPortManager::connect(const QString &portName,
                                 QSerialPort::BaudRate baudRate,
                                 QSerialPort::DataBits dataBits,
                                 QSerialPort::Parity parity,
                                 QSerialPort::StopBits stopBits,
                                 QSerialPort::FlowControl flowControl)
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }
    
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(dataBits);
    m_serialPort->setParity(parity);
    m_serialPort->setStopBits(stopBits);
    m_serialPort->setFlowControl(flowControl);
    
    // Set read buffer size for high-speed data
    m_serialPort->setReadBufferSize(65536);
    
    if (!m_serialPort->open(QIODevice::ReadWrite)) {
        emit errorOccurred(tr("Failed to open port: %1").arg(m_serialPort->errorString()));
        return false;
    }
    
    m_reconnectCount = 0;
    emit connectionStatusChanged(true);
    emit errorOccurred(tr("Connected to %1").arg(portName));
    
    return true;
}

bool SerialPortManager::disconnect()
{
    m_reconnectTimer->stop();
    
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
        emit connectionStatusChanged(false);
        return true;
    }
    
    return false;
}

bool SerialPortManager::isConnected() const
{
    return m_serialPort->isOpen();
}

qint64 SerialPortManager::write(const QByteArray &data)
{
    QMutexLocker locker(&m_writeMutex);
    
    if (!m_serialPort->isOpen()) {
        return -1;
    }
    
    // Add to queue for backpressure handling
    m_writeQueue.enqueue(data);
    
    if (!m_writing) {
        m_writing = true;
        // Process queue in next iteration
        QMetaObject::invokeMethod(this, [this]() {
            while (!m_writeQueue.isEmpty() && m_serialPort->isOpen()) {
                QByteArray data = m_writeQueue.dequeue();
                qint64 written = m_serialPort->write(data);
                if (written < 0) {
                    break;
                }
                m_serialPort->waitForBytesWritten(100);
            }
            m_writing = false;
        }, Qt::QueuedConnection);
    }
    
    return data.size();
}

QString SerialPortManager::errorString() const
{
    return m_serialPort->errorString();
}

void SerialPortManager::setAutoReconnectEnabled(bool enabled)
{
    m_autoReconnectEnabled = enabled;
}

void SerialPortManager::setMaxReconnectAttempts(int attempts)
{
    m_maxReconnectAttempts = attempts;
}

void SerialPortManager::setReconnectInterval(int ms)
{
    m_reconnectInterval = ms;
}

void SerialPortManager::onReadyRead()
{
    while (m_serialPort->bytesAvailable() > 0) {
        QByteArray data = m_serialPort->readAll();
        emit dataReceived(data);
    }
}

void SerialPortManager::onErrorOccurred(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) {
        return;
    }
    
    QString errorMsg = m_serialPort->errorString();
    emit errorOccurred(errorMsg);
    
    // Handle disconnection for auto-reconnect
    if (error == QSerialPort::DeviceError || error == QSerialPort::TimeoutError) {
        if (m_autoReconnectEnabled && m_reconnectCount < m_maxReconnectAttempts) {
            m_reconnectTimer->start(m_reconnectInterval);
        }
    }
}

void SerialPortManager::checkHotplug()
{
    QList<QSerialPortInfo> currentPorts = QSerialPortInfo::availablePorts();
    
    bool changed = false;
    if (currentPorts.size() != m_knownPorts.size()) {
        changed = true;
    } else {
        for (int i = 0; i < currentPorts.size(); ++i) {
            if (currentPorts[i].portName() != m_knownPorts[i].portName()) {
                changed = true;
                break;
            }
        }
    }
    
    if (changed) {
        m_knownPorts = currentPorts;
        emit portsUpdated(currentPorts);
    }
}

void SerialPortManager::attemptReconnect()
{
    if (!m_autoReconnectEnabled) {
        return;
    }
    
    m_reconnectCount++;
    
    // Get the last known port name
    QString portName = m_serialPort->portName();
    if (portName.isEmpty()) {
        return;
    }
    
    emit errorOccurred(tr("Attempting reconnect (%1/%2)...").arg(m_reconnectCount).arg(m_maxReconnectAttempts));
    
    if (connect(portName, 
                static_cast<QSerialPort::BaudRate>(m_serialPort->baudRate()),
                m_serialPort->dataBits(),
                m_serialPort->parity(),
                m_serialPort->stopBits(),
                m_serialPort->flowControl())) {
        emit errorOccurred(tr("Reconnected successfully"));
        m_reconnectCount = 0;
    } else if (m_reconnectCount >= m_maxReconnectAttempts) {
        emit errorOccurred(tr("Max reconnect attempts reached"));
    } else {
        m_reconnectTimer->start(m_reconnectInterval);
    }
}

void SerialPortManager::startHotplugMonitor()
{
    m_knownPorts = QSerialPortInfo::availablePorts();
    m_hotplugTimer->start(2000); // Check every 2 seconds
}

void SerialPortManager::stopHotplugMonitor()
{
    m_hotplugTimer->stop();
}

void SerialPortManager::emitPortsUpdated()
{
    emit portsUpdated(QSerialPortInfo::availablePorts());
}
