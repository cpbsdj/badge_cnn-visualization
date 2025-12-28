cmake_minimum_required(VERSION 3.16)
project(digit_viz)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找系统安装的依赖
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
find_package(ImGui-SFML REQUIRED)
find_package(nlohmann_json 3.9 REQUIRED)

# 可执行文件
add_executable(digit_viz
    src/main.cpp
    src/loader/ModelLoader.cpp
    src/renderer/BackgroundRenderer.cpp
    src/renderer/HotspotRenderer.cpp
    src/renderer/HotspotRenderer.cpp
    src/renderer/LayerDetailRenderer.cpp

    src/renderer/detail/Conv1Detail.cpp
    src/renderer/detail/Conv2Detail.cpp
    src/renderer/detail/Conv3Detail.cpp
    src/renderer/detail/Conv4Detail.cpp

    src/renderer/convanim/animations/Conv1Anim.cpp
    src/renderer/convanim/animations/Conv2Anim.cpp
    src/renderer/convanim/animations/Conv3Anim.cpp
    src/renderer/convanim/animations/Conv4Anim.cpp
    src/renderer/convanim/animations/MultiChannelConvAnim.cpp
    src/renderer/convanim/ConvAnimPanel.cpp
)

target_include_directories(digit_viz PRIVATE
    src
)

target_link_libraries(digit_viz
    sfml-graphics
    sfml-window 
    sfml-system
    ImGui-SFML::ImGui-SFML
)