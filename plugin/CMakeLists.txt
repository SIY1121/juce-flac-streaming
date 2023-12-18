cmake_minimum_required(VERSION 3.15)
project(JUCE_FLAC_STREAMING_DEMO VERSION 0.0.1)

include(FetchContent)

# 依存ライブラリの設定 --------------------------------------------
# JUCE
FetchContent_Declare(
  JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 7.0.9)
FetchContent_MakeAvailable(JUCE)

# libFLAC
set(WITH_OGG OFF CACHE BOOL "with_ogg" FORCE)
set(INSTALL_MANPAGES OFF CACHE BOOL "install_manpages" FORCE)
FetchContent_Declare(
  libFLAC
  GIT_REPOSITORY https://github.com/xiph/flac.git
  GIT_TAG 1.4.3)
FetchContent_MakeAvailable(libFLAC)

# oatpp
FetchContent_Declare(
  oatpp
  GIT_REPOSITORY https://github.com/oatpp/oatpp.git
  GIT_TAG 1.3.0)
FetchContent_MakeAvailable(oatpp)

# oatpp-websocket
set(OATPP_MODULES_LOCATION CUSTOM CACHE STRING "oatpp modules location" FORCE)
set(OATPP_DIR_SRC ${oatpp_SOURCE_DIR}/src CACHE STRING "oatpp source directory" FORCE)
FetchContent_Declare(
  oatppwebsocket
  GIT_REPOSITORY https://github.com/oatpp/oatpp-websocket.git
  GIT_TAG 1.3.0)
FetchContent_MakeAvailable(oatppwebsocket)

#mdns
FetchContent_Declare(
  mdns
  GIT_REPOSITORY https://github.com/mjansson/mdns.git
  GIT_TAG 1.4.3)
FetchContent_MakeAvailable(mdns)

# -------------------------------------------------------------------

# JUCE基本設定
juce_add_plugin(JuceFlacStreamingDemo
    PLUGIN_MANUFACTURER_CODE Sota
    PLUGIN_CODE jfsd
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "JuceFlacStreamingDemo"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "This applicaion requires a permission to use an audio input device.")

juce_generate_juce_header(JuceFlacStreamingDemo)

# src/*.cpp をソースコードに指定
file(GLOB_RECURSE sources CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)
target_sources(JuceFlacStreamingDemo
    PRIVATE ${sources})

# JUCEの機能設定
target_compile_definitions(JuceFlacStreamingDemo
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_USE_FLAC=0
        )

# ヘッダファイルのインクルードパス
target_include_directories(JuceFlacStreamingDemo
    PUBLIC
      ${oatpp_SOURCE_DIR}/include
      ${oatpp_SOURCE_DIR}/include
      ${oatppwebsocket_SOURCE_DIR}/include
      ${mdns_SOURCE_DIR}
  )

# ライブラリのリンク
target_link_libraries(JuceFlacStreamingDemo
    PRIVATE
        juce::juce_audio_utils
        FLAC++
        oatpp
        oatpp-websocket
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

if(WIN32)
    target_link_libraries(JuceFlacStreamingDemo INTERFACE iphlpapi ws2_32)
endif()

# artefact directory
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_BUILD_TYPE "Debug")
endif()
set(Artefact_DIR ${PROJECT_BINARY_DIR}/JuceFlacStreamingDemo_artefacts/${CMAKE_BUILD_TYPE})

# build frontend
add_custom_command(
    TARGET JuceFlacStreamingDemo
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/../client/dist ${Artefact_DIR}/Standalone/static
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/../client/dist ${Artefact_DIR}/VST3/static
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/../client/
    COMMENT "Build Frontend"
)
