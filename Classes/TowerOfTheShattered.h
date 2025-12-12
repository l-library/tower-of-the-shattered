//�Զ������ͷ�ļ����������ͷ�ļ���

// 游戏基础常量定义
#define GRID_SIZE 32               // 游戏格子大小（32x32像素）

// ��ײ����궨��
#define ENEMY_MASK          0x00000001  // ����
#define PLAYER_MASK         0x00000002  // ����
#define BORDER_MASK         0x00000004  // ��Ϸ�߿�
#define WALL_MASK           0x00000008  // ��Ϸ�����е�ǽ��
#define DAMAGE_WALL_MASK    0x00000010  // ��Ϸ�����п�����˺���ǽ��
#define BULLET_MASK         0x00000020  // �ӵ�
#define PLAYER_BULLET_MASK  0x00000040  // ��Ҳ������ӵ�
#define ENEMY_BULLET_MASK   0x00000080  // ���˲������ӵ�
#define PENETRATE_MASK      0x00000100  // ���Դ�ǽ������
#define BOUNCE_MASK         0x00000200  // ���Է���������

#include "Entities/Enemy/EnemyBase.h"
#include "Entities/Player/Player.h"
#include "Entities/Bullet/Bullet.h"
#include "Scenes/PlayerTestScene.h"