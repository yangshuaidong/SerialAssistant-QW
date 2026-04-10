#include "iserialplugin.h"
#include <QLibrary>
#include <QDir>
#include <QDebug>

PluginLoader* PluginLoader::m_instance = nullptr;

PluginLoader::PluginLoader(QObject *parent)
    : QObject(parent)
{
}

PluginLoader* PluginLoader::instance()
{
    if (!m_instance) {
        m_instance = new PluginLoader();
    }
    return m_instance;
}

bool PluginLoader::loadPlugin(const QString &path)
{
    QLibrary *library = new QLibrary(path, this);
    
    if (!library->load()) {
        qWarning() << "Failed to load plugin:" << library->errorString();
        delete library;
        return false;
    }
    
    // Resolve factory functions
    CreatePluginFunc createFunc = (CreatePluginFunc)library->resolve("createPlugin");
    DestroyPluginFunc destroyFunc = (DestroyPluginFunc)library->resolve("destroyPlugin");
    
    if (!createFunc || !destroyFunc) {
        qWarning() << "Plugin missing required factory functions:" << path;
        library->unload();
        delete library;
        return false;
    }
    
    // Create plugin instance
    ISerialPlugin *plugin = createFunc();
    if (!plugin) {
        qWarning() << "Failed to create plugin instance:" << path;
        library->unload();
        delete library;
        return false;
    }
    
    // Initialize plugin
    if (!plugin->initialize()) {
        qWarning() << "Plugin initialization failed:" << path;
        destroyFunc(plugin);
        library->unload();
        delete library;
        return false;
    }
    
    // Store plugin info
    PluginInfo info;
    info.path = path;
    info.handle = library;
    info.instance = plugin;
    info.createFunc = createFunc;
    info.destroyFunc = destroyFunc;
    
    m_plugins[plugin->pluginId()] = info;
    
    emit pluginLoaded(plugin->pluginId());
    
    qDebug() << "Plugin loaded successfully:" << plugin->pluginName();
    
    return true;
}

bool PluginLoader::unloadPlugin(const QString &pluginId)
{
    if (!m_plugins.contains(pluginId)) {
        return false;
    }
    
    PluginInfo &info = m_plugins[pluginId];
    
    // Shutdown plugin
    info.instance->shutdown();
    
    // Destroy instance
    info.destroyFunc(info.instance);
    
    // Unload library
    QLibrary *library = static_cast<QLibrary*>(info.handle);
    library->unload();
    delete library;
    
    m_plugins.remove(pluginId);
    
    emit pluginUnloaded(pluginId);
    
    qDebug() << "Plugin unloaded:" << pluginId;
    
    return true;
}

ISerialPlugin* PluginLoader::getPlugin(const QString &pluginId) const
{
    if (m_plugins.contains(pluginId)) {
        return m_plugins[pluginId].instance;
    }
    return nullptr;
}

QList<QString> PluginLoader::loadedPlugins() const
{
    return m_plugins.keys();
}

void PluginLoader::initializeAll()
{
    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        it.value().instance->initialize();
    }
}

void PluginLoader::shutdownAll()
{
    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        it.value().instance->shutdown();
    }
}

// Example plugin implementation
class ExampleSerialPlugin : public ISerialPlugin {
public:
    QString pluginId() const override { return "example.plugin"; }
    QString pluginName() const override { return "Example Plugin"; }
    QString pluginVersion() const override { return "1.0.0"; }
    QString pluginDescription() const override { 
        return "An example serial plugin demonstrating the interface"; 
    }
    
    bool initialize() override {
        qDebug() << "Example plugin initialized";
        return true;
    }
    
    void shutdown() override {
        qDebug() << "Example plugin shutdown";
    }
    
    QByteArray onDataReceived(const QByteArray &data) override {
        // Pass-through by default
        return data;
    }
    
    QByteArray onDataToSend(const QByteArray &data) override {
        // Pass-through by default
        return data;
    }
    
    QVariantMap getConfig() const override {
        return QVariantMap();
    }
    
    void setConfig(const QVariantMap &config) override {
        Q_UNUSED(config);
    }
    
    QWidget* createSettingsWidget(QWidget *parent) override {
        Q_UNUSED(parent);
        return nullptr;
    }
};

// Factory functions for example plugin
extern "C" {
    ISerialPlugin* createPlugin() {
        return new ExampleSerialPlugin();
    }
    
    void destroyPlugin(ISerialPlugin *plugin) {
        delete plugin;
    }
}
