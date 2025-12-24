#pragma once
#include "cocos2d.h"
#include "Entities/Player/Player.h"
#include "Scenes/GameCamera.h"

//����Scene������PlayerTestScene
//�ó������ڲ�����ҵĻ�������
class PlayerTestScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();//��������
    static cocos2d::Scene* createWithMap(const std::string& mapFile);

    virtual bool init() override;//��ʼ��

    void setupInput();//��ȡ����

    void PlayerTestScene::update(float dt);

    // implement the "static create()" method manually
    CREATE_FUNC(PlayerTestScene);//����һ��create����

    ~PlayerTestScene();
    std::string _currentMapFile = "maps/map_1.tmx";
private:
    Player* _player;
    GameCamera* _cameraController;

};