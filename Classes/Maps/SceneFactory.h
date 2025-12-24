#pragma once
#include "cocos2d.h"
#include "SceneInfo.h"

class SceneFactory {
public:
    // 当前：根据配置创建固定场景
    // 后期：可根据种子和算法动态创建场景
    cocos2d::Scene* createScene(const SceneInfo& info) {
        if (_isRandomMode) {
            return createRandomScene(info);
        }
        else {
            return createFixedScene(info);
        }
    }

    void setRandomMode(bool random, int seed = 0) {
        _isRandomMode = random;
        _randomSeed = seed;
    }

private:
    bool _isRandomMode = false;
    int _randomSeed = 0;
};