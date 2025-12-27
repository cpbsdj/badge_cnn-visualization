# 最小化Dockerfile
FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

# 只安装最基础的工具
RUN apt-get update && apt-get install -y \
    cmake \
    g++ \
    git \
    libsfml-dev \
    libgl1-mesa-dev \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

# 安装PyTorch
RUN pip3 install torch torchvision --index-url https://download.pytorch.org/whl/cpu

WORKDIR /workspace