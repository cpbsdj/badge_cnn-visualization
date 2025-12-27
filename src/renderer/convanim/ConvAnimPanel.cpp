#include "renderer/convanim/ConvAnimPanel.hpp"
#include <iostream>

std::unique_ptr<ConvAnimBase> ConvAnimPanel::createAnimator(int layer) {
        switch (layer) {
        case 1: return std::make_unique<Conv1Anim>();
        case 2: return std::make_unique<Conv2Anim>();
        case 3: return std::make_unique<Conv3Anim>();
        case 4: return std::make_unique<Conv4Anim>();
        default: return nullptr;
    }
}

void ConvAnimPanel::show(const std::string& title, bool* open, 
                        ConvAnimBase& anim, float& deltaTime) {
    // 设置窗口大小
    ImGui::SetNextWindowSize(ImVec2(1000, 600), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
    
    if (!ImGui::Begin(title.c_str(), open, 
                     ImGuiWindowFlags_NoCollapse | 
                     ImGuiWindowFlags_NoScrollbar)) {
        ImGui::End();
        return;
    }
    
    if (anim.isPlaying()) {
        anim.update(deltaTime);
    }

    // 分三列显示
    ImGui::Columns(3, "animation_columns", false);
    ImGui::SetColumnWidth(0, 320.0f);
    ImGui::SetColumnWidth(1, 320.0f);
    ImGui::SetColumnWidth(2, 320.0f);
    
    // 左侧：输入+卷积区域
    ImGui::BeginChild("Input", ImVec2(0, 400), true);
    showInputWindow(anim, "input");
    ImGui::EndChild();
    
    ImGui::NextColumn();
    
    // 中间：卷积核+点积
    ImGui::BeginChild("Kernel", ImVec2(0, 400), true);
    showKernelWindow(anim, "kernel");
    ImGui::EndChild();
    
    ImGui::NextColumn();
    
    // 右侧：输出
    ImGui::BeginChild("Output", ImVec2(0, 400), true);
    showOutputWindow(anim, "output");
    ImGui::EndChild();
    
    ImGui::Columns(1);
    ImGui::Separator();
    
    // 底部：控制面板和信息
    ImGui::BeginChild("Controls", ImVec2(0, 0), true);
    showControls(anim, "controls");
    ImGui::EndChild();
    
    ImGui::End();
}

void ConvAnimPanel::showInputWindow(ConvAnimBase& anim, const char* id) {
    ImGui::Text("输入特征图");
    ImGui::Separator();
    
    // 显示输入纹理
    ImGui::Text("尺寸: (%d+1) × (%d+1)  (含padding)", anim.getInputWidth(), anim.getInputHeight());
    ImVec2 inputSize(280, 280);
    
    // 获取当前卷积核位置
    int curX = anim.getCurrentX();
    int curY = anim.getCurrentY();
    int kernelSize = anim.getKernelSize();
    
    // 显示输入纹理
    ImGui::Image(
        (void*)(intptr_t)anim.getKernelFrameTexture().getNativeHandle(),
        inputSize,
        ImVec2(0, 0), ImVec2(1, 1)
    );
    
    // 显示当前卷积位置
    ImGui::Text("当前卷积位置: (%d, %d)", curX+1, curY+1);
}

void ConvAnimPanel::showKernelWindow(ConvAnimBase& anim, const char* id) {
    ImGui::Text("卷积核");
    ImGui::Separator();
    
    int kernelSize = anim.getKernelSize();
    ImGui::Text("尺寸: %d × %d", kernelSize, kernelSize);
    
    // 显示卷积核纹理
    ImVec2 kernelSizeV(kernelSize * 10, kernelSize * 10);
    ImGui::Image(
        (void*)(intptr_t)anim.getKernelTexture().getNativeHandle(),
        kernelSizeV,
        ImVec2(0, 0), ImVec2(1, 1)
    );
    
    ImGui::Separator();
    ImGui::Text("卷积计算");
    ImGui::Separator();
    
    // 显示卷积核权重
    const auto& weights = anim.getKernelWeights();
    if (ImGui::BeginTable("weights", kernelSize, ImGuiTableFlags_Borders)) {
        for (int y = 0; y < kernelSize; ++y) {
            ImGui::TableNextRow();
            for (int x = 0; x < kernelSize; ++x) {
                ImGui::TableSetColumnIndex(x);
                ImGui::Text("%.2f", weights[y * kernelSize + x]);
            }
        }
        ImGui::EndTable();
    }
    
    // 显示点积
    float dot = anim.getDotProduct();
    ImGui::Separator();
    ImGui::Text("点积: %.4f", dot);
}

void ConvAnimPanel::showOutputWindow(ConvAnimBase& anim, const char* id) {
    ImGui::Text("输出特征图");
    ImGui::Separator();
    
    // 显示输出纹理
    int outW = anim.getOutputWidth();
    int outH = anim.getOutputHeight();
    ImGui::Text("尺寸: %d × %d", outW, outH);
    
    ImVec2 outputSize(280, 280);
    ImGui::Image(
        (void*)(intptr_t)anim.getOutputTexture().getNativeHandle(),
        outputSize,
        ImVec2(0, 0), ImVec2(1, 1)
    );
    
    // 显示点积值
    ImGui::Separator();
    ImGui::Text("当前卷积点积: %.4f", anim.getDotProduct());
}

void ConvAnimPanel::showControls(ConvAnimBase& anim, const char* id) {

    // 速度控制
    ImGui::Separator();
    ImGui::Text("动画速度控制");
    
    // 获取当前速度
    float currentSpeed = anim.getAnimationSpeed();
    
    // 速度滑动条
    if (ImGui::SliderFloat("速度(帧/秒)", &currentSpeed, 0.1f, 20.0f, "%.1f")) {
        anim.setAnimationSpeed(currentSpeed);
    }
    
    // 速度快捷按钮
    ImGui::Text("快捷速度:");
    ImGui::SameLine();
    if (ImGui::Button("0.5帧/秒")) anim.setAnimationSpeed(0.5f);
    ImGui::SameLine();
    if (ImGui::Button("1.0帧/秒")) anim.setAnimationSpeed(1.0f);
    ImGui::SameLine();
    if (ImGui::Button("2.0帧/秒")) anim.setAnimationSpeed(2.0f);
    ImGui::SameLine();
    if (ImGui::Button("5.0帧/秒")) anim.setAnimationSpeed(5.0f);


    ImGui::Text("动画控制");
    ImGui::Separator();
    
    // 播放/暂停按钮
    if (anim.isPlaying()) {
        if (ImGui::Button("暂停##pause")) {
            anim.pause();
        }
    } else {
        if (ImGui::Button("播放##play")) {
            anim.play();
        }
    }
    
    ImGui::SameLine();
    if (ImGui::Button("重置##reset")) {
        anim.reset();
    }
    

    //卷积动画控制逻辑
    bool isPlaying = anim.isPlaying();
    if (isPlaying) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        ImGui::PopStyleVar();
    } else{
        ImGui::SameLine();
        if (ImGui::Button("单步##step", ImVec2(60, 0))) {
          anim.step();
        }
    }
    
    // 进度
    ImGui::Separator();
    int curX = anim.getCurrentX();
    int curY = anim.getCurrentY();
    int outW = anim.getOutputWidth();
    int outH = anim.getOutputHeight();
    
    if (outW > 0 && outH > 0) {
        int currentPos = curY * outW + curX + 1;
        int totalPos = outW * outH;
        float progress = currentPos / (float)totalPos;
        
        ImGui::Text("进度: %d / %d (%.1f%%)", 
                   currentPos, totalPos, progress * 100.0f);
        ImGui::ProgressBar(progress, ImVec2(-1, 20), "");
        
        // 位置信息
        ImGui::Text("当前坐标: X=%d, Y=%d", curX+1, curY+1);
    }
    
    // 卷积核选择
    ImGui::Separator();
    ImGui::Text("卷积核选择");
    
    // 从动画器获取当前卷积核索引
    int selectedKernel = anim.getKernelIndex();
    int numKernels = anim.getNumKernels();  // 获取卷积核总数
    
    
    // 添加切换控制
    if (ImGui::Button("<- 上一个")) { 
        selectedKernel = std::max(0, selectedKernel - 1);
        anim.setKernelIndex(selectedKernel);  // 切换卷积核
    }
    ImGui::SameLine();
    if (ImGui::Button("下一个 ->")) { 
        selectedKernel = std::min(numKernels - 1, selectedKernel + 1);
        anim.setKernelIndex(selectedKernel);  // 切换卷积核
    }
    
    // 滑动条控制
    ImGui::SetNextItemWidth(-1);
    int displayKernel = selectedKernel + 1;

    if (ImGui::SliderInt("##kernel_slider", &displayKernel, 1, numKernels, "核 #%d")) {
        selectedKernel = displayKernel - 1;
        anim.setKernelIndex(selectedKernel);  // 切换卷积核
    }
    
    // 状态指示器
    ImGui::Separator();
    ImGui::Text("动画状态: %s", anim.isPlaying() ? "播放中" : "已暂停");
}