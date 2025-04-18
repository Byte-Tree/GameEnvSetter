import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.15
import ConfigManagers 1.0

import Qt.labs.platform as Platform

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    title: qsTr("系统设置工具")
    property bool firstRun: false
    property bool configSaved: false

    MouseConfigManager { id: mouseConfig }
    KeyboardConfigManager { id: keyboardConfig }
    RegistryOperator { id: registryOperator }
    ConfigManager { id: configManager }

    Component.onCompleted: {
        firstRun = !configManager.configExists()
        if(firstRun) {
            firstRunDialog.open()
        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: -1
        clip: true
        
        ColumnLayout {
            width: parent.width
            spacing: 20
            Layout.margins: 20

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: "保存当前设置"
                onClicked: {
                    try {
                        saveFileDialog.open()
                    } catch(error) {
                        toolTip.show("保存配置失败：" + error, 3000)
                    }
                }
            }

            Button {
                text: "还原默认设置"
                onClicked: {
                    try {
                        var config = configManager.loadConfig()
                        mouseConfig.setDoubleClickSpeed(config.mouse.doubleClickSpeed)
                        mouseConfig.setMouseSpeed(config.mouse.mouseSpeed)
                        mouseConfig.setEnhancePointerPrecision(config.mouse.enhancePointerPrecision)
                        keyboardConfig.setRepeatDelay(config.keyboard.repeatDelay)
                        keyboardConfig.setRepeatRate(config.keyboard.repeatRate)
                        registryOperator.setWin32PrioritySeparation(config.registry.win32PrioritySeparation)
                        toolTip.show("默认配置已应用", 2000)
                    } catch(error) {
                        toolTip.show("加载默认配置失败：" + error, 3000)
                    }
                }
            }

            Button {
                text: "使用三方设置"
                onClicked: fileDialog.open()
            }
        }

        MouseSettings {}

        KeyboardSettings {}

        RegistrySettings {}
        }
    }


    Connections {
        target: mouseConfig
        function onErrorOccurred(msg) { toolTip.show(msg, 2000) }
    }

    Connections {
        target: keyboardConfig
        function onErrorOccurred(msg) { toolTip.show(msg, 2000) }
    }

    Connections {
        target: registryOperator
        function onRegistryErrorOccurred(msg) { toolTip.show(msg, 2000) }
    }

    Dialog {
        id: firstRunDialog
        title: "首次运行"
        standardButtons: Dialog.Yes | Dialog.No
        modal: true
        dim: true // 启用模态对话框并添加背景遮罩
        parent: ApplicationWindow.overlay
        anchors.centerIn: parent
        width: parent.width * 0.6
        height: 150
        Label {
            text: "这是您第一次运行本软件，是否要保存当前系统配置作为原始配置？"
            wrapMode: Text.WordWrap
            anchors.fill: parent
        }
        onAccepted: {
            configManager.saveConfig({
                "mouse": {
                    "doubleClickSpeed": mouseConfig.doubleClickSpeed,
                    "mouseSpeed": mouseConfig.mouseSpeed,
                    "enhancePointerPrecision": mouseConfig.enhancePointerPrecision
                },
                "keyboard": {
                    "repeatDelay": keyboardConfig.repeatDelay,
                    "repeatRate": keyboardConfig.repeatRate
                },
                "registry": {
                    "win32PrioritySeparation": registryOperator.win32PrioritySeparation
                }
            })
            configSaved = true
        }
        onRejected: {
            if(!configManager.userConfigExists()) {
                toolTip.show("原始配置未保存，还原默认设置功能将不可用", 3000)
            }
        }
    }

    Item {
        id: toolTipContainer
        anchors.centerIn: parent
        width: parent.width * 0.5
        height: 50

        ToolTip {
            id: toolTip
            width: parent.width
            height: parent.height
        }
    }

        Platform.FileDialog {
          id: saveFileDialog
          title: "保存配置文件"
          nameFilters: ["INI文件 (*.ini)"]
          fileMode: Platform.FileDialog.SaveFile
          onAccepted: {
              var currentConfig = {
                  "mouse": {
                      doubleClickSpeed: mouseConfig.doubleClickSpeed,
                      mouseSpeed: mouseConfig.mouseSpeed,
                      enhancePointerPrecision: mouseConfig.enhancePointerPrecision
                  },
                  "keyboard": {
                      repeatDelay: keyboardConfig.repeatDelay,
                      repeatRate: keyboardConfig.repeatRate
                  },
                  "registry": {
                      win32PrioritySeparation: registryOperator.win32PrioritySeparation
                  }
              };
              try {
    configManager.saveUserConfig(currentConfig, saveFileDialog.currentFile);
    toolTip.show("用户配置已保存", 2000);
} catch(error) {
    toolTip.show("保存失败：" + error, 3000);
}
              toolTip.show("用户配置已保存", 2000);
          }
        }

        Platform.FileDialog {
          id: fileDialog
          title: "选择配置文件"
          nameFilters: ["用户配置 (*.ini)"]
          onAccepted: {
              var config = configManager.loadFromPath(fileDialog.currentFile)
              mouseConfig.setDoubleClickSpeed(config.mouse.doubleClickSpeed)
              mouseConfig.setMouseSpeed(config.mouse.mouseSpeed)
              mouseConfig.setEnhancePointerPrecision(config.mouse.enhancePointerPrecision)
              keyboardConfig.setRepeatDelay(config.keyboard.repeatDelay)
              keyboardConfig.setRepeatRate(config.keyboard.repeatRate)
              registryOperator.setWin32PrioritySeparation(config.registry.win32PrioritySeparation)
              toolTip.show("三方配置已应用", 2000)
          }
    }

}
