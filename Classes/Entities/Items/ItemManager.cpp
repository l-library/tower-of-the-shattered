#include "ItemManager.h"
#include "../Player/Player.h"
#include "json/rapidjson.h"
#include "json/document.h"
#include "cocos2d.h"

USING_NS_CC;

ItemManager* ItemManager::_instance = nullptr;

ItemManager* ItemManager::getInstance() {
    if (!_instance) {
        _instance = new ItemManager();
    }
    return _instance;
}

void ItemManager::destroyInstance() {
    if (_instance) {
        delete _instance;
        _instance = nullptr;
    }
}

ItemManager::ItemManager() : _gold(100) {}

ItemManager::~ItemManager() {
    _itemConfigCache.clear();
    _ownedItemIds.clear();
}

bool ItemManager::init(const std::string& configFilename) {
    // 读取文件内容
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(configFilename);
    std::string contentStr = FileUtils::getInstance()->getStringFromFile(fullPath);

    if (contentStr.empty()) {
        log("ItemManager Error: Failed to load %s", configFilename.c_str());
        return false;
    }

    rapidjson::Document doc;
    doc.Parse(contentStr.c_str());

    if (doc.HasParseError()) {
        log("ItemManager Error: JSON Parse error %d", doc.GetParseError());
        return false;
    }

    if (!doc.IsArray()) {
        log("ItemManager Error: JSON root is not an array");
        return false;
    }

    // 解析每个物品
    for (const auto& itemJson : doc.GetArray()) {
        ItemData data;

        // 必须字段
        if (!itemJson.HasMember("id")) continue;
        data.id = itemJson["id"].GetInt();

        data.name = itemJson.HasMember("name") ? itemJson["name"].GetString() : "Unknown";
        data.description = itemJson.HasMember("desc") ? itemJson["desc"].GetString() : "";
        data.flavorText = itemJson.HasMember("information") ? itemJson["information"].GetString() : "";
        data.iconPath = itemJson.HasMember("icon") ? itemJson["icon"].GetString() : "items/default.png";

        // 解析效果类型
        std::string typeStr = itemJson.HasMember("effect_type") ? itemJson["effect_type"].GetString() : "NONE";
        data.effectValue = itemJson.HasMember("value") ? itemJson["value"].GetDouble() : 0.0;

        if (typeStr == "HP_MAX_UP") data.effectType = ItemEffectType::HP_MAX_UP;
        else if (typeStr == "MP_MAX_UP") data.effectType = ItemEffectType::MP_MAX_UP;
        else if (typeStr == "ATTACK_UP") data.effectType = ItemEffectType::ATTACK_UP;
        else if (typeStr == "SPEED_UP") data.effectType = ItemEffectType::SPEED_UP;
        else if (typeStr == "MP_RESTORE_UP") data.effectType = ItemEffectType::MP_RESTORE_UP;
        else if (typeStr == "DODGE_COOLDOWN") data.effectType = ItemEffectType::DODGE_COOLDOWN;
        else if (typeStr == "JUMP_FORCE_UP") data.effectType = ItemEffectType::JUMP_FORCE_UP;
        else if (typeStr == "DODGE_TIME_UP") data.effectType = ItemEffectType::DODGE_TIME_UP;
        else if (typeStr == "SKILL_DAMAGE_UP") data.effectType = ItemEffectType::SKILL_DAMAGE_UP;
        else if (typeStr == "DOUBLE_DODGE") data.effectType = ItemEffectType::DOUBLE_DODGE;
        else data.effectType = ItemEffectType::NONE;

        // 存入缓存
        _itemConfigCache[data.id] = data;
    }

    log("ItemManager: Loaded %d items.", (int)_itemConfigCache.size());
    return true;
}

void ItemManager::gainItem(Player* player, int itemId) {
    if (_itemConfigCache.find(itemId) == _itemConfigCache.end()) {
        log("ItemManager: Item ID %d not found.", itemId);
        return;
    }

    _ownedItemIds.push_back(itemId);

    // 立即应用效果
    if (player) {
        applyEffect(player, _itemConfigCache[itemId], true);
    }

    log("ItemManager: Gained item %s", _itemConfigCache[itemId].name.c_str());
}

bool ItemManager::hasItem(int itemId) const {
    for (int id : _ownedItemIds) {
        if (id == itemId) return true;
    }
    return false;
}

const ItemData* ItemManager::getItemConfig(int itemId) const {
    auto it = _itemConfigCache.find(itemId);
    if (it != _itemConfigCache.end()) {
        return &it->second;
    }
    return nullptr;
}

void ItemManager::applyEffect(Player* player, const ItemData& item, bool isAdd) {
    // 如果是移除物品，数值取反
    double value = isAdd ? item.effectValue : -item.effectValue;

    switch (item.effectType) {
        case ItemEffectType::HP_MAX_UP:
            player->modifyMaxHealth(value); 
            log("Applied HP change: %f", value);
            break;
        case ItemEffectType::MP_MAX_UP:
            player->modifyMaxMagic(value);
            log("Applied MP change: %f", value);
            break;

        case ItemEffectType::ATTACK_UP:
            player->modifyAttackDamage(value);
            log("Applied Attack change: %f", value);
            break;

        case ItemEffectType::SPEED_UP:
            player->modifySpeed(value);
            log("Applied Speed change: %f", value);
            break;

        case ItemEffectType::MP_RESTORE_UP:
            player->modifyMagicRestore(value);
            log("Applied mp resutore change: %f", value);
            break;
        case ItemEffectType::SKILL_DAMAGE_UP:
            player->setSkillDamage(value);
            log("Applied skill damage change: %f", value);
            break;
        case ItemEffectType::DODGE_TIME_UP:
            player->modifyDodgeTime(value);
            log("Applied dodge time change: %f", value);
            break;
        case ItemEffectType::DODGE_COOLDOWN:
            player->modifyDodgeCooldown(value);
            log("Applied dodge cool down change: %f", value);
            break;
        case ItemEffectType::JUMP_FORCE_UP:
            player->modifyJumpForce(value);
            log("Applied jump force change: %f", value);
            break;
        case ItemEffectType::DOUBLE_DODGE:
            player->setDoubleDODGE(value);
            log("Applied double_dodge change: %f", value);
            break;

        case ItemEffectType::NONE:
        default:
            break;
    }
}

// ---- 金币 ----
void ItemManager::addGold(int amount) {
    _gold += amount;
}

bool ItemManager::spendGold(int amount) {
    if (_gold >= amount) {
        _gold -= amount;
        return true;
    }
    return false;
}

// ---- UI ----
cocos2d::Sprite* ItemManager::createItemIcon(int itemId) {
    auto config = getItemConfig(itemId);
    if (!config) return nullptr;

    auto sprite = Sprite::create(config->iconPath);
    if (!sprite) {
        // 如果找不到图片，用一个默认的方块代替，避免崩溃
        sprite = Sprite::create();
        sprite->setTextureRect(Rect(0, 0, 32, 32));
        sprite->setColor(Color3B::GRAY);
    }
    return sprite;
}