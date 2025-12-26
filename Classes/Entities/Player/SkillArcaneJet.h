#pragma once

#include "Skills.h"

class SkillArcaneJet : public Skills {
public:
    static SkillArcaneJet* create();
    virtual bool execute(Player* owner) override;
    void SkillArcaneJet::spawnBullet(Player* owner);
    bool init();
private:
    SkillArcaneJet();
};
