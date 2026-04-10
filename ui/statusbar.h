#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QStatusBar>

class QLabel;

class StatusBar : public QStatusBar
{
    Q_OBJECT

public:
    explicit StatusBar(QWidget *parent = nullptr);
    ~StatusBar();

    void setConnectionStatus(bool connected);
    void setPortName(const QString &portName);
    void setBaudRate(int baudRate);
    void setBytesTransferred(quint64 received, quint64 sent);
    void setError(const QString &error);

private:
    QLabel *m_connectionLabel;
    QLabel *m_portLabel;
    QLabel *m_baudLabel;
    QLabel *m_bytesLabel;
    QLabel *m_statusLabel;
};

#endif // STATUSBAR_H
