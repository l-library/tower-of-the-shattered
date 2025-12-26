#pragma once
#include "cocos2d.h"
#include "SceneInfo.h"
#include "ISceneProvider.h"

class FixedSceneProvider : public ISceneProvider {
private:
    // 场景配置列表（可从JSON加载）
    std::vector<SceneInfo> _sceneConfigs;
    int _currentIndex = 0;

    // 场景ID到实例的映射（缓存已创建的场景）
    std::unordered_map<int, cocos2d::Scene*> _sceneCache;

public:
    // 初始化固定场景序列
    void initialize(const std::vector<SceneInfo>& configs) {
        _sceneConfigs = configs;
        reset();
    }

    virtual cocos2d::Scene* getScene(int sceneId) override {
        // 从缓存获取或创建新场景
        if (_sceneCache.find(sceneId) != _sceneCache.end()) {
            return _sceneCache[sceneId];
        }

        // 查找配置
        for (const auto& info : _sceneConfigs) {
            if (info.id == sceneId) {
                auto scene = createSceneFromInfo(info);
                _sceneCache[sceneId] = scene;
                return scene;
            }
        }
        return nullptr;
    }

    virtual cocos2d::Scene* getNextScene() override {
        if (_currentIndex >= _sceneConfigs.size()) {
            return nullptr; // 所有场景完成
        }

        auto& info = _sceneConfigs[_currentIndex];
        auto scene = getScene(info.id);
        _currentIndex++;
        return scene;
    }

private:
    // 根据配置创建场景（工厂模式）
    cocos2d::Scene* createSceneFromInfo(const SceneInfo& info) {
        // 这里根据sceneClass动态创建场景
        // 后期可以改为反射或注册表
        if (info.sceneClass == "TrainingCampScene") {
            return TrainingCampScene::createScene(info.tmxFile);
        }
        else if (info.sceneClass == "ForestScene") {
            return ForestScene::createScene(info.tmxFile);
        }
        // ... 其他场景
        return nullptr;
    }
};