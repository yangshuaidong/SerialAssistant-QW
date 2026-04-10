#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serialportmanager.h"
#include "datapipeline.h"
#include "configmanager.h"
#include "ui/receivewidget.h"
#include "ui/sendwidget.h"
#include "ui/statusbar.h"
#include <QMessageBox>
#include <QSettings>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_serialManager(nullptr)
    , m_dataPipeline(nullptr)
    , m_configManager(nullptr)
    , m_receiveWidget(nullptr)
    , m_sendWidget(nullptr)
    , m_statusBar(nullptr)
    , m_connected(false)
{
    ui->setupUi(this);
    
    setupUI();
    
    m_configManager = new ConfigManager(this);
    m_serialManager = new SerialPortManager(this);
    m_dataPipeline = new DataPipeline(this);
    
    m_receiveWidget = new ReceiveWidget(this);
    m_sendWidget = new SendWidget(this);
    m_statusBar = new StatusBar(this);
    
    setupConnections();
    loadConfiguration();
    
    setWindowTitle(tr("SerialMaster - Professional Serial Debugging Tool"));
}

MainWindow::~MainWindow()
{
    saveConfiguration();
    
    delete m_receiveWidget;
    delete m_sendWidget;
    delete m_statusBar;
    delete m_dataPipeline;
    delete m_serialManager;
    delete m_configManager;
    delete ui;
}

void MainWindow::setupUI()
{
    // Central widget layout
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    
    // Left panel - Serial settings and controls
    QWidget *leftPanel = new QWidget(centralWidget);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    
    // Port selection group
    QGroupBox *portGroup = new QGroupBox(tr("Serial Port Settings"), leftPanel);
    QVBoxLayout *portLayout = new QVBoxLayout(portGroup);
    
    m_portComboBox = new QComboBox(portGroup);
    m_baudRateCombo = new QComboBox(portGroup);
    m_dataBitsCombo = new QComboBox(portGroup);
    m_parityCombo = new QComboBox(portGroup);
    m_stopBitsCombo = new QComboBox(portGroup);
    m_flowControlCombo = new QComboBox(portGroup);
    
    // Populate baud rates
    QStringList baudRates = {"9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"};
    m_baudRateCombo->addItems(baudRates);
    m_baudRateCombo->setCurrentText("115200");
    
    // Data bits
    m_dataBitsCombo->addItems({"5", "6", "7", "8"});
    m_dataBitsCombo->setCurrentText("8");
    
    // Parity
    m_parityCombo->addItems({tr("None"), tr("Even"), tr("Odd"), tr("Mark"), tr("Space")});
    m_parityCombo->setCurrentIndex(0);
    
    // Stop bits
    m_stopBitsCombo->addItems({"1", "1.5", "2"});
    m_stopBitsCombo->setCurrentText("1");
    
    // Flow control
    m_flowControlCombo->addItems({tr("None"), tr("Hardware (RTS/CTS)"), tr("Software (XON/XOFF)")});
    m_flowControlCombo->setCurrentIndex(0);
    
    portLayout->addWidget(new QLabel(tr("Port:"), portGroup));
    portLayout->addWidget(m_portComboBox);
    portLayout->addWidget(new QLabel(tr("Baud Rate:"), portGroup));
    portLayout->addWidget(m_baudRateCombo);
    portLayout->addWidget(new QLabel(tr("Data Bits:"), portGroup));
    portLayout->addWidget(m_dataBitsCombo);
    portLayout->addWidget(new QLabel(tr("Parity:"), portGroup));
    portLayout->addWidget(m_parityCombo);
    portLayout->addWidget(new QLabel(tr("Stop Bits:"), portGroup));
    portLayout->addWidget(m_stopBitsCombo);
    portLayout->addWidget(new QLabel(tr("Flow Control:"), portGroup));
    portLayout->addWidget(m_flowControlCombo);
    
    leftLayout->addWidget(portGroup);
    
    // Connection buttons
    m_connectButton = new QPushButton(tr("Connect"), leftPanel);
    m_disconnectButton = new QPushButton(tr("Disconnect"), leftPanel);
    m_refreshButton = new QPushButton(tr("Refresh Ports"), leftPanel);
    
    m_disconnectButton->setEnabled(false);
    
    leftLayout->addWidget(m_connectButton);
    leftLayout->addWidget(m_disconnectButton);
    leftLayout->addWidget(m_refreshButton);
    leftLayout->addStretch();
    
    mainLayout->addWidget(leftPanel, 1);
    
    // Right panel - Receive and Send areas
    QWidget *rightPanel = new QWidget(centralWidget);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    
    // Splitter for receive and send
    QSplitter *splitter = new QSplitter(Qt::Vertical, rightPanel);
    
    splitter->addWidget(m_receiveWidget);
    splitter->addWidget(m_sendWidget);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    
    rightLayout->addWidget(splitter);
    
    mainLayout->addWidget(rightPanel, 3);
    
    setCentralWidget(centralWidget);
    
    // Status bar
    setStatusBar(m_statusBar);
    
    // Menu bar
    QMenuBar *menuBar = this->menuBar();
    QMenu *fileMenu = menuBar->addMenu(tr("File"));
    QMenu *toolsMenu = menuBar->addMenu(tr("Tools"));
    QMenu *helpMenu = menuBar->addMenu(tr("Help"));
    
    fileMenu->addAction(tr("Save Log"), this, [](){ qDebug() << "Save log"; });
    fileMenu->addAction(tr("Load Configuration"), this, [](){ qDebug() << "Load config"; });
    fileMenu->addAction(tr("Exit"), qApp, &QApplication::quit);
    
    toolsMenu->addAction(tr("Options"), this, [](){ qDebug() << "Options"; });
    toolsMenu->addAction(tr("Plugins"), this, [](){ qDebug() << "Plugins"; });
    
    helpMenu->addAction(tr("About"), this, [this](){
        QMessageBox::about(this, tr("About SerialMaster"),
            tr("<h2>SerialMaster v1.0.0</h2>"
               "<p>Professional Serial Port Debugging Tool</p>"
               "<p>Based on Qt6 with modern features:</p>"
               "<ul>"
               "<li>Real-time serial communication</li>"
               "<li>HEX/Text dual mode</li>"
               "<li>Data pipeline with plugins</li>"
               "<li>Python scripting support</li>"
               "<li>Protocol parsing</li>"
               "</ul>"));
    });
}

