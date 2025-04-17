#include "ConfigManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

ConfigManager::ConfigManager(QObject *parent) : QObject(parent)
{
    //QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString configDir = QCoreApplication::applicationDirPath();
    QDir().mkpath(configDir);
    m_configPath = configDir + "/config.ini";
}

bool ConfigManager::configExists() const
{
    return QFile::exists(m_configPath);
}

void ConfigManager::saveUserConfig(const QVariantMap &config, const QString &path)
{
    QSettings settings(path, QSettings::IniFormat);
    
    settings.beginGroup("Mouse");
    settings.setValue("DoubleClickSpeed", config["mouse"].toMap()["doubleClickSpeed"]);
    settings.setValue("MouseSpeed", config["mouse"].toMap()["mouseSpeed"]);
    settings.setValue("EnhancePointerPrecision", config["mouse"].toMap()["enhancePointerPrecision"]);
    settings.endGroup();
    
    settings.beginGroup("Keyboard");
    settings.setValue("RepeatDelay", config["keyboard"].toMap()["repeatDelay"]);
    settings.setValue("RepeatRate", config["keyboard"].toMap()["repeatRate"]);
    settings.endGroup();
    
    settings.beginGroup("Registry");
    settings.setValue("Win32PrioritySeparation", config["registry"].toMap()["win32PrioritySeparation"]);
    settings.endGroup();
}

void ConfigManager::saveConfig(const QVariantMap &config)
{
    QSettings settings(m_configPath, QSettings::IniFormat);
    
    settings.beginGroup("Mouse");
    settings.setValue("DoubleClickSpeed", config["mouse"].toMap()["doubleClickSpeed"]);
    settings.setValue("MouseSpeed", config["mouse"].toMap()["mouseSpeed"]);
    settings.setValue("EnhancePointerPrecision", config["mouse"].toMap()["enhancePointerPrecision"]);
    settings.endGroup();
    
    settings.beginGroup("Keyboard");
    settings.setValue("RepeatDelay", config["keyboard"].toMap()["repeatDelay"]);
    settings.setValue("RepeatRate", config["keyboard"].toMap()["repeatRate"]);
    settings.endGroup();
    
    settings.beginGroup("Registry");
    settings.setValue("Win32PrioritySeparation", config["registry"].toMap()["win32PrioritySeparation"]);
    settings.endGroup();
}

QVariantMap ConfigManager::loadConfig() const
{
    return loadFromPath(m_configPath);
}

QVariantMap ConfigManager::loadFromPath(const QString &filePath) const {
    QVariantMap config;
    QVariantMap mouseConfig;
    QVariantMap keyboardConfig;
    QVariantMap registryConfig;

    if(!QFile::exists(filePath)) {
        emit errorOccurred("配置文件不存在");
        return config;
    }

    QSettings settings(filePath, QSettings::IniFormat);
    
    settings.beginGroup("Mouse");
    mouseConfig["doubleClickSpeed"] = settings.value("DoubleClickSpeed");
    mouseConfig["mouseSpeed"] = settings.value("MouseSpeed");
    mouseConfig["enhancePointerPrecision"] = settings.value("EnhancePointerPrecision");
    settings.endGroup();
    
    settings.beginGroup("Keyboard");
    keyboardConfig["repeatDelay"] = settings.value("RepeatDelay");
    keyboardConfig["repeatRate"] = settings.value("RepeatRate");
    settings.endGroup();
    
    settings.beginGroup("Registry");
    registryConfig["win32PrioritySeparation"] = settings.value("Win32PrioritySeparation");
    settings.endGroup();
    
    config["mouse"] = mouseConfig;
    config["keyboard"] = keyboardConfig;
    config["registry"] = registryConfig;
    
    return config;
}
