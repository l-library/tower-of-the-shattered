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

    // 关键：设置锚点与朝向
    // 锚点决定了缩放的"固定端"。
    // 同时也需要设置 Sprite 的锚点，确保纹理和父节点对齐。
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

    // --- 3. 设置初始位置 ---
    // 将位置设置在玩家前方一点的"枪口"位置
    Vec2 playerCenter = owner->getSprite()->getPosition();
    Vec2 worldPos = owner->convertToWorldSpace(playerCenter);

    // Y轴调整：根据玩家中心高度微调
    worldPos.y += owner->getSprite()->getContentSize().height / 2 +20.0;
    // X轴调整：向朝向方向偏移，避免从身体内部发出
    worldPos += direction * 15.0f;

    skill->setPosition(worldPos);
    skill->setScaleY(2.0f);   // 设定激光的粗细
    skill->setScaleX(0.1f);   // 初始长度设为极短，准备开始生长

    // --- 4. 播放动画 ---
    auto action = Animate::create(bullet_animation);
    skill->getSprite()->runAction(Sequence::create(
        action,
        CallFunc::create([skill]() { skill->cleanupBullet(); }), // 动画结束即销毁
        nullptr
    ));

    // --- 5. 构建物理刚体 ---
    // 使用原始尺寸创建盒子，因为PhysicsBody会跟随Node的setScaleX自动变形
    auto body = PhysicsBody::createBox(originalSize);

    // 关键步骤：设置物理偏移 (Offset)
    // Box默认中心在(0,0)。
    // 如果锚点是(0, 0.5)，Sprite内容在局部坐标 [0, width]。Box中心需要移到 width/2。
    // 如果锚点是(1, 0.5)，Sprite内容在局部坐标 [-width, 0]。Box中心需要移到 -width/2。

    float xOffset = originalSize.width / 2.0f;
    if (direction.x > 0) {
        body->setPositionOffset(Vec2(xOffset+ 250.0, 0));
    }
    else {
        body->setPositionOffset(Vec2(-xOffset-250.0, 0));
    }

    body->setDynamic(false); // 激光通常不仅是动力学物体，也可以设为Kinematic或Dynamic(false)
    body->setCategoryBitmask(PLAYER_BULLET_MASK);
    body->setContactTestBitmask(ENEMY_MASK);
    body->setCollisionBitmask(0); // 激光通常穿透，不产生物理反弹

    // 覆盖 Bullet::init 中创建的默认刚体
    skill->setPhysicsBody(body);

    // --- 6. 动态延伸逻辑 ---
    // 这里的 dt 是每帧间隔。
    // 3.0f * dt 表示每秒伸长 3倍原始长度。
    // 由于设置了正确的锚点和Offset，ScaleX 变大时，碰撞框会沿着发射方向变长。
    skill->schedule([skill](float dt) {
        if (!skill || !skill->getParent()) return; // 安全检查
        float currentScaleX = skill->getScaleX();
        skill->setScaleX(currentScaleX + 3.0f * dt); // 调整增长速度
        }, "scale_task");

    // 添加到场景
    auto gameScene = Director::getInstance()->getRunningScene();
    if (gameScene) {
        gameScene->addChild(skill, 10);
    }
}