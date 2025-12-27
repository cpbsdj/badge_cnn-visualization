"""
export_model.py
读取 best.pth →
  1. model.json      网络结构 + 每层参数 + hotspots
  2. weights.bin     原始 float32 权重连续内存
  3. hotspots.json   热点参数文件
  4.各卷积层输出的数据文件
"""
import torch
import json
import struct
import pathlib
import sys
import importlib.util
import numpy as np
import os
from PIL import Image
import torchvision.transforms as transforms

model_path = pathlib.Path(__file__).parent.parent / "python" / "model" / "badge_cnn.py"
spec = importlib.util.spec_from_file_location("model", model_path)
model_module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(model_module)
BadgeCNN = model_module.BadgeCNN

OUTPUT_DIR = pathlib.Path(__file__).parent / "output"
OUTPUT_DIR.mkdir(exist_ok=True)

# ---------- 1. 加载模型 ----------
def load_model():
    """加载训练好的模型"""
    model = BadgeCNN()
    ckpt_path = pathlib.Path(__file__).parent.parent / "python" / "ckpts" / "badge9_best.pth"
    
    # 尝试不同的加载方式
    try:
        checkpoint = torch.load(ckpt_path, map_location="cpu")
        if isinstance(checkpoint, dict) and "model_state" in checkpoint:
            model.load_state_dict(checkpoint["model_state"])
        elif isinstance(checkpoint, dict) and "state_dict" in checkpoint:
            model.load_state_dict(checkpoint["state_dict"])
        else:
            model.load_state_dict(checkpoint)
    except Exception as e:
        print(f"加载模型失败: {e}")
        # 尝试直接加载为state_dict
        model.load_state_dict(torch.load(ckpt_path, map_location="cpu"))
    
    model.eval()
    return model

# ---------- 2. 定义热点区域 ----------
# 7个手工输入的交互热点区域坐标
HOT_SPOTS = {
    "input_layer": {
        "type": "rect", 
        "pts": [[156,402], [387,402], [387,633], [156,633]],  # 64x64输入区域
        "description": "输入层:接收预处理后的单通道 64×64 灰度图像"
    },
    "conv1": { 
        "type": "rect", 
        "pts": [[467,206], [668,206], [668,865], [467,865]],  # 第一个卷积层
        "description": "第一卷积层:\n1个输入通道->16个输出通道\n卷积核大小3×3，stride=1，padding=1\n批归一化后使用ReLU激活\n最大池化2×2，stride=2"
    },
    "conv2": { 
        "type": "rect", 
        "pts": [[729,206], [930,206], [930,865], [729,865]],  # 第二个卷积层
        "description": "第二卷积层:\n16个输入通道->32个输出通道\n卷积核大小3×3，stride=1，padding=1\n批归一化后使用ReLU激活\n最大池化2×2，stride=2"
    },
    "conv3": { 
        "type": "rect", 
        "pts": [[996,206], [1197,206], [1197,865], [996,865]],  # 第三个卷积层
        "description": "第三卷积层:\n32个输入通道->64个输出通道\n卷积核大小3×3，stride=1，padding=1\n批归一化后使用ReLU激活\n最大池化2×2，stride=2"
    },
    "conv4": { 
        "type": "rect", 
        "pts": [[1260,206], [1461,206], [1461,865], [1260,865]],  # 第四个卷积层
        "description": "第四卷积层:\n64个输入通道->64个输出通道\n卷积核大小3×3，stride=1，padding=1\n批归一化后使用ReLU激活\n最大池化2×2，stride=2"
    },
    "gap_layer": { 
        "type": "rect", 
        "pts": [[1549,206],[1629,206],[1629,865],[1549,865] ],  
        "description": "全局平均池化+展平层:\n64个输入通道，每个输入通道空间尺寸为4×4\n全局平均池化将64×4×4特征图每个通道求平均\n再接展平层将64×1×1张量转化为64维向量"
    },
    "classifier": { 
        "type": "rect", 
        "pts": [[1724,206], [1784,206], [1784,865], [1724,865]],  # 分类器层
        "description": "全连接分类器:\n将64维输入与9个输出类别全连接\n输出9个类别的原始得分(logits)，取得分最高者为预测类别"
    }
}

