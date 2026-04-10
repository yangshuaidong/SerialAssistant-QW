#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPortInfo>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class SerialPortManager;
class DataPipeline;
class ConfigManager;
class ReceiveWidget;
class SendWidget;
class StatusBar;
class WaveformWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onPortRefreshRequested();
    void onDataReceived(const QByteArray &data);
    void onSendData(const QByteArray &data);
    void updatePortList(const QList<QSerialPortInfo> &ports);
    void updateStatusBar(const QString &message);

private:
    void setupUI();
    void setupConnections();
    void loadConfiguration();
    void saveConfiguration();

    Ui::MainWindow *ui;
    SerialPortManager *m_serialManager;
    DataPipeline *m_dataPipeline;
    ConfigManager *m_configManager;
    ReceiveWidget *m_receiveWidget;
    SendWidget *m_sendWidget;
    StatusBar *m_statusBar;
    
    // UI controls
    QComboBox *m_portComboBox;
    QComboBox *m_baudRateCombo;
    QComboBox *m_dataBitsCombo;
    QComboBox *m_parityCombo;
    QComboBox *m_stopBitsCombo;
    QComboBox *m_flowControlCombo;
    QPushButton *m_connectButton;
    QPushButton *m_disconnectButton;
    QPushButton *m_refreshButton;

    bool m_connected;
};

#endif // MAINWINDOW_H
