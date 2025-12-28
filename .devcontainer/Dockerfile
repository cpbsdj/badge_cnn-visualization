FROM ubuntu:22.04

# 设置非交互式前端
ENV DEBIAN_FRONTEND=noninteractive

# 设置清华镜像源
RUN sed -i 's/archive.ubuntu.com/mirrors.tuna.tsinghua.edu.cn/g' /etc/apt/sources.list && \
    sed -i 's/security.ubuntu.com/mirrors.tuna.tsinghua.edu.cn/g' /etc/apt/sources.list

# 安装系统工具和构建依赖
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    pkg-config \
    software-properties-common \
    && rm -rf /var/lib/apt/lists/*

# 安装SFML 2.5.1（精确版本匹配）
RUN apt-get update && apt-get install -y \
    libsfml-dev=2.5.1+dfsg-2 \
    libsfml-audio2.5=2.5.1+dfsg-2 \
    libsfml-graphics2.5=2.5.1+dfsg-2 \
    libsfml-network2.5=2.5.1+dfsg-2 \
    libsfml-system2.5=2.5.1+dfsg-2 \
    libsfml-window2.5=2.5.1+dfsg-2 \
    && rm -rf /var/lib/apt/lists/*

# 安装图形库依赖
RUN apt-get update && apt-get install -y \
    libgl1-mesa-dev \
    libx11-dev \
    libxcb1-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    && rm -rf /var/lib/apt/lists/*

# 安装nlohmann_json 3.10.5
RUN apt-get update && apt-get install -y \
    nlohmann-json3-dev=3.10.5-2 \
    && rm -rf /var/lib/apt/lists/*

# 安装Python和PyTorch 2.9.0+cpu
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install --index-url https://download.pytorch.org/whl/cpu \
    torch==2.9.0+cpu \
    torchvision==0.24.0+cpu \
    torchaudio==2.9.0+cpu \
    numpy==2.1.2

# 从源码安装ImGui 1.90.4和ImGui-SFML 2.6（与你的环境完全一致）
WORKDIR /tmp

# 安装构建依赖
RUN apt-get update && apt-get install -y \
    libglfw3-dev \
    libgles2-mesa-dev \
    && rm -rf /var/lib/apt/lists/*

# 克隆并安装ImGui 1.90.4
RUN git clone https://github.com/ocornut/imgui.git && \
    cd imgui && \
    git checkout v1.90.4 && \
    # 只复制文件，不进行CMake构建
    mkdir -p /usr/local/include/imgui && \
    cp *.h /usr/local/include/imgui/ && \
    cp *.cpp /usr/local/include/imgui/

# 克隆并安装ImGui-SFML 2.6（使用对应的tag或commit）
RUN git clone https://github.com/eliasdaler/imgui-sfml.git && \
    cd imgui-sfml && \
    git checkout v2.6 && \
    mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc) && \
    make install

# 验证安装
RUN echo "=== 验证安装版本 ===" && \
    echo "ImGui版本:" && grep "IMGUI_VERSION" /usr/local/include/imgui.h | head -1 && \
    echo "ImGui-SFML文件:" && ls -la /usr/local/include/imgui-SFML.h && \
    ls -la /usr/local/lib/cmake/ImGui-SFML/ && \
    echo "安装完成"

# 设置工作目录
WORKDIR /workspace

# 复制项目文件
COPY . .

# 创建构建目录并编译
RUN mkdir -p build && cd build && \
    cmake .. && \
    make -j$(nproc)

# 设置默认运行命令
CMD ["./build/digit_viz"]
