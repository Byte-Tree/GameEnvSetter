import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.15
import ConfigManagers 1.0
import SystemValueConverter 1.0
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

    ColumnLayout {
        anchors.fill: parent
        spacing: 20
        Layout.margins: 20

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: "保存当前设置"
                onClicked: {
                    saveFileDialog.open()
                }
            }

            Button {
                text: "还原默认设置"
                onClicked: {
                    if(configManager.configExists()) {
                        var config = configManager.loadConfig()
                        mouseConfig.setDoubleClickSpeed(config.mouse.doubleClickSpeed)
                        mouseConfig.setMouseSpeed(config.mouse.mouseSpeed)
                        mouseConfig.setEnhancePointerPrecision(config.mouse.enhancePointerPrecision)
                        keyboardConfig.setRepeatDelay(config.keyboard.repeatDelay)
                        keyboardConfig.setRepeatRate(config.keyboard.repeatRate)
                        registryOperator.setWin32PrioritySeparation(config.registry.win32PrioritySeparation)
                        toolTip.show("已还原默认配置", 2000)
                    }
                }
            }

            Button {
                text: "使用三方设置"
                onClicked: fileDialog.open()
            }
        }

        // 鼠标设置区域
        GroupBox {
            title: "鼠标设置"
            Layout.fillWidth: true
            ColumnLayout {
                spacing: 15
                LabeledSlider {
                    label: "双击速度"
                    tooltipText: "控制鼠标双击的时间间隔（100-900毫秒），数值越小响应越快"
                    from: 100
                    to: 900
                    stepSize: 1
                    value: mouseConfig.doubleClickSpeed
                    onSliderMoved: function(value) {
                        const clampedValue = Math.max(100, Math.min(900, value))
                        if(clampedValue !== mouseConfig.doubleClickSpeed) {
                            mouseConfig.setDoubleClickSpeed(clampedValue)
                        }
                    }
                }
                LabeledSlider {
                    label: "移动速度"
                    tooltipText: "调节鼠标指针移动速度（1-20级），数值越大移动越快"
                    from: 1
                    to: 20
                    stepSize: 1
                    value: Math.round(mouseConfig.mouseSpeed)
                    onSliderMoved: function(value) {
                        const clampedValue = Math.max(1, Math.min(20, value))
                        if(clampedValue !== mouseConfig.mouseSpeed) {
                            mouseConfig.setMouseSpeed(clampedValue)
                        }
                    }
                }
                RowLayout {
                    CheckBox {
                        text: "提高指针精确度"
                        checked: mouseConfig.enhancePointerPrecision
                        onCheckedChanged: {
                            if(checked !== mouseConfig.enhancePointerPrecision) {
                                mouseConfig.setEnhancePointerPrecision(checked)
                            }
                        }
                    }
                    Button {
                        text: "?"
                        implicitWidth: 20
                        implicitHeight: 20
                        ToolTip.visible: hovered
                        ToolTip.text: "启用Windows鼠标加速功能，移动速度较慢时提供更精准的指针定位"
                        ToolTip.delay: 200
                    }
                }
                Button {
                    text: "推荐设置"
                    ToolTip.visible: hovered
                    ToolTip.text: "推荐设置：双击速度300ms，移动速度10级，关闭指针精确度"
                    ToolTip.delay: 200
                    onClicked: {
                        mouseConfig.setDoubleClickSpeed(300)
                        mouseConfig.setMouseSpeed(10)
                        mouseConfig.setEnhancePointerPrecision(false)
                    }
                }
            }
        }

        // 键盘设置区域
        GroupBox {
            title: "键盘设置"
            Layout.fillWidth: true
            ColumnLayout {
                spacing: 15
                LabeledSlider {
                    label: "重复延迟"
                    tooltipText: "设置按键重复前的初始延迟时间（0-3档对应250-1000毫秒）"
                    from: 0
                    to: 3
                    stepSize: 1
                    value: keyboardConfig.repeatDelay
                    onSliderMoved: keyboardConfig.setRepeatDelay(value)
                    valueDisplay: value + "档 (" + (250 * (value + 1)) + "ms)"
                }
                LabeledSlider {
                    label: "重复速度"
                    tooltipText: "调整按键重复频率（0-31档对应2.5-30字符/秒）"
                    from: 0
                    to: 31
                    stepSize: 1
                    value: keyboardConfig.repeatRate
                    onSliderMoved: keyboardConfig.setRepeatRate(value)
                    valueDisplay: value + "档 (" + (2.5 + (value * (30.0 - 2.5) / 31.0)).toFixed(1) + "cps)"
                }
                Button {
                    text: "推荐设置"
                    ToolTip.visible: hovered
                    ToolTip.text: "推荐设置：重复延迟1档(500ms)，重复速度20档(20字符/秒)"
                    ToolTip.delay: 200
                    onClicked: {
                        keyboardConfig.setRepeatDelay(1)
                        keyboardConfig.setRepeatRate(20)
                    }
                }
            }
        }

        GroupBox {
            title: "注册表设置"
            Layout.fillWidth: true
            implicitHeight: 180
            ColumnLayout {
                spacing: 15
                width: parent.width
                anchors.top: parent.top
                RowLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignLeft
                    spacing: 5
                    Label {
                        text: "Win32PrioritySeparation"
                    }
                    Button {
                        text: "?"
                        implicitWidth: 20
                        implicitHeight: 20
                        ToolTip.visible: hovered
                        ToolTip.text: "控制前台/后台进程的CPU优先级分配（0-255），数值越高前台进程优先级越高"
                        ToolTip.delay: 200
                    }
                }

                TextField {
                    Layout.fillWidth: true
                    id: regValueInput
                    placeholderText: "你可以输入形如\"0xAB\"，\"123\"的数据"
                    property bool isHex: false
                    onTextChanged: {
                        var converted = SystemValueConverter.convertToRegistryFormat(text)
                        if(converted.error) {
                            regValueInput.ToolTip.show(converted.error, 2000)
                        }
                    }
                }

                Button {
                    Layout.fillWidth: true
                    text: "应用设置"
                    onClicked: {
                        if(regValueInput.acceptableInput) {
                            registryOperator.setWin32PrioritySeparation(regValueInput.text)
                        }
                    }
                }

                Text {
                    text: "当前值：" + registryOperator.win32PrioritySeparation
                    color: "gray"
                    Layout.alignment: Qt.AlignRight
                    Layout.fillWidth: true
                }
                Button {
                    text: "推荐设置"
                    ToolTip.visible: hovered
                    ToolTip.text: "推荐设置：38(平衡前后台优先级)"
                    ToolTip.delay: 200
                    onClicked: {
                        registryOperator.setWin32PrioritySeparation(38)
                    }
                }
            }
        }
    }

    component LabeledSlider : RowLayout {
        property string valueDisplay
        property string label
        property string tooltipText
        property alias from: slider.from
        property alias to: slider.to
        property alias stepSize: slider.stepSize
        property alias value: slider.value
        signal sliderMoved(int value)

        Label { text: parent.label + ": " + (parent.valueDisplay ? parent.valueDisplay : (parent.label.indexOf("双击") > -1 ? slider.value + " ms" : slider.value + " 级数")) }
        Slider {
            id: slider
            Layout.fillWidth: true
            snapMode: Slider.SnapOnRelease
            onMoved: parent.sliderMoved(value)
        }
        Button {
            text: "?"
            implicitWidth: 20
            implicitHeight: 20
            ToolTip.visible: hovered
            ToolTip.text: parent.tooltipText
            ToolTip.delay: 200
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
              configManager.saveUserConfig(currentConfig, currentFile);
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
          }
    }

}
