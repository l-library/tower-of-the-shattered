#include "SkillArcaneJet.h"
#include "../Bullet/Bullet.h"

USING_NS_CC;

SkillArcaneJet::SkillArcaneJet()
    : Skills({
        "ArcaneJet",             // 名称和子弹动画名称
        "ArcaneJet-Action",      // 玩家动作名称
        "skill-icons/ArcaneJet.png",  // 图标名称
        30.0f,                  // 消耗魔法
        10.0f,                   // 冷却时间
        100.0f,                  // 伤害
        300.0f                  // 速度
        })
{
}

SkillArcaneJet* SkillArcaneJet::create() {
    auto res = new (std::nothrow) SkillArcaneJet();
    if (res) {
        res->autorelease();
        return res;
    }
    CC_SAFE_DELETE(res);
    return nullptr;
}

bool SkillArcaneJet::execute(Player* owner) {
    // 基础检查
    if (!isReady() || !isUnlocked()) return false;
    if (owner->getMagic() < _config.cost) return false;

    // 消耗与进入冷却 (立即触发动画)
    _currentCooldown = _config.cooldown;
    owner->setMagic(owner->getMagic() - _config.cost);

    // 玩家播放起手动作
    owner->playAnimation(_config.animationName);

    // 延迟 1.3 秒后再发射子弹
    auto delay = DelayTime::create(1.3f);
    auto fireFunc = CallFunc::create([this, owner]() {
        this->spawnBullet(owner);
        });

    // 在当前场景或玩家身上运行这个序列
    owner->runAction(Sequence::create(delay, fireFunc, nullptr));

    return true;
}

void SkillArcaneJet::spawnBullet(Player* owner) {
    auto bullet_animation = cocos2d::AnimationCache::getInstance()->getAnimation(_config.name);
    if (!bullet_animation) return;

    // 创建子弹对象，update逻辑留空，因为我们使用schedule手动控制缩放
    auto skill = Bullet::create("player/ArcaneJet-0.png", static_cast<int>(_config.basic_damage), [](Bullet* b, float d) {});
    if (!skill) return;

    // 获取基础信息
    Size originalSize = skill->getSprite()->getContentSize();
    Vec2 direction = (owner->getDirection() == Direction::RIGHT) ? cocos2d::Vec2(1, 0) : cocos2d::Vec2(-1, 0);

    // 设置锚点与朝向
    if (direction.x > 0) {
        // 向右：锚点在左侧 (0, 0.5)，向右延伸
        skill->setAnchorPoint(Vec2(0.0f, 0.5f));
        skill->getSprite()->setAnchorPoint(Vec2(0.0f, 0.5f));
        skill->getSprite()->setFlippedX(false);
    }
    else {
        // 向左：锚点在右侧 (1, 0.5)，向左延伸
        skill->setAnchorPoint(Vec2(1.0f, 0.5f));
        skill->getSprite()->setAnchorPoint(Vec2(1.0f, 0.5f));
        skill->getSprite()->setFlippedX(true); // 翻转贴图
    }

    // 设置初始位置
    // 将位置设置在玩家前方一点的"枪口"位置
    Vec2 playerCenter = owner->getSprite()->getPosition();
    Vec2 worldPos = owner->convertToWorldSpace(playerCenter);

    // Y轴调整：根据玩家中心高度微调
    worldPos.y += owner->getSprite()->getContentSize().height / 2 + 20.0;
    // X轴调整：向朝向方向偏移，避免从身体内部发出
    worldPos += direction * 15.0f;

    skill->setPosition(worldPos);
    skill->setScaleY(2.0f);   // 设定激光的粗细
    skill->getSprite()->setScaleX(0.1f);   // 初始长度设为极短，准备开始生长

    skill->getPhysicsBody()->setCategoryBitmask(PLAYER_BULLET_MASK);
    skill->getPhysicsBody()->setContactTestBitmask(ENEMY_MASK);
    skill->getPhysicsBody()->setCollisionBitmask(0);

    // 播放动画
    auto action = Animate::create(bullet_animation);
    skill->getSprite()->runAction(Sequence::create(
        action,
        CallFunc::create([skill]() { skill->cleanupBullet(); }), // 动画结束即销毁
        nullptr
    ));

    // 动态延伸逻辑
    float growSpeed = 100.0f;          // 激光每秒增长的“世界长度”
    float currentLength = originalSize.width;
    float maxLength = 100.f; // 激光的最大长度

    skill->schedule([=](float dt) mutable {

        // 增加激光长度
        currentLength += growSpeed * dt;
        currentLength = std::min(currentLength, maxLength);

        // 视觉缩放（只拉 X）
        float scaleX = currentLength / originalSize.width;
        skill->getSprite()->setScaleX(scaleX);

        // 重新创建 PhysicsBody
        auto recreate_body = PhysicsBody::createBox(Size(currentLength * scaleX, originalSize.height));
        if (!recreate_body) return;
        recreate_body->setDynamic(false);
        recreate_body->setGravityEnable(false);
        recreate_body->setCategoryBitmask(PLAYER_BULLET_MASK);
        recreate_body->setContactTestBitmask(ENEMY_MASK);
        recreate_body->setCollisionBitmask(0); // 激光通常穿透，不产生物理反弹

        // 更新 Offset（保证“起点不动”）
        float half = currentLength * 0.5f;
        if (direction.x > 0) {
            recreate_body->setPositionOffset(Vec2(half * scaleX  + direction.x * 15.0f, 0));
        }
        else {
            recreate_body->setPositionOffset(Vec2(-half * scaleX + direction.x * 15.0f, 0));
        }

        skill->setPhysicsBody(recreate_body);
        }, "laser_grow");

    // 添加到场景
    auto gameScene = Director::getInstance()->getRunningScene();
    if (gameScene) {
        gameScene->addChild(skill, 10);
    }
}