#pragma once
#include "cocos2d.h"
#include "SceneInfo.h"
#include "ISceneProvider.h"

class GameSceneManager {
private:
    static GameSceneManager* _instance;
    ISceneProvider* _sceneProvider = nullptr;
    SceneInfo _currentSceneInfo;

public:
    static GameSceneManager* getInstance();

    // 初始化（可切换不同的Provider）
    void initialize(ISceneProvider* provider) {
        if (_sceneProvider) {
            delete _sceneProvider;
        }
        _sceneProvider = provider;
    }

    // 切换到指定ID的场景
    void switchToScene(int sceneId) {
        if (!_sceneProvider) return;

        auto scene = _sceneProvider->getScene(sceneId);
        if (scene) {
            // 保存当前场景信息
            _currentSceneInfo = _sceneProvider->getCurrentSceneInfo();

            // 执行切换（可添加过渡效果）
            cocos2d::Director::getInstance()->replaceScene(
                cocos2d::TransitionFade::create(1.0f, scene)
            );
        }
    }

    // 切换到下一个场景（线性流程）
    void switchToNextScene() {
        if (!_sceneProvider) return;

        auto scene = _sceneProvider->getNextScene();
        if (scene) {
            // 切换场景
            cocos2d::Director::getInstance()->replaceScene(scene);
        }
    }

    // 重启当前场景
    void restartCurrentScene() {
        if (!_sceneProvider || _currentSceneInfo.id <= 0) return;
        switchToScene(_currentSceneInfo.id);
    }

    // 获取可用连接场景（用于后期分支选择）
    std::vector<SceneInfo> getAvailableConnections() {
        // 从_currentSceneInfo.connectedScenes获取
        // 当前版本可能返回空或只有一个选项
        std::vector<SceneInfo> result;
        // ... 实现
        return result;
    }
};