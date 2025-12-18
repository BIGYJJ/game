#include "Core/Game.h"
#include "States/GameState.h"
#include <iostream>
#include <fstream>
#include <filesystem>

// 全局日志文件
std::ofstream logFile;

// 自定义日志函数
void log(const std::string& msg) {
    logFile << msg << std::endl;
    logFile.flush();  // 立即写入，防止崩溃丢失
}

int main() {
    // 打开日志文件
    logFile.open("game_log.txt");
    if (!logFile.is_open()) {
        // 如果当前目录不能写，尝试用户目录
        logFile.open("C:/game_log.txt");
    }
    
    log("========================================");
    log("  PixelFarmRPG 启动诊断");
    log("========================================");
    
    try {
        // 1. 检查工作目录
        log("[1] 工作目录: " + std::filesystem::current_path().string());
        
        // 2. 检查 assets 目录
        if (std::filesystem::exists("assets")) {
            log("[2] assets 目录: 存在");
        } else if (std::filesystem::exists("../../assets")) {
            log("[2] assets 目录: 存在于 ../../assets");
        } else {
            log("[2] assets 目录: 不存在!");
        }
        
        // 3. 检查关键文件
        std::vector<std::string> checkFiles = {
            "assets/game_source/part1.tmj",
            "assets/player.png",
            "assets/map/part_1.png",
            "../../assets/player.png",
            "../../assets/game_source/part1.tmj"
        };
        
        log("[3] 检查关键文件:");
        for (const auto& f : checkFiles) {
            if (std::filesystem::exists(f)) {
                log("    [存在] " + f);
            } else {
                log("    [缺失] " + f);
            }
        }
        
        // 4. 列出 assets 目录内容
        log("[4] 列出目录内容:");
        if (std::filesystem::exists("assets")) {
            for (const auto& entry : std::filesystem::directory_iterator("assets")) {
                log("    " + entry.path().string());
            }
        }
        
        // 5. 创建游戏窗口
        log("[5] 创建 Game 对象...");
        Game game;
        log("[5] Game 对象创建成功");
        
        // 6. 创建游戏状态
        log("[6] 创建 GameState 对象...");
        auto gameState = std::make_unique<GameState>(&game);
        log("[6] GameState 对象创建成功");
        
        // 7. 推入状态
        log("[7] 推入游戏状态...");
        game.pushState(std::move(gameState));
        log("[7] 状态推入成功");
        
        // 8. 运行游戏
        log("[8] 开始游戏主循环...");
        game.run();
        log("[8] 游戏正常退出");
        
    } catch (const std::exception& e) {
        log("[错误] 异常: " + std::string(e.what()));
    } catch (...) {
        log("[错误] 未知异常!");
    }
    
    log("========================================");
    log("  诊断结束");
    log("========================================");
    
    logFile.close();
    return 0;
}