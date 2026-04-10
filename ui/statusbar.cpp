#include "statusbar.h"
#include <QLabel>

StatusBar::StatusBar(QWidget *parent)
    : QStatusBar(parent)
{
    // Connection status indicator
    m_connectionLabel = new QLabel(this);
    m_connectionLabel->setMinimumWidth(80);
    m_connectionLabel->setStyleSheet("background-color: #f44336; color: white; padding: 2px 8px; border-radius: 3px;");
    m_connectionLabel->setText("Disconnected");
    addPermanentWidget(m_connectionLabel);
    
    // Port name
    m_portLabel = new QLabel(this);
    m_portLabel->setText("");
    addPermanentWidget(m_portLabel);
    
    // Baud rate
    m_baudLabel = new QLabel(this);
    m_baudLabel->setText("");
    addPermanentWidget(m_baudLabel);
    
    // Bytes transferred
    m_bytesLabel = new QLabel(this);
    m_bytesLabel->setText("RX: 0 B | TX: 0 B");
    addPermanentWidget(m_bytesLabel);
    
    // Status message (left side)
    m_statusLabel = new QLabel(this);
    addWidget(m_statusLabel, 1);
}

StatusBar::~StatusBar()
{
}

void StatusBar::setConnectionStatus(bool connected)
{
    if (connected) {
        m_connectionLabel->setText("● Connected");
        m_connectionLabel->setStyleSheet("background-color: #4CAF50; color: white; padding: 2px 8px; border-radius: 3px;");
    } else {
        m_connectionLabel->setText("○ Disconnected");
        m_connectionLabel->setStyleSheet("background-color: #f44336; color: white; padding: 2px 8px; border-radius: 3px;");
    }
}

void StatusBar::setPortName(const QString &portName)
{
    m_portLabel->setText(portName.isEmpty() ? "" : QString("Port: %1").arg(portName));
}

void StatusBar::setBaudRate(int baudRate)
{
    m_baudLabel->setText(baudRate > 0 ? QString("%1 bps").arg(baudRate) : "");
}

void StatusBar::setBytesTransferred(quint64 received, quint64 sent)
{
    m_bytesLabel->setText(QString("RX: %1 B | TX: %2 B").arg(received).arg(sent));
}

void StatusBar::setError(const QString &error)
{
    showMessage(error, 5000);
}
