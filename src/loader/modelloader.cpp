#include "modelloader.hpp"
#include <iostream>
#include <algorithm>

bool ModelLoader::load(const std::string& jsonPath, const std::string& binPath) {
    // 清空之前的数据
    layers.clear();
    weights.clear();
    hotspots.clear();
    structure.clear();

    // 加载JSON文件
    std::ifstream jf(jsonPath);
    if (!jf.is_open()) {
        std::cerr << "无法打开JSON文件: " << jsonPath << std::endl;
        return false;
    }

    try {
        json j;
        jf >> j;

        // 解析模型信息
        if (j.contains("model_info")) {
            auto& info = j["model_info"];
            model_info.input_size = info["input_size"].get<std::vector<int>>();
            model_info.output_size = info["output_size"];
            model_info.num_classes = info["num_classes"];
            model_info.description = info["description"];
            
            std::cout << "加载模型: " << model_info.description << std::endl;
            std::cout << "输入尺寸: " << model_info.input_size[0] << "x" 
                      << model_info.input_size[1] << "x" << model_info.input_size[2] << std::endl;
            std::cout << "输出类别: " << model_info.num_classes << std::endl;
        }

        // 解析网络结构
        if (j.contains("structure")) {
            for (auto& s : j["structure"]) {
                LayerStructure layer_struct;
                layer_struct.name = s["name"];
                layer_struct.type = s["type"];
                
                // 解析参数
                for (auto it = s.begin(); it != s.end(); ++it) {
                    if (it.key() != "name" && it.key() != "type") {
                        if (it.value().is_number()) {
                            layer_struct.parameters[it.key()] = it.value();
                        }
                    }
                }
                structure.push_back(layer_struct);
            }
            std::cout << "网络结构层数: " << structure.size() << std::endl;
        }

        // 解析网络层参数
        for (auto& l : j["layers"]) {
            Layer lay;
            lay.name = l["name"];
            lay.shape = l["shape"].get<std::vector<int>>();
            lay.offset = l["offset"];
            lay.size_bytes = l["size_bytes"];
            lay.type = l["type"];
            
            if (l.contains("dtype")) {
                lay.dtype = l["dtype"];
            }

            // 解析热点区域
            if (!l["hotspot"].is_null()) {
                auto& h = l["hotspot"];
                lay.hotspot.type = h["type"];
                
                // 解析坐标点
                for (auto& p : h["pts"]) {
                    if (p.is_array() && p.size() >= 2) {
                        lay.hotspot.pts.emplace_back(p[0].get<float>(), p[1].get<float>());
                    }
                }
                
                if (h.contains("description")) {
                    lay.hotspot.description = h["description"];
                }
            }

            layers.push_back(std::move(lay));
        }

        // 解析独立的热点区域
        if (j.contains("hotspots")) {
            for (auto& [key, h] : j["hotspots"].items()) {
                HotSpot hotspot;
                hotspot.type = h["type"];
                
                // 解析坐标点
                for (auto& p : h["pts"]) {
                    if (p.is_array() && p.size() >= 2) {
                        hotspot.pts.emplace_back(p[0].get<float>(), p[1].get<float>());
                    }
                }
                
                if (h.contains("description")) {
                    hotspot.description = h["description"];
                }
                
                hotspots[key] = hotspot;
            }
        }

        std::cout << "成功加载 " << layers.size() << " 个参数层" << std::endl;
        std::cout << "成功加载 " << hotspots.size() << " 个热点区域" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "JSON解析错误: " << e.what() << std::endl;
        return false;
    }

    // 加载二进制权重文件
    std::ifstream bf(binPath, std::ios::binary);
    if (!bf.is_open()) {
        std::cerr << "无法打开BIN文件: " << binPath << std::endl;
        return false;
    }

    try {
        // 读取权重数量（文件头）
        uint32_t num_weights = 0;
        bf.read(reinterpret_cast<char*>(&num_weights), sizeof(uint32_t));
        
        if (num_weights > 0 && num_weights < 10000000) { // 合理的范围检查
            // 有文件头的情况
            weights.resize(num_weights * sizeof(float));
            bf.read(weights.data(), num_weights * sizeof(float));
            std::cout << "加载权重数量: " << num_weights << std::endl;
        } else {
            // 没有文件头的情况，读取整个文件
            bf.seekg(0, std::ios::end);
            size_t size = bf.tellg();
            bf.seekg(0, std::ios::beg);
            weights.resize(size);
            bf.read(weights.data(), size);
            std::cout << "加载权重文件大小: " << size << " 字节" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "BIN文件读取错误: " << e.what() << std::endl;
        return false;
    }

    return !layers.empty() && !weights.empty();
}

const float* ModelLoader::get_layer_weights(const Layer& layer) const {
    if (layer.offset + layer.size_bytes > weights.size()) {
        std::cerr << "权重数据越界: " << layer.name << std::endl;
        return nullptr;
    }
    return reinterpret_cast<const float*>(weights.data() + layer.offset);
}

std::vector<Layer> ModelLoader::get_conv_layers() const {
    std::vector<Layer> conv_layers;
    for (const auto& layer : layers) {
        if (layer.type.find("conv") != std::string::npos) {
            conv_layers.push_back(layer);
        }
    }
    return conv_layers;
}

std::vector<Layer> ModelLoader::get_fc_layers() const {
    std::vector<Layer> fc_layers;
    for (const auto& layer : layers) {
        if (layer.type.find("fc") != std::string::npos) {
            fc_layers.push_back(layer);
        }
    }
    return fc_layers;
}

std::vector<Layer> ModelLoader::get_bias_layers() const {
    std::vector<Layer> bias_layers;
    for (const auto& layer : layers) {
        if (layer.type == "bias") {
            bias_layers.push_back(layer);
        }
    }
    return bias_layers;
}

const HotSpot* ModelLoader::get_hotspot(const std::string& name) const {
    auto it = hotspots.find(name);
    return it != hotspots.end() ? &it->second : nullptr;
}

sf::Vector2i ModelLoader::get_input_size() const {
    if (model_info.input_size.size() >= 3) {
        return sf::Vector2i(model_info.input_size[2], model_info.input_size[1]);
    }
    return sf::Vector2i(64, 64);
}

const Layer* ModelLoader::find_layer(const std::string& name) const {
    for (const auto& layer : layers) {
        if (layer.name == name) {
            return &layer;
        }
    }
    return nullptr;
}

bool ModelLoader::is_point_in_hotspot(const std::string& hotspot_name, const sf::Vector2f& point) const {
    auto it = hotspots.find(hotspot_name);
    if (it == hotspots.end()) return false;
    
    const auto& hotspot = it->second;
    
    if (hotspot.type == "rect" && hotspot.pts.size() >= 4) {
        float min_x = std::min({hotspot.pts[0].x, hotspot.pts[1].x, hotspot.pts[2].x, hotspot.pts[3].x});
        float max_x = std::max({hotspot.pts[0].x, hotspot.pts[1].x, hotspot.pts[2].x, hotspot.pts[3].x});
        float min_y = std::min({hotspot.pts[0].y, hotspot.pts[1].y, hotspot.pts[2].y, hotspot.pts[3].y});
        float max_y = std::max({hotspot.pts[0].y, hotspot.pts[1].y, hotspot.pts[2].y, hotspot.pts[3].y});
        
        return point.x >= min_x && point.x <= max_x && point.y >= min_y && point.y <= max_y;
    }
    
    return false;
}