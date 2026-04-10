#include "receivewidget.h"
#include <QTextEdit>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTextCursor>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QRegularExpression>

ReceiveWidget::ReceiveWidget(QWidget *parent)
    : QWidget(parent)
    , m_bytesReceived(0)
    , m_bytesSent(0)
    , m_textMode(true)
{
    setupUI();
}

ReceiveWidget::~ReceiveWidget()
{
}

void ReceiveWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Group box
    QGroupBox *groupBox = new QGroupBox(tr("Received Data"), this);
    QVBoxLayout *groupLayout = new QVBoxLayout(groupBox);
    
    // Text edit with virtual list optimization
    m_textEdit = new QTextEdit(groupBox);
    m_textEdit->setReadOnly(true);
    m_textEdit->setLineWrapMode(QTextEdit::NoWrap);
    m_textEdit->setFont(QFont("Consolas", 10));
    
    // Optimize for large content (100k+ lines)
    m_textEdit->document()->setMaximumBlockCount(100000);
    
    groupLayout->addWidget(m_textEdit);
    
    mainLayout->addWidget(groupBox);
    
    // Control panel
    QWidget *controlPanel = new QWidget(this);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlPanel);
    controlLayout->setContentsMargins(0, 0, 0, 0);
    
    // Display options
    m_textModeCheck = new QCheckBox(tr("Text Mode"), controlPanel);
    m_textModeCheck->setChecked(true);
    connect(m_textModeCheck, &QCheckBox::toggled, this, [this](bool checked){
        m_textMode = checked;
    });
    
    m_timestampCheck = new QCheckBox(tr("Timestamp"), controlPanel);
    m_timestampCheck->setChecked(true);
    
    m_autoScrollCheck = new QCheckBox(tr("Auto Scroll"), controlPanel);
    m_autoScrollCheck->setChecked(true);
    
    m_lineWrapCheck = new QCheckBox(tr("Line Wrap"), controlPanel);
    connect(m_lineWrapCheck, &QCheckBox::toggled, m_textEdit, [this](bool checked){
        m_textEdit->setLineWrapMode(checked ? QTextEdit::WidgetWidth : QTextEdit::NoWrap);
    });
    
    controlLayout->addWidget(m_textModeCheck);
    controlLayout->addWidget(m_timestampCheck);
    controlLayout->addWidget(m_autoScrollCheck);
    controlLayout->addWidget(m_lineWrapCheck);
    controlLayout->addStretch();
    
    // Statistics label
    m_statsLabel = new QLabel(tr("RX: 0 B | TX: 0 B"), controlPanel);
    controlLayout->addWidget(m_statsLabel);
    
    // Buttons
    m_saveButton = new QPushButton(tr("Save"), controlPanel);
    connect(m_saveButton, &QPushButton::clicked, this, [this](){
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save Log"),
            QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss") + ".log",
            tr("Log Files (*.log);;All Files (*)"));
        
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                file.write(m_textEdit->toPlainText().toUtf8());
                file.close();
            }
        }
    });
    
    m_clearButton = new QPushButton(tr("Clear"), controlPanel);
    connect(m_clearButton, &QPushButton::clicked, this, [this](){
        m_textEdit->clear();
        emit clearRequested();
    });
    
    controlLayout->addWidget(m_saveButton);
    controlLayout->addWidget(m_clearButton);
    
    mainLayout->addWidget(controlPanel);
}

void ReceiveWidget::appendData(const QByteArray &data)
{
    m_bytesReceived += data.size();
    
    QString formatted = formatData(data, m_textMode);
    
    m_textEdit->moveCursor(QTextCursor::End);
    m_textEdit->insertHtml(formatted);
    
    if (m_autoScrollCheck->isChecked()) {
        m_textEdit->verticalScrollBar()->setValue(m_textEdit->verticalScrollBar()->maximum());
    }
    
    updateStatistics(m_bytesReceived, m_bytesSent);
}

void ReceiveWidget::appendSentData(const QByteArray &data)
{
    m_bytesSent += data.size();
    
    QString formatted = formatData(data, m_textMode);
    
    // Highlight sent data differently
    QString html = QString("<span style='color: #4CAF50;'>%1</span>").arg(formatted);
    
    m_textEdit->moveCursor(QTextCursor::End);
    m_textEdit->insertHtml(html);
    
    if (m_autoScrollCheck->isChecked()) {
        m_textEdit->verticalScrollBar()->setValue(m_textEdit->verticalScrollBar()->maximum());
    }
    
    updateStatistics(m_bytesReceived, m_bytesSent);
}

void ReceiveWidget::clear()
{
    m_textEdit->clear();
    m_bytesReceived = 0;
    m_bytesSent = 0;
    updateStatistics(0, 0);
}

void ReceiveWidget::setTextMode(bool textMode)
{
    m_textMode = textMode;
    m_textModeCheck->setChecked(textMode);
}

bool ReceiveWidget::isTextMode() const
{
    return m_textMode;
}

void ReceiveWidget::searchAndHighlight(const QString &pattern, bool useRegex)
{
    QTextDocument *doc = m_textEdit->document();
    QTextCursor cursor(doc);
    
    QTextCharFormat highlightFormat;
    highlightFormat.setBackground(Qt::yellow);
    
    if (useRegex) {
        QRegularExpression re(pattern);
        QRegularExpressionMatchIterator it = re.globalMatch(doc->toPlainText());
        
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            cursor.setPosition(match.capturedStart());
            cursor.setPosition(match.capturedEnd(), QTextCursor::KeepAnchor);
            cursor.setCharFormat(highlightFormat);
        }
    } else {
        doc->findAndReplace(pattern, pattern, Qt::CaseInsensitive);
    }
}

void ReceiveWidget::updateStatistics(quint64 received, quint64 sent)
{
    m_statsLabel->setText(tr("RX: %1 B | TX: %2 B").arg(received).arg(sent));
}

QString ReceiveWidget::formatData(const QByteArray &data, bool textMode) const
{
    QString result;
    
    if (m_timestampCheck->isChecked()) {
        result += QString("<span style='color: #888;'>[%1]</span> ")
            .arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"));
    }
    
    if (textMode) {
        // Try to detect encoding and convert
        QString text = QString::fromUtf8(data);
        if (text.contains(QChar::ReplacementCharacter)) {
            text = QString::fromLatin1(data);
        }
        
        // Escape HTML
        text = text.toHtmlEscaped();
        result += text;
    } else {
        // HEX mode
        result += "<span style='color: #2196F3;'>" + bytesToHex(data) + "</span>";
    }
    
    result += "<br>";
    
    return result;
}

QString ReceiveWidget::bytesToHex(const QByteArray &data) const
{
    QString hex;
    for (int i = 0; i < data.size(); ++i) {
        if (i > 0 && i % 16 == 0) {
            hex += "\n";
        } else if (i > 0) {
            hex += " ";
        }
        hex += QString("%1").arg(static_cast<quint8>(data[i]), 2, 16, QChar('0')).toUpper();
    }
    return hex;
}

QString ReceiveWidget::detectEncoding(const QByteArray &data) const
{
    // Simple heuristic: try UTF-8 first, fallback to Latin-1
    QString text = QString::fromUtf8(data);
    if (!text.contains(QChar::ReplacementCharacter)) {
        return "UTF-8";
    }
    return "Latin-1";
}
