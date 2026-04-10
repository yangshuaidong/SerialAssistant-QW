#ifndef DATAPIPELINE_H
#define DATAPIPELINE_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QMutex>
#include <functional>

// Data processing hook interface
class IDataHook {
public:
    virtual ~IDataHook() = default;
    virtual QByteArray process(const QByteArray &data) = 0;
    virtual QString name() const = 0;
};

class DataPipeline : public QObject
{
    Q_OBJECT

public:
    explicit DataPipeline(QObject *parent = nullptr);
    ~DataPipeline();

    // Register/unregister data processing hooks
    void registerHook(IDataHook *hook);
    void unregisterHook(IDataHook *hook);
    
    // Process received data through all hooks
    QByteArray processReceived(const QByteArray &data);
    QByteArray processToSend(const QByteArray &data);
    
    // Statistics
    quint64 totalBytesReceived() const;
    quint64 totalBytesSent() const;
    void resetStatistics();

signals:
    void dataProcessed(const QByteArray &original, const QByteArray &processed);
    void statisticsUpdated(quint64 received, quint64 sent);

private:
    QList<IDataHook*> m_receiveHooks;
    QList<IDataHook*> m_sendHooks;
    QMutex m_mutex;
    
    quint64 m_bytesReceived;
    quint64 m_bytesSent;
};

#endif // DATAPIPELINE_H