# ---------- 3. 网络结构解析 ----------
def analyze_model_structure(model):
    """分析模型结构，生成层信息"""
    layers_info = []
    
    # 手动定义网络结构
    structure = [
        {"name": "conv1", "type": "conv2d", "in_channels": 1, "out_channels": 16, 
         "kernel_size": 3, "stride": 1, "padding": 1, "activation": "relu"},
        {"name": "batchnorm1", "type": "batchnorm2d", "num_features": 16},
        {"name": "maxpool1", "type": "maxpool2d", "kernel_size": 2, "stride": 2},
        
        {"name": "conv2", "type": "conv2d", "in_channels": 16, "out_channels": 32, 
         "kernel_size": 3, "stride": 1, "padding": 1, "activation": "relu"},
        {"name": "batchnorm2", "type": "batchnorm2d", "num_features": 32},
        {"name": "maxpool2", "type": "maxpool2d", "kernel_size": 2, "stride": 2},
        
        {"name": "conv3", "type": "conv2d", "in_channels": 32, "out_channels": 64, 
         "kernel_size": 3, "stride": 1, "padding": 1, "activation": "relu"},
        {"name": "batchnorm3", "type": "batchnorm2d", "num_features": 64},
        {"name": "maxpool3", "type": "maxpool2d", "kernel_size": 2, "stride": 2},
        
        {"name": "conv4", "type": "conv2d", "in_channels": 64, "out_channels": 64, 
         "kernel_size": 3, "stride": 1, "padding": 1, "activation": "relu"},
        {"name": "batchnorm4", "type": "batchnorm2d", "num_features": 64},
        {"name": "maxpool4", "type": "maxpool2d", "kernel_size": 2, "stride": 2},
        
        {"name": "gap", "type": "adaptive_avg_pool2d", "output_size": 1},
        {"name": "classifier", "type": "linear", "in_features": 64, "out_features": 9}
    ]
    
    return structure

# ---------- 4. 参数提取和序列化 ----------
def extract_parameters(model):
    """提取模型参数并准备序列化"""
    layers = []
    weights = []
    offset = 0
    
    # 参数名称映射到热点
    param_to_hotspot = {
        "conv1.weight": "conv1",
        "conv1.bias": "conv1",
        "conv2.weight": "conv2", 
        "conv2.bias": "conv2",
        "conv3.weight": "conv3",
        "conv3.bias": "conv3",
        "conv4.weight": "conv4",
        "conv4.bias": "conv4",
        "classifier.weight": "classifier",
        "classifier.bias": "classifier"
    }
    
    for name, param in model.named_parameters():
        shape = list(param.shape)
        num_elements = param.numel()
        data = param.detach().view(-1).tolist()
        
        # 确定层类型
        if "weight" in name:
            if "conv" in name:
                layer_type = "conv_weight"
            elif "classifier" in name:
                layer_type = "fc_weight"
            else:
                layer_type = "weight"
        elif "bias" in name:
            layer_type = "bias"
        else:
            layer_type = "parameter"
        
        # 获取对应的热点
        hotspot_name = param_to_hotspot.get(name)
        hotspot = HOT_SPOTS.get(hotspot_name) if hotspot_name else None
        
        layer_info = {
            "name": name,
            "shape": shape,
            "dtype": "float32",
            "offset": offset,
            "size_bytes": num_elements * 4,
            "type": layer_type,
            "hotspot": hotspot
        }
        
        layers.append(layer_info)
        weights.extend(data)
        offset += num_elements * 4
    
    return layers, weights

