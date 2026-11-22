cmake_minimum_required(VERSION 3.21)
project(NVMeRecoveryStudioAI VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick Network)
qt_standard_project_setup(REQUIRES 6.5)
qt_add_executable(nvme-recovery-studio
    WIN32 main.cpp usb_bridge.h recovery_agent.h rescue_imager.h knowledge_base.h controller_catalog.h tool_manager.h platform_io.h linux_nvme.h firmware_manager.h firmware_archive.h stick_writer.h case_model.h cache_tester.h error_translator.h repair_chain.h board_map.h)
qt_add_qml_module(nvme-recovery-studio URI RecoveryStudio VERSION 1.0
    QML_FILES Main.qml
    RESOURCES
        icons/logo.svg
        fonts/Inter-Regular.otf
        fonts/Inter-SemiBold.otf
        fonts/Inter-Bold.otf
        icons/box-amber.svg
        icons/box-blue.svg
        icons/box-green.svg
        icons/box-grey.svg
        icons/box-red.svg
        icons/box-white.svg
        icons/cache-amber.svg
        icons/cache-blue.svg
        icons/cache-green.svg
        icons/cache-grey.svg
        icons/cache-red.svg
        icons/cache-white.svg
        icons/check-amber.svg
        icons/check-blue.svg
        icons/check-green.svg
        icons/check-grey.svg
        icons/check-red.svg
        icons/check-white.svg
        icons/chip-amber.svg
        icons/chip-blue.svg
        icons/chip-green.svg
        icons/chip-grey.svg
        icons/chip-red.svg
        icons/chip-white.svg
        icons/clock-amber.svg
        icons/clock-blue.svg
        icons/clock-green.svg
        icons/clock-grey.svg
        icons/clock-red.svg
        icons/clock-white.svg
        icons/folder-amber.svg
        icons/folder-blue.svg
        icons/folder-green.svg
        icons/folder-grey.svg
        icons/folder-red.svg
        icons/folder-white.svg
        icons/home-amber.svg
        icons/home-blue.svg
        icons/home-green.svg
        icons/home-grey.svg
        icons/home-red.svg
        icons/home-white.svg
        icons/refresh-amber.svg
        icons/refresh-blue.svg
        icons/refresh-green.svg
        icons/refresh-grey.svg
        icons/refresh-red.svg
        icons/refresh-white.svg
        icons/save-amber.svg
        icons/save-blue.svg
        icons/save-green.svg
        icons/save-grey.svg
        icons/save-red.svg
        icons/save-white.svg
        icons/spark-amber.svg
        icons/spark-blue.svg
        icons/spark-green.svg
        icons/spark-grey.svg
        icons/spark-red.svg
        icons/spark-white.svg
        icons/tool-amber.svg
        icons/tool-blue.svg
        icons/tool-green.svg
        icons/tool-grey.svg
        icons/tool-red.svg
        icons/tool-white.svg
        icons/usb-amber.svg
        icons/usb-blue.svg
        icons/usb-green.svg
        icons/usb-grey.svg
        icons/usb-red.svg
        icons/usb-white.svg
        icons/warn-amber.svg
        icons/warn-blue.svg
        icons/warn-green.svg
        icons/warn-grey.svg
        icons/warn-red.svg
        icons/warn-white.svg
        icons/x-amber.svg
        icons/x-blue.svg
        icons/x-green.svg
        icons/x-grey.svg
        icons/x-red.svg
        icons/x-white.svg)
qt_add_resources(nvme-recovery-studio "catalog"
    PREFIX "/catalog"
    BASE ../data/catalog
    FILES
        ../data/catalog/pci_nvme_ids.json
        ../data/catalog/usb_storage_bridges.json
        ../data/catalog/usb_nvme_bridges.json
        ../data/catalog/controller_families.json
        ../data/catalog/firmware_issues.json
        ../data/catalog/firmware_sources.json
        ../data/catalog/tools.json
        ../data/catalog/nvme_codes.json
        ../data/catalog/board_layouts.json
        ../data/catalog/ki-startmodell.json)
target_link_libraries(nvme-recovery-studio PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::Network)
set_target_properties(nvme-recovery-studio PROPERTIES
    WIN32_EXECUTABLE TRUE MACOSX_BUNDLE TRUE)
if(MSVC)
    target_compile_options(nvme-recovery-studio PRIVATE /utf-8)
    # Raw read access to \\.\PhysicalDriveN for the rescue image requires
    # administrator rights; Windows asks once via UAC at program start.
    set_property(TARGET nvme-recovery-studio APPEND_STRING PROPERTY
        LINK_FLAGS " /MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
endif()
if(UNIX AND NOT APPLE)
    # Linux Boot Edition: installed to /opt/nvme-recovery-studio by linux-boot/build.sh
    install(TARGETS nvme-recovery-studio RUNTIME DESTINATION bin)
endif()

if(WIN32)
    target_sources(nvme-recovery-studio PRIVATE app/app.rc)
endif()
