#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

#include "loader/ModelLoader.hpp"
#include "renderer/BackgroundRenderer.hpp"
#include "renderer/HotspotRenderer.hpp"
#include "renderer/LayerDetailRenderer.hpp"

#include <iostream>
#include <filesystem>

// 在文件顶部添加字体设置函数
bool setupChineseFont() {
    ImGuiIO& io = ImGui::GetIO();
    
    // 清除默认字体
    io.Fonts->Clear();
    
    // 尝试加载中文字体
    std::vector<std::string> fontPaths = {
        "assets/fonts/wqy-microhei.ttc",           // 文泉驿微米黑
        "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc", // 系统文泉驿
        "fonts/wqy-microhei.ttc"                   // 备用路径
    };
    
    ImFont* font = nullptr;
    for (const auto& fontPath : fontPaths) {
        if (std::filesystem::exists(fontPath)) {
            std::cout << "尝试加载字体: " << fontPath << std::endl;
            font = io.Fonts->AddFontFromFileTTF(
                fontPath.c_str(), 
                16.0f, 
                nullptr, 
                io.Fonts->GetGlyphRangesChineseFull()
            );
            if (font) {
                std::cout << "成功加载中文字体: " << fontPath << std::endl;
                break;
            } else {
                std::cout << "字体加载失败: " << fontPath << std::endl;
            }
        }
    }
    
    // 如果都没找到，使用默认字体
    if (!font) {
        std::cout << "使用默认字体，中文可能显示为方框" << std::endl;
        font = io.Fonts->AddFontDefault();
    }
    
    // 设置为主字体
    io.FontDefault = font;
    
    // 重新构建字体纹理
    if (!ImGui::SFML::UpdateFontTexture()) {
        std::cerr << "字体纹理更新失败" << std::endl;
        return false;
    }
    
    return true;
}

int main() {
    // 创建窗口
    sf::RenderWindow window(sf::VideoMode(1400, 900), "校徽分类器可视化工具");
    window.setFramerateLimit(60);
    
    // 初始化ImGui
    if (!ImGui::SFML::Init(window)) {
        std::cerr << "Failed to initialize ImGui-SFML" << std::endl;
        return -1;
    }




    // 设置中文字体
    if (!setupChineseFont()) {
        std::cerr << "中文字体设置失败，继续使用默认字体" << std::endl;
    } else {
        std::cout << "中文字体设置成功" << std::endl;
    }

    // 检查字体文件是否存在
    std::cout << "检查字体文件..." << std::endl;
    std::vector<std::string> checkPaths = {
        "assets/fonts/wqy-microhei.ttc",
    };
    
    for (const auto& path : checkPaths) {
        if (std::filesystem::exists(path)) {
            std::cout << "字体文件存在: " << path << std::endl;
        } else {
            std::cout << "字体文件不存在: " << path << std::endl;
        }
    }




    // 检查资源文件是否存在
    std::filesystem::path assetsDir = "assets";
    if (!std::filesystem::exists(assetsDir)) {
        std::cerr << "Assets directory not found!" << std::endl;
        std::cerr << "Current path: " << std::filesystem::current_path() << std::endl;
        return -1;
    }

    // 加载模型
    ModelLoader modelLoader;
    std::string modelJsonPath = "assets/model/model.json";
    std::string weightsBinPath = "assets/model/weights.bin";
    
    if (!modelLoader.load(modelJsonPath, weightsBinPath)) {
        std::cerr << "Failed to load model!" << std::endl;
        return -1;
    }
    std::cout << "Model loaded successfully!" << std::endl;

    // 加载背景
    BackgroundRenderer backgroundRenderer;
    std::string backgroundPath = "assets/textures/total_network.jpg";
    
    if (!backgroundRenderer.load(backgroundPath)) {
        std::cerr << "Failed to load background! Using fallback..." << std::endl;
    }

    // 初始化热点渲染器
    HotspotRenderer hotspotRenderer;
    hotspotRenderer.setWindow(&window);
    hotspotRenderer.build(modelLoader, backgroundRenderer.getSprite());

    // 更新布局
    backgroundRenderer.updateLayout(window.getSize());

    // 初始化图层详细渲染器
    LayerDetailRenderer layerDetailRenderer;
    
    // 加载详细结构纹理
    layerDetailRenderer.loadTexture("conv1", "assets/textures/conv1.jpg");
    layerDetailRenderer.loadTexture("conv2", "assets/textures/conv2.jpg"); 
    layerDetailRenderer.loadTexture("conv3", "assets/textures/conv3.jpg");
    layerDetailRenderer.loadTexture("conv4", "assets/textures/conv4.jpg");
    
    // 设置热点渲染器
    hotspotRenderer.setLayerDetailRenderer(&layerDetailRenderer);




    // 主循环
    sf::Clock deltaClock;
    bool showDemoWindow = false;
    bool showConvAnim = false;

    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::Resized) {
                // 创建新的视图
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                sf::View newView(visibleArea);
    
                // 设置窗口视图
                window.setView(newView);
    
                // 更新背景布局和视图
                backgroundRenderer.updateLayout(window.getSize());
                backgroundRenderer.setView(newView);
    
                // 重新构建热点
                hotspotRenderer.build(modelLoader, backgroundRenderer.getSprite());
            }
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
                else if (event.key.code == sf::Keyboard::D) {
                    showDemoWindow = !showDemoWindow;
                }
            }
        }

        // 更新ImGui
        ImGui::SFML::Update(window, deltaClock.restart());

        // 获取鼠标位置
        auto mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
        
        // 详细窗口的鼠标交互
        layerDetailRenderer.handleMouse(worldPos);

        // 处理热点交互
        hotspotRenderer.handleMouse(window);

        // 处理按钮点击
        layerDetailRenderer.handleButtons();

        // 绘制
        window.clear(sf::Color(30, 30, 30));

        // 绘制背景
        backgroundRenderer.draw(window);

        // 绘制热点区域（调试用）
        //hotspotRenderer.draw(window);

        // ImGUI 界面
        ImGui::Begin("校徽分类器控制面板");
        
        ImGui::Text("模型信息: %s", modelLoader.get_description().c_str());
        ImGui::Text("输入尺寸: %dx%d", 
                   modelLoader.get_input_size().x, modelLoader.get_input_size().y);
        ImGui::Text("输出类别: %d", modelLoader.get_num_classes());
        ImGui::Text("网络总层数: %zu", modelLoader.get_num_layers());
        ImGui::Separator();
        
        ImGui::Separator();
        ImGui::Text("应用信息");
        ImGui::Text("帧率: %.1f FPS", ImGui::GetIO().Framerate);
        ImGui::Text("窗口大小: %dx%d", window.getSize().x, window.getSize().y);
        
        ImGui::End();

        hotspotRenderer.handleMouseAndDrawUI();

        // 绘制详细结构窗口
        layerDetailRenderer.draw();

        // 渲染ImGui
        ImGui::SFML::Render(window);

        // 显示窗口
        window.display();
    }

    // 关闭ImGui
    ImGui::SFML::Shutdown();

    return 0;
}