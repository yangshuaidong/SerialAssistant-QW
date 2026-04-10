#include "sendwidget.h"
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDateTime>
#include <QRegularExpression>
#include <QMenu>

SendWidget::SendWidget(QWidget *parent)
    : QWidget(parent)
    , m_hexMode(false)
{
    setupUI();
}

SendWidget::~SendWidget()
{
}

void SendWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Group box
    QGroupBox *groupBox = new QGroupBox(tr("Send Data"), this);
    QVBoxLayout *groupLayout = new QVBoxLayout(groupBox);
    
    // Input edit
    m_inputEdit = new QTextEdit(groupBox);
    m_inputEdit->setPlaceholderText(tr("Enter data to send..."));
    m_inputEdit->setFont(QFont("Consolas", 10));
    m_inputEdit->setMaximumHeight(150);
    
    connect(m_inputEdit, &QTextEdit::textChanged, this, [this]() {
        QByteArray data = getInputData();
        m_lengthLabel->setText(tr("Length: %1 bytes").arg(data.size()));
    });
    
    groupLayout->addWidget(m_inputEdit);
    
    mainLayout->addWidget(groupBox);
    
    // Control panel
    QWidget *controlPanel = new QWidget(this);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlPanel);
    controlLayout->setContentsMargins(0, 0, 0, 0);
    
    // Mode options
    m_hexModeCheck = new QCheckBox(tr("HEX Mode"), controlPanel);
    connect(m_hexModeCheck, &QCheckBox::toggled, this, [this](bool checked){
        m_hexMode = checked;
    });
    
    m_appendNewlineCheck = new QCheckBox(tr("Append Newline"), controlPanel);
    m_echoCheck = new QCheckBox(tr("Echo"), controlPanel);
    m_echoCheck->setChecked(true);
    
    controlLayout->addWidget(m_hexModeCheck);
    controlLayout->addWidget(m_appendNewlineCheck);
    controlLayout->addWidget(m_echoCheck);
    controlLayout->addStretch();
    
    // Interval for auto-send
    m_intervalEdit = new QLineEdit(controlPanel);
    m_intervalEdit->setPlaceholderText(tr("Interval (ms)"));
    m_intervalEdit->setMaximumWidth(80);
    controlLayout->addWidget(m_intervalEdit);
    
    // Length label
    m_lengthLabel = new QLabel(tr("Length: 0 bytes"), controlPanel);
    controlLayout->addWidget(m_lengthLabel);
    
    mainLayout->addWidget(controlPanel);
    
    // Button panel
    QWidget *buttonPanel = new QWidget(this);
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonPanel);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    
    // Presets button with menu
    m_presetsButton = new QPushButton(tr("Presets ▼"), buttonPanel);
    QMenu *presetsMenu = new QMenu(this);
    presetsMenu->addAction(tr("Command 1"), this, [this]() {
        m_inputEdit->insertPlainText("AT\r\n");
    });
    presetsMenu->addAction(tr("Command 2"), this, [this]() {
        m_inputEdit->insertPlainText("HELP\r\n");
    });
    presetsMenu->addAction(tr("Add Custom..."), this, [this]() {
        // Would open dialog to add custom preset
    });
    m_presetsButton->setMenu(presetsMenu);
    
    m_sendButton = new QPushButton(tr("Send"), buttonPanel);
    m_sendButton->setShortcut(QKeySequence("Ctrl+Return"));
    connect(m_sendButton, &QPushButton::clicked, this, &SendWidget::onSendClicked);
    
    m_clearButton = new QPushButton(tr("Clear"), buttonPanel);
    connect(m_clearButton, &QPushButton::clicked, this, &SendWidget::onClearClicked);
    
    buttonLayout->addWidget(m_presetsButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_sendButton);
    buttonLayout->addWidget(m_clearButton);
    
    mainLayout->addWidget(buttonPanel);
}

void SendWidget::onSendClicked()
{
    QByteArray data = getInputData();
    if (!data.isEmpty()) {
        emit dataToSend(data);
    }
}

void SendWidget::onClearClicked()
{
    m_inputEdit->clear();
}

void SendWidget::setHexMode(bool hexMode)
{
    m_hexMode = hexMode;
    m_hexModeCheck->setChecked(hexMode);
}

bool SendWidget::isHexMode() const
{
    return m_hexMode;
}

void SendWidget::addPresetCommand(const QString &name, const QByteArray &data)
{
    // Would be implemented to add dynamic presets
    Q_UNUSED(name);
    Q_UNUSED(data);
}

void SendWidget::loadPresetCommands()
{
    // Would load from configuration
}

QByteArray SendWidget::getInputData() const
{
    QString input = m_inputEdit->toPlainText();
    
    // Parse variables like ${timestamp}, ${hex:...}
    input = parseVariables(input);
    
    if (m_hexMode) {
        // Convert HEX string to bytes
        QByteArray result;
        QString hex = input.remove(QRegularExpression("[^0-9A-Fa-f]"));
        
        for (int i = 0; i < hex.size(); i += 2) {
            bool ok;
            quint8 byte = hex.mid(i, 2).toInt(&ok, 16);
            if (ok) {
                result.append(static_cast<char>(byte));
            }
        }
        
        return result;
    } else {
        QByteArray result = input.toUtf8();
        
        if (m_appendNewlineCheck->isChecked()) {
            result.append("\r\n");
        }
        
        return result;
    }
}

QString SendWidget::parseVariables(const QString &input) const
{
    QString result = input;
    
    // Replace ${timestamp} with current timestamp
    QRegularExpression re("\\$\\{timestamp\\}");
    result.replace(re, QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    
    // Replace ${time:format} with custom format
    QRegularExpression reTime("\\$\\{time:([^}]+)\\}");
    QRegularExpressionMatchIterator it = reTime.globalMatch(result);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString format = match.captured(1);
        QString replacement = QDateTime::currentDateTime().toString(format);
        result.replace(match.capturedStart(), match.capturedLength(), replacement);
    }
    
    // Replace ${hex:XX XX XX} with binary data
    QRegularExpression reHex("\\$\\{hex:([^}]+)\\}");
    it = reHex.globalMatch(result);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString hexStr = match.captured(1).remove(" ");
        QString decoded;
        for (int i = 0; i < hexStr.size(); i += 2) {
            bool ok;
            int val = hexStr.mid(i, 2).toInt(&ok, 16);
            if (ok) {
                decoded.append(QChar(val));
            }
        }
        result.replace(match.capturedStart(), match.capturedLength(), decoded);
    }
    
    return result;
}
