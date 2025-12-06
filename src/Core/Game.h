#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <stack>

class State;

class Game {
public:
    Game();
    ~Game();
    
    void run();
    
    // 状态管理
    void pushState(std::unique_ptr<State> state);
    void popState();
    void changeState(std::unique_ptr<State> state);
    
    // 获取器
    sf::RenderWindow& getWindow() { return window; }
    float getDeltaTime() const { return deltaTime; }
    
private:
    void processEvents();
    void update(float dt);
    void render();
    
    sf::RenderWindow window;
    std::stack<std::unique_ptr<State>> states;
    
    sf::Clock clock;
    float deltaTime;
    
    const int FPS = 60;
    const sf::Time timePerFrame = sf::seconds(1.0f / FPS);
};