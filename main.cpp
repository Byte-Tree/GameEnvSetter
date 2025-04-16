#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "MouseConfigManager.h"
#include "KeyboardConfigManager.h"
#include "RegistryOperator.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<MouseConfigManager>("ConfigManagers", 1, 0, "MouseConfigManager");
    qmlRegisterType<KeyboardConfigManager>("ConfigManagers", 1, 0, "KeyboardConfigManager");
    qmlRegisterType<RegistryOperator>("ConfigManagers", 1, 0, "RegistryOperator");

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

    return app.exec();
}
