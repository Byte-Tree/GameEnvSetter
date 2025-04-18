#include "GraphicsConfigManager.h"
#include "nvapi.h"

// NVAPI 设置ID常量定义
#define NV_IMAGE_SHARPENING_SETTING_ID 0x00000000
#define NV_CUDA_EXCLUDED_GPUS_SETTING_ID 0x00000000
#define NV_MEMORY_FALLBACK_POLICY_SETTING_ID 0x00000000

// NVAPI 字符串长度定义
#define NVAPI_SHORT_STRING_LENGTH 64
#define NVAPI_BINARY_DATA_MAX_SIZE 4096

bool GraphicsConfigManager::getImageSharpeningStatus() {
    NvDRSSessionHandle hSession = 0;
    NvDRSProfileHandle hProfile = 0;
    NVDRS_SETTING setting = {0};
    
    if (NvAPI_Initialize() != NVAPI_OK) {
        return false;
    }
    
    if (NvAPI_DRS_CreateSession(&hSession) != NVAPI_OK) {
        NvAPI_Unload();
        return false;
    }
    
    if (NvAPI_DRS_LoadSettings(hSession) != NVAPI_OK) {
        NvAPI_DRS_DestroySession(hSession);
        NvAPI_Unload();
        return false;
    }
    
    if (NvAPI_DRS_GetBaseProfile(hSession, &hProfile) != NVAPI_OK) {
        NvAPI_DRS_DestroySession(hSession);
        NvAPI_Unload();
        return false;
    }
    
    if (NvAPI_DRS_GetSetting(hSession, hProfile, NV_IMAGE_SHARPENING_SETTING_ID, &setting) != NVAPI_OK) {
        NvAPI_DRS_DestroySession(hSession);
        NvAPI_Unload();
        return false;
    }
    
    NvAPI_DRS_DestroySession(hSession);
    NvAPI_Unload();
    return setting.u32CurrentValue != 0;
}

void GraphicsConfigManager::setImageSharpening(bool enabled) {
    NvDRSSessionHandle hSession = 0;
    NvDRSProfileHandle hProfile = 0;
    NVDRS_SETTING setting = {0};
    
    if (NvAPI_Initialize() != NVAPI_OK) {
        return;
    }
    
    if (NvAPI_DRS_CreateSession(&hSession) != NVAPI_OK) {
        NvAPI_Unload();
        return;
    }
    
    if (NvAPI_DRS_LoadSettings(hSession) != NVAPI_OK) {
        NvAPI_DRS_DestroySession(hSession);
        NvAPI_Unload();
        return;
    }
    
    if (NvAPI_DRS_GetBaseProfile(hSession, &hProfile) != NVAPI_OK) {
        NvAPI_DRS_DestroySession(hSession);
        NvAPI_Unload();
        return;
    }
    
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = NV_IMAGE_SHARPENING_SETTING_ID;
    setting.settingType = NVDRS_DWORD_TYPE;
    setting.u32CurrentValue = enabled ? 1 : 0;
    
    NvAPI_DRS_SetSetting(hSession, hProfile, &setting);
    NvAPI_DRS_SaveSettings(hSession);
    
    NvAPI_DRS_DestroySession(hSession);
    NvAPI_Unload();
    
    emit imageSharpeningChanged();
}

QStringList GraphicsConfigManager::getAvailableGPUs() {
    NvPhysicalGpuHandle gpuHandles[NVAPI_MAX_PHYSICAL_GPUS] = {0};
    NvU32 gpuCount = 0;
    QStringList gpus;
    
    if (NvAPI_Initialize() != NVAPI_OK) {
        return gpus;
    }
    
    if (NvAPI_EnumPhysicalGPUs(gpuHandles, &gpuCount) != NVAPI_OK) {
        NvAPI_Unload();
        return gpus;
    }
    
    for (NvU32 i = 0; i < gpuCount; i++) {
        NV_GPU_DISPLAYIDS displayIds = {0};
        displayIds.version = NV_GPU_DISPLAYIDS_VER;
        
        NvU32 displayIdCount = 0;
        if (NvAPI_GPU_GetConnectedDisplayIds(gpuHandles[i], &displayIds, &displayIdCount, 0) == NVAPI_OK) {
            char gpuNameStr[NVAPI_SHORT_STRING_LENGTH] = {0};
            
            if (NvAPI_GPU_GetFullName(gpuHandles[i], gpuNameStr) == NVAPI_OK) {
                gpus.push_back(QString(gpuNameStr));
            }
        }
    }
    
    NvAPI_Unload();
    return gpus;
}

QStringList GraphicsConfigManager::getSelectedGPUs() {
    NvDRSSessionHandle hSession = 0;
    NvDRSProfileHandle hProfile = 0;
    NVDRS_SETTING setting = {0};
    QStringList selectedGpus;
    
    if (NvAPI_Initialize() != NVAPI_OK) {
        return selectedGpus;
    }
    
    if (NvAPI_DRS_CreateSession(&hSession) != NVAPI_OK) {
        NvAPI_Unload();
        return selectedGpus;
    }
    
    if (NvAPI_DRS_LoadSettings(hSession) != NVAPI_OK) {
        NvAPI_DRS_DestroySession(hSession);
        NvAPI_Unload();
        return selectedGpus;
    }
    
    if (NvAPI_DRS_GetBaseProfile(hSession, &hProfile) != NVAPI_OK) {
        NvAPI_DRS_DestroySession(hSession);
        NvAPI_Unload();
        return selectedGpus;
    }
    
    if (NvAPI_DRS_GetSetting(hSession, hProfile, NV_CUDA_EXCLUDED_GPUS_SETTING_ID, &setting) == NVAPI_OK) {
        QString excludedGpus = QString::fromUtf8((char*)setting.binaryCurrentValue.valueData);
        QStringList gpuList = excludedGpus.split(',');
        selectedGpus.append(gpuList);
    }
    
    NvAPI_DRS_DestroySession(hSession);
    NvAPI_Unload();
    return selectedGpus;
}

