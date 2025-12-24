#include "NpcBase.h"


NpcBase::NpcBase() : 
    sprite_(nullptr),
    physicsBody_(nullptr),
    idleAnimation_(nullptr),
    isPlayerCollided_(new bool(false)),
    keyboardListener_(nullptr),
    contactListener_(nullptr)
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
        // 当玩家与NPC碰撞时，按E键进行交互
        if (*isPlayerCollided_ && keyCode == EventKeyboard::KeyCode::KEY_E)
        {
            this->handlePlayerInteraction();
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

void NpcBase::setDialogues(const std::vector<std::string>& dialogues)
{
    dialogues_ = dialogues;
}
