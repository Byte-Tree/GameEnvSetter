#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "MouseConfigManager.h"
#include "KeyboardConfigManager.h"
#include "RegistryOperator.h"
#include "SystemValueConverter.h"
#include "ConfigManager.h"
#include "NVIDIAGraphicsConfigManager.h"
#include "DisplayConfigManager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<MouseConfigManager>("ConfigManagers", 1, 0, "MouseConfigManager");
    qmlRegisterType<KeyboardConfigManager>("ConfigManagers", 1, 0, "KeyboardConfigManager");
    qmlRegisterType<RegistryOperator>("ConfigManagers", 1, 0, "RegistryOperator");
    qmlRegisterType<ConfigManager>("ConfigManagers", 1, 0, "ConfigManager");
    qmlRegisterType<NVIDIAGraphicsConfigManager>("ConfigManagers", 1, 0, "NVIDIAGraphicsConfigManager");
    qmlRegisterType<DisplayConfigManager>("ConfigManagers", 1, 0, "DisplayConfigManager");

    qmlRegisterSingletonType<SystemValueConverter>(
        "SystemValueConverter", // URI
        1, 0,                  // 版本号
        "SystemValueConverter",            // 在QML中使用的名称
        [](QQmlEngine*, QJSEngine*) -> QObject* { return new SystemValueConverter(); } // 构造函数指针或lambda表达式创建实例
        );

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/GameEnvSetter/Main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    QObject::connect(&app, &QCoreApplication::aboutToQuit, &NVIDIAGraphicsConfigManager::shutdownNvAPI);

    return app.exec();
}
