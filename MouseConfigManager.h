#pragma once
#include <QObject>
#include <Windows.h>

class MouseConfigManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int doubleClickSpeed READ doubleClickSpeed WRITE setDoubleClickSpeed NOTIFY doubleClickSpeedChanged)
    Q_PROPERTY(int mouseSpeed READ mouseSpeed WRITE setMouseSpeed NOTIFY mouseSpeedChanged)
    Q_PROPERTY(bool enhancePointerPrecision READ enhancePointerPrecision WRITE setEnhancePointerPrecision NOTIFY enhancePointerPrecisionChanged)

public:
    explicit MouseConfigManager(QObject* parent = nullptr);

    int doubleClickSpeed() const;
    Q_INVOKABLE void setDoubleClickSpeed(int ms);

    int mouseSpeed() const;
    Q_INVOKABLE void setMouseSpeed(int speed);

    bool enhancePointerPrecision() const;
    Q_INVOKABLE void setEnhancePointerPrecision(bool enabled);

signals:
    void doubleClickSpeedChanged();
    void mouseSpeedChanged();
    void enhancePointerPrecisionChanged();
    void errorOccurred(const QString& msg);

private:
    bool updateSystemParameter(UINT uiAction, int value) const;
};
