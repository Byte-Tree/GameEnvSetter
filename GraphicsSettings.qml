import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.15

GroupBox {
    title: "显卡设置"
    Layout.fillWidth: true
    implicitHeight: 1100

    ColumnLayout {
        width: parent.width
        spacing: 10

        TabBar {
            id: gpuTabBar
            Layout.fillWidth: true
            currentIndex: swipeView.currentIndex

            TabButton { text: "NVIDIA" }
            TabButton { text: "AMD" }
            TabButton { text: "Intel" }
        }

        SwipeView {
            id: swipeView
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: gpuTabBar.currentIndex

            Item {
                ColumnLayout {
                    width: parent.width
                    Label { text: "NVIDIA 显卡设置"; font.bold: true }
                    
                    GroupBox {
                        Layout.fillWidth: true
                        title: "管理3D设置"
                        ColumnLayout {
                            spacing: 8
                            RowLayout { Label { text: "图形增强" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "用AI提高帧数（适合看风景，但会加剧远处模糊感）。" } ComboBox { model: ["关", "开"]; currentIndex: graphicsConfig.getImageSharpeningStatus() ? 1 : 0; onActivated: function(index) { graphicsConfig.setImageSharpening(index === 1) } } }
                            //RowLayout { Label { text: "CUDA - GPUs" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "多显卡默认配置。" } ComboBox { model: graphicsConfig.availableGPUs; currentIndex: -1; onActivated: graphicsConfig.setSelectedGPUs([model[index]]) } }
                            //RowLayout { Label { text: "CUDA - 系统内存回退政策" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "显存不足时使用内存补充。" } ComboBox { model: ["驱动默认值", "偏好无系统内存回退", "偏好系统内存回退"]; currentIndex: graphicsConfig.memoryFallbackPolicy; onActivated: graphicsConfig.setMemoryFallbackPolicy(index) } }
                            RowLayout { Label { text: "DSR - 因数" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "伪分辨率技术（适合看风景）。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "OpenGL GDI兼容性" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "对老旧游戏的优化。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "OpenGL 渲染GPU" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "多显卡模式下选择需使用的GPU。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "Vulkan/OpenGL 现行方法" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "针对第9代及以后CPU使用DX12的优化。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "三重缓冲" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "减少画面撕裂（适合看风景，但增加输入延迟）。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "低延时模式" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "提高画面反馈速度（中低端显卡可能帧数波动）。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "各向异性过滤" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "提升远处物体清晰度（显卡性能越高可设置越高）。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "后台应用程序最大帧速率" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "省电节能。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "垂直同步" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "同步GPU渲染速率与显示器刷新率（适合看风景）。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "多重采样" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "提升画面细腻度（3060以下显卡建议关闭）。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "平滑处理 - FXAA" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "消除物体边缘锯齿。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "平滑处理 - 模式" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "抗锯齿模式选择。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "平滑处理 - 灰度纠正" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "优化画面对比度。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "平滑处理 - 设置" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "托管对象的抗锯齿处理。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "平滑处理 - 透明度" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "提升细节物体清晰度（显卡性能越高可设置越高，影响帧数）。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "最大帧速率" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "限制最大FPS。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "环境光吸收" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "优化画面光影效果。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "电源管理模式" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "性能优先级选择。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "着色器缓存大小" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "显存足够时可设为无限制。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "纹理过滤 - 三线性优化" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "动态降低远处清晰度。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "纹理过滤 - 各向异性采样优化" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "动态降低被遮挡物清晰度（可能停止绘制进程）。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "纹理过滤 - 负LOD偏移" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "是否动态调节细节物体清晰度。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "纹理过滤 - 质量" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "画质与帧数平衡（质量：画质+10%；高性能：帧数+20%）。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "线程优化" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "优化多核CPU性能。" } ComboBox { model: [] } }
                        }
                    }
                    
                    GroupBox {
                        Layout.fillWidth: true
                        title: "调整桌面尺寸和位置"
                        ColumnLayout {
                            spacing: 8
                            RowLayout { Label { text: "选择缩放模式" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "屏幕缩放模式。" } ComboBox { model: [] } }
                        }
                    }
                    
                    GroupBox {
                        Layout.fillWidth: true
                        title: "基础设置"
                        ColumnLayout {
                            spacing: 8
                            RowLayout { Label { text: "分辨率" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "屏幕分辨率适配。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "刷新率" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "显示器刷新率。" } ComboBox { model: [] } }
                        }
                    }
                }
            }

            Item {
                Label {
                    anchors.centerIn: parent
                    text: "AMD设置等待更新"
                }
            }

            Item {
                Label {
                    anchors.centerIn: parent
                    text: "Intel设置等待更新"
                }
            }
        }
    }
}
