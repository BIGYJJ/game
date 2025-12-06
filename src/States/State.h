#pragma once
#include <SFML/Graphics.hpp>

class Game;

class State {
public:
    State(Game* game) : game(game), popRequested(false) {}
    virtual ~State() = default;
    
    virtual void handleInput(const sf::Event& event) = 0;
    virtual void update(float dt) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
    
    bool shouldPop() const { return popRequested; }
    void requestPop() { popRequested = true; }
    
protected:
    Game* game;
    bool popRequested;
};