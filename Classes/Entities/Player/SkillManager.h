#pragma once

#include "cocos2d.h"
#include <map>
#include "SkillIceSpear.h"
#include "SkillArcaneJet.h"
#include "SkillArcaneShield.h"

class SkillManager : public cocos2d::Ref {
public:
    static SkillManager* create(Player* owner) {
        auto res = new SkillManager();
        res->_owner = owner;
        return res;
    }

    void addSkill(const std::string& name, Skills* skill) {
        _skills.insert(name, skill);
    }

    bool useSkill(const std::string& name) {
        if (_skills.find(name) != _skills.end()) {
            return _skills.at(name)->execute(_owner);
        }
        return false;
    }

    Skills* getSkill(const std::string& name) {
        return _skills.at(name);
    }

    void update(float dt) {
        for (auto& pair : _skills) {
            pair.second->update(dt);
        }
    }

private:
    Player* _owner;
    cocos2d::Map<std::string, Skills*> _skills;
};