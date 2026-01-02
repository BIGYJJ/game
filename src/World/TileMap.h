#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <iostream>

// ============================================================================
// TileMap - Supports loading Tiled exported .tmj/.json files
// 
// Supports external .tsx tileset references (Tiled recommended method)
// Supports object layers (objectgroup) for placing individual objects like trees
// 
// Usage:
//   TileMap tileMap;
//   tileMap.loadFromTiled("assets/map/farm.tmj", 48);
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

// 从tsx文件读取的Tile属性（用于树木、建筑、植物等对象）
struct TileProperty {
    int localId;                    // tile在tileset中的ID
    std::string name;               // 例如 "tree1", "apple_tree", "carrot_plant"
    std::string type;               // 例如 "tree", "stone_build", "plant"
    std::string base;               // 基类类型 "build", "plant" 等
    int hp;                         // 生命值
    int defense;                    // 防御力
    int dropMax;                    // 最大掉落数量
    std::vector<std::string> dropTypes;         // 掉落物品类型列表
    std::vector<float> dropProbabilities;       // 各物品掉落概率
    std::string imagePath;          // 该tile的图片路径
    
    // 击杀奖励
    int expMin;                     // 最小经验
    int expMax;                     // 最大经验
    int goldMin;                    // 最小金币
    int goldMax;                    // 最大金币
    
    // === 植物相关属性（wild_plants类型使用）===
    bool allowPickup;               // 是否允许拾取
    std::string pickupObject;       // 拾取后获得的物品ID
    int countMin;                   // 最小数量
    int countMax;                   // 最大数量
    float probability;              // 生成概率 (0-1)
    
    // === 碰撞盒（从objectgroup读取）===
    bool hasCollisionBox;           // 是否有自定义碰撞盒
    float collisionX;               // 碰撞盒X偏移
    float collisionY;               // 碰撞盒Y偏移
    float collisionWidth;           // 碰撞盒宽度
    float collisionHeight;          // 碰撞盒高度
    
    // 贴图（使用shared_ptr避免复制时失效）
    std::shared_ptr<sf::Texture> texture;
    bool hasTexture;                // 是否有贴图
    
    // === Spritesheet支持（仅用于记录原始位置）===
    sf::IntRect textureRect;        // 在spritesheet中的位置（供调试）
    
    TileProperty() : localId(0), hp(30), defense(5), dropMax(3), 
                     expMin(0), expMax(0), goldMin(0), goldMax(0),
                     allowPickup(false), countMin(1), countMax(1), probability(1.0f),
                     hasCollisionBox(false), collisionX(0), collisionY(0), 
                     collisionWidth(0), collisionHeight(0), hasTexture(false),
                     textureRect(0, 0, 32, 32) {}
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
    std::string name;                           // tileset名称（如"tree"）
    std::vector<TileProperty> tileProperties;   // 存储每个tile的属性
    
    TilesetInfo() : firstGid(1), tileWidth(32), tileHeight(32), 
                    columns(16), tileCount(256), loaded(false) {}
    
    // 根据localId查找tile属性
    const TileProperty* getTileProperty(int localId) const {
        for (const auto& prop : tileProperties) {
            if (prop.localId == localId) return &prop;
        }
        return nullptr;
    }
};

struct LayerInfo {
    std::string name;
    std::vector<int> data;
    bool isCollision;
};

// 地图对象（用于对象层中的树木、建筑等）
struct MapObject {
    int gid;
    float x, y;
    float width, height;
    std::string name;
    std::string type;
    int textureIndex;
    sf::Vector2i texCoords;
    
    // 从tsx文件读取的属性
    const TileProperty* tileProperty;  // 指向对应的tile属性
    
    MapObject() : gid(0), x(0), y(0), width(0), height(0), 
                  textureIndex(-1), texCoords(0, 0), tileProperty(nullptr) {}
};

class TileMap {
public:
    TileMap();
    TileMap(int w, int h, int displayTileSize);
    
    // ========================================
    // Load from Tiled .tmj file
    // displayTileSize: tile size for display (0 = use original size)
    // ========================================
    bool loadFromTiled(const std::string& tmjPath, int displayTileSize = 0);
    
    // ========================================
    // Original array initialization (kept for compatibility)
    // ========================================
    bool loadTilesets();
    void setTile(int x, int y, int rawID, bool isGround);
    void initializeFromArray(const std::vector<std::vector<int>>& ground, 
                            const std::vector<std::vector<int>>& decor);
    
    // ========================================
    // Render
    // ========================================
    void render(sf::RenderWindow& window, const sf::View& view);
    
    // ========================================
    // Collision detection
    // ========================================
    bool isColliding(const sf::FloatRect& box) const;
    
