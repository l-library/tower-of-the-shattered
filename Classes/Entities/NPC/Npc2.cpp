#include "NPC2.h"
#include "TowerOfTheShattered.h"
#include "../../Tools/SaveManager.h"

USING_NS_CC;

NPC2::NPC2()
{
    has_enough_soul = false;
}

NPC2::~NPC2()
{
}

NPC2* NPC2::create()
{
    NPC2* npc = new NPC2();
    if (npc && npc->init())
    {
        npc->autorelease();
        return npc;
    }
    delete npc;
    return nullptr;
}

bool NPC2::init()
{
    // 初始化父类，使用第一张图片作为初始精灵
    if (!NpcBase::init("NPC/NPC2/npc2_001.png"))
    {
        return false;
    }
    
    // 加载精灵帧缓存
    auto cache = SpriteFrameCache::getInstance();
    cache->addSpriteFramesWithFile("NPC/NPC2/npc2_idle.plist");
    
    // 创建待机动画
    Vector<SpriteFrame*> idleFrames;
    for (int i = 1; i <= 6; i++)
    {
        char frameName[32];
        sprintf(frameName, "%03d.png", i);
        SpriteFrame* frame = cache->getSpriteFrameByName(frameName);
        if (frame != nullptr)
        {
            idleFrames.pushBack(frame);
        }
    }
    
    if (!idleFrames.empty())
    {
        // 创建动画
        auto idleAnimation = Animation::createWithSpriteFrames(idleFrames, 0.1f);
        if (idleAnimation != nullptr)
        {
            idleAnimation->setLoops(-1); // 无限循环
            idleAnimation->setRestoreOriginalFrame(true);
            
            // 创建动画动作
            idleAnimation_ = Animate::create(idleAnimation);
            if (idleAnimation_ != nullptr)
            {
                // 播放待机动画
                sprite_->runAction(idleAnimation_);
            }
        }
    }
    std::string NPC2Path = "config/NPC/NPC2.json";
    // 设置对话内容
    std::vector<DialogueEntry> dialogues = 
    {
        DialogueEntry(ReadJson::getString(NPC2Path,"1"),
                     nullptr, // 通用回调函数(固定触发)
                     []() {}, // 按键1回调
                     []() {}, // 按键2回调
                     []() {}), // 按键3回调
        DialogueEntry(ReadJson::getString(NPC2Path,"2"),
                     [this]()
            {
                if (ItemManager::getInstance()->getSoul() < 100)
                    currentDialogueIndex_ = 10;
            },
                     []() {},
                     []() {},
                     []() {}),
        DialogueEntry(ReadJson::getString(NPC2Path,"3"),
                     nullptr,
                     []() {},
                     []() {},
                     [this]() {

            }),
        DialogueEntry(ReadJson::getString(NPC2Path,"4"),
                     []() {    ItemManager::getInstance()->spendSoul(100); },
                     [this]()
            {//强化战斗

                player_->modifyAttack(100);
                currentDialogueIndex_ = 3;
                SaveManager::getInstance()->setAttackUp(100);
            },
                     [this]()
            {//强化机动

                player_->modifyMove(100);
                currentDialogueIndex_ = 4;
                SaveManager::getInstance()->setMoveSpeedUp(100);
            },
                     [this]()
            {//解锁技能

               if (!player_->isUnlocked("IceSpear"))
               {
                   player_->getSkillManager()->getSkill("IceSpear")->setUnlocked(true);
                   currentDialogueIndex_ = 5;
                   SaveManager::getInstance()->unlockNextSkill();
               }
               else if (!player_->isUnlocked("ArcaneShield"))
               {
                   player_->getSkillManager()->getSkill("ArcaneShield")->setUnlocked(true);
                   currentDialogueIndex_ = 6;
                   SaveManager::getInstance()->unlockNextSkill();
               }
               else if (!player_->isUnlocked("ArcaneJet"))
               {
                   player_->getSkillManager()->getSkill("ArcaneJet")->setUnlocked(true);
                   currentDialogueIndex_ = 7;
                   SaveManager::getInstance()->unlockNextSkill();
               }
               else
               {
                   currentDialogueIndex_ = 8;
               }


            }),
        DialogueEntry(ReadJson::getString(NPC2Path,"5"),
                     [this]() {   currentDialogueIndex_ = 10; },
                     []() {},
                     []() {},
                     []() {}),
        DialogueEntry(ReadJson::getString(NPC2Path,"6"),
                     [this]() {   currentDialogueIndex_ = 10; },
                     []() {},
                     []() {},
                     []() {}),
        DialogueEntry(ReadJson::getString(NPC2Path,"7"),
                     [this]() {   currentDialogueIndex_ = 10; },
                     []() {},
                     []() {},
                     []() {}),
        DialogueEntry(ReadJson::getString(NPC2Path,"8"),
                     [this]() {   currentDialogueIndex_ = 10; },
                     []() {},
                     []() {},
                     []() {}),
        DialogueEntry(ReadJson::getString(NPC2Path,"9"),
                     [this]() {   currentDialogueIndex_ = 10; },
                     []() {},
                     []() {},
                     []() {}),
        DialogueEntry(ReadJson::getString(NPC2Path,"10"),
                     []() {},
                     []() {},
                     []() {},
                     []() {})
    };
    this->setDialogues(dialogues);
    
    //设置立绘
    Illustration_ = Sprite::create("NPC/NPC2/npc2_illustration.png");
    if (Illustration_)
    {
        Illustration_->retain();
    }

    return true;
}

void NPC2::handlePlayerInteraction()
{
    // 显示对话背景并调用显示对话内容的函数
    showDialogueBackground();
}

