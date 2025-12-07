#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <iostream>

// 瓦片类型
enum class TileType {
    Ground = 0,
    Obstacle = 1,
    Water = 2
};

struct Tile {
    int id;                  // 原始ID
    int textureIndex;        // 使用哪张贴图 (0=part_1, 1=part_2)
    sf::Vector2i texCoords;  // 纹理坐标
    TileType type;
    
    Tile() : id(0), textureIndex(0), texCoords(0, 0), type(TileType::Ground) {}
};

class TileMap {
public:
    TileMap(int width, int height, int tileSize) 
        : width(width)
        , height(height)
        , tileSize(tileSize)
        , tilesPerRow(16)
    {
        groundLayer.resize(width * height);
        decorationLayer.resize(width * height);
        collisionLayer.resize(width * height, false);
        loadTilesets();
    }
    
    bool loadTilesets() {
        // 加载 part_1 (自然景观) 和 part_2 (农场建筑/道路)
        std::vector<std::string> paths = {
            "../../assets/map/part_1.png",
            "../../assets/map/part_2.png" 
        };
        
        for (const auto& path : paths) {
            sf::Texture tex;
            if (!tex.loadFromFile(path)) {
                std::cerr << "加载失败: " << path << std::endl;
                //即使失败也push一个空纹理防止崩溃
                tilesets.push_back(sf::Texture());
            } else {
                tilesets.push_back(tex);
            }
        }
        return true;
    }
    
    // 设置瓦片（增加了智能判断图集的逻辑）
    void setTile(int x, int y, int rawID, bool isGround) {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        
        int index = y * width + x;
        Tile& tile = isGround ? groundLayer[index] : decorationLayer[index];
        
        tile.id = rawID;
        
        // --- 核心修复逻辑 ---
        // 如果 ID >= 1000，说明是用第二张图 (part_2)
        // 例如：ID 1160 -> 使用 part_2 的第 160 号瓦片
        if (rawID >= 1000) {
            tile.textureIndex = 1; // 使用 part_2.png
            int localID = rawID - 1000;
            tile.texCoords.x = (localID % tilesPerRow) * 32;
            tile.texCoords.y = (localID / tilesPerRow) * 32;
        } else {
            tile.textureIndex = 0; // 使用 part_1.png
            tile.texCoords.x = (rawID % tilesPerRow) * 32;
            tile.texCoords.y = (rawID / tilesPerRow) * 32;
        }
        
        // 简单的碰撞逻辑
        // 树木(100-199) 或 篱笆(>1000 且不是路) 视为障碍
        // 这里简化处理：只要不是空气(-1)且不是地面层，大部分都是障碍
        if (!isGround && rawID != -1) {
            // 排除一些特定的非障碍物 (比如花草)
            if (rawID == 67 || rawID == 68 || rawID == 48) {
                 tile.type = TileType::Ground;
                 collisionLayer[index] = false;
            } else {
                 tile.type = TileType::Obstacle;
                 collisionLayer[index] = true;
            }
        } else {
            tile.type = TileType::Ground;
            collisionLayer[index] = false;
        }
    }
    
    void initializeFromArray(const std::vector<std::vector<int>>& ground, 
                           const std::vector<std::vector<int>>& decor) {
        for (int y = 0; y < height && y < ground.size(); y++) {
            for (int x = 0; x < width && x < ground[y].size(); x++) {
                setTile(x, y, ground[y][x], true);
                if (y < decor.size() && x < decor[y].size()) {
                    setTile(x, y, decor[y][x], false);
                }
            }
        }
    }
    
    void render(sf::RenderWindow& window, const sf::View& view) {
        // 计算可见区域 (Culling)
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
                drawTile(window, groundLayer[y*width+x], x, y);
            }
        }
        // 绘制装饰
        for (int y = startY; y < endY; y++) {
            for (int x = startX; x < endX; x++) {
                if (decorationLayer[y*width+x].id != -1)
                    drawTile(window, decorationLayer[y*width+x], x, y);
            }
        }
    }
    
    void drawTile(sf::RenderWindow& window, const Tile& tile, int x, int y) {
        if (tile.textureIndex >= tilesets.size()) return;
        
        sf::Sprite s;
        s.setTexture(tilesets[tile.textureIndex]); // 自动切换贴图
        s.setTextureRect(sf::IntRect(tile.texCoords.x, tile.texCoords.y, 32, 32));
        s.setPosition(x * tileSize, y * tileSize);
        s.setScale(tileSize / 32.0f, tileSize / 32.0f);
        window.draw(s);
    }
    
    bool isColliding(const sf::FloatRect& box) const {
        int left = std::max(0, (int)(box.left / tileSize));
        int top = std::max(0, (int)(box.top / tileSize));
        int right = std::min(width-1, (int)((box.left+box.width)/tileSize));
        int bottom = std::min(height-1, (int)((box.top+box.height)/tileSize));
        
        for(int y=top; y<=bottom; y++) {
            for(int x=left; x<=right; x++) {
                if(collisionLayer[y*width+x]) return true;
            }
        }
        return false;
    }
    
    sf::Vector2i getMapSize() const { return sf::Vector2i(width*tileSize, height*tileSize); }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    int width, height, tileSize, tilesPerRow;
    std::vector<Tile> groundLayer;
    std::vector<Tile> decorationLayer;
    std::vector<bool> collisionLayer;
    std::vector<sf::Texture> tilesets;
};