void GraphicsConfigManager::setSelectedGPUs(const QStringList& gpus) {
    NvDRSSessionHandle hSession = 0;
    NvDRSProfileHandle hProfile = 0;
    NVDRS_SETTING setting = {0};
    
    if (NvAPI_Initialize() != NVAPI_OK) {
        return;
    }
    
    if (NvAPI_DRS_CreateSession(&hSession) != NVAPI_OK) {
        NvAPI_Unload();
        return;
    }
    
    if (NvAPI_DRS_LoadSettings(hSession) != NVAPI_OK) {
        NvAPI_DRS_DestroySession(hSession);
        NvAPI_Unload();
        return;
    }
    
    if (NvAPI_DRS_GetBaseProfile(hSession, &hProfile) != NVAPI_OK) {
        NvAPI_DRS_DestroySession(hSession);
        NvAPI_Unload();
        return;
    }
    
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = NV_CUDA_EXCLUDED_GPUS_SETTING_ID;
    setting.settingType = NVDRS_BINARY_TYPE;
    
    QString gpuList = gpus.join(",");
    
    setting.binaryCurrentValue.valueLength = gpuList.size();
    strncpy((char*)setting.binaryCurrentValue.valueData, gpuList.toUtf8().constData(), NVAPI_BINARY_DATA_MAX_SIZE);
    
    NvAPI_DRS_SetSetting(hSession, hProfile, &setting);
    NvAPI_DRS_SaveSettings(hSession);
    
    NvAPI_DRS_DestroySession(hSession);
    NvAPI_Unload();
}

GraphicsConfigManager::MemoryFallbackPolicy GraphicsConfigManager::getMemoryFallbackPolicy() {
    NvDRSSessionHandle hSession = 0;
    NvDRSProfileHandle hProfile = 0;
    NVDRS_SETTING setting = {0};
    
    if (NvAPI_Initialize() != NVAPI_OK) {
        return MemoryFallbackPolicy::DriverDefault;
    }
    
    if (NvAPI_DRS_CreateSession(&hSession) != NVAPI_OK) {
        NvAPI_Unload();
        return MemoryFallbackPolicy::DriverDefault;
    }
    
    if (NvAPI_DRS_LoadSettings(hSession) != NVAPI_OK) {
        NvAPI_DRS_DestroySession(hSession);
        NvAPI_Unload();
        return MemoryFallbackPolicy::DriverDefault;
    }
    
    if (NvAPI_DRS_GetBaseProfile(hSession, &hProfile) != NVAPI_OK) {
        NvAPI_DRS_DestroySession(hSession);
        NvAPI_Unload();
        return MemoryFallbackPolicy::DriverDefault;
    }
    
    if (NvAPI_DRS_GetSetting(hSession, hProfile, NV_MEMORY_FALLBACK_POLICY_SETTING_ID, &setting) != NVAPI_OK) {
        NvAPI_DRS_DestroySession(hSession);
        NvAPI_Unload();
        return MemoryFallbackPolicy::DriverDefault;
    }
    
    NvAPI_DRS_DestroySession(hSession);
    NvAPI_Unload();
    
    switch (setting.u32CurrentValue) {
        case 1: return MemoryFallbackPolicy::PreferNoFallback;
        case 2: return MemoryFallbackPolicy::PreferFallback;
        default: return MemoryFallbackPolicy::DriverDefault;
    }
}

void GraphicsConfigManager::setMemoryFallbackPolicy(MemoryFallbackPolicy policy) {
    NvDRSSessionHandle hSession = 0;
    NvDRSProfileHandle hProfile = 0;
    NVDRS_SETTING setting = {0};
    
    if (NvAPI_Initialize() != NVAPI_OK) {
        return;
    }
    
    if (NvAPI_DRS_CreateSession(&hSession) != NVAPI_OK) {
        NvAPI_Unload();
        return;
    }
    
    if (NvAPI_DRS_LoadSettings(hSession) != NVAPI_OK) {
        NvAPI_DRS_DestroySession(hSession);
        NvAPI_Unload();
        return;
    }
    
    if (NvAPI_DRS_GetBaseProfile(hSession, &hProfile) != NVAPI_OK) {
        NvAPI_DRS_DestroySession(hSession);
        NvAPI_Unload();
        return;
    }
    
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = NV_MEMORY_FALLBACK_POLICY_SETTING_ID;
    setting.settingType = NVDRS_DWORD_TYPE;
    
    switch (policy) {
        case MemoryFallbackPolicy::PreferNoFallback: setting.u32CurrentValue = 1; break;
        case MemoryFallbackPolicy::PreferFallback: setting.u32CurrentValue = 2; break;
        default: setting.u32CurrentValue = 0;
    }
    
    NvAPI_DRS_SetSetting(hSession, hProfile, &setting);
    NvAPI_DRS_SaveSettings(hSession);
    
    NvAPI_DRS_DestroySession(hSession);
    NvAPI_Unload();
    
    emit memoryFallbackPolicyChanged();
}
