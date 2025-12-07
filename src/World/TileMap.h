#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

// ============================================================================
// TileMap - 支持加载 Tiled 导出的 .tmj/.json 文件
// 
// 支持外部 .tsx tileset 引用（Tiled推荐方式）
// 
// 使用方法:
//   TileMap tileMap;
//   tileMap.loadFromTiled("../../assets/maps/farm.tmj", 48);
// ============================================================================

enum class TileType {
    Ground = 0,
    Obstacle = 1,
    Water = 2
};

struct Tile {
    int id;
    int textureIndex;
    sf::Vector2i texCoords;
    TileType type;
    
    Tile() : id(0), textureIndex(0), texCoords(0, 0), type(TileType::Ground) {}
};

struct TilesetInfo {
    int firstGid;
    int tileWidth;
    int tileHeight;
    int columns;
    int tileCount;
    std::string imagePath;
    sf::Texture texture;
    bool loaded;
    
    TilesetInfo() : firstGid(1), tileWidth(32), tileHeight(32), 
                    columns(16), tileCount(256), loaded(false) {}
};

struct LayerInfo {
    std::string name;
    std::vector<int> data;
    bool isCollision;
};

class TileMap {
public:
    TileMap() 
        : width(0), height(0), tileSize(32), srcTileSize(32), tilesPerRow(16)
    {}
    
    TileMap(int w, int h, int displayTileSize) 
        : width(w), height(h), tileSize(displayTileSize), srcTileSize(32), tilesPerRow(16)
    {
        groundLayer.resize(width * height);
        decorationLayer.resize(width * height);
        collisionLayer.resize(width * height, false);
    }
    
