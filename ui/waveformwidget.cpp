#include "waveformwidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QPen>
#include <QFont>

WaveformWidget::WaveformWidget(QWidget *parent)
    : QWidget(parent)
    , m_maxPoints(1000)
    , m_autoScale(true)
    , m_yMin(0)
    , m_yMax(100)
    , m_scrollOffset(0)
{
    // Default single channel with blue color
    setChannelCount(1);
    setChannelColor(0, Qt::blue);
    setChannelName(0, tr("Channel 1"));
    
    setMinimumSize(400, 200);
    setStyleSheet("background-color: #1e1e1e;");
}

WaveformWidget::~WaveformWidget()
{
}

void WaveformWidget::addDataPoint(qreal value)
{
    addDataPoint(0, value);
}

void WaveformWidget::addDataPoint(int channel, qreal value)
{
    if (channel < 0 || channel >= m_channels.size()) {
        return;
    }
    
    m_channels[channel].append(value);
    
    // Remove old points if exceeding max
    while (m_channels[channel].size() > m_maxPoints) {
        m_channels[channel].removeFirst();
    }
    
    // Auto-scale
    if (m_autoScale && !m_channels[channel].isEmpty()) {
        m_yMin = m_channels[channel].min();
        m_yMax = m_channels[channel].max();
        
        // Add some padding
        qreal range = m_yMax - m_yMin;
        if (range < 0.001) range = 1.0;
        m_yMin -= range * 0.1;
        m_yMax += range * 0.1;
    }
    
    update();
}

void WaveformWidget::clear()
{
    for (auto &channel : m_channels) {
        channel.clear();
    }
    m_scrollOffset = 0;
    update();
}

void WaveformWidget::setChannelCount(int count)
{
    m_channels.resize(count);
    m_channelColors.resize(count);
    m_channelNames.resize(count);
    
    // Initialize default colors if not set
    static const QColor defaultColors[] = {
        Qt::blue, Qt::red, Qt::green, Qt::magenta,
        Qt::cyan, Qt::yellow, Qt::white
    };
    
    for (int i = 0; i < count; ++i) {
        if (i < sizeof(defaultColors)/sizeof(defaultColors[0])) {
            m_channelColors[i] = defaultColors[i];
        } else {
            m_channelColors[i] = Qt::white;
        }
        
        if (m_channelNames[i].isEmpty()) {
            m_channelNames[i] = tr("Channel %1").arg(i + 1);
        }
    }
    
    update();
}

void WaveformWidget::setChannelColor(int channel, const QColor &color)
{
    if (channel >= 0 && channel < m_channelColors.size()) {
        m_channelColors[channel] = color;
    }
}

void WaveformWidget::setChannelName(int channel, const QString &name)
{
    if (channel >= 0 && channel < m_channelNames.size()) {
        m_channelNames[channel] = name;
    }
}

void WaveformWidget::setMaxPoints(int maxPoints)
{
    m_maxPoints = maxPoints;
}

void WaveformWidget::setAutoScale(bool enabled)
{
    m_autoScale = enabled;
}

void WaveformWidget::setYRange(qreal min, qreal max)
{
    m_autoScale = false;
    m_yMin = min;
    m_yMax = max;
    update();
}

void WaveformWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Fill background
    painter.fillRect(rect(), QColor("#1e1e1e"));
    
    drawGrid(painter);
    
    // Draw each channel
    for (int i = 0; i < m_channels.size(); ++i) {
        if (!m_channels[i].isEmpty()) {
            drawChannel(painter, i);
        }
    }
    
    drawLabels(painter);
}

void WaveformWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    update();
}

void WaveformWidget::updateView()
{
    update();
}

void WaveformWidget::drawGrid(QPainter &painter)
{
    painter.save();
    
    QPen gridPen(QColor("#333333"), 1);
    painter.setPen(gridPen);
    
    int width = this->width();
    int height = this->height();
    
    // Vertical lines
    int vLines = 10;
    for (int i = 0; i <= vLines; ++i) {
        int x = i * width / vLines;
        painter.drawLine(x, 0, x, height);
    }
    
    // Horizontal lines
    int hLines = 5;
    for (int i = 0; i <= hLines; ++i) {
        int y = i * height / hLines;
        painter.drawLine(0, y, width, y);
    }
    
    painter.restore();
}

void WaveformWidget::drawChannel(QPainter &painter, int channel)
{
    if (m_channels[channel].isEmpty()) {
        return;
    }
    
    painter.save();
    
    QPen pen(m_channelColors[channel], 2);
    painter.setPen(pen);
    
    int width = this->width() - 60; // Leave space for labels
    int height = this->height() - 40;
    int offsetX = 50;
    int offsetY = 20;
    
    qreal range = m_yMax - m_yMin;
    if (range < 0.001) range = 1.0;
    
    // Draw polyline
    QPainterPath path;
    bool first = true;
    
    int pointsToShow = qMin(m_channels[channel].size(), width);
    int startIndex = m_channels[channel].size() - pointsToShow;
    if (startIndex < 0) startIndex = 0;
    
    for (int i = 0; i < pointsToShow; ++i) {
        int dataIndex = startIndex + i;
        qreal value = m_channels[channel][dataIndex];
        
        int x = offsetX + i;
        int y = offsetY + height - static_cast<int>((value - m_yMin) / range * height);
        
        if (first) {
            path.moveTo(x, y);
            first = false;
        } else {
            path.lineTo(x, y);
        }
    }
    
    painter.drawPath(path);
    
    painter.restore();
}

void WaveformWidget::drawLabels(QPainter &painter)
{
    painter.save();
    
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    
    int height = this->height();
    
    // Y-axis labels
    painter.drawText(5, 15, QString::number(m_yMax, 'f', 1));
    painter.drawText(5, height - 5, QString::number(m_yMin, 'f', 1));
    
    // Channel names
    int y = 15;
    for (int i = 0; i < m_channelNames.size() && i < 4; ++i) {
        painter.setPen(m_channelColors[i]);
        painter.drawText(width() - 100, y, m_channelNames[i]);
        y += 15;
    }
    
    painter.restore();
}
