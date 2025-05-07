import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.15

GroupBox {
    id: graphicsSettingsRoot
    title: "显卡设置"
    Layout.fillWidth: true
    implicitHeight: 1150
    property string gpuVendor

    Component.onCompleted: {
        console.log("GraphicsSettings.qml中gpuVendor的值: " + gpuVendor);
    }
    ColumnLayout {
        width: parent.width
        spacing: 10

        TabBar {
            id: gpuTabBar
            Layout.fillWidth: true
            currentIndex: swipeView.currentIndex

            // 未知厂商时禁用所有标签
            onCurrentIndexChanged: { if(currentIndex === -1) { nvidiaTab.enabled = false; amdTab.enabled = false; intelTab.enabled = false; } }

            TabButton { 
                id: nvidiaTab
                text: "NVIDIA" 
                enabled: gpuVendor === "NVIDIA"
            }
            TabButton { 
                id: amdTab
                text: "AMD" 
                enabled: gpuVendor === "AMD"
            }
            TabButton { 
                id: intelTab
                text: "Intel" 
                enabled: gpuVendor === "Intel"
            }
        }



        SwipeView {
            id: swipeView
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: gpuVendor === "NVIDIA" ? 0 : (gpuVendor === "AMD" ? 1 : (gpuVendor === "Intel" ? 2 : 3))

            Loader {
                id: nvidiaSettingsLoader
                width: parent.width
                active: gpuVendor === "NVIDIA"
                sourceComponent: nvidiaSettingsComponent
                
                Component {
                    id: nvidiaSettingsComponent
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
                                    currentIndex: nvidiaGraphicsConfig.imageSharpening; onActivated: function(index) { nvidiaGraphicsConfig.setImageSharpening(index) } } }
                            RowLayout { Label { text: "CUDA - GPUs" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "多显卡默认配置。" } ComboBox { enabled: false; } }
                            RowLayout { Label { text: "CUDA - 系统内存回退政策" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "显存不足时使用内存补充。" } ComboBox { enabled: false; model: ["驱动默认值", "偏好无系统内存回退", "偏好系统内存回退"]; } }
                            RowLayout { Label { text: "DSR - 因数" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "伪分辨率技术（适合看风景）。" } ComboBox { enabled: false; model: [] } }
                            RowLayout { Label { text: "OpenGL GDI兼容性" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "对老旧游戏的优化。" }
                                ComboBox { model: ["未知","优先性能","优先兼容性","自动"];
                                    currentIndex: nvidiaGraphicsConfig.openGLGDICompatibility; onActivated: function(index) { nvidiaGraphicsConfig.setOpenGLGDICompatibility(index) } } }
                            RowLayout { Label { text: "OpenGL 渲染GPU" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "多显卡模式下选择需使用的GPU。" } ComboBox { enabled: false; } }
                            RowLayout { Label { text: "Vulkan/OpenGL 现行方法" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "针对第9代及以后CPU使用DX12的优化。" }
                                ComboBox { model: ["未知","优先本机","优先在DIGX交换链上分层","自动"];
                                    currentIndex: nvidiaGraphicsConfig.openGLPresentMethod; onActivated: function(index) { nvidiaGraphicsConfig.setOpenGLPresentMethod(index) } } }
                            RowLayout { Label { text: "三重缓冲" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "减少画面撕裂（适合看风景，但增加输入延迟）。" }
                                ComboBox { model: ["未知","关","开"];
                                    currentIndex: nvidiaGraphicsConfig.tripleBuffer; onActivated: function(index) { nvidiaGraphicsConfig.setTripleBuffer(index) } } }
                            RowLayout { Label { text: "低延时模式" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "提高画面反馈速度（中低端显卡可能帧数波动）。" }
                                ComboBox { model: ["未知","关","开(超高)"]
                                    currentIndex: nvidiaGraphicsConfig.lowLatencyMode
                                    onActivated: function(index) { nvidiaGraphicsConfig.setLowLatencyMode(index) }
                                } }
                            RowLayout { Label { text: "各向异性过滤" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "提升远处物体清晰度（显卡性能越高可设置越高）。" } 
                                ComboBox { 
                                    model: Array.from({length: 16}, (_, i) => i+1);
                                    currentIndex: nvidiaGraphicsConfig.anisotropicFiltering;
                                    onActivated: function(index) { nvidiaGraphicsConfig.setAnisotropicFiltering(index) }
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
                                    value: nvidiaGraphicsConfig.appIdleFPSLimit
                                    onMoved: nvidiaGraphicsConfig.setAppIdleFPSLimit(value)
                                    Layout.fillWidth: true
                                }
                                Text {
                                    text: "当前值：" + idleFpsSlider.value
                                    Layout.alignment: Qt.AlignRight
                                }
                            }
                            RowLayout { Label { text: "垂直同步" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "同步GPU渲染速率与显示器刷新率（适合看风景）。" }
                                ComboBox { model: ["未知","使用3D应用程序设置","关", "开","VSYNCMODE_FLIPINTERVAL2","VSYNCMODE_FLIPINTERVAL3","VSYNCMODE_FLIPINTERVAL4","VSYNCMODE_VIRTUAL"];
                                    currentIndex: nvidiaGraphicsConfig.vSyncMode; onActivated: function(index) { nvidiaGraphicsConfig.setVSyncMode(index) } } }
                            // RowLayout { Label { text: "多重采样" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "提升画面细腻度（3060以下显卡建议关闭）。" }
                            //     ComboBox {
                            //         model: ["关", "2x", "4x", "8x", "16x"]
                            //         currentIndex: nvidiaGraphicsConfig.aaModeMethod
                            //         onActivated: function(index) {nvidiaGraphicsConfig.setAAModeMethod(index)}
                            //     }
                            // }
                            RowLayout { Label { text: "平滑处理 - FXAA" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "消除物体边缘锯齿。" } 
                                ComboBox { 
                                    model: ["未知","关","开"]
                                    currentIndex: nvidiaGraphicsConfig.fxaaEnable
                                    onActivated: function(index) {nvidiaGraphicsConfig.setFXAAEnable(index)}
                                } 
                            }
                            RowLayout { Label { text: "平滑处理 - 模式" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "抗锯齿模式选择。" }
                                ComboBox {
                                    model: ["应用程序控制的","关","提高应用程序设置","置换任何应用程序设置"];
                                    currentIndex: nvidiaGraphicsConfig.aaModeSelector;
                                    onActivated: function(index) { nvidiaGraphicsConfig.setAAModeSelector(index) }
                                } }
                            RowLayout { Label { text: "平滑处理 - 灰度纠正" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "优化画面对比度。" } 
                                ComboBox { 
                                    model: ["关", "条件开启", "始终开启"]
                                    currentIndex: nvidiaGraphicsConfig.aaGammaCorrection
                                    onActivated: function(index) {nvidiaGraphicsConfig.setAAGammaCorrection(index)}
                                } 
                            }
                            RowLayout { Label { text: "平滑处理 - 设置（多重采样）" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "托管对象的抗锯齿处理。提升画面细腻度（3060以下显卡建议关闭）。" }
                                ComboBox { 
                                    model: ["应用程序控制的(关)", "2x", "4x","8x","16x"]
                                    currentIndex: nvidiaGraphicsConfig.aaModeMethod
                                    onActivated: function(index) {nvidiaGraphicsConfig.setAAModeMethod(index)}
                                } }
                            RowLayout { Label { text: "平滑处理 - 透明度" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "提升细节物体清晰度（显卡性能越高可设置越高，影响帧数）。" } ComboBox { model: ["关", "多重采样", "超级采样(2x)", "超级采样(4x)","超级采样(8x)"]
                                currentIndex: nvidiaGraphicsConfig.aaTransparency
                                onActivated: function(index) {nvidiaGraphicsConfig.setAATransparency(index)}
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
                                    value: nvidiaGraphicsConfig.maxFPSLimit
                                    onMoved: nvidiaGraphicsConfig.setMaxFPSLimit(value)
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
                                    currentIndex: nvidiaGraphicsConfig.aoMode
                                    onActivated: nvidiaGraphicsConfig.setAoMode(index)
                                } }
                            RowLayout { Label { text: "电源管理模式" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "控制显卡功耗策略（游戏推荐最大性能）。" } 
                                ComboBox { model: ["未知","自适应","最大性能","驱动控制","一致性能","最小功耗","最优电源"];
                                    currentIndex: nvidiaGraphicsConfig.powerManagementMode; onActivated: function(index) { nvidiaGraphicsConfig.setPowerManagementMode(index) } }}
                            RowLayout { 
                                Label { text: "着色器缓存大小" }
                                Button { 
                                    text: "?"
                                    ToolTip.visible: hovered
                                    ToolTip.text: "显存足够时可设为无限制(即：4,294,967,295)。"
                                }
                                Text {
                                    text: "当前值：" + nvidiaGraphicsConfig.shaderCacheSize + "MB"
                                    Layout.alignment: Qt.AlignRight
                                }
                                TextField {
                                    placeholderText: "例如：4096 单位：MB"
                                    Layout.fillWidth: true
                                }
                                Button {
                                    text: "设置"
                                    onClicked: nvidiaGraphicsConfig.setShaderCacheSize(parent.children[3].text)
                                }
                            }
                            RowLayout { Label { text: "纹理过滤 - 三线性优化" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "动态降低远处清晰度。" }
                                ComboBox { model: ["开","关"]
                                    currentIndex: nvidiaGraphicsConfig.trilinearOptimization
                                    onActivated: function(index) { nvidiaGraphicsConfig.setTrilinearOptimization(index) } } }
                            RowLayout { Label { text: "纹理过滤 - 各向异性采样优化" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "动态降低被遮挡物清晰度（可能停止绘制进程）。" } 
                                ComboBox { model: ["关","开"]
                                    currentIndex: nvidiaGraphicsConfig.anisotropicSampleOptimization
                                    onActivated: function(index) { nvidiaGraphicsConfig.setAnisotropicSampleOptimization(index) }
                                } }
                            RowLayout { Label { text: "纹理过滤 - 负LOD偏移" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "是否动态调节细节物体清晰度。" } ComboBox {
                                    model: ["允许","锁定"]; currentIndex: nvidiaGraphicsConfig.negativeLODBias; onActivated: function(index) { nvidiaGraphicsConfig.setNegativeLODBias(index) } } }
                            RowLayout { Label { text: "纹理过滤 - 质量" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "画质与帧数平衡（质量：画质+10%；高性能：帧数+20%）。" } 
                                ComboBox { 
                                    model: ["高质量", "质量", "性能", "高性能"]
                                    currentIndex: nvidiaGraphicsConfig.textureFilterQuality
                                    onActivated: function(index) { nvidiaGraphicsConfig.setTextureFilterQuality(index) }
                                } }
                            RowLayout { Label { text: "线程优化" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "优化多核CPU性能。" } ComboBox { 
                                model: ["自动", "开", "关"];
                                currentIndex: nvidiaGraphicsConfig.threadControl;
                                onActivated: function(index) { nvidiaGraphicsConfig.setThreadControl(index) }
                            } }
                            Button {
                                text: "推荐设置"
                                ToolTip.visible: hovered
                                ToolTip.text: "推荐设置：低延时模式开(超高)，三重缓冲关，各向异性过滤16x，最大帧速率240，垂直同步关，电源管理模式最大性能，纹理过滤质量高性能，线程优化开"
                                ToolTip.delay: 200
                                onClicked: {
                                    nvidiaGraphicsConfig.setLowLatencyMode(2);
                                    nvidiaGraphicsConfig.setTripleBuffer(1);
                                    nvidiaGraphicsConfig.setAnisotropicFiltering(15);
                                    nvidiaGraphicsConfig.setMaxFPSLimit(240);
                                    nvidiaGraphicsConfig.setVSyncMode(2);
                                    nvidiaGraphicsConfig.setPowerManagementMode(2);
                                    nvidiaGraphicsConfig.setTextureFilterQuality(3);
                                    nvidiaGraphicsConfig.setThreadControl(1);
                                }
                            }
                        }
                    }
                    
                    GroupBox {
                        Layout.fillWidth: true
                        title: "调整桌面尺寸和位置"
                        ColumnLayout {
                            spacing: 8
                            RowLayout { Label { text: "选择缩放模式" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "屏幕缩放模式。" } Button { text: "?"; ToolTip.visible: hovered; ToolTip.text: "\"GPU\"和\"显示器\"前缀的意思是：缩放画面这个事er,是由谁来处理的。GPU来处理缩放后的图像，没有投放延迟，但是影响帧数，推荐GPU来处理。显示器来处理缩放后的图像，会有5ms的投放延迟，但是不会降低帧数。" } 
                                ComboBox {
                                    id: displayIdCombo
                                    model: [] // 初始模型为空
                                    onActivated: updateScalingModeCombo // 选择项变化时触发更新缩放模式
                                    property var scalingData: JSON.parse(nvidiaGraphicsConfig.scalingMode)
                                    
                                    // 组件完成时初始化显示器ID列表
                                    Component.onCompleted: {
                                        var displayIds = [];
                                        
                                        // 遍历所有设备的目标显示器ID
                                        for (var i = 0; i < scalingData.devices.length; i++) {
                                            for (var j = 0; j < scalingData.devices[i].targets.length; j++) {
                                                displayIds.push(scalingData.devices[i].targets[j].displayId);
                                            }
                                        }
                                        
                                        // 设置下拉框模型为收集到的显示器ID
                                        model = displayIds;
                                        
                                        // 如果有显示器ID，默认选择第一个并更新缩放模式
                                        if (displayIds.length > 0) {
                                            currentIndex = 0;
                                            updateScalingModeCombo(0);
                                        }
                                    }
                                    
                                    // 根据选择的显示器索引更新缩放模式下拉框
                                    function updateScalingModeCombo(displayIndex) {
                                        var scalingValue = 0;

                                        // 查找对应显示器的缩放值
                                        for (var i = 0; i < scalingData.devices.length; i++) {
                                            for (var j = 0; j < scalingData.devices[i].targets.length; j++) {
                                                if (j === displayIndex) {
                                                    scalingValue = scalingData.devices[i].targets[j].scaling;
                                                    break;
                                                }
                                            }
                                        }

                                        // 设置缩放模式下拉框的当前索引
                                        scalingModeCombo.currentIndex = scalingModeCombo.mapScalingValue(scalingValue);
                                    }
                                }
                                
                                // 缩放模式选择下拉框
                                ComboBox {
                                    id: scalingModeCombo
                                    model: ["默认值/跟随应用程序:0","GPU->无缩放:3", "GPU->纵横比:5", "GPU->全屏:2","GPU->整数缩放:8","显示器->无缩放:7", "显示器->纵横比:6", "显示器->全屏:1"] // 6种缩放模式选项,实际上有7种
                                    
                                    // 将缩放值映射到下拉框索引
                                    function mapScalingValue(scaling) {
                                        switch(scaling) {
                                            case 0: return 0; // 跟随应用程序
                                            case 3: return 1; // GPU->无缩放
                                            case 5: return 2; // GPU->纵横比
                                            case 2: return 3; // GPU->全屏
                                            case 8: return 4; // GPU->整数缩放
                                            case 7: return 5; // 显示器->无缩放
                                            case 6: return 6; // 显示器->纵横比
                                            case 1: return 7; // 显示器->全屏
                                            default: return 0;
                                        }
                                    }
                                    
                                    // 当选择缩放模式时更新配置
                                    onActivated: function(index) {
                                        var scalingValue = 0;
                                        switch(index) {
                                            case 0: scalingValue = 0; break;
                                            case 1: scalingValue = 3; break;
                                            case 2: scalingValue = 5; break;
                                            case 3: scalingValue = 2; break;
                                            case 4: scalingValue = 8; break;
                                            case 5: scalingValue = 7; break;
                                            case 6: scalingValue = 6; break;
                                            case 7: scalingValue = 1; break;
                                        }
                                        var params = {};
                                        params["displayId"] = displayIdCombo.model[displayIdCombo.currentIndex];
                                        params["mode"] = scalingValue;
                                        nvidiaGraphicsConfig.setScalingMode(Qt.btoa(JSON.stringify(params)));
                                    }
                                }

                            }


                            
                        }

                    }
                    
                }
            }
            }

            Loader {
                id: amdSettingsLoader
                width: parent.width
                active: gpuVendor === "AMD"
                sourceComponent: amdSettingsComponent
                
                Component {
                    id: amdSettingsComponent
                    Item {
                        Label {
                            anchors.centerIn: parent
                            text: "AMD设置等待更新"
                        }
                    }
                }
            }

            Loader {
                id: intelSettingsLoader
                width: parent.width
                active: gpuVendor === "Intel"
                sourceComponent: intelSettingsComponent
                
                Component {
                    id: intelSettingsComponent
                    Item {
                        Label {
                            anchors.centerIn: parent
                            text: "Intel设置等待更新"
                        }
                    }
                }
            }

            Item {
                ColumnLayout {
                    width: parent.width
                    anchors.centerIn: parent
                    Label {
                        text: "未检测到已知的显卡厂商"
                        color: "gray"
                        font.pixelSize: 16
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }
        }
    }
}


