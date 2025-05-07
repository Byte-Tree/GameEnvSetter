import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.15

GroupBox {
    title: "鼠标设置"
    Layout.fillWidth: true
    ColumnLayout {
        spacing: 15
        
        // 双击速度滑块
        GroupBox {
            Layout.fillWidth: true
            ColumnLayout {
                RowLayout {
                    Label { text: "双击速度" }
                    Button {
                        text: "?"
                        ToolTip.visible: hovered
                        ToolTip.text: "控制鼠标双击的时间间隔（100-900毫秒），数值越小响应越快"
                    }
                }
                Slider {
                    from: 100
                    to: 900
                    stepSize: 1
                    value: mouseConfig.doubleClickSpeed
                    onMoved: mouseConfig.setDoubleClickSpeed(value)
                }
                Text {
                    text: "当前值：" + mouseConfig.doubleClickSpeed + "ms"
                }
            }
        }

        // 移动速度滑块
        GroupBox {
            Layout.fillWidth: true
            ColumnLayout {
                RowLayout {
                    Label { text: "移动速度" }
                    Button {
                        text: "?"
                        ToolTip.visible: hovered
                        ToolTip.text: "调节鼠标指针移动速度（1-20级），数值越大移动越快"
                    }
                }
                Slider {
                    from: 1
                    to: 20
                    stepSize: 1
                    value: mouseConfig.mouseSpeed
                    onMoved: mouseConfig.setMouseSpeed(value)
                }
                Text {
                    text: "当前级别：" + mouseConfig.mouseSpeed
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
            ToolTip.text: "推荐设置：双击速度100ms，移动速度10级，关闭指针精确度"
            ToolTip.delay: 200
            onClicked: {
                mouseConfig.setDoubleClickSpeed(100)
                mouseConfig.setMouseSpeed(10)
                mouseConfig.setEnhancePointerPrecision(false)
            }
        }
    }
}
