#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

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

// 地图对象（用于对象层中的树木、建筑等）
struct MapObject {
    int gid;
    float x, y;
    float width, height;
    std::string name;
    std::string type;
    int textureIndex;
    sf::Vector2i texCoords;
    
    MapObject() : gid(0), x(0), y(0), width(0), height(0), textureIndex(-1), texCoords(0, 0) {}
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