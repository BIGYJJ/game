#include "Game.h"
#include "../States/State.h"

Game::Game() 
    : window(sf::VideoMode(1280, 720), "Pixel Farm RPG")
    , deltaTime(0.0f)
{
    window.setFramerateLimit(FPS);
}

Game::~Game() {
}

void Game::run() {
    while (window.isOpen() && !states.empty()) {
        deltaTime = clock.restart().asSeconds();
        
        processEvents();
        update(deltaTime);
        render();
    }
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
            
        if (!states.empty())
            states.top()->handleInput(event);
    }
}

void Game::update(float dt) {
    if (!states.empty()) {
        states.top()->update(dt);
        
        // 处理状态切换请求
        if (states.top()->shouldPop()) {
            popState();
        }
    }
}

void Game::render() {
    window.clear(sf::Color(34, 139, 34)); // 草地绿色背景
    
    if (!states.empty())
        states.top()->render(window);
    
    window.display();
}

void Game::pushState(std::unique_ptr<State> state) {
    states.push(std::move(state));
}

void Game::popState() {
    if (!states.empty())
        states.pop();
}

void Game::changeState(std::unique_ptr<State> state) {
    popState();
    pushState(std::move(state));
}