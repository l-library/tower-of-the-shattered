#pragma once
#include "cocos2d.h"
#include "SceneInfo.h"

class ISceneProvider {
public:
    virtual ~ISceneProvider() = default;

    // 获取指定ID的场景
    virtual cocos2d::Scene* getScene(int sceneId) = 0;

    // 获取下一个场景（用于线性流程）
    virtual cocos2d::Scene* getNextScene() = 0;

    // 获取当前场景信息
    virtual SceneInfo getCurrentSceneInfo() = 0;

    // 重置场景序列（用于重新开始）
    virtual void reset() = 0;
};