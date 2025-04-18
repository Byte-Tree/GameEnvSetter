import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.15
import SystemValueConverter 1.0

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
