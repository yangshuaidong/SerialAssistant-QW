#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QMap>

class ConfigManager : public QObject
{
    Q_OBJECT

public:
    explicit ConfigManager(QObject *parent = nullptr);
    ~ConfigManager();

    // Generic get/set methods
    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;
    
    void remove(const QString &key);
    void clear();
    
    // Serial port configuration
    void saveSerialConfig(const QString &portName, int baudRate, int dataBits, 
                          int parity, int stopBits, int flowControl);
    QMap<QString, QVariant> loadSerialConfig() const;
    
    // Window layout
    void saveWindowGeometry(const QByteArray &geometry);
    void saveWindowState(const QByteArray &state);
    QByteArray windowGeometry() const;
    QByteArray windowState() const;
    
    // Plugin configurations (isolated namespace)
    void savePluginConfig(const QString &pluginId, const QMap<QString, QVariant> &config);
    QMap<QString, QVariant> loadPluginConfig(const QString &pluginId) const;
    
    // Application settings
    void saveTheme(const QString &theme);
    QString theme() const;
    
    void saveLanguage(const QString &language);
    QString language() const;

signals:
    void configChanged(const QString &key);

private:
    QSettings *m_settings;
    
    static const QString SERIAL_GROUP;
    static const QString WINDOW_GROUP;
    static const QString PLUGIN_PREFIX;
    static const QString APP_GROUP;
};

#endif // CONFIGMANAGER_H
