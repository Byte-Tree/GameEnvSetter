cmake_minimum_required(VERSION 3.16)

project(GameEnvSetter VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)


qt_add_executable(appGameEnvSetter
    main.cpp
    KeyboardConfigManager.cpp
    KeyboardConfigManager.h
    MouseConfigManager.cpp
    MouseConfigManager.h
    RegistryOperator.cpp
    RegistryOperator.h
    SystemValueConverter.cpp
    SystemValueConverter.h
    ConfigManager.cpp
    ConfigManager.h
    NVIDIAGraphicsConfigManager.cpp
    NVIDIAGraphicsConfigManager.h
    DisplayConfigManager.cpp
    DisplayConfigManager.h

)

target_link_options(appGameEnvSetter PRIVATE "/MANIFESTUAC:level='requireAdministrator'")


qt_add_qml_module(appGameEnvSetter
    URI GameEnvSetter
    VERSION 1.0
    QML_FILES
        Main.qml
        MouseSettings.qml
        KeyboardSettings.qml
        RegistrySettings.qml
        GraphicsSettings.qml
        DisplaySettings.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appGameEnvSetter PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appGameEnvSetter
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appGameEnvSetter
    PRIVATE Qt6::Quick
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/R560-developer/amd64/nvapi64.lib"
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/R560-developer")

include(GNUInstallDirs)
install(TARGETS appGameEnvSetter
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
