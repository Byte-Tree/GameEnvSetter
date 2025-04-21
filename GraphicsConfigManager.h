#pragma once

#include <QObject>
#include "nvapi.h"

class GraphicsConfigManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool imageSharpening READ getImageSharpeningStatus WRITE setImageSharpening NOTIFY imageSharpeningChanged)
    Q_PROPERTY(int vSyncMode READ getVSyncMode WRITE setVSyncMode NOTIFY vSyncModeChanged)
    Q_PROPERTY(MemoryFallbackPolicy memoryFallbackPolicy READ getMemoryFallbackPolicy WRITE setMemoryFallbackPolicy NOTIFY memoryFallbackPolicyChanged)
private:
    static bool isNvAPIInitialized;
    static NvDRSSessionHandle globalSession;
private:
    NVDRS_SETTING queryNvidiaSetting(const wchar_t* settingName);

public:
    static bool initializeNvAPI();
    static void shutdownNvAPI();

    // 图形增强功能
    Q_INVOKABLE bool getImageSharpeningStatus();
    Q_INVOKABLE void setImageSharpening(bool enabled);
    
    // CUDA GPU多选功能
    QStringList getAvailableGPUs();
    QStringList getSelectedGPUs();
    void setSelectedGPUs(const QStringList& gpus);
    
    // 内存回退策略
    enum class MemoryFallbackPolicy {
        DriverDefault,
        PreferNoFallback,
        PreferFallback
    };
    
    MemoryFallbackPolicy getMemoryFallbackPolicy();
    Q_INVOKABLE int getVSyncMode();
    Q_INVOKABLE void setVSyncMode(int mode);
    Q_INVOKABLE void setMemoryFallbackPolicy(MemoryFallbackPolicy policy);

    //获取所有可用的显卡设置
    QList<QPair<QString, NvU32>> getAllNvidiaSettings();


signals:
    void imageSharpeningChanged();
    void memoryFallbackPolicyChanged();
    void vSyncModeChanged();
};
