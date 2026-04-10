#ifndef SENDWIDGET_H
#define SENDWIDGET_H

#include <QWidget>
#include <QByteArray>

class QTextEdit;
class QCheckBox;
class QPushButton;
class QLabel;
class QLineEdit;

class SendWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SendWidget(QWidget *parent = nullptr);
    ~SendWidget();

    void setHexMode(bool hexMode);
    bool isHexMode() const;
    
    void addPresetCommand(const QString &name, const QByteArray &data);
    void loadPresetCommands();

signals:
    void dataToSend(const QByteArray &data);
    void sendRequested(const QByteArray &data);

private slots:
    void onSendClicked();
    void onClearClicked();

private:
    void setupUI();
    QByteArray getInputData() const;
    QString parseVariables(const QString &input) const;

    QTextEdit *m_inputEdit;
    QCheckBox *m_hexModeCheck;
    QCheckBox *m_appendNewlineCheck;
    QCheckBox *m_echoCheck;
    QPushButton *m_sendButton;
    QPushButton *m_clearButton;
    QPushButton *m_presetsButton;
    QLabel *m_lengthLabel;
    QLineEdit *m_intervalEdit;
    
    bool m_hexMode;
};

#endif // SENDWIDGET_H
