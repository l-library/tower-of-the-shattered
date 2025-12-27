#include "NpcBase.h"


NpcBase::NpcBase() :
    sprite_(nullptr),
    physicsBody_(nullptr),
    dialogueBackground_(nullptr),
    dialogueLabel_(nullptr),
    idleAnimation_(nullptr),
    isPlayerCollided_(new bool(false)),
    keyboardListener_(nullptr),
    contactListener_(nullptr),
    currentDialogueIndex_(0),
    isDialogueDisplaying_(false),
    player_(nullptr),
    Illustration_(nullptr)
{
}

NpcBase::~NpcBase()
{
    if (isPlayerCollided_)
    {
        delete isPlayerCollided_;
        isPlayerCollided_ = nullptr;
    }
    
    if (keyboardListener_)
    {
        Director::getInstance()->getEventDispatcher()->removeEventListener(keyboardListener_);
        keyboardListener_ = nullptr;
    }
    
    if (contactListener_)
    {
        Director::getInstance()->getEventDispatcher()->removeEventListener(contactListener_);
        contactListener_ = nullptr;
    }
    
    // 处理对话背景精灵的释放
    if (dialogueBackground_)
    {
        dialogueBackground_->removeFromParentAndCleanup(true);
        dialogueBackground_ = nullptr;
    }
    
    // 处理对话标签的释放
    if (dialogueLabel_)
    {
        dialogueLabel_->removeFromParentAndCleanup(true);
        dialogueLabel_ = nullptr;
    }
    
    // 处理立绘的释放
    if (Illustration_)
    {
        Illustration_->removeFromParentAndCleanup(true);
        Illustration_ = nullptr;
    }
}

