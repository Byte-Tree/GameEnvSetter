import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.15

GroupBox {
    title: "显卡设置"
    Layout.fillWidth: true
    implicitHeight: 1150

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
                            RowLayout { Label { text: "图形增强" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "用AI提高帧数（适合看风景，但会加剧远处模糊感）。" }
                                ComboBox { model: ["未知","关", "开"];
                                    currentIndex: graphicsConfig.imageSharpening; onActivated: function(index) { graphicsConfig.setImageSharpening(index) } } }
                            RowLayout { Label { text: "CUDA - GPUs" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "多显卡默认配置。" } ComboBox { enabled: false; } }
                            RowLayout { Label { text: "CUDA - 系统内存回退政策" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "显存不足时使用内存补充。" } ComboBox { enabled: false; model: ["驱动默认值", "偏好无系统内存回退", "偏好系统内存回退"]; } }
                            RowLayout { Label { text: "DSR - 因数" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "伪分辨率技术（适合看风景）。" } ComboBox { enabled: false; model: [] } }
                            RowLayout { Label { text: "OpenGL GDI兼容性" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "对老旧游戏的优化。" }
                                ComboBox { model: ["未知","优先性能","优先兼容性","自动"];
                                    currentIndex: graphicsConfig.openGLGDICompatibility; onActivated: function(index) { graphicsConfig.setOpenGLGDICompatibility(index) } } }
                            RowLayout { Label { text: "OpenGL 渲染GPU" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "多显卡模式下选择需使用的GPU。" } ComboBox { model: [] } }
                            RowLayout { Label { text: "Vulkan/OpenGL 现行方法" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "针对第9代及以后CPU使用DX12的优化。" }
                                ComboBox { model: ["未知","优先本机","优先在DIGX交换链上分层","自动"];
                                    currentIndex: graphicsConfig.openGLPresentMethod; onActivated: function(index) { graphicsConfig.setOpenGLPresentMethod(index) } } }
                            RowLayout { Label { text: "三重缓冲" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "减少画面撕裂（适合看风景，但增加输入延迟）。" }
                                ComboBox { model: ["未知","关","开"];
                                    currentIndex: graphicsConfig.tripleBuffer; onActivated: function(index) { graphicsConfig.setTripleBuffer(index) } } }
                            RowLayout { Label { text: "低延时模式" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "提高画面反馈速度（中低端显卡可能帧数波动）。" }
                                ComboBox { model: ["未知","关","开(超高)"]
                                    currentIndex: graphicsConfig.lowLatencyMode
                                    onActivated: function(index) { graphicsConfig.setLowLatencyMode(index) }
                                } }
                            RowLayout { Label { text: "各向异性过滤" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "提升远处物体清晰度（显卡性能越高可设置越高）。" } 
                                ComboBox { 
                                    model: Array.from({length: 16}, (_, i) => i+1);
                                    currentIndex: graphicsConfig.anisotropicFiltering;
                                    onActivated: function(index) { graphicsConfig.setAnisotropicFiltering(index) } 
                                } 
                            }
                            RowLayout {
                                Label { text: "空闲应用最大帧率" }
                                Button {
                                    text: "?"
                                    ToolTip.visible: hovered
                                    ToolTip.text: "限制后台程序帧率节省资源（0表示关闭限制）"
                                }
                                Slider {
                                    id:idleFpsSlider
                                    from: 0
                                    to: 1023
                                    stepSize: 1
                                    value: graphicsConfig.appIdleFPSLimit
                                    onMoved: graphicsConfig.setAppIdleFPSLimit(value)
                                    Layout.fillWidth: true
                                }
                                Text {
                                    text: "当前值：" + idleFpsSlider.value
                                    Layout.alignment: Qt.AlignRight
                                }
                            }
                            RowLayout { Label { text: "垂直同步" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "同步GPU渲染速率与显示器刷新率（适合看风景）。" }
                                ComboBox { model: ["未知","使用3D应用程序设置","关", "开","VSYNCMODE_FLIPINTERVAL2","VSYNCMODE_FLIPINTERVAL3","VSYNCMODE_FLIPINTERVAL4","VSYNCMODE_VIRTUAL"];
                                    currentIndex: graphicsConfig.vSyncMode; onActivated: function(index) { graphicsConfig.setVSyncMode(index) } } }
                            // RowLayout { Label { text: "多重采样" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "提升画面细腻度（3060以下显卡建议关闭）。" }
                            //     ComboBox {
                            //         model: ["关", "2x", "4x", "8x", "16x"]
                            //         currentIndex: graphicsConfig.aaModeMethod
                            //         onActivated: function(index) {graphicsConfig.setAAModeMethod(index)}
                            //     }
                            // }
                            RowLayout { Label { text: "平滑处理 - FXAA" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "消除物体边缘锯齿。" } 
                                ComboBox { 
                                    model: ["未知","关","开"]
                                    currentIndex: graphicsConfig.fxaaEnable
                                    onActivated: function(index) {graphicsConfig.setFXAAEnable(index)}
                                } 
                            }
                            RowLayout { Label { text: "平滑处理 - 模式" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "抗锯齿模式选择。" }
                                ComboBox {
                                    model: ["应用程序控制的","关","提高应用程序设置","置换任何应用程序设置"];
                                    currentIndex: graphicsConfig.aaModeSelector;
                                    onActivated: function(index) { graphicsConfig.setAAModeSelector(index) }
                                } }
                            RowLayout { Label { text: "平滑处理 - 灰度纠正" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "优化画面对比度。" } 
                                ComboBox { 
                                    model: ["关", "条件开启", "始终开启"]
                                    currentIndex: graphicsConfig.aaGammaCorrection
                                    onActivated: function(index) {graphicsConfig.setAAGammaCorrection(index)}
                                } 
                            }
                            RowLayout { Label { text: "平滑处理 - 设置（多重采样）" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "托管对象的抗锯齿处理。提升画面细腻度（3060以下显卡建议关闭）。" }
                                ComboBox { 
                                    model: ["应用程序控制的(关)", "2x", "4x","8x","16x"]
                                    currentIndex: graphicsConfig.aaModeMethod
                                    onActivated: function(index) {graphicsConfig.setAAModeMethod(index)}
                                } }
                            RowLayout { Label { text: "平滑处理 - 透明度" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "提升细节物体清晰度（显卡性能越高可设置越高，影响帧数）。" } ComboBox { model: ["关", "多重采样", "超级采样(2x)", "超级采样(4x)","超级采样(8x)"]
                                currentIndex: graphicsConfig.aaTransparency
                                onActivated: function(index) {graphicsConfig.setAATransparency(index)}
                                } }
                            RowLayout { 
                                Label { text: "最大帧速率" }
                                Button {
                                    text: "?"
                                    ToolTip.visible: hovered
                                    ToolTip.text: "限制最大FPS（0表示关闭限制）"
                                }
                                Slider {
                                    id:maxFpsSlider
                                    from: 0
                                    to: 1023
                                    stepSize: 1
                                    value: graphicsConfig.maxFPSLimit
                                    onMoved: graphicsConfig.setMaxFPSLimit(value)
                                    Layout.fillWidth: true
                                }
                                Text {
                                    text: "当前值：" + maxFpsSlider.value
                                    Layout.alignment: Qt.AlignRight
                                }
                            }
                            RowLayout { Label { text: "环境光吸收" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "优化画面光影效果。" }
                                ComboBox { 
                                    model: ["关","性能(低)","质量(中)","高"]
                                    currentIndex: graphicsConfig.aoMode
                                    onActivated: graphicsConfig.setAoMode(index)
                                } }
                            RowLayout { Label { text: "电源管理模式" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "控制显卡功耗策略（游戏推荐最大性能）。" } 
                                ComboBox { model: ["未知","自适应","最大性能","驱动控制","一致性能","最小功耗","最优电源"];
                                    currentIndex: graphicsConfig.powerManagementMode; onActivated: function(index) { graphicsConfig.setPowerManagementMode(index) } }}
                            RowLayout { 
                                Label { text: "着色器缓存大小" }
                                Button { 
                                    text: "?"
                                    ToolTip.visible: hovered
                                    ToolTip.text: "显存足够时可设为无限制(即：4,294,967,295)。"
                                }
                                Text {
                                    text: "当前值：" + graphicsConfig.shaderCacheSize + "MB"
                                    Layout.alignment: Qt.AlignRight
                                }
                                TextField {
                                    placeholderText: "例如：4096 单位：MB"
                                    Layout.fillWidth: true
                                }
                                Button {
                                    text: "设置"
                                    onClicked: graphicsConfig.setShaderCacheSize(parent.children[3].text)
                                }
                            }
                            RowLayout { Label { text: "纹理过滤 - 三线性优化" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "动态降低远处清晰度。" }
                                ComboBox { model: ["开","关"]
                                    currentIndex: graphicsConfig.trilinearOptimization
                                    onActivated: function(index) { graphicsConfig.setTrilinearOptimization(index) } } }
                            RowLayout { Label { text: "纹理过滤 - 各向异性采样优化" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "动态降低被遮挡物清晰度（可能停止绘制进程）。" } 
                                ComboBox { model: ["关","开"]
                                    currentIndex: graphicsConfig.anisotropicSampleOptimization
                                    onActivated: function(index) { graphicsConfig.setAnisotropicSampleOptimization(index) }
                                } }
                            RowLayout { Label { text: "纹理过滤 - 负LOD偏移" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "是否动态调节细节物体清晰度。" } ComboBox {
                                    model: ["允许","锁定"]; currentIndex: graphicsConfig.negativeLODBias; onActivated: function(index) { graphicsConfig.setNegativeLODBias(index) } } }
                            RowLayout { Label { text: "纹理过滤 - 质量" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "画质与帧数平衡（质量：画质+10%；高性能：帧数+20%）。" } 
                                ComboBox { 
                                    model: ["高质量", "质量", "性能", "高性能"]
                                    currentIndex: graphicsConfig.textureFilterQuality
                                    onActivated: function(index) { graphicsConfig.setTextureFilterQuality(index) }
                                } }
                            RowLayout { Label { text: "线程优化" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "优化多核CPU性能。" } ComboBox { 
                                model: ["自动", "开", "关"];
                                currentIndex: graphicsConfig.threadControl;
                                onActivated: function(index) { graphicsConfig.setThreadControl(index) }
                            } }
                        }
                    }
                    
                    GroupBox {
                        Layout.fillWidth: true
                        title: "调整桌面尺寸和位置"
                        ColumnLayout {
                            spacing: 8
                            RowLayout { Label { text: "选择缩放模式" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "屏幕缩放模式。" } 
                            ComboBox { 
                                model: ["默认","纵横比","全屏","无缩放"]
                                currentIndex: graphicsConfig.scalingMode
                                onActivated: function(index) { graphicsConfig.setScalingMode(index) }
                            } }

                            RowLayout { Label { text: "对以下项目执行缩放" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "缩放画面这个事er,是由谁来处理的。GPU来处理缩放后的图像，没有投放延迟，但是影响帧数，推荐这个。显示器来处理缩放后的图像，会有5ms的投放延迟，但是不会降低帧数。" }
                                ComboBox { 
                                model: ["GPU","显示器"]
                                currentIndex: graphicsConfig.scalingSource
                                onActivated: function(index) { graphicsConfig.setScalingSource(index) }
                            } }

                            RowLayout{
                                CheckBox {
                                text: "覆盖由游戏和程序设置的缩放模式"
                                checked: graphicsConfig.scalingOverride
                                onClicked: graphicsConfig.setScalingOverride(checked)
                            }
                                Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "覆盖后进入游戏不会切换分辨率。" }
                            }
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
