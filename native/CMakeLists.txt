cmake_minimum_required(VERSION 3.21)
project(NVMeRecoveryStudioAI VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Qml Quick)
qt_standard_project_setup(REQUIRES 6.4)
qt_add_executable(nvme-recovery-studio WIN32 main.cpp)
qt_add_qml_module(nvme-recovery-studio URI RecoveryStudio VERSION 1.0
    QML_FILES Main.qml)
target_link_libraries(nvme-recovery-studio PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick)
set_target_properties(nvme-recovery-studio PROPERTIES
    WIN32_EXECUTABLE TRUE MACOSX_BUNDLE TRUE)
