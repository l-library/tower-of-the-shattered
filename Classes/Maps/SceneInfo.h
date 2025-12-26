#pragma once
#include "cocos2d.h"

enum class SceneType {
    NORMAL = 0,     // 普通战斗房间
    BOSS,           // Boss房间
    SHOP,           // 商店房间
};

struct SceneInfo {
    int id;                       // 场景ID
    std::string name;             // 场景名称
    SceneType type;               // 场景类型（普通/商店/Boss）
    std::string sceneClass;       // 场景类名
    std::string tmxFile;          // TMX地图文件
    //std::string bgm;              // 背景音乐

    // 连接关系
    int nextSceneId = -1;         // 下一个场景ID（-1表示没有）

    // 构造函数方便创建
    SceneInfo(int id, const std::string& name, SceneType type,
        const std::string& sceneClass, const std::string& tmxFile)
        : id(id), name(name), type(type), sceneClass(sceneClass), tmxFile(tmxFile) {
    }
};