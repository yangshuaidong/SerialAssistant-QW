#ifndef PYTHONENGINE_H
#define PYTHONENGINE_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QMutex>
#include <QThread>

class PythonEngine : public QObject
{
    Q_OBJECT

public:
    static PythonEngine* instance();
    
    bool initialize();
    void shutdown();
    bool isInitialized() const;
    
    // Execute Python code
    QVariant executeScript(const QString &code, const QString &moduleName = "__main__");
    bool executeFile(const QString &filePath);
    
    // Register C++ objects for Python access
    void registerObject(const QString &name, QObject *object);
    
    // Set/get Python variables
    void setVariable(const QString &name, const QVariant &value);
    QVariant getVariable(const QString &name);
    
    // Sandbox control
    void enableSandbox(bool enabled);
    bool isSandboxEnabled() const;
    
    // Module whitelist (when sandbox is enabled)
    void addAllowedModule(const QString &moduleName);
    void removeAllowedModule(const QString &moduleName);
    bool isModuleAllowed(const QString &moduleName) const;

signals:
    void initializationComplete(bool success);
    void scriptError(const QString &error);
    void scriptOutput(const QString &output);

private slots:
    void executeInThread();

private:
    PythonEngine(QObject *parent = nullptr);
    ~PythonEngine();
    
    static PythonEngine *m_instance;
    
    bool m_initialized;
    bool m_sandboxEnabled;
    QSet<QString> m_allowedModules;
    
    QMutex m_mutex;
    QThread *m_workerThread;
    
    // Internal implementation pointer
    void *m_impl;
};

#endif // PYTHONENGINE_H
