#include "SystemValueConverter.h"
#include <dxgi.h>
#include <wrl/client.h>

using namespace Microsoft::WRL;

SystemValueConverter::SystemValueConverter(QObject* parent) : QObject(parent) {}

double SystemValueConverter::normalizeMouseSpeed(int speed) {
    return qBound(0.0, (speed - 1) * (100.0 / 19.0), 100.0);
}

int SystemValueConverter::denormalizeMouseSpeed(double normalized) {
    return qBound(1, static_cast<int>(normalized * 19.0 / 100.0 + 1), 20);
}

QVariant SystemValueConverter::registryToDisplay(DWORD value) {
    switch(value) {
        case 0x26: return QObject::tr("后台服务");
        case 0x18: return QObject::tr("前台优化");
        default: return static_cast<int>(value);
    }
}

DWORD SystemValueConverter::displayToRegistry(const QVariant& value) {
    bool ok;
    QString str = value.toString().trimmed();
    
    // 处理十六进制输入
    if(str.startsWith("0x")) {
        return str.toUInt(&ok, 16);
    }
    
    // 处理预设字符串值
    if(str == QObject::tr("后台服务")) return 0x26;
    if(str == QObject::tr("前台优化")) return 0x18;

    // 处理十进制数值
    return str.toUInt(&ok);
}

QVariant SystemValueConverter::convertToRegistryFormat(const QString& value) {
    bool ok;
    QString str = value.trimmed();
    
    // 处理十六进制输入
    if(str.startsWith("0x")) {
        uint hexValue = str.toUInt(&ok, 16);
        if(!ok) return QVariant::fromValue(QString("无效的十六进制值"));
        return QVariant(hexValue);
    }
    
    // 处理十进制数值
    uint decValue = str.toUInt(&ok);
    if(!ok) return QVariant::fromValue(QString("无效的十进制值"));
    return QVariant(decValue);
}

QString SystemValueConverter::getGPUVendor() {
    ComPtr<IDXGIFactory> pFactory;
    if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory))) {
        return "Unknown";
    }

    ComPtr<IDXGIAdapter> pAdapter;
    if (FAILED(pFactory->EnumAdapters(0, &pAdapter))) {
        return "Unknown";
    }

    DXGI_ADAPTER_DESC desc;
    if (FAILED(pAdapter->GetDesc(&desc))) {
        return "Unknown";
    }

    QString vendorName = QString::fromWCharArray(desc.Description);
    if (vendorName.contains("NVIDIA", Qt::CaseInsensitive)) {
        return "NVIDIA";
    } else if (vendorName.contains("AMD", Qt::CaseInsensitive) || 
               vendorName.contains("ATI", Qt::CaseInsensitive)) {
        return "AMD";
    } else if (vendorName.contains("Intel", Qt::CaseInsensitive)) {
        return "Intel";
    }

    return "Unknown";
}