#include "pythonengine.h"
#include <QDebug>
#include <QFile>
#include <QSet>

PythonEngine* PythonEngine::m_instance = nullptr;

PythonEngine::PythonEngine(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
    , m_sandboxEnabled(true)
    , m_workerThread(nullptr)
    , m_impl(nullptr)
{
    // Default allowed modules for sandbox
    m_allowedModules << "math" << "json" << "time" << "datetime" << "re";
}

PythonEngine::~PythonEngine()
{
    shutdown();
}

PythonEngine* PythonEngine::instance()
{
    if (!m_instance) {
        m_instance = new PythonEngine();
    }
    return m_instance;
}

bool PythonEngine::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    qDebug() << "Initializing Python engine...";
    
    // Note: Actual Python initialization would use pybind11 or Python C API
    // This is a stub implementation showing the structure
    
    // Create worker thread for script execution
    m_workerThread = new QThread(this);
    m_workerThread->start();
    
    m_initialized = true;
    
    qDebug() << "Python engine initialized successfully";
    emit initializationComplete(true);
    
    return true;
}

void PythonEngine::shutdown()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        return;
    }
    
    qDebug() << "Shutting down Python engine...";
    
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
        delete m_workerThread;
        m_workerThread = nullptr;
    }
    
    m_initialized = false;
    
    qDebug() << "Python engine shutdown complete";
}

bool PythonEngine::isInitialized() const
{
    return m_initialized;
}

QVariant PythonEngine::executeScript(const QString &code, const QString &moduleName)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        emit scriptError("Python engine not initialized");
        return QVariant();
    }
    
    qDebug() << "Executing Python script in module:" << moduleName;
    qDebug() << "Code:" << code.left(100) << "...";
    
    // Note: Actual implementation would use pybind11 to execute Python code
    // This is a stub that returns a sample result
    
    // Simulate script execution
    emit scriptOutput(QString("Executed: %1").arg(code.left(50)));
    
    return QVariant("Script executed successfully");
}

bool PythonEngine::executeFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit scriptError(QString("Failed to open file: %1").arg(filePath));
        return false;
    }
    
    QString code = QString::fromUtf8(file.readAll());
    file.close();
    
    QVariant result = executeScript(code, "__main__");
    return result.isValid();
}

void PythonEngine::registerObject(const QString &name, QObject *object)
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(name);
    Q_UNUSED(object);
    
    // Note: Would use pybind11 to expose QObject to Python
    qDebug() << "Registering object:" << name;
}

void PythonEngine::setVariable(const QString &name, const QVariant &value)
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(name);
    Q_UNUSED(value);
    
    // Note: Would set Python variable via pybind11
    qDebug() << "Setting variable:" << name;
}

QVariant PythonEngine::getVariable(const QString &name)
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(name);
    
    // Note: Would get Python variable via pybind11
    return QVariant();
}

void PythonEngine::enableSandbox(bool enabled)
{
    m_sandboxEnabled = enabled;
    qDebug() << "Python sandbox" << (enabled ? "enabled" : "disabled");
}

bool PythonEngine::isSandboxEnabled() const
{
    return m_sandboxEnabled;
}

void PythonEngine::addAllowedModule(const QString &moduleName)
{
    m_allowedModules.insert(moduleName);
}

void PythonEngine::removeAllowedModule(const QString &moduleName)
{
    m_allowedModules.remove(moduleName);
}

bool PythonEngine::isModuleAllowed(const QString &moduleName) const
{
    return m_allowedModules.contains(moduleName);
}

void PythonEngine::executeInThread()
{
    // Worker slot for thread-safe execution
}