void MainWindow::setupConnections()
{
    connect(m_connectButton, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(m_disconnectButton, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &MainWindow::onPortRefreshRequested);
    
    connect(m_serialManager, &SerialPortManager::dataReceived, this, &MainWindow::onDataReceived);
    connect(m_serialManager, &SerialPortManager::portsUpdated, this, &MainWindow::updatePortList);
    
    connect(m_sendWidget, &SendWidget::dataToSend, this, &MainWindow::onSendData);
}

void MainWindow::loadConfiguration()
{
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    
    QString lastPort = settings.value("serial/lastPort").toString();
    QString lastBaud = settings.value("serial/baudRate", "115200").toString();
    
    if (!lastPort.isEmpty()) {
        int index = m_portComboBox->findText(lastPort);
        if (index >= 0) {
            m_portComboBox->setCurrentIndex(index);
        }
    }
    if (!lastBaud.isEmpty()) {
        int index = m_baudRateCombo->findText(lastBaud);
        if (index >= 0) {
            m_baudRateCombo->setCurrentIndex(index);
        }
    }
    
    onPortRefreshRequested();
}

void MainWindow::saveConfiguration()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("serial/lastPort", m_portComboBox->currentText());
    settings.setValue("serial/baudRate", m_baudRateCombo->currentText());
}

void MainWindow::onConnectClicked()
{
    QString portName = m_portComboBox->currentText();
    if (portName.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a serial port"));
        return;
    }
    
    QSerialPort::BaudRate baudRate = static_cast<QSerialPort::BaudRate>(
        m_baudRateCombo->currentText().toInt());
    
    QSerialPort::DataBits dataBits = static_cast<QSerialPort::DataBits>(
        m_dataBitsCombo->currentText().toInt());
    
    QSerialPort::Parity parity = static_cast<QSerialPort::Parity>(
        m_parityCombo->currentIndex());
    
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;
    if (m_stopBitsCombo->currentText() == "1.5")
        stopBits = QSerialPort::OneAndHalfStop;
    else if (m_stopBitsCombo->currentText() == "2")
        stopBits = QSerialPort::TwoStop;
    
    QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;
    if (m_flowControlCombo->currentIndex() == 1)
        flowControl = QSerialPort::HardwareControl;
    else if (m_flowControlCombo->currentIndex() == 2)
        flowControl = QSerialPort::SoftwareControl;
    
    if (m_serialManager->connect(portName, baudRate, dataBits, parity, stopBits, flowControl)) {
        m_connected = true;
        m_connectButton->setEnabled(false);
        m_disconnectButton->setEnabled(true);
        m_portComboBox->setEnabled(false);
        m_baudRateCombo->setEnabled(false);
        m_dataBitsCombo->setEnabled(false);
        m_parityCombo->setEnabled(false);
        m_stopBitsCombo->setEnabled(false);
        m_flowControlCombo->setEnabled(false);
        
        updateStatusBar(tr("Connected to %1").arg(portName));
    } else {
        QMessageBox::critical(this, tr("Error"), 
            tr("Failed to connect to %1: %2").arg(portName).arg(m_serialManager->errorString()));
    }
}

void MainWindow::onDisconnectClicked()
{
    if (m_serialManager->disconnect()) {
        m_connected = false;
        m_connectButton->setEnabled(true);
        m_disconnectButton->setEnabled(false);
        m_portComboBox->setEnabled(true);
        m_baudRateCombo->setEnabled(true);
        m_dataBitsCombo->setEnabled(true);
        m_parityCombo->setEnabled(true);
        m_stopBitsCombo->setEnabled(true);
        m_flowControlCombo->setEnabled(true);
        
        updateStatusBar(tr("Disconnected"));
    }
}

void MainWindow::onPortRefreshRequested()
{
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    updatePortList(ports);
    updateStatusBar(tr("Refreshed port list, found %1 port(s)").arg(ports.size()));
}

void MainWindow::onDataReceived(const QByteArray &data)
{
    m_dataPipeline->processReceived(data);
    m_receiveWidget->appendData(data);
}

void MainWindow::onSendData(const QByteArray &data)
{
    if (m_connected) {
        m_serialManager->write(data);
        m_receiveWidget->appendSentData(data);
    } else {
        updateStatusBar(tr("Cannot send: not connected"));
    }
}

void MainWindow::updatePortList(const QList<QSerialPortInfo> &ports)
{
    QString currentPort = m_portComboBox->currentText();
    m_portComboBox->clear();
    
    for (const QSerialPortInfo &port : ports) {
        QString display = port.portName();
        if (!port.description().isEmpty())
            display += QString(" - %1").arg(port.description());
        m_portComboBox->addItem(display, port.portName());
    }
    
    // Restore previous selection if possible
    if (!currentPort.isEmpty()) {
        int index = m_portComboBox->findData(currentPort);
        if (index >= 0)
            m_portComboBox->setCurrentIndex(index);
    }
}

void MainWindow::updateStatusBar(const QString &message)
{
    m_statusBar->showMessage(message);
}