    // ========================================
    // Getters
    // ========================================
    sf::Vector2i getMapSize() const;
    int getWidth() const;
    int getHeight() const;
    int getTileSize() const;
    const std::vector<MapObject>& getObjects() const;
    
    // 根据gid获取tile属性
    const TileProperty* getTilePropertyByGid(int gid) const;
    
    // 【调试】打印剩余的对象列表
    void debugPrintRemainingObjects() const {
        std::cout << "[TileMap DEBUG] Remaining objects: " << objects.size() << std::endl;
        for (const auto& obj : objects) {
            std::cout << "  - gid=" << obj.gid 
                      << " name=\"" << obj.name << "\""
                      << " type=\"" << obj.type << "\""
                      << " at (" << obj.x << "," << obj.y << ")"
                      << " size=" << obj.width << "x" << obj.height;
            if (obj.tileProperty) {
                std::cout << " [prop: " << obj.tileProperty->name << "/" << obj.tileProperty->type << "]";
            }
            std::cout << std::endl;
        }
    }
    
    // 清除对象（当TreeManager接管树木渲染后调用，避免重复渲染）
    void clearObjects() { objects.clear(); }
    
    // 移除树木类型的对象（当TreeManager接管树木渲染后调用）
    void removeTreeObjects() {
        objects.erase(
            std::remove_if(objects.begin(), objects.end(), 
                [this](const MapObject& obj) {
                    // 方法1：通过 tileProperty 识别
                    if (obj.tileProperty) {
                        if (obj.tileProperty->type == "tree") return true;
                        if (obj.tileProperty->name.find("tree") != std::string::npos) return true;
                    }
                    // 方法2：通过对象自身的 type/name 识别
                    if (obj.type == "tree") return true;
                    if (obj.name.find("tree") != std::string::npos) return true;
                    // 方法3：通过 gid 范围识别 (tree.tsx 的 firstGid)
                    // 查找 tree tileset
                    for (const auto& ts : tilesets) {
                        if (ts.name == "tree" && obj.gid >= ts.firstGid && 
                            obj.gid < ts.firstGid + ts.tileCount) {
                            return true;
                        }
                    }
                    return false;
                }),
            objects.end()
        );
        std::cout << "[TileMap] Removed tree objects, remaining: " << objects.size() << std::endl;
    }
    
    // 移除石头建筑类型的对象（当StoneBuildManager接管渲染后调用）
    void removeStoneObjects() {
        objects.erase(
            std::remove_if(objects.begin(), objects.end(), 
                [](const MapObject& obj) {
                    if (!obj.tileProperty) return false;
                    return obj.tileProperty->type == "stone_build" ||
                           obj.tileProperty->name.find("stone_build") != std::string::npos;
                }),
            objects.end()
        );
    }
    
    // 移除野生植物类型的对象（当WildPlantManager接管渲染后调用）
    void removeWildPlantObjects() {
        objects.erase(
            std::remove_if(objects.begin(), objects.end(), 
                [](const MapObject& obj) {
                    if (!obj.tileProperty) return false;
                    return obj.tileProperty->type == "wild_plants" ||
                           obj.tileProperty->name.find("carrot") != std::string::npos ||
                           obj.tileProperty->name.find("bean") != std::string::npos;
                }),
            objects.end()
        );
    }

private:
    // ========================================
    // Path handling utilities
    // ========================================
    std::string getDirectory(const std::string& path);
    std::string cleanPath(const std::string& path);
    std::string normalizePath(const std::string& basePath, const std::string& relativePath);
    
    // ========================================
    // JSON parsing helper functions
    // ========================================
    int getJsonInt(const std::string& json, const std::string& key);
    float getJsonFloat(const std::string& json, const std::string& key);
    std::string getJsonString(const std::string& json, const std::string& key);
    std::vector<int> getJsonIntArray(const std::string& json, const std::string& key);
    std::vector<std::string> getJsonObjectArray(const std::string& json, const std::string& key);
    
    // ========================================
    // XML parsing helper functions (for .tsx files)
    // ========================================
    int getXmlAttrInt(const std::string& xml, const std::string& attr);
    std::string getXmlAttrStr(const std::string& xml, const std::string& attr);
    
    // ========================================
    // Internal parsing functions
    // ========================================
    void parseTilesets(const std::string& json);
    bool loadTsxFile(const std::string& tsxPath, TilesetInfo& ts);
    void parseLayers(const std::string& json);
    void parseObjectGroups(const std::string& json);
    void initializeFromLayers();
    
    // ========================================
    // Rendering helpers
    // ========================================
    void drawTile(sf::RenderWindow& window, const Tile& tile, int x, int y);
    void renderObjects(sf::RenderWindow& window, const sf::View& view);
    void removeObjectsByType(const std::string& type);
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
    std::vector<MapObject> objects;  // 对象层中的对象
};