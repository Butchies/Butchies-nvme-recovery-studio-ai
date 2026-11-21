cmake_minimum_required(VERSION 3.21)
project(NVMeRecoveryStudioAI VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Qml Quick)
qt_standard_project_setup(REQUIRES 6.4)
qt_add_executable(nvme-recovery-studio WIN32 main.cpp usb_bridge.h recovery_agent.h rescue_imager.h)
qt_add_qml_module(nvme-recovery-studio URI RecoveryStudio VERSION 1.0
    QML_FILES Main.qml)
target_link_libraries(nvme-recovery-studio PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick)
set_target_properties(nvme-recovery-studio PROPERTIES
    WIN32_EXECUTABLE TRUE MACOSX_BUNDLE TRUE)
if(MSVC)
    target_compile_options(nvme-recovery-studio PRIVATE /utf-8)
    # Raw read access to \\.\PhysicalDriveN for the rescue image requires
    # administrator rights; Windows asks once via UAC at program start.
    set_property(TARGET nvme-recovery-studio APPEND_STRING PROPERTY
        LINK_FLAGS " /MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
endif()
