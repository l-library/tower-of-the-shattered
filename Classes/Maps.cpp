#include "Maps.h"

USING_NS_CC;

Scene* Maps::createScene()
{
    Scene* scene = Scene::createWithPhysics();
    scene->getPhysicsWorld()->setGravity(Vec2(0, -980));

    Maps* layer = Maps::create();
    scene->addChild(layer);
    return scene;
}

bool Maps::init()
{
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto map_1 = TMXTiledMap::create("maps/maps.tmx");
    auto layer = map_1->getLayer("layer_1");
    auto mapsize = map_1->getMapSize();
    auto tilesize = map_1->getTileSize();
    auto siz = visibleSize.height / (mapsize.height * tilesize.height);

    for (int y = 0; y < mapsize.height; ++y)
    {
        for (int x = 0; x < mapsize.width; ++x)
        {
            int gid = layer->getTileGIDAt(Vec2(x, y));
            if (gid == 0) 
                continue;          // Ìø¹ý¿Õ°×¸ñ

            auto shape = PhysicsShapeBox::create(tilesize);
            auto body = PhysicsBody::create();
            body->addShape(shape);
            body->setDynamic(false);
            auto node = Sprite::create("maps/platform.png");
            node->setPosition(siz * Vec2(
                x * tilesize.width + tilesize.width / 2,
                (mapsize.height - 1 - y) * tilesize.height + tilesize.height / 2));
            node->setPhysicsBody(body);
            node->setScale(siz);
            this->addChild(node);
        }
    }
    return true;
}

