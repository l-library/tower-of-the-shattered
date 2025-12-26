#pragma once
#include "cocos2d.h"
#include "TowerOfTheShattered.h"

// 房间出口信息
struct Exit {
    std::string direction;
    bool isActive = false;
    int targetRoomId = -1;
    std::string targetEnterFrom;     // 目标房间的进入方向
    cocos2d::Vec2 Position;
    cocos2d::Vec2 spawnPosition;     // 从这个口进来的出生点
};

// 对应入口
std::string targetEnterID(const std::string& enter)
{
    if (enter == UP)
        return DN;
    if (enter == DN)
        return UP;
    if (enter == RD)
        return LD;
    if (enter == RU)
        return LU;
    if (enter == LD)
        return RD;
    if (enter == LU)
        return RU;
}

// 房间数据
struct RoomData {
    int id;
    std::string tmxFile;
    std::unordered_map<std::string, Exit> exits;

    RoomData() : id(0), tmxFile("") {}

    RoomData(int roomId, const std::string& mapFile): id(roomId), tmxFile(mapFile){}

    // 进入后人物坐标
    cocos2d::Vec2 targetEnterLocation(const std::string& enter);

    // 设置出口
    void setExit(const std::string& dir, int targetRoom) 
    {
        cocos2d::Vec2 spawnPos;
        Exit exit;
        exit.direction = dir;
        exit.isActive = true;
        exit.targetRoomId = targetRoom;
        exit.targetEnterFrom = targetEnterID(dir);
        exit.spawnPosition = spawnPos;
        exits[dir] = exit;
    }

    // 获取出口
    Exit* getExit(const std::string& dir) {
        auto it = exits.find(dir);
        if (it != exits.end() && it->second.isActive) {
            return &it->second;
        }
        return nullptr;
    }
};

extern std::unordered_map<int, RoomData> g_roomDatabase;
extern int g_currentRoomId = 1; // 当前所在房间ID

cocos2d::Vec2 RoomData::targetEnterLocation(const std::string& enter)
{
    for (const auto& pair : exits)
    {
        const Exit& exit = pair.second;
        if (exit.targetEnterFrom == enter && exit.isActive)
        {
            // 直接使用全局数据库
            auto targetIt = g_roomDatabase.find(exit.targetRoomId);
            if (targetIt != g_roomDatabase.end())
            {
                const RoomData& targetRoom = targetIt->second;
                return exit.spawnPosition; // 返回存储的出生点坐标
            }
        }
    }
    return cocos2d::Vec2::ZERO;
}