    // ========================================
    // 从 Tiled .tmj 文件加载
    // displayTileSize: 显示时的瓦片大小（0表示使用原始大小）
    // ========================================
    bool loadFromTiled(const std::string& tmjPath, int displayTileSize = 0) {
        std::cout << "\n========================================" << std::endl;
        std::cout << "  加载 Tiled 地图: " << tmjPath << std::endl;
        std::cout << "========================================" << std::endl;
        
        // 读取文件
        std::ifstream file(tmjPath);
        if (!file.is_open()) {
            std::cerr << "✗ 无法打开文件: " << tmjPath << std::endl;
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string json = buffer.str();
        file.close();
        
        // 获取 .tmj 文件所在目录
        tmjBasePath = getDirectory(tmjPath);
        
        // 解析地图基本信息
        width = getJsonInt(json, "width");
        height = getJsonInt(json, "height");
        srcTileSize = getJsonInt(json, "tilewidth");
        
        tileSize = (displayTileSize > 0) ? displayTileSize : srcTileSize;
        
        std::cout << "✓ 地图尺寸: " << width << "x" << height << " 瓦片" << std::endl;
        std::cout << "✓ 源瓦片大小: " << srcTileSize << "x" << srcTileSize << std::endl;
        std::cout << "✓ 显示瓦片大小: " << tileSize << "x" << tileSize << std::endl;
        
        // 解析 tilesets
        parseTilesets(json);
        
        // 解析图层
        parseLayers(json);
        
        // 初始化内部数据
        initializeFromLayers();
        
        std::cout << "✓ 地图像素尺寸: " << getMapSize().x << "x" << getMapSize().y << std::endl;
        std::cout << "========================================\n" << std::endl;
        
        return true;
    }
    
    // ========================================
    // 原有的数组初始化方法（保留兼容）
    // ========================================
    bool loadTilesets() {
        std::vector<std::string> paths = {
            "../../assets/map/part_1.png",
            "../../assets/map/part_2.png" 
        };
        
        tilesets.clear();
        for (size_t i = 0; i < paths.size(); i++) {
            TilesetInfo ts;
            ts.firstGid = (i == 0) ? 0 : 1000;
            ts.tileWidth = 32;
            ts.tileHeight = 32;
            ts.columns = 16;
            ts.imagePath = paths[i];
            
            if (ts.texture.loadFromFile(paths[i])) {
                ts.loaded = true;
            } else {
                std::cerr << "加载失败: " << paths[i] << std::endl;
            }
            tilesets.push_back(ts);
        }
        return true;
    }
    
    void setTile(int x, int y, int rawID, bool isGround) {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        
        int index = y * width + x;
        Tile& tile = isGround ? groundLayer[index] : decorationLayer[index];
        
        tile.id = rawID;
        
        if (rawID >= 1000) {
            tile.textureIndex = 1;
            int localID = rawID - 1000;
            tile.texCoords.x = (localID % 16) * 32;
            tile.texCoords.y = (localID / 16) * 32;
        } else {
            tile.textureIndex = 0;
            tile.texCoords.x = (rawID % 16) * 32;
            tile.texCoords.y = (rawID / 16) * 32;
        }
        
        if (!isGround && rawID != -1) {
            if (rawID == 67 || rawID == 68 || rawID == 48) {
                tile.type = TileType::Ground;
                collisionLayer[index] = false;
            } else {
                tile.type = TileType::Obstacle;
                collisionLayer[index] = true;
            }
        }
    }
    
    void initializeFromArray(const std::vector<std::vector<int>>& ground, 
                           const std::vector<std::vector<int>>& decor) {
        groundLayer.resize(width * height);
        decorationLayer.resize(width * height);
        collisionLayer.resize(width * height, false);
        
        loadTilesets();
        
        for (int y = 0; y < height && y < (int)ground.size(); y++) {
            for (int x = 0; x < width && x < (int)ground[y].size(); x++) {
                setTile(x, y, ground[y][x], true);
                if (y < (int)decor.size() && x < (int)decor[y].size()) {
                    setTile(x, y, decor[y][x], false);
                }
            }
        }
    }
    
    // ========================================
    // 渲染
    // ========================================
    void render(sf::RenderWindow& window, const sf::View& view) {
        sf::FloatRect bounds(
            view.getCenter().x - view.getSize().x/2,
            view.getCenter().y - view.getSize().y/2,
            view.getSize().x, view.getSize().y
        );
        
        int startX = std::max(0, (int)(bounds.left / tileSize) - 1);
        int startY = std::max(0, (int)(bounds.top / tileSize) - 1);
        int endX = std::min(width, (int)((bounds.left + bounds.width) / tileSize) + 2);
        int endY = std::min(height, (int)((bounds.top + bounds.height) / tileSize) + 2);
        
        // 绘制地面
        for (int y = startY; y < endY; y++) {
            for (int x = startX; x < endX; x++) {
                int idx = y * width + x;
                if (idx < (int)groundLayer.size() && groundLayer[idx].id > 0) {
                    drawTile(window, groundLayer[idx], x, y);
                }
            }
        }
        
        // 绘制装饰
        for (int y = startY; y < endY; y++) {
            for (int x = startX; x < endX; x++) {
                int idx = y * width + x;
                if (idx < (int)decorationLayer.size() && decorationLayer[idx].id > 0) {
                    drawTile(window, decorationLayer[idx], x, y);
                }
            }
        }
    }
    
    void drawTile(sf::RenderWindow& window, const Tile& tile, int x, int y) {
        if (tile.textureIndex < 0 || tile.textureIndex >= (int)tilesets.size()) return;
        
        const TilesetInfo& ts = tilesets[tile.textureIndex];
        if (!ts.loaded) return;
        
        sf::Sprite s;
        s.setTexture(ts.texture);
        s.setTextureRect(sf::IntRect(tile.texCoords.x, tile.texCoords.y, 
                                      ts.tileWidth, ts.tileHeight));
        s.setPosition(x * tileSize, y * tileSize);
        
        float scale = (float)tileSize / ts.tileWidth;
        s.setScale(scale, scale);
        
        window.draw(s);
    }
    
    // ========================================
    // 碰撞检测
    // ========================================
    bool isColliding(const sf::FloatRect& box) const {
        int left = std::max(0, (int)(box.left / tileSize));
        int top = std::max(0, (int)(box.top / tileSize));
        int right = std::min(width-1, (int)((box.left+box.width)/tileSize));
        int bottom = std::min(height-1, (int)((box.top+box.height)/tileSize));
        
        for(int y = top; y <= bottom; y++) {
            for(int x = left; x <= right; x++) {
                int idx = y * width + x;
                if(idx < (int)collisionLayer.size() && collisionLayer[idx]) {
                    return true;
                }
            }
        }
        return false;
    }
    
    // ========================================
    // Getters
    // ========================================
    sf::Vector2i getMapSize() const { return sf::Vector2i(width * tileSize, height * tileSize); }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getTileSize() const { return tileSize; }

private:
    // ========================================
    // 路径处理工具
    // ========================================
    
    // 获取目录部分 (包含尾部斜杠)
    std::string getDirectory(const std::string& path) {
        size_t pos = path.find_last_of("/\\");
        if (pos == std::string::npos) return "./";
        return path.substr(0, pos + 1);
    }
    
    // 清理路径中的转义字符 (\/ -> /)
    std::string cleanPath(const std::string& path) {
        std::string result = path;
        size_t pos;
        while ((pos = result.find("\\/")) != std::string::npos) {
            result.replace(pos, 2, "/");
        }
        while ((pos = result.find("\\\\")) != std::string::npos) {
            result.replace(pos, 2, "/");
        }
        return result;
    }
    
    // 规范化路径 (处理 ../ 和 ./)
    std::string normalizePath(const std::string& basePath, const std::string& relativePath) {
        std::string cleanRelative = cleanPath(relativePath);
        
        // 如果是绝对路径或特殊路径，直接返回
        if (cleanRelative.empty() || cleanRelative[0] == ':') {
            return cleanRelative;
        }
        
        std::string result = basePath + cleanRelative;
        
        // 简单处理 ../
        size_t pos;
        while ((pos = result.find("/../")) != std::string::npos) {
            if (pos == 0) break;
            size_t prevSlash = result.rfind('/', pos - 1);
            if (prevSlash == std::string::npos) prevSlash = 0;
            result = result.substr(0, prevSlash) + result.substr(pos + 3);
        }
        
        // 处理 ./
        while ((pos = result.find("/./")) != std::string::npos) {
            result = result.substr(0, pos) + result.substr(pos + 2);
        }
        
        return result;
    }
    
    // ========================================
    // JSON 解析辅助函数
    // ========================================
    int getJsonInt(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\"";
        size_t pos = json.find(search);
        if (pos == std::string::npos) return 0;
        
        pos = json.find(":", pos);
        if (pos == std::string::npos) return 0;
        
        pos++;
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n')) pos++;
        
        std::string numStr;
        while (pos < json.size() && (isdigit(json[pos]) || json[pos] == '-')) {
            numStr += json[pos++];
        }
        
        return numStr.empty() ? 0 : std::stoi(numStr);
    }
    
    std::string getJsonString(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\"";
        size_t pos = json.find(search);
        if (pos == std::string::npos) return "";
        
        pos = json.find(":", pos);
        if (pos == std::string::npos) return "";
        
        size_t start = json.find("\"", pos + 1);
        if (start == std::string::npos) return "";
        
        // 处理转义字符
        std::string result;
        for (size_t i = start + 1; i < json.size(); i++) {
            if (json[i] == '\\' && i + 1 < json.size()) {
                if (json[i+1] == '/' || json[i+1] == '\\' || json[i+1] == '"') {
                    result += json[i+1];
                    i++;
                    continue;
                }
            }
            if (json[i] == '"') break;
            result += json[i];
        }
        
        return result;
    }
    
    std::vector<int> getJsonIntArray(const std::string& json, const std::string& key) {
        std::vector<int> result;
        
        std::string search = "\"" + key + "\"";
        size_t pos = json.find(search);
        if (pos == std::string::npos) return result;
        
        size_t start = json.find("[", pos);
        size_t end = json.find("]", start);
        if (start == std::string::npos || end == std::string::npos) return result;
        
        std::string arrayStr = json.substr(start + 1, end - start - 1);
        std::stringstream ss(arrayStr);
        std::string item;
        
        while (std::getline(ss, item, ',')) {
            item.erase(0, item.find_first_not_of(" \t\n\r"));
            item.erase(item.find_last_not_of(" \t\n\r") + 1);
            if (!item.empty()) {
                try {
                    result.push_back(std::stoi(item));
                } catch (...) {
                    result.push_back(0);
                }
            }
        }
        
        return result;
    }
    
    std::vector<std::string> getJsonObjectArray(const std::string& json, const std::string& key) {
        std::vector<std::string> result;
        
        std::string search = "\"" + key + "\"";
        size_t pos = json.find(search);
        if (pos == std::string::npos) return result;
        
        size_t arrayStart = json.find("[", pos);
        if (arrayStart == std::string::npos) return result;
        
        int depth = 0;
        size_t objStart = 0;
        
        for (size_t i = arrayStart; i < json.size(); i++) {
            char c = json[i];
            if (c == '[' && depth == 0) { depth = 1; continue; }
            if (c == ']' && depth == 1) break;
            
            if (c == '{') {
                if (depth == 1) objStart = i;
                depth++;
            } else if (c == '}') {
                depth--;
                if (depth == 1) {
                    result.push_back(json.substr(objStart, i - objStart + 1));
                }
            }
        }
        
        return result;
    }
    
    // ========================================
    // XML 解析辅助函数 (用于 .tsx 文件)
    // ========================================
    int getXmlAttrInt(const std::string& xml, const std::string& attr) {
        std::string search = attr + "=\"";
        size_t pos = xml.find(search);
        if (pos == std::string::npos) return 0;
        pos += search.length();
        size_t end = xml.find("\"", pos);
        if (end == std::string::npos) return 0;
        try {
            return std::stoi(xml.substr(pos, end - pos));
        } catch (...) {
            return 0;
        }
    }
    
    std::string getXmlAttrStr(const std::string& xml, const std::string& attr) {
        std::string search = attr + "=\"";
        size_t pos = xml.find(search);
        if (pos == std::string::npos) return "";
        pos += search.length();
        size_t end = xml.find("\"", pos);
        if (end == std::string::npos) return "";
        return xml.substr(pos, end - pos);
    }
    
    // ========================================
    // 解析 Tilesets
    // ========================================
    void parseTilesets(const std::string& json) {
        tilesets.clear();
        
        auto tsObjects = getJsonObjectArray(json, "tilesets");
        std::cout << "  发现 " << tsObjects.size() << " 个 tileset 引用" << std::endl;
        
        for (const auto& tsJson : tsObjects) {
            TilesetInfo ts;
            ts.firstGid = getJsonInt(tsJson, "firstgid");
            
            // 检查是否有 source (外部 .tsx 引用)
            std::string source = getJsonString(tsJson, "source");
            
            if (!source.empty()) {
                // 外部 .tsx 文件
                std::cout << "  → 外部 tileset: " << source << " (firstGid=" << ts.firstGid << ")" << std::endl;
                
                // 跳过 Tiled 内置的 automap-tiles
                if (source.find("automap-tiles") != std::string::npos || 
                    source[0] == ':') {
                    std::cout << "    (跳过内置 tileset)" << std::endl;
                    ts.loaded = false;
                    tilesets.push_back(ts);
                    continue;
                }
                
                // 构建 .tsx 文件完整路径
                std::string tsxPath = normalizePath(tmjBasePath, source);
                std::cout << "    解析路径: " << tsxPath << std::endl;
                
                // 加载 .tsx 文件
                if (loadTsxFile(tsxPath, ts)) {
                    std::cout << "    ✓ 成功加载" << std::endl;
                } else {
                    std::cout << "    ✗ 加载失败" << std::endl;
                }
            } else {
                // 内嵌的 tileset
                ts.tileWidth = getJsonInt(tsJson, "tilewidth");
                ts.tileHeight = getJsonInt(tsJson, "tileheight");
                ts.columns = getJsonInt(tsJson, "columns");
                ts.tileCount = getJsonInt(tsJson, "tilecount");
                ts.imagePath = getJsonString(tsJson, "image");
                
                std::string fullPath = normalizePath(tmjBasePath, ts.imagePath);
                if (ts.texture.loadFromFile(fullPath)) {
                    ts.loaded = true;
                    std::cout << "  → 内嵌 tileset: " << fullPath << " ✓" << std::endl;
                } else {
                    std::cout << "  → 内嵌 tileset: " << fullPath << " ✗" << std::endl;
                }
            }
            
            // 设置默认值
            if (ts.tileWidth == 0) ts.tileWidth = 32;
            if (ts.tileHeight == 0) ts.tileHeight = 32;
            if (ts.columns == 0) ts.columns = 16;
            
            tilesets.push_back(ts);
        }
        
        // 按 firstGid 排序
        std::sort(tilesets.begin(), tilesets.end(), 
            [](const TilesetInfo& a, const TilesetInfo& b) {
                return a.firstGid < b.firstGid;
            });
        
        int loadedCount = 0;
        for (const auto& ts : tilesets) {
            if (ts.loaded) loadedCount++;
        }
        std::cout << "✓ 共 " << tilesets.size() << " 个 tileset, " 
                  << loadedCount << " 个成功加载" << std::endl;
    }
    
    // ========================================
    // 加载外部 .tsx 文件
    // ========================================
    bool loadTsxFile(const std::string& tsxPath, TilesetInfo& ts) {
        std::ifstream file(tsxPath);
        if (!file.is_open()) {
            std::cout << "    无法打开: " << tsxPath << std::endl;
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string xml = buffer.str();
        file.close();
        
        // 获取 .tsx 文件所在目录
        std::string tsxDir = getDirectory(tsxPath);
        
        // 解析 tileset 标签属性
        ts.tileWidth = getXmlAttrInt(xml, "tilewidth");
        ts.tileHeight = getXmlAttrInt(xml, "tileheight");
        ts.columns = getXmlAttrInt(xml, "columns");
        ts.tileCount = getXmlAttrInt(xml, "tilecount");
        
        std::cout << "    瓦片: " << ts.tileWidth << "x" << ts.tileHeight 
                  << ", 列数: " << ts.columns << ", 总数: " << ts.tileCount << std::endl;
        
        // 查找 <image> 标签
        size_t imgPos = xml.find("<image");
        if (imgPos == std::string::npos) {
            std::cout << "    未找到 <image> 标签" << std::endl;
            return false;
        }
        
        // 获取 image 的 source 属性
        size_t imgEnd = xml.find(">", imgPos);
        if (imgEnd == std::string::npos) imgEnd = xml.find("/>", imgPos);
        std::string imgTag = xml.substr(imgPos, imgEnd - imgPos);
        
        std::string imgSource = getXmlAttrStr(imgTag, "source");
        if (imgSource.empty()) {
            std::cout << "    未找到图片 source" << std::endl;
            return false;
        }
        
        // 构建图片完整路径 (相对于 .tsx 文件)
        std::string imgFullPath = normalizePath(tsxDir, imgSource);
        std::cout << "    图片路径: " << imgFullPath << std::endl;
        
        // 加载纹理
        if (ts.texture.loadFromFile(imgFullPath)) {
            ts.loaded = true;
            ts.imagePath = imgFullPath;
            return true;
        }
        
        // 如果失败，尝试其他可能的路径
        std::vector<std::string> altPaths = {
            tsxDir + imgSource,                          // 直接拼接
            tmjBasePath + imgSource,                     // 相对于 .tmj
            "../../assets/" + imgSource,                 // 常见资源目录
        };
        
        for (const auto& altPath : altPaths) {
            if (ts.texture.loadFromFile(altPath)) {
                ts.loaded = true;
                ts.imagePath = altPath;
                std::cout << "    (从备选路径加载: " << altPath << ")" << std::endl;
                return true;
            }
        }
        
        std::cout << "    所有路径尝试失败" << std::endl;
        return false;
    }
    
    // ========================================
    // 解析图层
    // ========================================
    void parseLayers(const std::string& json) {
        layers.clear();
        
        auto layerObjects = getJsonObjectArray(json, "layers");
        
        for (const auto& layerJson : layerObjects) {
            std::string type = getJsonString(layerJson, "type");
            if (type != "tilelayer") continue;
            
            LayerInfo layer;
            layer.name = getJsonString(layerJson, "name");
            layer.data = getJsonIntArray(layerJson, "data");
            
            std::string nameLower = layer.name;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            layer.isCollision = (nameLower.find("collision") != std::string::npos ||
                                 nameLower.find("obstacle") != std::string::npos);
            
            std::cout << "✓ 图层: \"" << layer.name << "\" (" << layer.data.size() << " 瓦片)"
                      << (layer.isCollision ? " [碰撞]" : "") << std::endl;
            
            layers.push_back(layer);
        }
    }
    
    // ========================================
    // 从图层数据初始化内部结构
    // ========================================
    void initializeFromLayers() {
        groundLayer.resize(width * height);
        decorationLayer.resize(width * height);
        collisionLayer.resize(width * height, false);
        
        bool groundFilled = false;
        
        for (const auto& layer : layers) {
            for (size_t i = 0; i < layer.data.size() && i < (size_t)(width * height); i++) {
                int gid = layer.data[i];
                if (gid <= 0) continue;
                
                Tile tile;
                tile.id = gid;
                
                // 找到对应的 tileset (从后往前找，因为按 firstGid 排序)
                int tsIndex = -1;
                for (int t = (int)tilesets.size() - 1; t >= 0; t--) {
                    if (gid >= tilesets[t].firstGid && tilesets[t].loaded) {
                        tsIndex = t;
                        break;
                    }
                }
                
                if (tsIndex >= 0) {
                    tile.textureIndex = tsIndex;
                    const TilesetInfo& ts = tilesets[tsIndex];
                    
                    int localId = gid - ts.firstGid;
                    int cols = ts.columns > 0 ? ts.columns : 16;
                    tile.texCoords.x = (localId % cols) * ts.tileWidth;
                    tile.texCoords.y = (localId / cols) * ts.tileHeight;
                }
                
                // 碰撞层处理
                if (layer.isCollision) {
                    collisionLayer[i] = true;
                } else if (!groundFilled) {
                    groundLayer[i] = tile;
                } else {
                    decorationLayer[i] = tile;
                }
            }
            
            if (!layer.isCollision && !groundFilled) {
                groundFilled = true;
            }
        }
    }

private:
    int width, height;
    int tileSize;
    int srcTileSize;
    int tilesPerRow;
    std::string tmjBasePath;
    
    std::vector<Tile> groundLayer;
    std::vector<Tile> decorationLayer;
    std::vector<bool> collisionLayer;
    std::vector<TilesetInfo> tilesets;
    std::vector<LayerInfo> layers;
};