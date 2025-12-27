#include "RoomData.h"

USING_NS_CC;

// 定义并初始化全局变量（在一个cpp文件中）
std::unordered_map<int, RoomData> g_roomDatabase;
int g_currentRoomId = 1; // 在这里初始化

// 实现 RoomData::targetEnterLocation 方法
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