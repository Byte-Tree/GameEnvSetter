#include "GraphicsConfigManager.h"

#include <QtCore/QThread>
#include <QtCore/QDebug>

typedef int NVAPI_STATUS; // 添加NVAPI_STATUS类型定义
#include "NvApiDriverSettings.h"

// NVAPI 字符串长度定义
#define NVAPI_SHORT_STRING_LENGTH 64
#define NVAPI_BINARY_DATA_MAX_SIZE 4096

// 静态成员初始化
bool GraphicsConfigManager::isNvAPIInitialized = false;
NvDRSSessionHandle GraphicsConfigManager::globalSession = 0;

bool GraphicsConfigManager::initializeNvAPI()
{
    if (!isNvAPIInitialized) {
        NVAPI_STATUS status = NvAPI_Initialize();
        if (status != NVAPI_OK) {
            qWarning() << "NvAPI_Initialize failed with error:" << status;
            return false;
        }
        status = NvAPI_DRS_CreateSession(&globalSession);
        if (status != NVAPI_OK) {
            qWarning() << "NvAPI_DRS_CreateSession failed with error:" << status;
            NvAPI_Unload();
            return false;
        }
        isNvAPIInitialized = true;
    }
    return true;
}

void GraphicsConfigManager::shutdownNvAPI()
{
    if (isNvAPIInitialized) {
        NvAPI_DRS_DestroySession(globalSession);
        NvAPI_Unload();
        isNvAPIInitialized = false;
    }
}

bool GraphicsConfigManager::getImageSharpeningStatus() {
    NVDRS_SETTING setting = queryNvidiaSetting(NV_QUALITY_UPSCALING_STRING);
    return setting.u32CurrentValue != 0;
}

// 新增的通用查询方法
NVDRS_SETTING GraphicsConfigManager::queryNvidiaSetting(const wchar_t* settingName) {
    NVDRS_SETTING setting = {0};
    setting.version = NVDRS_SETTING_VER;

    if (!initializeNvAPI()) {
        return setting;
    }

    // 加载最新配置（确保数据同步）
    NVAPI_STATUS status = NvAPI_DRS_LoadSettings(globalSession);
    qDebug() << "配置加载状态:" << status << "会话句柄:" << globalSession;
    if (status != NVAPI_OK) {
        qWarning() << "NvAPI_DRS_LoadSettings failed with error:" << status;
        return setting;
    }

    // 优先尝试当前全局配置（而非仅基础配置）
    NvDRSProfileHandle hProfile = 0;
    status = NvAPI_DRS_GetCurrentGlobalProfile(globalSession, &hProfile);
    qDebug() << "当前全局配置文件句柄:" << hProfile << "状态码:" << status;
    if (status != NVAPI_OK) {
        qWarning() << "NvAPI_DRS_GetCurrentGlobalProfile failed with error:" << status;
        return setting;
    }

    // 安全处理宽字符字符串
    std::wstring wideName(settingName);
    NvU32 settingId = 0;

    status = NvAPI_DRS_GetSettingIdFromName(
        reinterpret_cast<NvU16*>(const_cast<wchar_t*>(wideName.c_str())),
        &settingId
        );
    qDebug() << "设置项名称转ID结果 - 名称:" << QString::fromWCharArray(settingName) << "ID:" << settingId << "状态码:" << status;
    if (status != NVAPI_OK) {
        qWarning() << "Failed to get setting ID for " << settingName;
        return setting;
    }

    // 反向验证设置ID有效性
    NvAPI_UnicodeString settingNameVerify = {0};
    if (NvAPI_DRS_GetSettingNameFromId(settingId, &settingNameVerify) != NVAPI_OK) {
        qWarning() << "无效的设置ID:" << settingId << "名称:" << QString::fromWCharArray(settingName);
        return setting;
    }

    // 查询设置
    qDebug() << "正在查询设置项 ID:" << settingId << "名称:" << QString::fromWCharArray(settingName);

    // 验证设置项存在于当前配置
    auto list = getAllNvidiaSettings();
    QString qstr_settingName = QString::fromWCharArray(settingName);
    for(auto e : list)
    {
        if(qstr_settingName.compare(e.first) == 0)
        {
            qDebug()<<"在"<<list.size()<<"个可设置项中"<<"找到设置项："<<e;
            break;
        }
    }
    status = NvAPI_DRS_GetSetting(globalSession, hProfile, settingId, &setting);
    qDebug() << "NvAPI_DRS_GetSetting返回状态:" << status << "配置文件句柄:" << hProfile;
    if (status != NVAPI_OK) {
        qWarning() << "NvAPI_DRS_GetSetting failed with error:" << status;
    }

    return setting;
}

int GraphicsConfigManager::getVSyncMode() {
    NVDRS_SETTING setting = queryNvidiaSetting(VSYNCMODE_STRING);
    return setting.u32CurrentValue;
}

