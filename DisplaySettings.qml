import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.15

GroupBox {

    title: "显示器设置"
    Layout.fillWidth: true

    ColumnLayout {
        width: parent.width
        spacing: 8
        
        // 对比度设置
        // RowLayout {
        //     Label { text: "对比度" }
        //     Slider {
        //         id: contrastSlider
        //         from: 0
        //         to: 100
        //         value: displayConfig.contrast
        //         onMoved: displayConfig.setContrast(value)
        //     }
        //     Label { text: contrastSlider.value + "%" }
        // }
        
        // 刷新率设置
        RowLayout { 
            Label { text: "刷新率" } 
            ComboBox {
                id: refreshRateCombo
                model: displayConfig.getAvailableRefreshRates()
                currentIndex: model.indexOf(displayConfig.refreshRate)
                onActivated: displayConfig.setRefreshRate(model[index])
            }
        }
    }
}
