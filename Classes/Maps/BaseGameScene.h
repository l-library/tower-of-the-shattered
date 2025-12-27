#pragma once
#include "cocos2d.h"
#include "SceneInfo.h"
#include "ISceneProvider.h"

class BaseGameScene : public cocos2d::Scene {
protected:
    SceneInfo _sceneInfo;           // 场景配置信息
    cocos2d::TMXTiledMap* _tileMap; // 地图

public:
    // 统一初始化接口
    virtual bool initWithInfo(const SceneInfo& info);

    // 公共接口（所有场景都需要实现）
    virtual void onPlayerEnter() = 0;
    virtual void onPlayerExit() = 0;
    virtual void onRoomCleared() = 0;

    // 加载地图（可被重写以支持动态生成）
    virtual void loadMap(const std::string& tmxFile);

    // 生成房间内容（预留接口，后期实现随机）
    virtual void generateRoomContents() {
        // 当前版本：从TMX加载固定内容
        // 后期：根据随机种子生成敌人、宝箱等
    }

    setupNormalLevel();
};

// 具体场景实现
class ForestScene : public BaseGameScene {
public:
    static cocos2d::Scene* createScene(const SceneInfo& info) {
        auto scene = ForestScene::create();
        scene->initWithInfo(info);
        return scene;
    }

    virtual bool initWithInfo(const SceneInfo& info) override {
        if (!BaseGameScene::initWithInfo(info)) {
            return false;
        }

        // 森林特有的初始化
        setBGM("music/forest.mp3");
        setAmbientLight(Color3B(100, 150, 100));

        return true;
    }
};