void GraphicsConfigManager::setVSyncMode(int mode) {
    NvDRSSessionHandle hSession = 0;
    NvDRSProfileHandle hProfile = 0;
    NVDRS_SETTING setting = {0};
    
    if (NvAPI_Initialize() != NVAPI_OK) return;
    
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
    NvU32 settingId = 0;
    QString qstr = QString::fromWCharArray(L"VSYNCMODE_STRING");
    if (NvAPI_DRS_GetSettingIdFromName((NvU16*)qstr.utf16(), &settingId) != NVAPI_OK) {
        qWarning() << "Failed to get setting ID for VSYNCMODE_STRING";
        return;
    }
    setting.settingId = settingId;
    setting.settingType = NVDRS_DWORD_TYPE;
    setting.u32CurrentValue = mode;
    
    NvAPI_DRS_SetSetting(hSession, hProfile, &setting);
    NvAPI_DRS_SaveSettings(hSession);
    
    NvAPI_DRS_DestroySession(hSession);
    NvAPI_Unload();
    
    emit vSyncModeChanged();
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
    NvU32 settingId = 0;
    QString qstr = QString::fromWCharArray(NV_QUALITY_UPSCALING_STRING);
    if (NvAPI_DRS_GetSettingIdFromName((NvU16*)qstr.utf16(), &settingId) != NVAPI_OK) {
        qWarning() << "Failed to get setting ID for " << NV_QUALITY_UPSCALING_STRING;
        return;
    }
    setting.settingId = settingId;
    setting.settingType = NVDRS_DWORD_TYPE;
    setting.u32CurrentValue = enabled ? 1 : 0;
    
    // NvAPI_DRS_SetSetting(hSession, hProfile, &setting);
    // NvAPI_DRS_SaveSettings(hSession);
    
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
    
    // if (NvAPI_DRS_GetSetting(hSession, hProfile, NV_CUDA_EXCLUDED_GPUS_SETTING_ID, &setting) == NVAPI_OK) {
    //     QString excludedGpus = QString::fromUtf8((char*)setting.binaryCurrentValue.valueData);
    //     QStringList gpuList = excludedGpus.split(',');
    //     selectedGpus.append(gpuList);
    // }
    
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
    
    // setting.version = NVDRS_SETTING_VER;
    // setting.settingId = NV_CUDA_EXCLUDED_GPUS_SETTING_ID;
    // setting.settingType = NVDRS_BINARY_TYPE;
    
    // QString gpuList = gpus.join(",");
    
    // setting.binaryCurrentValue.valueLength = gpuList.size();
    // strncpy((char*)setting.binaryCurrentValue.valueData, gpuList.toUtf8().constData(), NVAPI_BINARY_DATA_MAX_SIZE);
    
    // NvAPI_DRS_SetSetting(hSession, hProfile, &setting);
    // NvAPI_DRS_SaveSettings(hSession);
    
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
    
    // if (NvAPI_DRS_GetSetting(hSession, hProfile, NV_MEMORY_FALLBACK_POLICY_SETTING_ID, &setting) != NVAPI_OK) {
    //     NvAPI_DRS_DestroySession(hSession);
    //     NvAPI_Unload();
    //     return MemoryFallbackPolicy::DriverDefault;
    // }
    
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
    
    // setting.version = NVDRS_SETTING_VER;
    // setting.settingId = NV_MEMORY_FALLBACK_POLICY_SETTING_ID;
    // setting.settingType = NVDRS_DWORD_TYPE;
    
    // switch (policy) {
    //     case MemoryFallbackPolicy::PreferNoFallback: setting.u32CurrentValue = 1; break;
    //     case MemoryFallbackPolicy::PreferFallback: setting.u32CurrentValue = 2; break;
    //     default: setting.u32CurrentValue = 0;
    // }
    
    // NvAPI_DRS_SetSetting(hSession, hProfile, &setting);
    // NvAPI_DRS_SaveSettings(hSession);
    
    NvAPI_DRS_DestroySession(hSession);
    NvAPI_Unload();
    
    emit memoryFallbackPolicyChanged();
}


QList<QPair<QString, NvU32>> GraphicsConfigManager::getAllNvidiaSettings() {
    QList<QPair<QString, NvU32>> settingsList;

    if (!initializeNvAPI()) {
        qWarning() << "NVIDIA API 初始化失败";
        return settingsList;
    }

    NvU32 settingCount = 0;
    NVAPI_STATUS status = NvAPI_DRS_EnumAvailableSettingIds(nullptr,&settingCount);
    if (status != NVAPI_OK || settingCount == 0) {
        qWarning() << "无法获取设置项数量，错误码:" << status;
        return settingsList;
    }

    NvU32* settingIds = new NvU32[settingCount];
    status = NvAPI_DRS_EnumAvailableSettingIds(settingIds,&settingCount);
    if (status != NVAPI_OK) {
        qWarning() << "无法枚举设置项ID，错误码:" << status;
        delete[] settingIds;
        return settingsList;
    }

    for (NvU32 i = 0; i < settingCount; ++i) {
        NvAPI_UnicodeString  settingName = {0};
        status = NvAPI_DRS_GetSettingNameFromId(settingIds[i], &settingName);
        if (status == NVAPI_OK) {
            QString name = QString::fromWCharArray(reinterpret_cast<wchar_t*>(settingName));
            settingsList.append(qMakePair(name, settingIds[i]));
        } else {
            qWarning() << "无法获取设置名称，ID:" << settingIds[i] << "错误码:" << status;
        }
    }

    delete[] settingIds;

    // for(auto e : settingsList)
    // {
    //     qDebug()<<e<<Qt::endl;
    // }
    return settingsList;
}

