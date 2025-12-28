# .devcontainer/Dockerfile
FROM ubuntu:22.04

# 安装基础工具
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake=3.22.1-1ubuntu1.22.04.2 \
    g++=4:11.2.0-1ubuntu1 \
    gdb=12.1-0ubuntu1~22.04.2 \
    git=1:2.34.1-1ubuntu1.15 \
    ninja-build \
    pkg-config \
    python3=3.10.6-1~22.04.1 \
    python3-pip=22.0.2+dfsg-1ubuntu0.7 \
    && rm -rf /var/lib/apt/lists/*

# 安装SFML
RUN apt-get update && apt-get install -y \
    libsfml-dev=2.5.1+dfsg-2 \
    libgl1-mesa-dev \
    libglfw3-dev \
    && rm -rf /var/lib/apt/lists/*

# 安装PyTorch
RUN pip3 install torch==2.9.0+cpu torchvision==0.24.0+cpu torchaudio==2.9.0+cpu --index-url https://download.pytorch.org/whl/cpu

# 安装ImGui-SFML
RUN git clone https://github.com/SFML/imgui-sfml.git /tmp/imgui-sfml \
    && cd /tmp/imgui-sfml \
    && mkdir build && cd build \
    && cmake .. \
    && make -j$(nproc) \
    && make install \
    && rm -rf /tmp/imgui-sfml

# 安装nlohmann_json
RUN git clone https://github.com/nlohmann/json.git /tmp/json \
    && cd /tmp/json \
    && mkdir build && cd build \
    && cmake .. -DJSON_BuildTests=OFF \
    && make -j$(nproc) \
    && make install \
    && rm -rf /tmp/json

# 创建非root用户
RUN useradd -m -s /bin/bash vscode
USER vscode
WORKDIR /workspace