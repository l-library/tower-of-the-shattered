#include "SkillArcaneShield.h"
#include "../Bullet/Bullet.h"

USING_NS_CC;

// 定义护盾的最大生命值
const int SHIELD_MAX_HP = 3;

SkillArcaneShield::SkillArcaneShield()
    : Skills({
        "ArcaneShield",
        "ArcaneShield-Action",
        "skill-icons/ArcaneShield.png",
        30.0f,
        30.0f,
        0.0f,
        15.0f
        })
{
}

SkillArcaneShield* SkillArcaneShield::create() {
    auto res = new (std::nothrow) SkillArcaneShield();
    if (res) {
        res->autorelease();
        return res;
    }
    CC_SAFE_DELETE(res);
    return nullptr;
}

bool SkillArcaneShield::execute(Player* owner , double damage) {
    if (!isReady() || !isUnlocked()) return false;
    if (owner->getMagic() < _config.cost) return false;

    // 播放音效
    int sound_id = AudioManager::getInstance()->playEffect("sounds/ArcaneShield.ogg",true);
    AudioManager::getInstance()->setEffectsVolumeById(sound_id, 0.6f);

    auto shield_animation = cocos2d::AnimationCache::getInstance()->getAnimation(_config.name);

    // 创建护盾实体
    auto shield = Bullet::create("player/ArcaneShield-0.png", 0, [sound_id](Bullet* b, float d) {});

    // 延迟数秒后结束音效
    auto delay = DelayTime::create(_config.speed);
    auto fireFunc = CallFunc::create([sound_id]() {
        // 播放音效
        AudioManager::getInstance()->stopById(sound_id);
        });
    // 在当前玩家身上运行这个序列
    owner->runAction(Sequence::create(delay, fireFunc, nullptr));

    if (!shield) return false;

    // 播放护盾循环动画
    if (shield_animation) {
        auto action = Animate::create(shield_animation);
        shield->getSprite()->runAction(RepeatForever::create(action));
    }

    // 设置物理属性
    shield->getPhysicsBody()->setMass(0); // 质量为0，不受重力影响
    shield->setDamage(_config.basic_damage * damage); // 如果有反伤可以保留，没有则为0

    // 设置掩码：作为墙壁，与敌人和子弹产生碰撞；作为玩家，会受到敌人攻击
    shield->setCategoryBitmask(PLAYER_MASK | WALL_MASK);
    shield->setCollisionBitmask(ENEMY_MASK | ENEMY_BULLET_MASK);
    shield->setContactTestBitmask(ENEMY_MASK | ENEMY_BULLET_MASK); 
    shield->setCLearBitmask(NULL);
    shield->getPhysicsBody()->setDynamic(false); // 静态物体，跟随玩家移动

    // 定位与层级
    Size playerSize = owner->getSprite()->getContentSize();
    shield->setPosition(Vec2(0.0f, playerSize.height / 2));
    shield->setScale(1.5f);
    shield->setMaxExistTime(_config.speed); // 设置自然消失时间

    // 设置血量和名称
    shield->setName("ActiveArcaneShield"); //以此名来识别是否是我们的护盾
    shield->setTag(SHIELD_MAX_HP);         // 利用Tag存储当前血量

    // 添加碰撞监听器 (受击与破碎逻辑)
    auto contactListener = EventListenerPhysicsContact::create();
    contactListener->onContactBegin = [shield, owner,sound_id](PhysicsContact& contact) -> bool {

        // 安全检查：确保 shield 和它的渲染节点都存在
        if (!shield || !shield->getParent() || !shield->getSprite()) return false;

        auto nodeA = contact.getShapeA()->getBody()->getNode();
        auto nodeB = contact.getShapeB()->getBody()->getNode();

        Node* target = nullptr;
        if (nodeA == shield) target = nodeB;
        else if (nodeB == shield) target = nodeA;

        if (target) {
            // 增强受击反馈 (视觉修正)
            auto sprite = shield->getSprite();
            sprite->stopActionByTag(999); // 停止上一次受击动作

            // 方案：变红 + 放大弹动 + 透明度变化
            // 保存原始缩放比例 (假设初始是 1.0f，如果不是请读取当前scale)
            float originalScale = target->getScale();

            auto hitSequence = Sequence::create(
                // 瞬间变红，变大，变半透明
                Spawn::create(
                    CallFunc::create([sprite]() { sprite->setColor(Color3B(255, 50, 50)); }), // 鲜艳的红
                    ScaleTo::create(0.05f, originalScale * 2.0f), // 瞬间膨胀
                    FadeTo::create(0.05f, 150), // 变半透明
                    nullptr
                ),
                // 恢复原状
                Spawn::create(
                    TintTo::create(0.1f, Color3B::WHITE), // 渐变回白
                    ScaleTo::create(0.1f, originalScale), // 恢复大小
                    FadeTo::create(0.1f, 255), // 恢复不透明
                    nullptr
                ),
                nullptr
            );
            AudioManager::getInstance()->playEffect("sounds/ShieldHit.ogg");
            hitSequence->setTag(999);
            sprite->runAction(hitSequence);

            // 扣血
            int currentHP = shield->getTag();
            currentHP--;
            shield->setTag(currentHP);

            // 破碎逻辑
            if (currentHP <= 0) {
                auto particle = ParticleSystemQuad::createWithTotalParticles(50);
                AudioManager::getInstance()->stopById(sound_id);
                AudioManager::getInstance()->playEffect("sounds/ShieldBroken.ogg");
                // 设置纹理
                auto texture = Director::getInstance()->getTextureCache()->addImage("player/ArcaneShield-03.png");
                if (texture) {
                    particle->setTexture(texture);
                }

                // --- 核心参数配置 ---
                particle->setDuration(0.1f);
                particle->setAutoRemoveOnFinish(true);
                particle->setEmitterMode(ParticleSystem::Mode::GRAVITY);
                particle->setGravity(Vec2(0, -800));

                particle->setSpeed(300);
                particle->setSpeedVar(100);
                particle->setAngle(90);
                particle->setAngleVar(360);

                particle->setLife(0.6f);
                particle->setLifeVar(0.2f);

                particle->setStartSize(30.0f);
                particle->setStartSizeVar(10.0f);
                particle->setEndSize(0.0f);

                particle->setStartColor(Color4F(0.4f, 0.7f, 1.0f, 1.0f));
                particle->setEndColor(Color4F(0.4f, 0.7f, 1.0f, 0.0f));

                
                // 显式设置发射速率，保证瞬间爆发
                particle->setEmissionRate(1000); // 设置一个足够大的值，或者 total/duration

                // 增加位置随机范围，让碎片看起来是从整个护盾体积散开的，而不是中心一个点
                if (shield->getSprite()) {
                    Size size = shield->getSprite()->getContentSize();
                    particle->setPosVar(Vec2(size.width * 0.5f, size.height * 0.5f));
                }

                // 强制重置系统以应用新参数
                particle->resetSystem();

                Vec2 worldPos = shield->getParent()->convertToWorldSpace(shield->getPosition());
                particle->setPosition(worldPos);

                if (Director::getInstance()->getRunningScene()) {
                    Director::getInstance()->getRunningScene()->addChild(particle, 100);
                }

                shield->removeFromParent();
                return false;
            }
        }
        return true;
        };

    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener, shield);

    owner->addChild(shield, 10);
    owner->playAnimation(_config.animationName);

    // 结算
    _currentCooldown = _config.cooldown;
    owner->setMagic(owner->getMagic() - _config.cost);

    return true;
}