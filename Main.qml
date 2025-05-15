import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.15
import ConfigManagers 1.0
import SystemValueConverter 1.0
import Qt.labs.platform as Platform

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 800
    height: 600
    title: qsTr("系统设置工具")
    property bool firstRun: false
    property bool configSaved: false
    property string gpuVendor: ""

    //给后端类在qml声明,注册在main.cpp里面
    MouseConfigManager { id: mouseConfig }
    KeyboardConfigManager { id: keyboardConfig }
    RegistryOperator { id: registryOperator }
    NVIDIAGraphicsConfigManager { id: nvidiaGraphicsConfig }
    DisplayConfigManager { id: displayConfig }
    ConfigManager { id: configManager }

    Component.onCompleted: {
        firstRun = !configManager.configExists()
        if(firstRun) {
            firstRunDialog.open()
        }
        // 获取显卡厂商并存储
        gpuVendor = SystemValueConverter.getGPUVendor();
        console.log("Main.qml中gpuVendor的值: " + gpuVendor);
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
                        loadConfigToUI(configManager.loadConfig())
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

        DisplaySettings {}

        GraphicsSettings {
            gpuVendor: mainWindow.gpuVendor//带控件id赋值，不然有歧义会导致值传过不过去
        }
        }
    }


    //传递错误信号
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

    Connections {
        target: nvidiaGraphicsConfig
        function onErrorOccurred(msg) { toolTip.show(msg, 2000) }
    }

    Connections {
        target: displayConfig
        function onErrorOccurred(msg) { toolTip.show(msg, 2000) }
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
            try {
                var configData = generateCurrentConfig();
                console.log("首次运行保存配置数据:", JSON.stringify(configData, null, 2));
                configManager.saveConfig(configData);
                configSaved = true;
                toolTip.show("原始配置已保存", 2000);
            } catch(error) {
                toolTip.show("保存失败：" + error, 3000);
            }
        }
        onRejected: {
            if(!configManager.userConfigExists()) {
                toolTip.show("原始配置未保存，还原默认设置功能将不可用", 3000)
            }
        }
    }

    //自定义动感弹窗
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
          try {
                    var userConfig = generateCurrentConfig();
                    //console.log("用户保存配置数据:", JSON.stringify(userConfig, null, 2));
                    configManager.saveUserConfig(userConfig, saveFileDialog.currentFile);
                    toolTip.show("用户配置已保存", 2000);
            }
          catch(error){
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
          
          // 加载显卡配置
          if(config.graphics) {
              switch(gpuVendor) {
                  case "NVIDIA":
                      nvidiaGraphicsConfig.setLowLatencyMode(config.graphics.lowLatencyMode)
                      nvidiaGraphicsConfig.setTripleBuffer(config.graphics.tripleBuffer)
                      nvidiaGraphicsConfig.setAnisotropicFiltering(config.graphics.anisotropicFiltering)
                      nvidiaGraphicsConfig.setMaxFPSLimit(config.graphics.maxFPSLimit)
                      nvidiaGraphicsConfig.setVSyncMode(config.graphics.vSyncMode)
                      nvidiaGraphicsConfig.setPowerManagementMode(config.graphics.powerManagementMode)
                      nvidiaGraphicsConfig.setTextureFilterQuality(config.graphics.textureFilterQuality)
                      nvidiaGraphicsConfig.setThreadControl(config.graphics.threadControl)
                      nvidiaGraphicsConfig.setImageSharpening(config.graphics.imageSharpening)
                      nvidiaGraphicsConfig.setOpenGLGDICompatibility(config.graphics.openGLGDICompatibility)
                      nvidiaGraphicsConfig.setOpenGLPresentMethod(config.graphics.openGLPresentMethod)
                      nvidiaGraphicsConfig.setFXAAEnable(config.graphics.fxaaEnable)
                      nvidiaGraphicsConfig.setAAModeSelector(config.graphics.aaModeSelector)
                      nvidiaGraphicsConfig.setAAGammaCorrection(config.graphics.aaGammaCorrection)
                      nvidiaGraphicsConfig.setAAModeMethod(config.graphics.aaModeMethod)
                      nvidiaGraphicsConfig.setAATransparency(config.graphics.aaTransparency)
                      nvidiaGraphicsConfig.setAoMode(config.graphics.aoMode)
                      nvidiaGraphicsConfig.setAppIdleFPSLimit(config.graphics.appIdleFPSLimit)
                      nvidiaGraphicsConfig.setShaderCacheSize(config.graphics.shaderCacheSize)
                      nvidiaGraphicsConfig.setNegativeLODBias(config.graphics.negativeLODBias)
                      break
                  case "AMD":
                      // AMD显卡设置逻辑
                      break
                  case "Intel":
                      // Intel显卡设置逻辑
                      break
              }
          }
          
          toolTip.show("三方配置已应用", 2000)
      }
    }


    function loadConfigToUI(config) {
        // 鼠标设置
        mouseConfig.setDoubleClickSpeed(config.mouse.doubleClickSpeed)
        mouseConfig.setMouseSpeed(config.mouse.mouseSpeed)
        mouseConfig.setEnhancePointerPrecision(config.mouse.enhancePointerPrecision)

        // 键盘设置
        keyboardConfig.setRepeatDelay(config.keyboard.repeatDelay)
        keyboardConfig.setRepeatRate(config.keyboard.repeatRate)

        // 注册表设置
        registryOperator.setWin32PrioritySeparation(config.registry.win32PrioritySeparation)

        // 根据显卡类型加载图形设置（使用switch结构）
        switch(gpuVendor) {
            case "NVIDIA":
                nvidiaGraphicsConfig.imageSharpening = config.graphics.imageSharpening
                nvidiaGraphicsConfig.openGLGDICompatibility = config.graphics.openGLGDICompatibility
                nvidiaGraphicsConfig.openGLPresentMethod = config.graphics.openGLPresentMethod
                nvidiaGraphicsConfig.tripleBuffer = config.graphics.tripleBuffer
                nvidiaGraphicsConfig.lowLatencyMode = config.graphics.lowLatencyMode
                nvidiaGraphicsConfig.anisotropicFiltering = config.graphics.anisotropicFiltering
                nvidiaGraphicsConfig.appIdleFPSLimit = config.graphics.appIdleFPSLimit
                nvidiaGraphicsConfig.vSyncMode = config.graphics.vSyncMode
                nvidiaGraphicsConfig.fxaaEnable = config.graphics.fxaaEnable
                nvidiaGraphicsConfig.aaModeSelector = config.graphics.aaModeSelector
                nvidiaGraphicsConfig.aaGammaCorrection = config.graphics.aaGammaCorrection
                nvidiaGraphicsConfig.aaModeMethod = config.graphics.aaModeMethod
                nvidiaGraphicsConfig.aaTransparency = config.graphics.aaTransparency
                nvidiaGraphicsConfig.maxFPSLimit = config.graphics.maxFPSLimit
                nvidiaGraphicsConfig.aoMode = config.graphics.aoMode
                nvidiaGraphicsConfig.powerManagementMode = config.graphics.powerManagementMode
                nvidiaGraphicsConfig.shaderCacheSize = config.graphics.shaderCacheSize
                nvidiaGraphicsConfig.trilinearOptimization = config.graphics.trilinearOptimization
                nvidiaGraphicsConfig.anisotropicSampleOptimization = config.graphics.anisotropicSampleOptimization
                nvidiaGraphicsConfig.negativeLODBias = config.graphics.negativeLODBias
                nvidiaGraphicsConfig.textureFilterQuality = config.graphics.textureFilterQuality
                nvidiaGraphicsConfig.threadControl = config.graphics.threadControl
                break;
            case "AMD":
                // 可扩展AMD显卡加载逻辑
                break;
            case "Intel":
                // 可扩展Intel显卡加载逻辑
                break;
            default:
                break;
        }
    }

    function generateCurrentConfig() {
        return {
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
            },
            "graphics": (function() {
                var graphicsObj = {
                    gpuVendor: gpuVendor
                };
                switch(gpuVendor) {
                    case "NVIDIA":
                        return Object.assign(graphicsObj, {
                            imageSharpening: nvidiaGraphicsConfig.imageSharpening,
                            openGLGDICompatibility: nvidiaGraphicsConfig.openGLGDICompatibility,
                            openGLPresentMethod: nvidiaGraphicsConfig.openGLPresentMethod,
                            tripleBuffer: nvidiaGraphicsConfig.tripleBuffer,
                            lowLatencyMode: nvidiaGraphicsConfig.lowLatencyMode,
                            anisotropicFiltering: nvidiaGraphicsConfig.anisotropicFiltering,
                            appIdleFPSLimit: nvidiaGraphicsConfig.appIdleFPSLimit,
                            vSyncMode: nvidiaGraphicsConfig.vSyncMode,
                            fxaaEnable: nvidiaGraphicsConfig.fxaaEnable,
                            aaModeSelector: nvidiaGraphicsConfig.aaModeSelector,
                            aaGammaCorrection: nvidiaGraphicsConfig.aaGammaCorrection,
                            aaModeMethod: nvidiaGraphicsConfig.aaModeMethod,
                            aaTransparency: nvidiaGraphicsConfig.aaTransparency,
                            maxFPSLimit: nvidiaGraphicsConfig.maxFPSLimit,
                            aoMode: nvidiaGraphicsConfig.aoMode,
                            powerManagementMode: nvidiaGraphicsConfig.powerManagementMode,
                            shaderCacheSize: nvidiaGraphicsConfig.shaderCacheSize,
                            trilinearOptimization: nvidiaGraphicsConfig.trilinearOptimization,
                            anisotropicSampleOptimization: nvidiaGraphicsConfig.anisotropicSampleOptimization,
                            negativeLODBias: nvidiaGraphicsConfig.negativeLODBias,
                            textureFilterQuality: nvidiaGraphicsConfig.textureFilterQuality,
                            threadControl: nvidiaGraphicsConfig.threadControl
                        });
                    case "AMD":
                        // 可扩展AMD显卡配置项
                        return {};
                    case "Intel":
                        // 可扩展Intel显卡配置项
                        return {};
                    default:
                        return {};
                }
            })()
        };
    }

}

