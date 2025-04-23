#pragma once

#include <QObject>
#include "nvapi.h"
#include "NvApiDriverSettings.h"

class GraphicsConfigManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int imageSharpening READ getImageSharpeningStatus WRITE setImageSharpening NOTIFY imageSharpeningChanged)
    Q_PROPERTY(int vSyncMode READ getVSyncMode WRITE setVSyncMode NOTIFY vSyncModeChanged)
    Q_PROPERTY(int powerManagementMode READ getPowerManagementMode WRITE setPowerManagementMode NOTIFY powerManagementModeChanged)
    Q_PROPERTY(int openGLGDICompatibility READ getOpenGLGDICompatibility WRITE setOpenGLGDICompatibility NOTIFY openGLGDICompatibilityChanged)
    Q_PROPERTY(int openGLPresentMethod READ getOpenGLPresentMethod WRITE setOpenGLPresentMethod NOTIFY openGLPresentMethodChanged)
    Q_PROPERTY(int tripleBuffer READ getTripleBuffer WRITE setTripleBuffer NOTIFY tripleBufferChanged)
    Q_PROPERTY(int appIdleFPSLimit READ getAppIdleFPSLimit WRITE setAppIdleFPSLimit NOTIFY appIdleFPSLimitChanged)
    Q_PROPERTY(int anisotropicFiltering READ getAnisotropicFiltering WRITE setAnisotropicFiltering NOTIFY anisotropicFilteringChanged)
    Q_PROPERTY(int aaModeMethod READ getAAModeMethod WRITE setAAModeMethod NOTIFY aaModeMethodChanged)
    Q_PROPERTY(int aaModeSelector READ getAAModeSelector WRITE setAAModeSelector NOTIFY aaModeSelectorChanged)
    Q_PROPERTY(int fxaaEnable READ getFXAAEnable WRITE setFXAAEnable NOTIFY fxaaEnableChanged)
    Q_PROPERTY(int aaGammaCorrection READ getAAGammaCorrection WRITE setAAGammaCorrection NOTIFY aaGammaCorrectionChanged)
    Q_PROPERTY(int aaTransparency READ getAATransparency WRITE setAATransparency NOTIFY aaTransparencyChanged)
    Q_PROPERTY(int lowLatencyMode READ getLowLatencyMode WRITE setLowLatencyMode NOTIFY lowLatencyModeChanged)
private:
    static bool isNvAPIInitialized;
    static NvDRSSessionHandle globalSession;
private:
    NVDRS_SETTING queryNvidiaSetting(const wchar_t* settingName);
    bool applyNvidiaSetting(NVDRS_SETTING& drsSetting);
public:
    static void PrintError(NvAPI_Status status);
    //通过设置名称找设置名称对应的ID，或者直接看头文件就不用这个函数了
    static NvU32 getSettingId(const wchar_t* settingName);
    //获取所有可用的显卡设置
    QList<QPair<QString, QPair<NvU32, NvU32>>> getAllNvidiaSettings();
public:
    static bool initializeNvAPI();
    static void shutdownNvAPI();

    // 图形增强功能
    Q_INVOKABLE int getImageSharpeningStatus();
    Q_INVOKABLE void setImageSharpening(int index);

    // 垂直同步功能
    Q_INVOKABLE int getVSyncMode();
    Q_INVOKABLE void setVSyncMode(int mode);

    // OpenGL GDI 兼容性设置
    Q_INVOKABLE int getOpenGLGDICompatibility();
    Q_INVOKABLE void setOpenGLGDICompatibility(int mode);
    
    // OpenGL 显示方法设置
    Q_INVOKABLE int getOpenGLPresentMethod();
    Q_INVOKABLE void setOpenGLPresentMethod(int mode);

    // 三重缓冲设置
    Q_INVOKABLE int getTripleBuffer();
    Q_INVOKABLE void setTripleBuffer(int mode);

    // 电源管理模式设置
    Q_INVOKABLE int getPowerManagementMode();
    Q_INVOKABLE void setPowerManagementMode(int mode);

    //异性过滤设置
    Q_INVOKABLE int getAnisotropicFiltering();
    Q_INVOKABLE void setAnisotropicFiltering(int level);

    // 应用空闲时的 FPS 限制设置
    Q_INVOKABLE int getAppIdleFPSLimit();
    Q_INVOKABLE void setAppIdleFPSLimit(int limit);

    // 抗锯齿模式设置
    Q_INVOKABLE int getAAModeMethod();
    Q_INVOKABLE void setAAModeMethod(int mode);

    // FXAA 启用设置
    Q_INVOKABLE int getFXAAEnable();
    Q_INVOKABLE void setFXAAEnable(int index);

    // 抗锯齿伽马校正设置
    Q_INVOKABLE int getAAGammaCorrection();
    Q_INVOKABLE void setAAGammaCorrection(int level);

    // 抗锯齿透明度设置
    Q_INVOKABLE int getAATransparency();
    Q_INVOKABLE void setAATransparency(int level);
    
    // 抗锯齿模式选择器设置
    Q_INVOKABLE int getAAModeSelector();
    Q_INVOKABLE void setAAModeSelector(int level);

    // 低延迟模式设置
    Q_INVOKABLE int getLowLatencyMode();
    Q_INVOKABLE void setLowLatencyMode(int mode);
signals:
    void errorOccurred(const QString& msg);
    void imageSharpeningChanged();
    void memoryFallbackPolicyChanged();
    void vSyncModeChanged();
    void openGLGDICompatibilityChanged();
    void openGLPresentMethodChanged();
    void tripleBufferChanged();
    void powerManagementModeChanged();
    void appIdleFPSLimitChanged();
    void anisotropicFilteringChanged();
    void aaModeMethodChanged();
    void fxaaEnableChanged();
    void aaGammaCorrectionChanged();
    void aaTransparencyChanged();
    void aaModeSelectorChanged();
    void lowLatencyModeChanged();
};
