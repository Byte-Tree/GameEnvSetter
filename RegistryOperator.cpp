#include "RegistryOperator.h"
#include <QDebug>
#include "SystemValueConverter.h"

const wchar_t* RegistryOperator::REG_KEY_PATH = L"SYSTEM\\CurrentControlSet\\Control\\PriorityControl";
const wchar_t* RegistryOperator::REG_VALUE_NAME = L"Win32PrioritySeparation";

RegistryOperator::RegistryOperator(QObject* parent) : QObject(parent) {}

DWORD RegistryOperator::win32PrioritySeparation() const {
    HKEY hKey;
    DWORD value = 0;
    DWORD size = sizeof(DWORD);

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_PATH, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueEx(hKey, REG_VALUE_NAME, NULL, NULL, (LPBYTE)&value, &size);
        RegCloseKey(hKey);
    }
    return value;
}

bool RegistryOperator::setWin32PrioritySeparation(DWORD value) {
    if(value > 0x3F) {
        emit registryErrorOccurred(tr("值必须在0x0-0x3F(0-63)范围内"));
        return false;
    }
    DWORD dwValue = value;
    HKEY hKey;
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_PATH, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        emit registryErrorOccurred(tr("无法打开注册表键"));
        return false;
    }

    if(RegSetValueEx(hKey, REG_VALUE_NAME, 0, REG_DWORD, (const BYTE*)&dwValue, sizeof(dwValue)) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        emit registryErrorOccurred(tr("写入注册表失败"));
        return false;
    }

    RegCloseKey(hKey);
    emit win32PrioritySeparationChanged();
    return true;
}
