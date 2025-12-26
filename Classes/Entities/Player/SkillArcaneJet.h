#pragma once

#include "Skills.h"

class SkillArcaneJet : public Skills {
public:
    static SkillArcaneJet* create();
    virtual bool execute(Player* owner, double damage) override;
    void SkillArcaneJet::spawnBullet(Player* owne, double damager);
    bool init();
private:
    SkillArcaneJet();
};
