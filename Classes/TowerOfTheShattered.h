/* 这个头文件存放开发者创建的文件路径，方便之后的引用 */

// 游戏基础常量定义
#define GRID_SIZE	32		// 游戏格子大小（32x32像素）
#define MAP_SIZE	1.25f	// 地图缩放

// 碰撞掩码定义
#define ENEMY_MASK          0x00000001  // 敌人
#define PLAYER_MASK         0x00000002  // 玩家
#define BORDER_MASK         0x00000004  // 游戏边界
#define WALL_MASK           0x00000008  // 游戏场景中的墙壁
#define DAMAGE_WALL_MASK    0x00000010  // 游戏场景中有伤害的墙壁
#define BULLET_MASK         0x000000c0  // 子弹(包含下面两种子弹，4+8=c)
#define PLAYER_BULLET_MASK  0x00000040  // 玩家发射的子弹
#define ENEMY_BULLET_MASK   0x00000080  // 敌人发射的子弹
#define PENETRATE_MASK      0x00000100  // 可穿透墙壁的物体
#define BOUNCE_MASK         0x00000200  // 可弹跳的物体
#define NPC_MASK            0x00000400  // NPC
#define SENSOR_MASK			0x00001000	// 切换关卡检测碰撞体
#define MACHINE_MASK		0x00002000	// 触发机关检测碰撞体
#define ITEM_MASK           0x00004000  // 掉落物
#define EVERYTHING_MASK		0xFFFFFFFF	// 碰撞一切

// 房间出口
#define LD "left_down"
#define LU "left_up"
#define RD "right_down"
#define RU "right_up"
#define UP "up"
#define DN "down"

#include "Entities/Enemy/EnemyBase.h"
#include "Entities/Player/Player.h"
#include "Entities/Bullet/Bullet.h"
#include "Scenes/PlayerTestScene.h"
#include "Audio/AudioManager.h"
#include "Entities/Items/ItemManager.h"
#include "Entities/Items/Items.h"
#include "Scenes/MainMenuScene.h"
#include "Tools/ReadJson.h"
