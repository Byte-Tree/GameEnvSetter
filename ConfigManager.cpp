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
        
        saveCommonSettings(settings, config);
        saveGraphicsSettings(settings, config);
        //settings.sync();
        handleSaveResult(settings);
    handleSaveResult(settings);
    }    catch (const std::exception &e) {
        emit errorOccurred(QString("保存失败: ") + e.what());
        //throw;
    }

}

void ConfigManager::saveConfig(const QVariantMap &config)
{
    try{
        QSettings settings(m_configPath, QSettings::IniFormat);
        
        saveCommonSettings(settings, config);
        saveGraphicsSettings(settings, config);
        //settings.sync();
        handleSaveResult(settings);
    handleSaveResult(settings);
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
    
    QVariantMap graphicsConfig;
    settings.beginGroup("Graphics");
    QString gpuVendor = settings.value("GPUVendor").toString();
    graphicsConfig["gpuVendor"] = gpuVendor;
    
    if(gpuVendor == "NVIDIA") {
        qDebug() << "开始保存NVIDIA显卡配置";
        settings.beginGroup("NVIDIA");
// 图形设置键名已统一为小驼峰命名
        graphicsConfig["imageSharpening"] = settings.value("ImageSharpening");
        graphicsConfig["openGLGDICompatibility"] = settings.value("OpenGLGDICompatibility");
        graphicsConfig["openGLPresentMethod"] = settings.value("OpenGLPresentMethod");
        graphicsConfig["tripleBuffer"] = settings.value("TripleBuffer");
        graphicsConfig["lowLatencyMode"] = settings.value("LowLatencyMode");
        graphicsConfig["anisotropicFiltering"] = settings.value("AnisotropicFiltering");
        graphicsConfig["appIdleFPSLimit"] = settings.value("AppIdleFPSLimit");
        graphicsConfig["vSyncMode"] = settings.value("VSyncMode");
        graphicsConfig["fxaaEnable"] = settings.value("FXAAEnable");
        graphicsConfig["aaModeSelector"] = settings.value("AAModeSelector");
        graphicsConfig["aaGammaCorrection"] = settings.value("AAGammaCorrection");
        graphicsConfig["aaModeMethod"] = settings.value("AAModeMethod");
        graphicsConfig["aaTransparency"] = settings.value("AATransparency");
        graphicsConfig["maxFPSLimit"] = settings.value("MaxFPSLimit");
        graphicsConfig["aoMode"] = settings.value("AOMode");
        graphicsConfig["powerManagementMode"] = settings.value("PowerManagementMode");
        graphicsConfig["shaderCacheSize"] = settings.value("ShaderCacheSize");
        graphicsConfig["trilinearOptimization"] = settings.value("TrilinearOptimization");
        graphicsConfig["anisotropicSampleOptimization"] = settings.value("AnisotropicSampleOptimization");
        graphicsConfig["negativeLODBias"] = settings.value("NegativeLODBias");
        graphicsConfig["textureFilterQuality"] = settings.value("TextureFilterQuality");
        graphicsConfig["threadControl"] = settings.value("ThreadControl");
        settings.endGroup();
    } else if(gpuVendor == "AMD") {
        settings.beginGroup("AMD");
        // 可扩展AMD显卡配置
        settings.endGroup();
    } else if(gpuVendor == "Intel") {
        settings.beginGroup("Intel");
        // 可扩展Intel显卡配置
        settings.endGroup();
    }
    settings.endGroup();
    
    config["mouse"] = mouseConfig;
    config["keyboard"] = keyboardConfig;
    config["registry"] = registryConfig;
    config["graphics"] = graphicsConfig;
    
    return config;
}

void ConfigManager::saveCommonSettings(QSettings &settings, const QVariantMap &config)
{
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

void ConfigManager::saveGraphicsSettings(QSettings &settings, const QVariantMap &config)
{
    settings.beginGroup("Graphics");
    QString gpuVendor = config["graphics"].toMap()["gpuVendor"].toString();
    settings.setValue("GPUVendor", gpuVendor);

    if(gpuVendor == "NVIDIA") {
        qDebug() << "开始保存NVIDIA显卡配置";
        settings.beginGroup("NVIDIA");
// 图形设置键名已统一为小驼峰命名
        settings.setValue("ImageSharpening", config["graphics"].toMap()["imageSharpening"]);
        settings.setValue("OpenGLGDICompatibility", config["graphics"].toMap()["openGLGDICompatibility"]);
        settings.setValue("OpenGLPresentMethod", config["graphics"].toMap()["openGLPresentMethod"]);
        settings.setValue("TripleBuffer", config["graphics"].toMap()["tripleBuffer"]);
        settings.setValue("LowLatencyMode", config["graphics"].toMap()["lowLatencyMode"]);
        settings.setValue("AnisotropicFiltering", config["graphics"].toMap()["anisotropicFiltering"]);
        settings.setValue("AppIdleFPSLimit", config["graphics"].toMap()["appIdleFPSLimit"]);
        settings.setValue("VSyncMode", config["graphics"].toMap()["vSyncMode"]);
        settings.setValue("FXAAEnable", config["graphics"].toMap()["fxaaEnable"]);
        settings.setValue("AAModeSelector", config["graphics"].toMap()["aaModeSelector"]);
        settings.setValue("AAGammaCorrection", config["graphics"].toMap()["aaGammaCorrection"]);
        settings.setValue("AAModeMethod", config["graphics"].toMap()["aaModeMethod"]);
        settings.setValue("AATransparency", config["graphics"].toMap()["aaTransparency"]);
        settings.setValue("MaxFPSLimit", config["graphics"].toMap()["maxFPSLimit"]);
        settings.setValue("AOMode", config["graphics"].toMap()["aoMode"]);
        settings.setValue("PowerManagementMode", config["graphics"].toMap()["powerManagementMode"]);
        settings.setValue("ShaderCacheSize", config["graphics"].toMap()["shaderCacheSize"]);
        settings.setValue("TrilinearOptimization", config["graphics"].toMap()["trilinearOptimization"]);
        settings.setValue("AnisotropicSampleOptimization", config["graphics"].toMap()["anisotropicSampleOptimization"]);
        settings.setValue("NegativeLODBias", config["graphics"].toMap()["negativeLODBias"]);
        settings.setValue("TextureFilterQuality", config["graphics"].toMap()["textureFilterQuality"]);
        settings.setValue("ThreadControl", config["graphics"].toMap()["threadControl"]);
        settings.endGroup();
        qDebug() << "NVIDIA显卡配置保存完成";
    } else if(gpuVendor == "AMD") {
        settings.beginGroup("AMD");
        settings.endGroup();
    } else if(gpuVendor == "Intel") {
        settings.beginGroup("Intel");
        settings.endGroup();
    }
    settings.endGroup();
}

void ConfigManager::handleSaveResult(QSettings &settings)
{
    settings.sync();
    if(settings.status() != QSettings::NoError) {
        emit errorOccurred("配置文件写入失败");
    } else {
        emit configSaved();
    }
}