bool NpcBase::init(const std::string& spritePath)
{
    if (!Node::init())
    {
        return false;
    }
    
    // 创建精灵
    if (!InitSprite(spritePath))
    {
        return false;
    }
    
    // 初始化碰撞箱（不设碰撞效果，只检测玩家碰撞）
    physicsBody_ = PhysicsBody::createBox(cocos2d::Size(GRID_SIZE * 3, GRID_SIZE * 3));
    if (!physicsBody_)
    {
        CCLOG("Failed to create NPC physics body");
        return false;
    }
    physicsBody_->setDynamic(false);
    physicsBody_->setCategoryBitmask(NPC_MASK);
    physicsBody_->setCollisionBitmask(WALL_MASK);
    physicsBody_->setContactTestBitmask(PLAYER_MASK | WALL_MASK);

    this->setPhysicsBody(physicsBody_);
    
    // 注册碰撞监听器
    contactListener_ = EventListenerPhysicsContact::create();
    contactListener_->onContactBegin = CC_CALLBACK_1(NpcBase::onContactBegin, this);
    contactListener_->onContactSeparate = CC_CALLBACK_1(NpcBase::onContactSeparate, this);
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener_, this);
    
    // 注册按键监听器
    keyboardListener_ = EventListenerKeyboard::create();
    keyboardListener_->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event* event) {
        int nextIndex;
        if (keyCode == EventKeyboard::KeyCode::KEY_E)
        {
            // 如果正在显示对话，按E键显示下一条对话
            if (isDialogueDisplaying_)
            {
                nextIndex = currentDialogueIndex_ + 1;
                
                // 如果还有下一条对话，显示它
                if (nextIndex < dialogues_.size())
                {
                    this->displayDialogueContent(nextIndex);
                }
                else
                {
                    // 没有更多对话了，隐藏对话背景、标签和立绘并重置状态
                        if (dialogueBackground_)
                        {
                            dialogueBackground_->removeFromParentAndCleanup(true);
                            dialogueBackground_ = nullptr;
                        }
                        
                        if (dialogueLabel_)
                        {
                            dialogueLabel_->removeFromParentAndCleanup(true);
                            dialogueLabel_ = nullptr;
                        }
                        
                        if (Illustration_)
                        {
                            Illustration_->removeFromParent();
                        }
                        
                        isDialogueDisplaying_ = false;
                        currentDialogueIndex_ = 0;
                        if (player_ != nullptr)
                            player_->setControlEnabled(true);
                        CCLOG("对话结束");
                }
            }
            // 当玩家与NPC碰撞但没有显示对话时，按E键开始对话
            else if (*isPlayerCollided_)
            {
                if (player_ != nullptr)
                {
                    player_->setControlEnabled(false);
                    player_->stopMoving();
                }
                this->handlePlayerInteraction();
            }
        }
        // 处理数字键1、2、3的选项选择
        else if (isDialogueDisplaying_)
        {
            // 确保当前对话索引有效
            if (currentDialogueIndex_ >= 0 && currentDialogueIndex_ < dialogues_.size())
            {
                const auto& currentEntry = dialogues_[currentDialogueIndex_];
                
                switch (keyCode)
                {
                    case EventKeyboard::KeyCode::KEY_1:
                        if (currentEntry.option1Callback)
                        {
                            currentEntry.option1Callback();
                        }
                        nextIndex = currentDialogueIndex_ + 1;
                        // 如果还有下一条对话，显示它
                        if (nextIndex < dialogues_.size())
                        {
                            this->displayDialogueContent(nextIndex);
                        }
                        break;
                    case EventKeyboard::KeyCode::KEY_2:
                        if (currentEntry.option2Callback)
                        {
                            currentEntry.option2Callback();
                        }
                        nextIndex = currentDialogueIndex_ + 1;
                        // 如果还有下一条对话，显示它
                        if (nextIndex < dialogues_.size())
                        {
                            this->displayDialogueContent(nextIndex);
                        }
                        break;
                    case EventKeyboard::KeyCode::KEY_3:
                        if (currentEntry.option3Callback)
                        {
                            currentEntry.option3Callback();
                        }
                        nextIndex = currentDialogueIndex_ + 1;
                        // 如果还有下一条对话，显示它
                        if (nextIndex < dialogues_.size())
                        {
                            this->displayDialogueContent(nextIndex);
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    };
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyboardListener_, this);
    
    return true;
}

bool NpcBase::onContactBegin(PhysicsContact& contact)
{
    auto nodeA = contact.getShapeA()->getBody()->getNode();
    auto nodeB = contact.getShapeB()->getBody()->getNode();
    
    // 确定哪个是NPC，哪个是其他物体
    Node* npcNode = nullptr;
    Node* otherNode = nullptr;
    
    if (nodeA == this) {
        npcNode = nodeA;
        otherNode = nodeB;
    } else if (nodeB == this) {
        npcNode = nodeB;
        otherNode = nodeA;
    }
    
    if (!npcNode || !otherNode) {
        return true;
    }
    
    // 检测是否与玩家碰撞
    if ((otherNode->getPhysicsBody()->getCategoryBitmask() & PLAYER_MASK) != 0)
    {
        *isPlayerCollided_ = true;
        player_ = dynamic_cast<Player*>(otherNode);

    }
    
    return true;
}

bool NpcBase::onContactSeparate(PhysicsContact& contact)
{
    auto nodeA = contact.getShapeA()->getBody()->getNode();
    auto nodeB = contact.getShapeB()->getBody()->getNode();
    
    Node* npcNode = nullptr;
    Node* otherNode = nullptr;
    
    if (nodeA == this) {
        npcNode = nodeA;
        otherNode = nodeB;
    } else if (nodeB == this) {
        npcNode = nodeB;
        otherNode = nodeA;
    }
    
    if (!npcNode || !otherNode) {
        return true;
    }
    
    // 检测是否与玩家分离
    if ((otherNode->getPhysicsBody()->getCategoryBitmask() & PLAYER_MASK) != 0)
    {
        *isPlayerCollided_ = false;
    }
    
    return true;
}

bool NpcBase::InitSprite(const std::string& spritePath)
{
    sprite_ = Sprite::create(spritePath);
    if (!sprite_)
    {
        return false;
    }
    
    // 设置精灵尺寸
    this->sprite_->setScale(getScale() * 3);
    
    // 将精灵添加到节点
    this->addChild(sprite_);
    
    return true;
}

void NpcBase::setDialogues(const std::vector<DialogueEntry>& dialogues)
{
    dialogues_ = dialogues;
}

void NpcBase::showDialogueBackground()
{

    // 如果背景已经存在，先移除
    if (dialogueBackground_)
    {
        dialogueBackground_->removeFromParentAndCleanup(true);
        dialogueBackground_ = nullptr;
    }
    
    // 如果立绘已经在场景中，先移除
    if (Illustration_ && Illustration_->getParent())
    {
        Illustration_->removeFromParentAndCleanup(false); // 不移除纹理，只从场景中移除
    }
    
    // 创建对话背景精灵
    dialogueBackground_ = Sprite::create("NPC/background.png");
    if (!dialogueBackground_)
    {
        CCLOG("Failed to create dialogue background sprite");
        return;
    }
    
    // 获取屏幕尺寸
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    // 设置对话背景位置在屏幕底部中央
    dialogueBackground_->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + dialogueBackground_->getContentSize().height / 2));
    dialogueBackground_->setScale(1.5);
    // 获取当前场景并添加对话背景
    auto scene = dynamic_cast<PlayerTestScene*> (this->getParent());
    if (scene)
    {
        dialogueBackground_->setCameraMask((uint16_t)CameraFlag::USER2, true);
        scene->getGamera()->getUIRoot()->addChild(dialogueBackground_, 100); // 设置较高的层级以确保显示在最上层
        if (Illustration_ != nullptr)
        {
            // 设置立绘位置在背景正上偏右
            Vec2 illustrationPos = Vec2(
                dialogueBackground_->getPositionX(), // 偏右30%
                dialogueBackground_->getPositionY() + dialogueBackground_->getContentSize().height * 1.2// 正上方
            );
            Illustration_->setScale(1.5);
            Illustration_->setPosition(illustrationPos);
            Illustration_->setCameraMask((uint16_t)CameraFlag::USER2, true);
            scene->getGamera()->getUIRoot()->addChild(Illustration_, 101);
        }
    }
    
    // 调用显示对话内容的函数，默认显示第一条对话
    displayDialogueContent();
}

