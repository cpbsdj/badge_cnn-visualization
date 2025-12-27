#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics.hpp>

using json = nlohmann::json;

// 热点区域数据结构
struct HotSpot {
    std::string type;                 // "rect", "poly", "circle"
    std::vector<sf::Vector2f> pts;    // 坐标点
    std::string description;          // 描述信息
};

// 网络层数据结构
struct Layer {
    std::string name;
    std::vector<int> shape;
    size_t offset = 0;
    size_t size_bytes = 0;
    std::string dtype = "float32";
    std::string type;                 // "conv_weight", "fc_weight", "bias", "parameter"
    HotSpot hotspot;
};

// 模型信息结构
struct ModelInfo {
    std::vector<int> input_size;      // [1, 64, 64]
    int output_size;                  // 9
    int num_classes;                  // 9
    std::string description;          // "CNN校徽分类器"
};

// 网络结构信息
struct LayerStructure {
    std::string name;
    std::string type;
    std::unordered_map<std::string, int> parameters;
};

class ModelLoader {
public:
    std::vector<Layer> layers;
    std::vector<char> weights;
    std::unordered_map<std::string, HotSpot> hotspots;
    ModelInfo model_info;
    std::vector<LayerStructure> structure;

    ModelLoader() = default;

    bool load(const std::string& jsonPath, const std::string& binPath);

    // 获取指定层的权重数据
    const float* get_layer_weights(const Layer& layer) const;
    
    // 获取指定层的权重数据（通过索引）
    const float* get_layer_weights(int index) const {
        if (index < 0 || index >= layers.size()) return nullptr;
        return get_layer_weights(layers[index]);
    }

    // 获取卷积层
    std::vector<Layer> get_conv_layers() const;

    // 获取全连接层
    std::vector<Layer> get_fc_layers() const;

    // 获取偏置层
    std::vector<Layer> get_bias_layers() const;

    // 根据名称获取热点区域
    const HotSpot* get_hotspot(const std::string& name) const;

    // 获取所有热点区域
    const std::unordered_map<std::string, HotSpot>& get_all_hotspots() const {
        return hotspots;
    }

    // 获取模型输入尺寸
    sf::Vector2i get_input_size() const;

    // 获取输出类别数量
    int get_num_classes() const {
        return model_info.num_classes;
    }

    // 获取模型描述
    const std::string& get_description() const {
        return model_info.description;
    }

    // 获取网络结构信息
    const std::vector<LayerStructure>& get_structure() const {
        return structure;
    }

    // 获取层数量
    size_t get_num_layers() const {
        return layers.size();
    }

    // 根据名称查找层
    const Layer* find_layer(const std::string& name) const;

    // 检查点是否在热点区域内
    bool is_point_in_hotspot(const std::string& hotspot_name, const sf::Vector2f& point) const;
};