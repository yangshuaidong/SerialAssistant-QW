#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QQueue>
#include <QMutex>

class SerialPortManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialPortManager(QObject *parent = nullptr);
    ~SerialPortManager();

    bool connect(const QString &portName, 
                 QSerialPort::BaudRate baudRate,
                 QSerialPort::DataBits dataBits,
                 QSerialPort::Parity parity,
                 QSerialPort::StopBits stopBits,
                 QSerialPort::FlowControl flowControl);
    
    bool disconnect();
    bool isConnected() const;
    
    qint64 write(const QByteArray &data);
    QString errorString() const;

    // Auto-reconnect settings
    void setAutoReconnectEnabled(bool enabled);
    void setMaxReconnectAttempts(int attempts);
    void setReconnectInterval(int ms);

signals:
    void dataReceived(const QByteArray &data);
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString &error);
    void portsUpdated(const QList<QSerialPortInfo> &ports);

private slots:
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);
    void checkHotplug();
    void attemptReconnect();

private:
    void startHotplugMonitor();
    void stopHotplugMonitor();
    void emitPortsUpdated();

    QSerialPort *m_serialPort;
    QTimer *m_hotplugTimer;
    QTimer *m_reconnectTimer;
    
    QList<QSerialPortInfo> m_knownPorts;
    
    bool m_autoReconnectEnabled;
    int m_maxReconnectAttempts;
    int m_reconnectInterval;
    int m_reconnectCount;
    
    QMutex m_writeMutex;
    
    // Backpressure queue for high-speed data
    QQueue<QByteArray> m_writeQueue;
    bool m_writing;
};

#endif // SERIALPORTMANAGER_H
