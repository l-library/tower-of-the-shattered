#include "SkillIceSpear.h"
#include "../Bullet/Bullet.h"

USING_NS_CC;

SkillIceSpear::SkillIceSpear()
    : Skills({
        "IceSpear",             // 名称和子弹动画名称
        "IceSpear-Action",      // 玩家动作名称
        "skill-icons/IceSpear.png",  // 图标名称
        25.0f,                  // 消耗魔法
        5.0f,                   // 冷却时间
        50.0f,                  // 伤害
        300.0f                  // 速度
        })
{}

SkillIceSpear* SkillIceSpear::create() {
    auto res = new (std::nothrow) SkillIceSpear();
    if (res) {
        res->autorelease();
        return res;
    }
    CC_SAFE_DELETE(res);
    return nullptr;
}

bool SkillIceSpear::execute(Player* owner) {
    // 确认技能能被释放
    if (!isReady() || !isUnlocked()) return false;
    if (owner->getMagic() < _config.cost) return false;
    if (!isUnlocked()) return false;

    // 播放音效
    AudioManager::getInstance()->playEffect("sounds/IceSpire.ogg");

    // 创建子弹
    auto bullet_animation = cocos2d::AnimationCache::getInstance()->getAnimation(_config.name);
    if (!bullet_animation) return false;
    auto skill = Bullet::create("player/IceSpear-0.png", 0, [](Bullet* b, float d) {});
    if (!bullet_animation || !skill)return false;

    // 获取玩家当前位置
    Vec2 current_pos = owner->getSprite()->getPosition();
    auto action = Animate::create(bullet_animation);

    // 设置方向 (同时处理物理速度方向和贴图翻转)
    cocos2d::Vec2 direction = (owner->getDirection() == Direction::RIGHT) ? cocos2d::Vec2(1, 0) : cocos2d::Vec2(-1, 0);
    if (direction == Vec2(1,0)) {
        skill->getSprite()->setFlippedX(false);
    }
    else {
        skill->getSprite()->setFlippedX(true);
    }

    // 设置子弹各属性
    skill->setDamage(_config.basic_damage);

    skill->setCategoryBitmask(PLAYER_BULLET_MASK);
    skill->setCollisionBitmask(NULL);
    skill->setContactTestBitmask(WALL_MASK | ENEMY_MASK | BORDER_MASK);
    skill->setCLearBitmask(WALL_MASK | ENEMY_MASK | BORDER_MASK);

    skill->getSprite()->runAction(RepeatForever::create(action));
    skill->setMaxExistTime(3.0f);
    Vec2 worldPos = owner->convertToWorldSpace(current_pos);
    worldPos += (direction * 30.0f);
    worldPos.y += owner->getSprite()->getContentSize().height;
    skill->setPosition(worldPos);

    // 设置速度
    skill->getPhysicsBody()->setVelocity(_config.speed * direction);
    //微调碰撞箱
    skill->getPhysicsBody()->setPositionOffset(direction * 20);
    // 添加为场景的子节点
    auto gameScene = Director::getInstance()->getRunningScene();
    if (gameScene) {
        gameScene->addChild(skill, 10);
    }

    // 玩家播放动画
    owner->playAnimation(_config.animationName);

    // 进入冷却
    _currentCooldown = _config.cooldown;

    // 减少魔力值
    owner->setMagic(owner->getMagic() - _config.cost);

    return true;
}