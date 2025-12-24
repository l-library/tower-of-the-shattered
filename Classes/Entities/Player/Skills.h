#pragma once

#include "cocos2d.h"
#include <string>
#include "../../Audio/AudioManager.h"

class Player;

class Skills : public cocos2d::Ref {
public:
    struct Config {
        std::string name;
        std::string animationName;
        std::string iconPath;
        float cost;
        float cooldown;
        float basic_damage;
        float speed;
    };

    virtual bool execute(Player* owner) = 0; // 纯虚函数，由子类实现具体逻辑

    // 公共接口
    void update(float dt);
    bool isReady() const { return _currentCooldown <= 0; }
    bool isUnlocked() const { return _unlocked; }
    void setUnlocked(bool v) { _unlocked = v; }

    float getCooldownPercent() const { return _currentCooldown / _config.cooldown; }
    const Config& getConfig() const { return _config; }

protected:
    Skills(const Config& config) : _config(config), _currentCooldown(0), _unlocked(false) {}

    Config _config;
    float _currentCooldown;
    bool _unlocked;
};