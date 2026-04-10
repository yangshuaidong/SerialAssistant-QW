#ifndef RECEIVEWIDGET_H
#define RECEIVEWIDGET_H

#include <QWidget>
#include <QByteArray>

class QTextEdit;
class QCheckBox;
class QLabel;
class QPushButton;

class ReceiveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ReceiveWidget(QWidget *parent = nullptr);
    ~ReceiveWidget();

    void appendData(const QByteArray &data);
    void appendSentData(const QByteArray &data);
    
    void clear();
    
    // Display mode
    void setTextMode(bool textMode);
    bool isTextMode() const;
    
    // Search and highlight
    void searchAndHighlight(const QString &pattern, bool useRegex = false);
    
    // Statistics
    void updateStatistics(quint64 received, quint64 sent);

signals:
    void clearRequested();

private:
    void setupUI();
    QString formatData(const QByteArray &data, bool textMode) const;
    QString bytesToHex(const QByteArray &data) const;
    QString detectEncoding(const QByteArray &data) const;

    QTextEdit *m_textEdit;
    QCheckBox *m_textModeCheck;
    QCheckBox *m_timestampCheck;
    QCheckBox *m_autoScrollCheck;
    QCheckBox *m_lineWrapCheck;
    QLabel *m_statsLabel;
    QPushButton *m_clearButton;
    QPushButton *m_saveButton;
    
    quint64 m_bytesReceived;
    quint64 m_bytesSent;
    bool m_textMode;
};

#endif // RECEIVEWIDGET_H
