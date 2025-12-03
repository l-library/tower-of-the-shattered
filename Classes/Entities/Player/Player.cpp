#include "Player.h"

USING_NS_CC;

Node* Player::createNode()
{
	return Player::create();
}

bool Player::init()
{
	if (!Node::init())
	{
		return false;
	}

	//加载主角图像
	_sprite = Sprite::create("player/Knight_idle_first.png");
	this->addChild(_sprite);

	//获取动画缓存的单个实例
	auto cache = AnimationCache::getInstance();
	//从缓存中取出动画
	//初始为待机动画idle
	auto animation2 = cache->getAnimation("idle");

	auto action2 = Animate::create(animation2);
	_sprite->runAction(Sequence::create(action2, action2->reverse(), NULL));

	return true;
}
