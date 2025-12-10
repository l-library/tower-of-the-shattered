//自定义类的头文件包含在这个头文件中

// 碰撞掩码宏定义
#define ENEMY_MASK          0x00000001  // 敌人
#define PLAYER_MASK         0x00000002  // 主角
#define BORDER_MASK         0x00000004  // 游戏边框
#define WALL_MASK           0x00000008  // 游戏场景中的墙壁
#define DAMAGE_WALL_MASK    0x00000010  // 游戏场景中可造成伤害的墙壁
#define BULLET_MASK         0x00000020  // 子弹
#define PLAYER_BULLET_MASK  0x00000040  // 玩家产生的子弹
#define ENEMY_BULLET_MASK   0x00000080  // 敌人产生的子弹
#define PENETRATE_MASK      0x00000100  // 可以穿墙的物体
#define BOUNCE_MASK         0x00000200  // 可以反弹的物体

#include "Entities/Enemy/Enemy.h"
#include "Entities/Player/Player.h"
#include "Entities/Bullet.h"
#include "Scenes/PlayerTestScene.h"