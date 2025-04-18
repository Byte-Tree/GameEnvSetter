#include "ConfigManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

ConfigManager::ConfigManager(QObject *parent) : QObject(parent)
{

    
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
    try {
        QString cleanPath = path.startsWith("file:///") ? path.mid(8) : path;//qml传过来的路径有这个字符串，得去除才能保存
        QSettings settings(cleanPath, QSettings::IniFormat);
        settings.setValue("LastModified", QDateTime::currentDateTime());
    
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
    settings.sync();
    if(settings.status() != QSettings::NoError) {
        emit errorOccurred("配置文件写入失败");
    }
    else {
        emit configSaved();
    }
    }    catch (const std::exception &e) {
        emit errorOccurred(QString("保存失败: ") + e.what());
        //throw;
    }

}

void ConfigManager::saveConfig(const QVariantMap &config)
{
    try{
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
    settings.sync();
    if(settings.status() != QSettings::NoError) {
        emit errorOccurred("配置文件写入失败");
    }
    else {
        emit configSaved();
    }
    }
    catch (const std::exception &e) {
        emit errorOccurred(QString("保存失败: ") + e.what());
        //throw;
    }
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

    QString cleanPath = filePath.startsWith("file:///") ? filePath.mid(8) : filePath;//qml传过来的路径有这个字符串，得去除才能保存
    if(!QFile::exists(cleanPath)) {
        emit errorOccurred("配置文件不存在");
        return config;
    }

    QSettings settings(cleanPath, QSettings::IniFormat);
    
    settings.beginGroup("Mouse");
    mouseConfig["doubleClickSpeed"] = settings.value("DoubleClickSpeed");
    mouseConfig["mouseSpeed"] = settings.value("MouseSpeed");
    mouseConfig["enhancePointerPrecision"] = settings.value("EnhancePointerPrecision").toBool();
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
