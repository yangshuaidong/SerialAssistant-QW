#include "configmanager.h"

const QString ConfigManager::SERIAL_GROUP = QStringLiteral("serial");
const QString ConfigManager::WINDOW_GROUP = QStringLiteral("window");
const QString ConfigManager::PLUGIN_PREFIX = QStringLiteral("plugins/");
const QString ConfigManager::APP_GROUP = QStringLiteral("app");

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings(this))
{
}

ConfigManager::~ConfigManager()
{
    m_settings->sync();
}

void ConfigManager::setValue(const QString &key, const QVariant &value)
{
    m_settings->setValue(key, value);
    emit configChanged(key);
}

QVariant ConfigManager::value(const QString &key, const QVariant &defaultValue) const
{
    return m_settings->value(key, defaultValue);
}

void ConfigManager::remove(const QString &key)
{
    m_settings->remove(key);
}

void ConfigManager::clear()
{
    m_settings->clear();
}

void ConfigManager::saveSerialConfig(const QString &portName, int baudRate, 
                                      int dataBits, int parity, int stopBits, int flowControl)
{
    m_settings->beginGroup(SERIAL_GROUP);
    m_settings->setValue("port", portName);
    m_settings->setValue("baudRate", baudRate);
    m_settings->setValue("dataBits", dataBits);
    m_settings->setValue("parity", parity);
    m_settings->setValue("stopBits", stopBits);
    m_settings->setValue("flowControl", flowControl);
    m_settings->endGroup();
}

QMap<QString, QVariant> ConfigManager::loadSerialConfig() const
{
    QMap<QString, QVariant> config;
    m_settings->beginGroup(SERIAL_GROUP);
    
    config["port"] = m_settings->value("port", "");
    config["baudRate"] = m_settings->value("baudRate", 115200);
    config["dataBits"] = m_settings->value("dataBits", 8);
    config["parity"] = m_settings->value("parity", 0);
    config["stopBits"] = m_settings->value("stopBits", 1);
    config["flowControl"] = m_settings->value("flowControl", 0);
    
    m_settings->endGroup();
    return config;
}

void ConfigManager::saveWindowGeometry(const QByteArray &geometry)
{
    m_settings->beginGroup(WINDOW_GROUP);
    m_settings->setValue("geometry", geometry);
    m_settings->endGroup();
}

void ConfigManager::saveWindowState(const QByteArray &state)
{
    m_settings->beginGroup(WINDOW_GROUP);
    m_settings->setValue("state", state);
    m_settings->endGroup();
}

QByteArray ConfigManager::windowGeometry() const
{
    m_settings->beginGroup(WINDOW_GROUP);
    QByteArray geom = m_settings->value("geometry").toByteArray();
    m_settings->endGroup();
    return geom;
}

QByteArray ConfigManager::windowState() const
{
    m_settings->beginGroup(WINDOW_GROUP);
    QByteArray state = m_settings->value("state").toByteArray();
    m_settings->endGroup();
    return state;
}

void ConfigManager::savePluginConfig(const QString &pluginId, const QMap<QString, QVariant> &config)
{
    m_settings->beginGroup(PLUGIN_PREFIX + pluginId);
    for (auto it = config.begin(); it != config.end(); ++it) {
        m_settings->setValue(it.key(), it.value());
    }
    m_settings->endGroup();
}

QMap<QString, QVariant> ConfigManager::loadPluginConfig(const QString &pluginId) const
{
    QMap<QString, QVariant> config;
    m_settings->beginGroup(PLUGIN_PREFIX + pluginId);
    
    for (const QString &key : m_settings->childKeys()) {
        config[key] = m_settings->value(key);
    }
    
    m_settings->endGroup();
    return config;
}

void ConfigManager::saveTheme(const QString &theme)
{
    m_settings->beginGroup(APP_GROUP);
    m_settings->setValue("theme", theme);
    m_settings->endGroup();
}

QString ConfigManager::theme() const
{
    m_settings->beginGroup(APP_GROUP);
    QString t = m_settings->value("theme", "dark").toString();
    m_settings->endGroup();
    return t;
}

void ConfigManager::saveLanguage(const QString &language)
{
    m_settings->beginGroup(APP_GROUP);
    m_settings->setValue("language", language);
    m_settings->endGroup();
}

QString ConfigManager::language() const
{
    m_settings->beginGroup(APP_GROUP);
    QString lang = m_settings->value("language", "en_US").toString();
    m_settings->endGroup();
    return lang;
}
