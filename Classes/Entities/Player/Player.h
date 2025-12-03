#pragma once
#include "cocos2d.h"

/**
* @brief 储存主角的各类信息 
* @details 这个类继承自Node
***/
class Player :cocos2d::Node
{
public:
	/**
	* @brief 创建一个player对象
	* @param[in] 无
	* @return 指向创建好的对象的指针Node*
	***/
	static cocos2d::Node* createNode();

	/**
	* @brief 初始化主角对象
	* @details 这个函数会在create函数中被调用
	***/
	virtual bool init();

	//利用宏生成一个create函数
	CREATE_FUNC(Player);
protected:
	cocos2d::Sprite* _sprite;//主角图像_sprite（待机动作的第一帧）

};