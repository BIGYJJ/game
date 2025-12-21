#include "TileMap.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

// ============================================================================
// Constructors
// ============================================================================

TileMap::TileMap() 
    : width(0), height(0), tileSize(32), srcTileSize(32), tilesPerRow(16)
{}

TileMap::TileMap(int w, int h, int displayTileSize) 
    : width(w), height(h), tileSize(displayTileSize), srcTileSize(32), tilesPerRow(16)
{
    groundLayer.resize(width * height);
    decorationLayer.resize(width * height);
    collisionLayer.resize(width * height, false);
}

// ============================================================================
// Load from Tiled .tmj file
// ============================================================================

bool TileMap::loadFromTiled(const std::string& tmjPath, int displayTileSize) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Loading Tiled map: " << tmjPath << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Read file
    std::ifstream file(tmjPath);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open map file: " << tmjPath << std::endl;
        std::cerr << "[DEBUG] Please check:" << std::endl;
        std::cerr << "  1. File exists at this path" << std::endl;
        std::cerr << "  2. Working directory is correct" << std::endl;
        std::cerr << "  3. File has read permissions" << std::endl;
        return false;
    }
    
    std::cout << "[OK] File opened successfully" << std::endl;
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json = buffer.str();
    file.close();
    
    std::cout << "[DEBUG] File size: " << json.size() << " bytes" << std::endl;
    
    if (json.empty()) {
        std::cerr << "[ERROR] File is empty!" << std::endl;
        return false;
    }
    
    // Get directory of .tmj file
    tmjBasePath = getDirectory(tmjPath);
    std::cout << "[DEBUG] Base path: " << tmjBasePath << std::endl;
    
    // Parse basic map info
    width = getJsonInt(json, "width");
    height = getJsonInt(json, "height");
    srcTileSize = getJsonInt(json, "tilewidth");
    
    if (width == 0 || height == 0) {
        std::cerr << "[ERROR] Failed to parse map dimensions!" << std::endl;
        std::cerr << "[DEBUG] width=" << width << ", height=" << height << std::endl;
        std::cerr << "[DEBUG] First 200 chars of file:" << std::endl;
        std::cerr << json.substr(0, 200) << std::endl;
        return false;
    }
    
    tileSize = (displayTileSize > 0) ? displayTileSize : srcTileSize;
    
    std::cout << "[OK] Map size: " << width << "x" << height << " tiles" << std::endl;
    std::cout << "[OK] Source tile size: " << srcTileSize << "x" << srcTileSize << std::endl;
    std::cout << "[OK] Display tile size: " << tileSize << "x" << tileSize << std::endl;
    
    // Parse tilesets
    parseTilesets(json);
    
    // Parse tile layers
    parseLayers(json);
    
    // Parse object groups (trees, buildings, etc.)
    parseObjectGroups(json);
    
    // Initialize internal data
    initializeFromLayers();
    
    std::cout << "[OK] Map pixel size: " << getMapSize().x << "x" << getMapSize().y << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    return true;
}

// ============================================================================
// Original array initialization (kept for compatibility)
// ============================================================================

bool TileMap::loadTilesets() {
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
            std::cerr << "Failed to load: " << paths[i] << std::endl;
        }
        tilesets.push_back(ts);
    }
    return true;
}

void TileMap::setTile(int x, int y, int rawID, bool isGround) {
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

void TileMap::initializeFromArray(const std::vector<std::vector<int>>& ground, 
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

// ============================================================================
// Render
// ============================================================================

void TileMap::render(sf::RenderWindow& window, const sf::View& view) {
    sf::FloatRect bounds(
        view.getCenter().x - view.getSize().x/2,
        view.getCenter().y - view.getSize().y/2,
        view.getSize().x, view.getSize().y
    );
    
    int startX = std::max(0, (int)(bounds.left / tileSize) - 1);
    int startY = std::max(0, (int)(bounds.top / tileSize) - 1);
    int endX = std::min(width, (int)((bounds.left + bounds.width) / tileSize) + 2);
    int endY = std::min(height, (int)((bounds.top + bounds.height) / tileSize) + 2);
    
    // Draw ground layer
    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            int idx = y * width + x;
            if (idx < (int)groundLayer.size() && groundLayer[idx].id > 0) {
                drawTile(window, groundLayer[idx], x, y);
            }
        }
    }
    
    // Draw decoration layer
    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            int idx = y * width + x;
            if (idx < (int)decorationLayer.size() && decorationLayer[idx].id > 0) {
                drawTile(window, decorationLayer[idx], x, y);
            }
        }
    }
    
    // Draw objects (trees, buildings, etc.)
    renderObjects(window, view);
}

