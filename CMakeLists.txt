cmake_minimum_required(VERSION 3.16)
project(digit_viz)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动下载依赖
include(FetchContent)

# ImGui-SFML
FetchContent_Declare(
    imgui-sfml
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG master
)
FetchContent_MakeAvailable(imgui-sfml)

# nlohmann_json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

# 查找系统SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

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

target_include_directories(digit_viz PRIVATE src)

target_link_libraries(digit_viz
    sfml-graphics
    sfml-window 
    sfml-system
    ImGui-SFML
)