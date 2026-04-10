#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H

#include <QWidget>
#include <QVector>
#include <QColor>

class WaveformWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WaveformWidget(QWidget *parent = nullptr);
    ~WaveformWidget();

    void addDataPoint(qreal value);
    void addDataPoint(int channel, qreal value);
    
    void clear();
    void setChannelCount(int count);
    void setChannelColor(int channel, const QColor &color);
    void setChannelName(int channel, const QString &name);
    
    void setMaxPoints(int maxPoints);
    void setAutoScale(bool enabled);
    void setYRange(qreal min, qreal max);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateView();

private:
    void drawGrid(QPainter &painter);
    void drawChannel(QPainter &painter, int channel);
    void drawLabels(QPainter &painter);

    QVector<QVector<qreal>> m_channels;
    QVector<QColor> m_channelColors;
    QVector<QString> m_channelNames;
    
    int m_maxPoints;
    bool m_autoScale;
    qreal m_yMin;
    qreal m_yMax;
    
    int m_scrollOffset;
};

#endif // WAVEFORMWIDGET_H