# ---------- 5. 主导出函数 ----------
def export_model():
    """导出模型的主要函数"""
    print("正在加载模型...")
    model = load_model()
    
    print("分析模型结构...")
    model_structure = analyze_model_structure(model)
    
    print("提取模型参数...")
    layers, weights = extract_parameters(model)
    
    # ---------- 写 JSON 文件 ----------
    json_data = {
        "model_info": {
            "input_size": [1, 64, 64],
            "output_size": 9,
            "num_classes": 9,
            "description": "CNN校徽分类器"
        },
        "structure": model_structure,
        "layers": layers,
        "hotspots": HOT_SPOTS
    }
    
    json_path = OUTPUT_DIR / "model.json"
    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(json_data, f, indent=2, ensure_ascii=False)
    print(f"[+] JSON → {json_path}")
    
    # ---------- 写二进制权重文件 ----------
    bin_path = OUTPUT_DIR / "weights.bin"
    with open(bin_path, "wb") as f:
        # 写入权重数量作为文件头
        f.write(struct.pack("I", len(weights)))
        # 写入所有权重数据
        f.write(struct.pack(f"{len(weights)}f", *weights))
    print(f"[+] BIN  → {bin_path}  ({len(weights)*4} bytes, {len(weights)}个参数)")
    
    # ---------- 单独的热点文件 ----------
    hot_path = OUTPUT_DIR / "hotspots.json"
    with open(hot_path, "w", encoding="utf-8") as f:
        json.dump(HOT_SPOTS, f, indent=2, ensure_ascii=False)
    print(f"[+] HOT  → {hot_path}")
    
    # 打印摘要信息
    print(f"\n导出完成！")
    print(f"总参数数量: {len(weights)}")
    print(f"总文件大小: {len(weights)*4} 字节")
    print(f"热点区域数量: {len(HOT_SPOTS)}")

# ---------- 6. 验证函数 ----------
def verify_export():
    """验证导出的文件"""
    try:
        # 验证JSON文件
        with open(OUTPUT_DIR / "model.json", "r", encoding="utf-8") as f:
            model_data = json.load(f)
            print("JSON验证通过，层数量:", len(model_data["layers"]))
        
        # 验证BIN文件
        with open(OUTPUT_DIR / "weights.bin", "rb") as f:
            num_weights = struct.unpack("I", f.read(4))[0]
            weights = struct.unpack(f"{num_weights}f", f.read(num_weights * 4))
            print("BIN验证通过，权重数量:", len(weights))
            
    except Exception as e:
        print(f"验证失败: {e}")



# ---------- 加载测试图片 ----------
def load_test_image(image_path=None):
    """加载测试图片，优先使用真实图片，失败则生成随机张量"""
    
    if image_path is None:
        # 默认使用ustc.png
        base_dir = pathlib.Path(__file__).parent.parent
        image_path = base_dir / "python" / "data" / "mol_ustc_test" / "ustc.jpg"
    
    # 尝试加载真实图片
    if os.path.exists(image_path):
        print(f"尝试加载图片: {image_path}")
        try:
            # 图片预处理，转为灰度64x64
            transform = transforms.Compose([
                transforms.Grayscale(),
                transforms.Resize((64, 64)),
                transforms.ToTensor(),
            ])
            
            img = Image.open(image_path)
            img_tensor = transform(img).unsqueeze(0)  # [1, 1, 64, 64]
            
            print(f"成功加载图片: {img.size} -> 64×64灰度图")
            return img_tensor
            
        except Exception as e:
            print(f"加载图片失败: {e}")
    
    # 如果加载失败，创建随机张量
    print("使用随机张量作为输入")
    return torch.randn(1, 1, 64, 64)

# ---------- 各层输出数据导出函数 ----------
def export_layer_outputs(model, test_input, output_dir, input_name="m_ustc"):
    """导出各层的输出数据"""
    print("导出各层输出数据...")
    layer_outputs = {}
    
    with torch.no_grad():
        x = test_input
        layer_outputs['input'] = x.numpy()           # 1×64×64
        
        x = model.conv1(x)
        layer_outputs['conv1_output'] = x.numpy()    # 16×32×32
        
        x = model.conv2(x)
        layer_outputs['conv2_output'] = x.numpy()    # 32×16×16
        
        x = model.conv3(x)
        layer_outputs['conv3_output'] = x.numpy()    # 64×8×8
        
        x = model.conv4(x)
        layer_outputs['conv4_output'] = x.numpy()    # 64×4×4
    
    # 保存为二进制文件
    for name, data in layer_outputs.items():
        data.tofile(f"{output_dir}/{input_name}_{name}.bin")
        print(f"  {name}: shape={data.shape}")
    
    return layer_outputs

if __name__ == "__main__":
    export_model()

    model = load_model()
    # 加载ustc.png
    test_input = load_test_image()
    # 导出各层输出数据
    export_layer_outputs(model, test_input, OUTPUT_DIR, "m_ustc")
    

    verify_export()