#pragma once

#include <QObject>
#include <QtCore/QVariant>
#include "nvapi.h"

class NVIDIAGraphicsConfigManager : public QObject {
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
    Q_PROPERTY(int aoMode READ getAoMode WRITE setAoMode NOTIFY aoModeChanged)
    Q_PROPERTY(int aaGammaCorrection READ getAAGammaCorrection WRITE setAAGammaCorrection NOTIFY aaGammaCorrectionChanged)
    Q_PROPERTY(int aaTransparency READ getAATransparency WRITE setAATransparency NOTIFY aaTransparencyChanged)
    Q_PROPERTY(int lowLatencyMode READ getLowLatencyMode WRITE setLowLatencyMode NOTIFY lowLatencyModeChanged)
    Q_PROPERTY(int maxFPSLimit READ getMaxFPSLimit WRITE setMaxFPSLimit NOTIFY maxFPSLimitChanged)
    Q_PROPERTY(int shaderCacheSize READ getShaderCacheSize WRITE setShaderCacheSize NOTIFY shaderCacheSizeChanged)
    Q_PROPERTY(int trilinearOptimization READ getTrilinearOptimization WRITE setTrilinearOptimization NOTIFY trilinearOptimizationChanged)
    Q_PROPERTY(int anisotropicSampleOptimization READ getAnisotropicSampleOptimization WRITE setAnisotropicSampleOptimization NOTIFY anisotropicSampleOptimizationChanged)
    Q_PROPERTY(int negativeLODBias READ getNegativeLODBias WRITE setNegativeLODBias NOTIFY negativeLODBiasChanged)
    Q_PROPERTY(int textureFilterQuality READ getTextureFilterQuality WRITE setTextureFilterQuality NOTIFY textureFilterQualityChanged)
    Q_PROPERTY(int threadControl READ getThreadControl WRITE setThreadControl NOTIFY threadControlChanged)
    Q_PROPERTY(QByteArray scalingMode READ getScalingMode WRITE setScalingMode NOTIFY scalingModeChanged)
    //Q_PROPERTY(int scalingSource READ getScalingSource WRITE setScalingSource NOTIFY scalingSourceChanged)
    //Q_PROPERTY(bool scalingOverride READ getScalingOverride WRITE setScalingOverride NOTIFY scalingOverrideChanged)
    
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
    //获取所有显示器设置
    NvAPI_Status CheckDisplayConfig();
    //申请内存并获取显示器设置
    NvAPI_Status AllocateAndGetDisplayConfig(NvU32* pathCount, NV_DISPLAYCONFIG_PATH_INFO_V2** pathInfo);
    void FreeDisplayConfigResources(NV_DISPLAYCONFIG_PATH_INFO_V2* pathInfo, NvU32 pathCount);
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

    // 最大 FPS 限制设置
    Q_INVOKABLE int getMaxFPSLimit();
    Q_INVOKABLE void setMaxFPSLimit(int limit);

    //环境光吸收
    Q_INVOKABLE int getAoMode();
    Q_INVOKABLE void setAoMode(int mode);

    // 着色器缓存大小设置
    Q_INVOKABLE int getShaderCacheSize();
    Q_INVOKABLE void setShaderCacheSize(int size);

    // 三线性优化设置
    Q_INVOKABLE int getTrilinearOptimization();
    Q_INVOKABLE void setTrilinearOptimization(int mode);
    
    // 各向异性采样优化设置
    Q_INVOKABLE int getAnisotropicSampleOptimization();
    Q_INVOKABLE void setAnisotropicSampleOptimization(int mode);

    //负LOD偏置
    Q_INVOKABLE int getNegativeLODBias();
    Q_INVOKABLE void setNegativeLODBias(int bias);

    //纹理过滤质量
    Q_INVOKABLE int getTextureFilterQuality();
    Q_INVOKABLE void setTextureFilterQuality(int quality);
    
    // 线程控制设置
    Q_INVOKABLE int getThreadControl();
    Q_INVOKABLE void setThreadControl(int mode);

    // 缩放模式设置
    Q_INVOKABLE QByteArray getScalingMode();
    Q_INVOKABLE void setScalingMode(const QByteArray& params);

    // // 缩放源设置
    // Q_INVOKABLE int getScalingSource();
    // Q_INVOKABLE void setScalingSource(int source);

    // // 缩放覆盖设置
    // Q_INVOKABLE bool getScalingOverride();
    // Q_INVOKABLE void setScalingOverride(bool enabled);

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
    void maxFPSLimitChanged();
    void aoModeChanged();
    void shaderCacheSizeChanged();
    void trilinearOptimizationChanged();
    void anisotropicSampleOptimizationChanged();
    void negativeLODBiasChanged();
    void textureFilterQualityChanged();
    void threadControlChanged();
    void scalingModeChanged();
    void scalingSourceChanged();
    void scalingOverrideChanged();
};