void TileMap::drawTile(sf::RenderWindow& window, const Tile& tile, int x, int y) {
    if (tile.textureIndex < 0 || tile.textureIndex >= (int)tilesets.size()) return;
    
    const TilesetInfo& ts = tilesets[tile.textureIndex];
    if (!ts.loaded) return;
    
    sf::Sprite s;
    s.setTexture(ts.texture);
    s.setTextureRect(sf::IntRect(tile.texCoords.x, tile.texCoords.y, 
                                  ts.tileWidth, ts.tileHeight));
    s.setPosition((float)(x * tileSize), (float)(y * tileSize));
    
    float scale = (float)tileSize / ts.tileWidth;
    s.setScale(scale, scale);
    
    window.draw(s);
}

void TileMap::renderObjects(sf::RenderWindow& window, const sf::View& view) {
    // Calculate view bounds for culling
    sf::FloatRect bounds(
        view.getCenter().x - view.getSize().x/2 - 100,  // Extra margin for large objects
        view.getCenter().y - view.getSize().y/2 - 100,
        view.getSize().x + 200,
        view.getSize().y + 200
    );
    
    float scale = (float)tileSize / srcTileSize;
    
    for (const auto& obj : objects) {
        if (obj.textureIndex < 0 || obj.textureIndex >= (int)tilesets.size()) continue;
        
        const TilesetInfo& ts = tilesets[obj.textureIndex];
        if (!ts.loaded) continue;
        
        // Calculate draw position
        // Tiled objects have y at the bottom of the object
        float drawX = obj.x * scale;
        float drawY = (obj.y - obj.height) * scale;
        
        // Simple view culling
        if (drawX + obj.width * scale < bounds.left || 
            drawX > bounds.left + bounds.width ||
            drawY + obj.height * scale < bounds.top || 
            drawY > bounds.top + bounds.height) {
            continue;
        }
        
        sf::Sprite sprite;
        sprite.setTexture(ts.texture);
        sprite.setTextureRect(sf::IntRect(obj.texCoords.x, obj.texCoords.y, 
                                          (int)obj.width, (int)obj.height));
        sprite.setPosition(drawX, drawY);
        sprite.setScale(scale, scale);
        
        window.draw(sprite);
    }
}

// ============================================================================
// Collision detection
// ============================================================================

bool TileMap::isColliding(const sf::FloatRect& box) const {
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
    
    // Also check collision with objects
    float scale = (float)tileSize / srcTileSize;
    for (const auto& obj : objects) {
        if (obj.gid <= 0) continue;
        
        // Object collision box (in display coordinates)
        sf::FloatRect objBox(
            obj.x * scale,
            (obj.y - obj.height) * scale,
            obj.width * scale,
            obj.height * scale
        );
        
        if (box.intersects(objBox)) {
            return true;
        }
    }
    
    return false;
}

// ============================================================================
// Getters
// ============================================================================

sf::Vector2i TileMap::getMapSize() const { 
    return sf::Vector2i(width * tileSize, height * tileSize); 
}

int TileMap::getWidth() const { return width; }
int TileMap::getHeight() const { return height; }
int TileMap::getTileSize() const { return tileSize; }

const std::vector<MapObject>& TileMap::getObjects() const {
    return objects;
}

// ============================================================================
// Path handling utilities
// ============================================================================

std::string TileMap::getDirectory(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return "./";
    return path.substr(0, pos + 1);
}

