#pragma once

#include <QObject>

// NVIDIA 设置ID常量
extern const unsigned int NV_IMAGE_SHARPENING_SETTING_ID;
extern const unsigned int NV_CUDA_EXCLUDED_GPUS_SETTING_ID;
extern const unsigned int NV_MEMORY_FALLBACK_POLICY_SETTING_ID;

class GraphicsConfigManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool imageSharpening READ getImageSharpeningStatus WRITE setImageSharpening NOTIFY imageSharpeningChanged)
    Q_PROPERTY(MemoryFallbackPolicy memoryFallbackPolicy READ getMemoryFallbackPolicy WRITE setMemoryFallbackPolicy NOTIFY memoryFallbackPolicyChanged)

public:
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
    Q_INVOKABLE void setMemoryFallbackPolicy(MemoryFallbackPolicy policy);


signals:
    void imageSharpeningChanged();
    void memoryFallbackPolicyChanged();
};