void NpcBase::displayDialogueContent(int dialogueIndex)
{
    // 参数验证：如果索引无效或没有对话内容，返回
    if (dialogues_.empty() || dialogueIndex < 0 || dialogueIndex >= dialogues_.size())
    {
        return;
    }
    
    // 如果标签已经存在，先移除
    if (dialogueLabel_)
    {
        dialogueLabel_->removeFromParentAndCleanup(true);
        dialogueLabel_ = nullptr;
    }
    
    // 创建对话文本标签
    dialogueLabel_ = Label::createWithTTF(dialogues_[dialogueIndex].content, "fonts/Gothic.ttf", 20);
    if (!dialogueLabel_)
    {
        CCLOG("Failed to create dialogue label");
        return;
    }
    
    // 设置标签颜色为黑色
    dialogueLabel_->setColor(Color3B::BLACK);
    dialogueLabel_->setScale(1.5);
    // 设置标签自动换行
    float backgroundWidth = dialogueBackground_->getContentSize().width;
    float maxWidth = backgroundWidth * 0.8f; // 设置最大宽度为背景宽度的80%
    float maxHeight = 1000.0f; // 设置足够大的高度，让文本可以自动换行
    dialogueLabel_->setDimensions(maxWidth, maxHeight);
    
    // 设置文本水平居中对齐
    dialogueLabel_->setHorizontalAlignment(TextHAlignment::CENTER);
    
    // 设置文本垂直居中对齐
    dialogueLabel_->setVerticalAlignment(TextVAlignment::CENTER);
    
    // 获取当前场景
    auto scene = dynamic_cast<PlayerTestScene*> (this->getParent());
    if (!scene)
    {
        return;
    }
    
    // 获取屏幕尺寸
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    // 设置标签位置在对话背景上方中央
    dialogueLabel_->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + dialogueBackground_->getContentSize().height / 2 + 10));
    
    // 将标签添加到场景
    dialogueLabel_->setCameraMask((uint16_t)CameraFlag::USER2, true);
    scene->getGamera()->getUIRoot()->addChild(dialogueLabel_, 101);
    // 记录当前显示的对话索引
    currentDialogueIndex_ = dialogueIndex;
    
    // 设置对话显示状态为true
    isDialogueDisplaying_ = true;
    
    // 执行回调函数（如果存在）
    if (dialogues_[dialogueIndex].callback) {
        dialogues_[dialogueIndex].callback();
    }
}
