cmake_minimum_required(VERSION 3.25.0)

set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "macOS deployment target")
set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for macOS")

# Keep SDK build lean; leave VSTGUI and the validator enabled.
set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES OFF CACHE BOOL "Enable VST 3 Plug-in Examples")
set(SMTG_ENABLE_VST3_HOSTING_EXAMPLES OFF CACHE BOOL "Enable VST 3 Hosting Examples")
set(SMTG_ENABLE_VSTGUI_SUPPORT ON CACHE BOOL "Enable VSTGUI Support")
set(SMTG_BUILD_UNIVERSAL_BINARY OFF CACHE BOOL "Build universal binary (x86_64 & arm64)")

project(EventHorizon
    VERSION 0.1.0.0
    DESCRIPTION "Event Horizon wavetable synthesizer"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(APPLE)
    enable_language(OBJCXX)
endif()

set(vst3sdk_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/vst3sdk")
if(NOT EXISTS "${vst3sdk_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "VST3 SDK not found at ${vst3sdk_SOURCE_DIR}. "
        "Run: git submodule update --init --recursive")
endif()

# The VST3 SDK's platform detection calls `xcodebuild -version`, which requires
# a full Xcode.app. Allow Apple Silicon / Command Line Tools builds by seeding
# the version when full Xcode is not selected.
if(APPLE AND NOT DEFINED ENV{XCODE_VERSION})
    execute_process(
        COMMAND xcodebuild -version
        OUTPUT_VARIABLE _xcodebuild_version_out
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT _xcodebuild_version_out)
        message(STATUS
            "[EventHorizon] Full Xcode not detected; using Command Line Tools "
            "with XCODE_VERSION=16.0 for the VST3 SDK")
        set(ENV{XCODE_VERSION} "16.0")
        set(XCODE_VERSION "16.0")
    endif()
endif()

add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(EventHorizon
    source/version.h
    source/eventhorizon_cids.h
    source/eventhorizon_messaging.h
    source/dsp/sine_engine.h
    source/eventhorizon_processor.h
    source/eventhorizon_processor.cpp
    source/eventhorizon_controller.h
    source/eventhorizon_controller.cpp
    source/eventhorizon_entry.cpp
    source/ui/eventhorizon_keyboard_ui.h
    source/ui/eventhorizon_keyboard_ui.cpp
    ${vst3sdk_SOURCE_DIR}/vstgui4/vstgui/contrib/keyboardview.cpp
    ${vst3sdk_SOURCE_DIR}/vstgui4/vstgui/contrib/keyboardview.h
)

target_compile_features(EventHorizon
    PUBLIC
        cxx_std_20
)

target_link_libraries(EventHorizon
    PRIVATE
        sdk
        vstgui_support
)

target_include_directories(EventHorizon
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

smtg_target_add_plugin_resources(EventHorizon
    RESOURCES
        resource/eventhorizon.uidesc
)

smtg_target_configure_version_file(EventHorizon)

if(SMTG_MAC)
    smtg_target_set_bundle(EventHorizon
        BUNDLE_IDENTIFIER com.eliallen.eventhorizon
        COMPANY_NAME "Eli Allen"
    )
endif()
