#pragma once
#include <string>

// 暂时的简化版TimeSystem，后续可以扩展
class TimeSystem {
public:
    TimeSystem() : hour(6), minute(0) {}
    
    void update(float dt) {
        // 暂时什么都不做
    }
    
    std::string getTimeString() const {
        return "06:00";
    }
    
private:
    int hour;
    int minute;
};