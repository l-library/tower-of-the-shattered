#pragma once

#include "Skills.h"

class SkillIceSpear : public Skills {
public:
    static SkillIceSpear* create();
    virtual bool execute(Player* owner, double damage) override;
    bool init();
private:
    SkillIceSpear();
};
