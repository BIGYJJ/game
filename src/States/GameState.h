#pragma once
#include "State.h"
#include "../Entity/Player.h"
#include "../World/TileMap.h"
#include "../World/Camera.h"
#include "../Systems/TimeSystem.h"
#include <memory>
#include <string>

// 地图类型枚举
enum class MapType {
    Farm,    // 农场地图
    Forest   // 森林地图
};

class GameState : public State {
public:
    // 构造函数 - 可以指定初始地图类型
    GameState(Game* game, MapType mapType = MapType::Farm);
    
    // 事件处理
    void handleInput(const sf::Event& event) override;
    
    // 更新游戏逻辑
    void update(float dt) override;
    
    // 渲染
    void render(sf::RenderWindow& window) override;

private:
    // 加载指定地图
    void loadMap(MapType mapType);
    
    // 切换到新地图
    void switchMap(MapType newMap);
    
    // 渲染UI层
    void renderUI(sf::RenderWindow& window);
    
    // 获取地图名称
    std::string getMapName(MapType mapType) const;

private:
    // 游戏对象
    std::unique_ptr<Player> player;
    std::unique_ptr<TileMap> tileMap;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<TimeSystem> timeSystem;
    
    // 当前地图类型
    MapType currentMap;
};