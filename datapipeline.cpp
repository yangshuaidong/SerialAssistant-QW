#include "datapipeline.h"

DataPipeline::DataPipeline(QObject *parent)
    : QObject(parent)
    , m_bytesReceived(0)
    , m_bytesSent(0)
{
}

DataPipeline::~DataPipeline()
{
}

void DataPipeline::registerHook(IDataHook *hook)
{
    QMutexLocker locker(&m_mutex);
    if (hook && !m_receiveHooks.contains(hook)) {
        m_receiveHooks.append(hook);
    }
}

void DataPipeline::unregisterHook(IDataHook *hook)
{
    QMutexLocker locker(&m_mutex);
    m_receiveHooks.removeAll(hook);
    m_sendHooks.removeAll(hook);
}

QByteArray DataPipeline::processReceived(const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    
    m_bytesReceived += data.size();
    
    QByteArray processed = data;
    for (IDataHook *hook : m_receiveHooks) {
        processed = hook->process(processed);
    }
    
    emit dataProcessed(data, processed);
    emit statisticsUpdated(m_bytesReceived, m_bytesSent);
    
    return processed;
}

QByteArray DataPipeline::processToSend(const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    
    m_bytesSent += data.size();
    
    QByteArray processed = data;
    for (IDataHook *hook : m_sendHooks) {
        processed = hook->process(processed);
    }
    
    emit statisticsUpdated(m_bytesReceived, m_bytesSent);
    
    return processed;
}

quint64 DataPipeline::totalBytesReceived() const
{
    return m_bytesReceived;
}

quint64 DataPipeline::totalBytesSent() const
{
    return m_bytesSent;
}

void DataPipeline::resetStatistics()
{
    QMutexLocker locker(&m_mutex);
    m_bytesReceived = 0;
    m_bytesSent = 0;
    emit statisticsUpdated(0, 0);
}
