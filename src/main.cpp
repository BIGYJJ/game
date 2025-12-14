#include "Core/Game.h"
#include "States/GameState.h"
#include <iostream>

int main() {
    try {
        Game game;
        
        // 创建并推入初始游戏状态
        auto gameState = std::make_unique<GameState>(&game);
        game.pushState(std::move(gameState));
        
        // 运行游戏主循环
        game.run();
        
    } catch (const std::exception& e){
        std::cerr << "Error1: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}