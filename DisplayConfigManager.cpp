#include "DisplayConfigManager.h"
#include <QDebug>
#include <QVariantList>

DisplayConfigManager::DisplayConfigManager(QObject *parent) : QObject(parent) {
    // 初始化代码
}

bool DisplayConfigManager::isDynamicRefreshRateSupported() {
    QVariantList rates = getAvailableRefreshRates();
    return rates.size() > 1;
}

QVariantList DisplayConfigManager::getAvailableRefreshRates() {
    QList<int> uniqueRates;
    DEVMODE devMode;
    
    for (DWORD modeNum = 0; EnumDisplaySettings(NULL, modeNum, &devMode); modeNum++) {
        if (devMode.dmFields & DM_DISPLAYFREQUENCY) {
            int rate = devMode.dmDisplayFrequency;
            if (!uniqueRates.contains(rate)) {
                uniqueRates.append(rate);
            }
        }
    }
    
    std::sort(uniqueRates.begin(), uniqueRates.end());
    QVariantList sortedRates;
    for (int rate : uniqueRates) {
        sortedRates.append(rate);
    }
    return sortedRates;
}

int DisplayConfigManager::getRefreshRate() {
    DEVMODE devMode;
    devMode.dmSize = sizeof(DEVMODE);
    devMode.dmFields = DM_DISPLAYFREQUENCY;
    
    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode)) {
        int currentRate = devMode.dmDisplayFrequency;
    QVariantList rates = getAvailableRefreshRates();
    if (!rates.contains(currentRate)) {
        return rates.isEmpty() ? 60 : rates.first().toInt();
    }
    return currentRate;
    }
    return getAvailableRefreshRates().isEmpty() ? 60 : getAvailableRefreshRates().first().toInt();
}

void DisplayConfigManager::setRefreshRate(int rate) {
    int currentRate = getRefreshRate();
    if (currentRate == rate) {
        return;
    }
    DEVMODE devMode;
    devMode.dmSize = sizeof(DEVMODE);
    devMode.dmFields = DM_DISPLAYFREQUENCY;
    devMode.dmDisplayFrequency = rate;
    
    if (ChangeDisplaySettings(&devMode, CDS_TEST) != DISP_CHANGE_SUCCESSFUL) {
        emit errorOccurred("不支持该刷新率设置");
        return;
    }
    
    LONG result = ChangeDisplaySettings(&devMode, 0);
    if (result == DISP_CHANGE_SUCCESSFUL) {
        emit refreshRateChanged();
    } else {
        emit errorOccurred("刷新率设置失败");
    }
}