std::string TileMap::cleanPath(const std::string& path) {
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

std::string TileMap::normalizePath(const std::string& basePath, const std::string& relativePath) {
    std::string cleanRelative = cleanPath(relativePath);
    
    // If absolute path or special path, return directly
    if (cleanRelative.empty() || cleanRelative[0] == ':') {
        return cleanRelative;
    }
    
    std::string result = basePath + cleanRelative;
    
    // Simple handling of ../
    size_t pos;
    while ((pos = result.find("/../")) != std::string::npos) {
        if (pos == 0) break;
        size_t prevSlash = result.rfind('/', pos - 1);
        if (prevSlash == std::string::npos) prevSlash = 0;
        result = result.substr(0, prevSlash) + result.substr(pos + 3);
    }
    
    // Handle ./
    while ((pos = result.find("/./")) != std::string::npos) {
        result = result.substr(0, pos) + result.substr(pos + 2);
    }
    
    return result;
}

// ============================================================================
// JSON parsing helper functions
// ============================================================================

int TileMap::getJsonInt(const std::string& json, const std::string& key) {
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

float TileMap::getJsonFloat(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t searchStart = 0;
    
    while (searchStart < json.size()) {
        size_t pos = json.find(search, searchStart);
        if (pos == std::string::npos) return 0.0f;
        
        // 检查是否在数组内
        int arrayDepth = 0;
        for (size_t i = 0; i < pos; i++) {
            if (json[i] == '[') arrayDepth++;
            else if (json[i] == ']') arrayDepth--;
        }
        
        if (arrayDepth == 0) {
            pos = json.find(":", pos);
            if (pos == std::string::npos) return 0.0f;
            
            pos++;
            while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n')) pos++;
            
            std::string numStr;
            while (pos < json.size() && (isdigit(json[pos]) || json[pos] == '.' || json[pos] == '-')) {
                numStr += json[pos++];
            }
            
            return numStr.empty() ? 0.0f : std::stof(numStr);
        }
        
        searchStart = pos + 1;
    }
    
    return 0.0f;
}

std::string TileMap::getJsonString(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t searchStart = 0;
    
    // 查找顶级属性（不在嵌套数组内的）
    while (searchStart < json.size()) {
        size_t pos = json.find(search, searchStart);
        if (pos == std::string::npos) return "";
        
        // 检查这个位置是否在数组内（通过计算之前的 [ 和 ] 数量）
        int arrayDepth = 0;
        for (size_t i = 0; i < pos; i++) {
            if (json[i] == '[') arrayDepth++;
            else if (json[i] == ']') arrayDepth--;
        }
        
        // 如果不在数组内（arrayDepth == 0），这是顶级属性
        if (arrayDepth == 0) {
            pos = json.find(":", pos);
            if (pos == std::string::npos) return "";
            
            size_t start = json.find("\"", pos + 1);
            if (start == std::string::npos) return "";
            
            // Handle escape characters
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
        
        // 继续搜索下一个匹配
        searchStart = pos + 1;
    }
    
    return "";
}

std::vector<int> TileMap::getJsonIntArray(const std::string& json, const std::string& key) {
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

std::vector<std::string> TileMap::getJsonObjectArray(const std::string& json, const std::string& key) {
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

// ============================================================================
// XML parsing helper functions (for .tsx files)
// ============================================================================

int TileMap::getXmlAttrInt(const std::string& xml, const std::string& attr) {
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

std::string TileMap::getXmlAttrStr(const std::string& xml, const std::string& attr) {
    std::string search = attr + "=\"";
    size_t pos = xml.find(search);
    if (pos == std::string::npos) return "";
    pos += search.length();
    size_t end = xml.find("\"", pos);
    if (end == std::string::npos) return "";
    return xml.substr(pos, end - pos);
}

// ============================================================================
// Parse Tilesets
// ============================================================================

void TileMap::parseTilesets(const std::string& json) {
    tilesets.clear();
    
    auto tsObjects = getJsonObjectArray(json, "tilesets");
    std::cout << "[DEBUG] Found " << tsObjects.size() << " tileset reference(s)" << std::endl;
    
    if (tsObjects.empty()) {
        std::cerr << "[WARNING] No tilesets found in map file!" << std::endl;
    }
    
    for (const auto& tsJson : tsObjects) {
        TilesetInfo ts;
        ts.firstGid = getJsonInt(tsJson, "firstgid");
        
        // Check if has source (external .tsx reference)
        std::string source = getJsonString(tsJson, "source");
        
        if (!source.empty()) {
            // External .tsx file
            std::cout << "  -> External tileset: " << source << " (firstGid=" << ts.firstGid << ")" << std::endl;
            
            // Skip Tiled built-in automap-tiles
            if (source.find("automap-tiles") != std::string::npos || 
                source[0] == ':') {
                std::cout << "     (Skipping built-in tileset)" << std::endl;
                ts.loaded = false;
                tilesets.push_back(ts);
                continue;
            }
            
            // Build full .tsx file path
            std::string tsxPath = normalizePath(tmjBasePath, source);
            std::cout << "     Resolved path: " << tsxPath << std::endl;
            
            // Load .tsx file
            if (loadTsxFile(tsxPath, ts)) {
                std::cout << "     [OK] Loaded successfully" << std::endl;
            } else {
                std::cout << "     [FAILED] Load failed" << std::endl;
            }
        } else {
            // Embedded tileset
            ts.tileWidth = getJsonInt(tsJson, "tilewidth");
            ts.tileHeight = getJsonInt(tsJson, "tileheight");
            ts.columns = getJsonInt(tsJson, "columns");
            ts.tileCount = getJsonInt(tsJson, "tilecount");
            ts.imagePath = getJsonString(tsJson, "image");
            
            std::string fullPath = normalizePath(tmjBasePath, ts.imagePath);
            std::cout << "  -> Embedded tileset image: " << fullPath << std::endl;
            
            if (ts.texture.loadFromFile(fullPath)) {
                ts.loaded = true;
                std::cout << "     [OK] Texture loaded" << std::endl;
            } else {
                std::cout << "     [FAILED] Texture load failed" << std::endl;
            }
        }
        
        // Set default values
        if (ts.tileWidth == 0) ts.tileWidth = 32;
        if (ts.tileHeight == 0) ts.tileHeight = 32;
        if (ts.columns == 0) ts.columns = 16;
        
        tilesets.push_back(ts);
    }
    
    // Sort by firstGid
    std::sort(tilesets.begin(), tilesets.end(), 
        [](const TilesetInfo& a, const TilesetInfo& b) {
            return a.firstGid < b.firstGid;
        });
    
    int loadedCount = 0;
    for (const auto& ts : tilesets) {
        if (ts.loaded) loadedCount++;
    }
    std::cout << "[OK] Total " << tilesets.size() << " tileset(s), " 
              << loadedCount << " loaded successfully" << std::endl;
              
    if (loadedCount == 0 && !tilesets.empty()) {
        std::cerr << "[WARNING] No tilesets loaded! Map will appear blank." << std::endl;
    }
}

// ============================================================================
// Load external .tsx file
// ============================================================================

bool TileMap::loadTsxFile(const std::string& tsxPath, TilesetInfo& ts) {
    std::ifstream file(tsxPath);
    if (!file.is_open()) {
        std::cout << "     Cannot open: " << tsxPath << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string xml = buffer.str();
    file.close();
    
    // Get directory of .tsx file
    std::string tsxDir = getDirectory(tsxPath);
    
    // Parse tileset tag attributes
    ts.tileWidth = getXmlAttrInt(xml, "tilewidth");
    ts.tileHeight = getXmlAttrInt(xml, "tileheight");
    ts.columns = getXmlAttrInt(xml, "columns");
    ts.tileCount = getXmlAttrInt(xml, "tilecount");
    ts.name = getXmlAttrStr(xml, "name");
    
    std::cout << "     Tileset name: " << ts.name << std::endl;
    std::cout << "     Tile: " << ts.tileWidth << "x" << ts.tileHeight 
              << ", columns: " << ts.columns << ", count: " << ts.tileCount << std::endl;
    
    // ========================================
    // 解析所有 <tile> 元素及其属性
    // ========================================
    size_t searchPos = 0;
    while (true) {
        size_t tilePos = xml.find("<tile", searchPos);
        if (tilePos == std::string::npos) break;
        
        // 找到这个tile的结束位置
        size_t tileEnd = xml.find("</tile>", tilePos);
        if (tileEnd == std::string::npos) {
            // 可能是自闭合标签
            tileEnd = xml.find("/>", tilePos);
            if (tileEnd == std::string::npos) break;
            tileEnd += 2;
        } else {
            tileEnd += 7; // length of "</tile>"
        }
        
        std::string tileXml = xml.substr(tilePos, tileEnd - tilePos);
        
        TileProperty prop;
        prop.localId = getXmlAttrInt(tileXml, "id");
        
        // 解析 <properties> 中的所有属性
        size_t propsStart = tileXml.find("<properties>");
        size_t propsEnd = tileXml.find("</properties>");
        
        if (propsStart != std::string::npos && propsEnd != std::string::npos) {
            std::string propsXml = tileXml.substr(propsStart, propsEnd - propsStart);
            
            // 查找所有 <property> 元素
            size_t propPos = 0;
            while (true) {
                propPos = propsXml.find("<property", propPos);
                if (propPos == std::string::npos) break;
                
                size_t propEnd = propsXml.find("/>", propPos);
                if (propEnd == std::string::npos) propEnd = propsXml.find(">", propPos);
                
                std::string propTag = propsXml.substr(propPos, propEnd - propPos);
                
                std::string propName = getXmlAttrStr(propTag, "name");
                std::string propValue = getXmlAttrStr(propTag, "value");
                
                // 解析各种属性
                if (propName == "name") {
                    prop.name = propValue;
                } else if (propName == "type") {
                    prop.type = propValue;
                } else if (propName == "base") {
                    prop.base = propValue;
                } else if (propName == "HP") {
                    prop.hp = std::stoi(propValue);
                } else if (propName == "defense") {
                    prop.defense = std::stoi(propValue);
                } else if (propName == "drop_max") {
                    prop.dropMax = (int)std::stof(propValue);
                } else if (propName == "exp_min") {
                    prop.expMin = std::stoi(propValue);
                } else if (propName == "exp_max") {
                    prop.expMax = std::stoi(propValue);
                } else if (propName == "gold_min") {
                    prop.goldMin = std::stoi(propValue);
                } else if (propName == "gold_max") {
                    prop.goldMax = std::stoi(propValue);
                } else if (propName == "allow_pick") {
                    prop.allowPickup = (propValue == "true" || propValue == "1");
                } else if (propName == "pickup_object") {
                    // 移除引号
                    prop.pickupObject = propValue;
                    if (prop.pickupObject.size() >= 2 && 
                        prop.pickupObject.front() == '"' && prop.pickupObject.back() == '"') {
                        prop.pickupObject = prop.pickupObject.substr(1, prop.pickupObject.size() - 2);
                    }
                    // 也添加到dropTypes中
                    if (!prop.pickupObject.empty()) {
                        prop.dropTypes.push_back(prop.pickupObject);
                    }
                } else if (propName == "count_min") {
                    prop.countMin = std::stoi(propValue);
                } else if (propName == "count_max") {
                    prop.countMax = std::stoi(propValue);
                } else if (propName == "probability") {
                    prop.probability = std::stof(propValue);
                } else if (propName == "drop_type") {
                    // 解析掉落类型列表，格式: "\"wood\",\"seed\",\"stick\""
                    std::string dropStr = propValue;
                    size_t pos = 0;
                    while (pos < dropStr.length()) {
                        size_t start = dropStr.find("\"", pos);
                        if (start == std::string::npos) break;
                        size_t end = dropStr.find("\"", start + 1);
                        if (end == std::string::npos) break;
                        std::string item = dropStr.substr(start + 1, end - start - 1);
                        if (!item.empty()) {
                            prop.dropTypes.push_back(item);
                        }
                        pos = end + 1;
                    }
                } else if (propName.find("drop") != std::string::npos && 
                           propName.find("Probability") != std::string::npos) {
                    // dropX_Probability
                    float prob = std::stof(propValue);
                    prop.dropProbabilities.push_back(prob);
                }
                
                propPos = propEnd;
            }
        }
        
        // 解析这个tile的 <image> 标签
        size_t imgPos = tileXml.find("<image");
        if (imgPos != std::string::npos) {
            size_t imgEnd = tileXml.find("/>", imgPos);
            if (imgEnd == std::string::npos) imgEnd = tileXml.find(">", imgPos);
            std::string imgTag = tileXml.substr(imgPos, imgEnd - imgPos);
            prop.imagePath = getXmlAttrStr(imgTag, "source");
        }
        
        // 输出调试信息
        if (!prop.name.empty()) {
            std::cout << "     [Tile " << prop.localId << "] name=" << prop.name 
                      << ", type=" << prop.type << ", HP=" << prop.hp 
                      << ", defense=" << prop.defense << std::endl;
            if (!prop.dropTypes.empty()) {
                std::cout << "       drops: ";
                for (size_t i = 0; i < prop.dropTypes.size(); i++) {
                    std::cout << prop.dropTypes[i];
                    if (i < prop.dropProbabilities.size()) {
                        std::cout << "(" << (int)(prop.dropProbabilities[i] * 100) << "%)";
                    }
                    if (i < prop.dropTypes.size() - 1) std::cout << ", ";
                }
                std::cout << std::endl;
            }
        }
        
        ts.tileProperties.push_back(prop);
        searchPos = tileEnd;
    }
    
    std::cout << "     Parsed " << ts.tileProperties.size() << " tile properties" << std::endl;
    
    // ========================================
    // 处理 "collection of images" tileset (columns == 0)
    // 每个 tile 有独立的图片文件
    // 使用 shared_ptr 避免 vector 重新分配时贴图失效
    // ========================================
    if (ts.columns == 0) {
        std::cout << "     [INFO] Collection of images tileset detected" << std::endl;
        
        // 为每个 tile 加载独立的贴图
        bool anyLoaded = false;
        for (auto& prop : ts.tileProperties) {
            if (prop.imagePath.empty()) continue;
            
            // 创建 shared_ptr 管理的贴图
            prop.texture = std::make_shared<sf::Texture>();
            
            std::string imgFullPath = normalizePath(tsxDir, prop.imagePath);
            
            // 尝试加载
            if (prop.texture->loadFromFile(imgFullPath)) {
                prop.hasTexture = true;
                anyLoaded = true;
                std::cout << "     [Tile " << prop.localId << "] Loaded: " << imgFullPath << std::endl;
                continue;
            }
            
            // 尝试替代路径
            std::vector<std::string> altPaths = {
                tsxDir + prop.imagePath,
                tmjBasePath + "../game_source/" + prop.imagePath,
                "assets/game_source/" + prop.imagePath,
            };
            
            for (const auto& altPath : altPaths) {
                if (prop.texture->loadFromFile(altPath)) {
                    prop.hasTexture = true;
                    anyLoaded = true;
                    std::cout << "     [Tile " << prop.localId << "] Loaded: " << altPath << std::endl;
                    break;
                }
            }
            
            if (!prop.hasTexture) {
                prop.texture.reset();  // 释放未成功加载的贴图
                std::cout << "     [Tile " << prop.localId << "] FAILED to load texture" << std::endl;
            }
        }
        
        // 使用第一个 tile 的贴图作为 tileset 的默认贴图（用于向后兼容）
        if (!ts.tileProperties.empty() && ts.tileProperties[0].hasTexture && ts.tileProperties[0].texture) {
            ts.texture = *ts.tileProperties[0].texture;
            ts.imagePath = ts.tileProperties[0].imagePath;
        }
        
        ts.loaded = anyLoaded;
        ts.columns = 1;  // 标记为 collection 类型
        return anyLoaded;
    }
    
    // ========================================
    // Standard spritesheet tileset
    // ========================================
    size_t imgPos = xml.find("<image");
    if (imgPos == std::string::npos) {
        std::cout << "     <image> tag not found" << std::endl;
        return false;
    }
    
    size_t imgEnd = xml.find(">", imgPos);
    if (imgEnd == std::string::npos) imgEnd = xml.find("/>", imgPos);
    std::string imgTag = xml.substr(imgPos, imgEnd - imgPos);
    
    std::string imgSource = getXmlAttrStr(imgTag, "source");
    if (imgSource.empty()) {
        std::cout << "     Image source not found" << std::endl;
        return false;
    }
    
    std::string imgFullPath = normalizePath(tsxDir, imgSource);
    std::cout << "     Image path: " << imgFullPath << std::endl;
    
    if (ts.texture.loadFromFile(imgFullPath)) {
        ts.loaded = true;
        ts.imagePath = imgFullPath;
        
        // ========================================
        // 为所有TileProperty创建独立贴图（从spritesheet提取）
        // ========================================
        sf::Image fullImage = ts.texture.copyToImage();
        
        for (auto& prop : ts.tileProperties) {
            // 计算在spritesheet中的位置
            int cols = ts.columns > 0 ? ts.columns : 1;
            int texX = (prop.localId % cols) * ts.tileWidth;
            int texY = (prop.localId / cols) * ts.tileHeight;
            prop.textureRect = sf::IntRect(texX, texY, ts.tileWidth, ts.tileHeight);
            
            // 检查边界
            if (texX + ts.tileWidth > (int)fullImage.getSize().x ||
                texY + ts.tileHeight > (int)fullImage.getSize().y) {
                std::cout << "     [Tile " << prop.localId << "] Out of bounds, skipped" << std::endl;
                continue;
            }
            
            // 创建独立贴图
            sf::Image subImg;
            subImg.create(ts.tileWidth, ts.tileHeight);
            
            for (int y = 0; y < ts.tileHeight; ++y) {
                for (int x = 0; x < ts.tileWidth; ++x) {
                    sf::Color pixel = fullImage.getPixel(texX + x, texY + y);
                    subImg.setPixel(x, y, pixel);
                }
            }
            
            prop.texture = std::make_shared<sf::Texture>();
            if (prop.texture->loadFromImage(subImg)) {
                prop.hasTexture = true;
                std::cout << "     [Tile " << prop.localId << "] Extracted from spritesheet: (" 
                          << texX << ", " << texY << ", " << ts.tileWidth << ", " << ts.tileHeight << ")" << std::endl;
            }
        }
        
        return true;
    }
    
    // Try alternative paths
    std::vector<std::string> altPaths = {
        tsxDir + imgSource,
        tmjBasePath + imgSource,
        "assets/" + imgSource,
        "assets/map/" + imgSource,
    };
    
    for (const auto& altPath : altPaths) {
        if (ts.texture.loadFromFile(altPath)) {
            ts.loaded = true;
            ts.imagePath = altPath;
            
            // ========================================
            // 为所有TileProperty创建独立贴图（从spritesheet提取）
            // ========================================
            sf::Image fullImage = ts.texture.copyToImage();
            
            for (auto& prop : ts.tileProperties) {
                // 计算在spritesheet中的位置
                int cols = ts.columns > 0 ? ts.columns : 1;
                int texX = (prop.localId % cols) * ts.tileWidth;
                int texY = (prop.localId / cols) * ts.tileHeight;
                prop.textureRect = sf::IntRect(texX, texY, ts.tileWidth, ts.tileHeight);
                
                // 检查边界
                if (texX + ts.tileWidth > (int)fullImage.getSize().x ||
                    texY + ts.tileHeight > (int)fullImage.getSize().y) {
                    std::cout << "     [Tile " << prop.localId << "] Out of bounds, skipped" << std::endl;
                    continue;
                }
                
                // 创建独立贴图
                sf::Image subImg;
                subImg.create(ts.tileWidth, ts.tileHeight);
                
                for (int y = 0; y < ts.tileHeight; ++y) {
                    for (int x = 0; x < ts.tileWidth; ++x) {
                        sf::Color pixel = fullImage.getPixel(texX + x, texY + y);
                        subImg.setPixel(x, y, pixel);
                    }
                }
                
                prop.texture = std::make_shared<sf::Texture>();
                if (prop.texture->loadFromImage(subImg)) {
                    prop.hasTexture = true;
                    std::cout << "     [Tile " << prop.localId << "] Extracted from spritesheet: (" 
                              << texX << ", " << texY << ", " << ts.tileWidth << ", " << ts.tileHeight << ")" << std::endl;
                }
            }
            
            return true;
        }
    }
    
    std::cout << "     All paths failed" << std::endl;
    return false;
}

// ============================================================================
// Parse Layers
// ============================================================================

void TileMap::parseLayers(const std::string& json) {
    layers.clear();
    
    auto layerObjects = getJsonObjectArray(json, "layers");
    std::cout << "[DEBUG] Found " << layerObjects.size() << " layer(s)" << std::endl;
    
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
        
        std::cout << "[OK] Layer: \"" << layer.name << "\" (" << layer.data.size() << " tiles)"
                  << (layer.isCollision ? " [Collision]" : "") << std::endl;
        
        layers.push_back(layer);
    }
    
    if (layers.empty()) {
        std::cerr << "[WARNING] No tile layers found!" << std::endl;
    }
}

// ============================================================================
// Parse Object Groups (for trees, buildings, etc.)
// ============================================================================

void TileMap::parseObjectGroups(const std::string& json) {
    objects.clear();
    
    auto layerObjects = getJsonObjectArray(json, "layers");
    std::cout << "[DEBUG] parseObjectGroups: checking " << layerObjects.size() << " layers" << std::endl;
    
    for (size_t idx = 0; idx < layerObjects.size(); idx++) {
        const auto& layerJson = layerObjects[idx];
        std::string type = getJsonString(layerJson, "type");
        std::cout << "[DEBUG] Layer " << idx << " type: \"" << type << "\"" << std::endl;
        
        if (type != "objectgroup") continue;
        
        std::string layerName = getJsonString(layerJson, "name");
        std::cout << "[DEBUG] Parsing object layer: \"" << layerName << "\"" << std::endl;
        
        // Parse objects array
        auto objectsArray = getJsonObjectArray(layerJson, "objects");
        std::cout << "[DEBUG] Found " << objectsArray.size() << " object(s)" << std::endl;
        
        for (const auto& objJson : objectsArray) {
            MapObject obj;
            obj.gid = getJsonInt(objJson, "gid");
            obj.x = getJsonFloat(objJson, "x");
            obj.y = getJsonFloat(objJson, "y");
            obj.width = getJsonFloat(objJson, "width");
            obj.height = getJsonFloat(objJson, "height");
            obj.name = getJsonString(objJson, "name");
            obj.type = getJsonString(objJson, "type");
            obj.tileProperty = nullptr;
            
            if (obj.gid <= 0) continue;
            
            // Find corresponding tileset
            int tsIndex = -1;
            for (int t = (int)tilesets.size() - 1; t >= 0; t--) {
                if (obj.gid >= tilesets[t].firstGid && tilesets[t].loaded) {
                    tsIndex = t;
                    break;
                }
            }
            
            if (tsIndex >= 0) {
                obj.textureIndex = tsIndex;
                TilesetInfo& ts = tilesets[tsIndex];
                
                int localId = obj.gid - ts.firstGid;
                int cols = ts.columns > 0 ? ts.columns : 1;
                
                // For collection of images (columns == 1 after our fix), 
                // the entire texture is the tile
                if (ts.columns == 1 && ts.tileCount == 1) {
                    obj.texCoords.x = 0;
                    obj.texCoords.y = 0;
                } else {
                    obj.texCoords.x = (localId % cols) * ts.tileWidth;
                    obj.texCoords.y = (localId / cols) * ts.tileHeight;
                }
                
                // ========================================
                // 关联 tile 属性（从 tsx 文件读取）
                // ========================================
                obj.tileProperty = ts.getTileProperty(localId);
                
                if (obj.tileProperty) {
                    // 如果对象没有设置name/type，从tile属性继承
                    if (obj.name.empty()) obj.name = obj.tileProperty->name;
                    if (obj.type.empty()) obj.type = obj.tileProperty->type;
                    
                    std::cout << "[OK] Object: gid=" << obj.gid 
                              << " (localId=" << localId << ")"
                              << " -> " << obj.tileProperty->name
                              << " [" << obj.tileProperty->type << "]"
                              << " HP=" << obj.tileProperty->hp << std::endl;
                } else {
                    std::cout << "[OK] Object: gid=" << obj.gid 
                              << " at (" << obj.x << ", " << obj.y << ")"
                              << " (no tile properties)" << std::endl;
                }
                
                objects.push_back(obj);
            } else {
                std::cerr << "[WARNING] No tileset found for object gid=" << obj.gid << std::endl;
            }
        }
    }
    
    std::cout << "[OK] Total " << objects.size() << " map object(s) loaded" << std::endl;
}

// ============================================================================
// Initialize internal structures from layer data
// ============================================================================

void TileMap::initializeFromLayers() {
    groundLayer.resize(width * height);
    decorationLayer.resize(width * height);
    collisionLayer.resize(width * height, false);
    
    bool groundFilled = false;
    int totalTilesPlaced = 0;
    
    for (const auto& layer : layers) {
        for (size_t i = 0; i < layer.data.size() && i < (size_t)(width * height); i++) {
            int gid = layer.data[i];
            if (gid <= 0) continue;
            
            Tile tile;
            tile.id = gid;
            
            // Find corresponding tileset (search from back since sorted by firstGid)
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
                totalTilesPlaced++;
            }
            
            // Collision layer handling
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
    
    std::cout << "[OK] Placed " << totalTilesPlaced << " tiles with valid textures" << std::endl;
    
    if (totalTilesPlaced == 0) {
        std::cerr << "[WARNING] No tiles placed! Check tileset paths1." << std::endl;
    }
}

// ============================================================================
// 根据 gid 获取 tile 属性
// ============================================================================

const TileProperty* TileMap::getTilePropertyByGid(int gid) const {
    // 查找对应的 tileset
    for (int t = (int)tilesets.size() - 1; t >= 0; t--) {
        if (gid >= tilesets[t].firstGid) {
            int localId = gid - tilesets[t].firstGid;
            return tilesets[t].getTileProperty(localId);
        }
    }
    return nullptr;
}