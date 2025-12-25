#pragma once

#include "Skills.h"

class SkillArcaneShield : public Skills {
public:
    SkillArcaneShield();
    static SkillArcaneShield* create();
    virtual bool execute(Player* owner, double damage) override;
};