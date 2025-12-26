/**
* 这是一个工具类，仅含有一个函数getString
* 利用这个类，可以从特定的文件路径中的json读取指定字段的数据
* 使用这个类的前提是json文件不是数组
***/


#pragma once
#include "cocos2d.h"
#include "json/rapidjson.h"
#include "json/document.h"

class ReadJson {
public:
    /**
    * @brief 从指定的json文件中读取指定字段的内容
    * @param[in] json路径path，字段名称name
    * @return json字段对应内容
    ***/
    static std::string getString(std::string path , std::string name) {
        // 读取文件内容
        std::string fullPath = cocos2d::FileUtils::getInstance()->fullPathForFilename(path);
        std::string contentStr = cocos2d::FileUtils::getInstance()->getStringFromFile(fullPath);

        if (contentStr.empty()) {
            cocos2d::log("ReadJson Error: Failed to load %s", name.c_str());
            return false;
        }

        rapidjson::Document doc;
        doc.Parse(contentStr.c_str());

        if (doc.HasParseError()) {
            cocos2d::log("ReadJson Error: JSON Parse error %d", doc.GetParseError());
            return NULL;
        }

        if (doc.HasMember(name.c_str()))
            return doc[name.c_str()].GetString();
    }
};