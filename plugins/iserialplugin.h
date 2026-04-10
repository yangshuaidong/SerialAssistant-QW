#ifndef ISERIALPLUGIN_H
#define ISERIALPLUGIN_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QByteArray>

// Plugin interface for serial port extensions
class ISerialPlugin {
public:
    virtual ~ISerialPlugin() = default;
    
    // Plugin metadata
    virtual QString pluginId() const = 0;
    virtual QString pluginName() const = 0;
    virtual QString pluginVersion() const = 0;
    virtual QString pluginDescription() const = 0;
    
    // Lifecycle
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    
    // Data processing hooks
    virtual QByteArray onDataReceived(const QByteArray &data) = 0;
    virtual QByteArray onDataToSend(const QByteArray &data) = 0;
    
    // Configuration
    virtual QVariantMap getConfig() const = 0;
    virtual void setConfig(const QVariantMap &config) = 0;
    
    // UI integration (optional)
    virtual QWidget* createSettingsWidget(QWidget *parent = nullptr) = 0;
};

// Plugin factory function type
typedef ISerialPlugin* (*CreatePluginFunc)();
typedef void (*DestroyPluginFunc)(ISerialPlugin*);

// Plugin loader
class PluginLoader : public QObject {
    Q_OBJECT
    
public:
    static PluginLoader* instance();
    
    bool loadPlugin(const QString &path);
    bool unloadPlugin(const QString &pluginId);
    
    ISerialPlugin* getPlugin(const QString &pluginId) const;
    QList<QString> loadedPlugins() const;
    
    void initializeAll();
    void shutdownAll();

signals:
    void pluginLoaded(const QString &pluginId);
    void pluginUnloaded(const QString &pluginId);

private:
    PluginLoader(QObject *parent = nullptr);
    static PluginLoader *m_instance;
    
    struct PluginInfo {
        QString path;
        void *handle;
        ISerialPlugin *instance;
        CreatePluginFunc createFunc;
        DestroyPluginFunc destroyFunc;
    };
    
    QMap<QString, PluginInfo> m_plugins;
};

#endif // ISERIALPLUGIN_H
