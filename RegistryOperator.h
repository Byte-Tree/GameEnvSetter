#pragma once
#include <QObject>
#include <Windows.h>

class RegistryOperator : public QObject {
    Q_OBJECT
    Q_PROPERTY(DWORD win32PrioritySeparation READ win32PrioritySeparation WRITE setWin32PrioritySeparation NOTIFY win32PrioritySeparationChanged)

public:
    explicit RegistryOperator(QObject* parent = nullptr);

    DWORD win32PrioritySeparation() const;
    Q_INVOKABLE bool setWin32PrioritySeparation(DWORD value);

signals:
    void win32PrioritySeparationChanged();
    void registryErrorOccurred(const QString& msg);

private:
    static const wchar_t* REG_KEY_PATH;
    static const wchar_t* REG_VALUE_NAME;
};