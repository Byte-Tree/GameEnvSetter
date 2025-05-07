import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.15

GroupBox {
    title: "键盘设置"
    Layout.fillWidth: true
    ColumnLayout {
        spacing: 15
        
        GroupBox {
            Layout.fillWidth: true
            ColumnLayout {
                RowLayout {
                    Label { text: "重复延迟" }
                    Button {
                        text: "?"
                        ToolTip.visible: hovered
                        ToolTip.text: "设置按键重复前的初始延迟时间（0-3档对应250-1000毫秒）"
                    }
                }
                Slider {
                    from: 0
                    to: 3
                    stepSize: 1
                    value: keyboardConfig.repeatDelay
                    onMoved: keyboardConfig.setRepeatDelay(value)
                }
                Text {
                    text: valueDisplay
                    property string valueDisplay: keyboardConfig.repeatDelay + "档 (" + (250 * (keyboardConfig.repeatDelay + 1)) + "ms)"
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true
            ColumnLayout {
                RowLayout {
                    Label { text: "重复速度" }
                    Button {
                        text: "?"
                        ToolTip.visible: hovered
                        ToolTip.text: "调整按键重复频率（0-31档对应2.5-30字符/秒）"
                    }
                }
                Slider {
                    from: 0
                    to: 31
                    stepSize: 1
                    value: keyboardConfig.repeatRate
                    onMoved: keyboardConfig.setRepeatRate(value)
                }
                Text {
                    text: valueDisplay
                    property string valueDisplay: keyboardConfig.repeatRate + "档 (" + (2.5 + (keyboardConfig.repeatRate * (30.0 - 2.5) / 31.0)).toFixed(1) + "cps)"
                }
            }
        }
        Button {
            text: "推荐设置"
            ToolTip.visible: hovered
            ToolTip.text: "推荐设置：重复延迟0档(250ms)，重复速度31档(31字符/秒)"
            ToolTip.delay: 200
            onClicked: {
                keyboardConfig.setRepeatDelay(0)
                keyboardConfig.setRepeatRate(31)
            }
        }
    }
}
