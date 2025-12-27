#pragma once
#include "cocos2d.h"
#include "json/rapidjson.h"
#include "json/document.h"
#include "json/stringbuffer.h"
#include "json/writer.h"
#include <fstream> // 用于文件写入

class WriteJson {
public:
    /**
    * @brief 向指定的json文件中写入或更新指定字段的内容
    * @param[in] filename 文件名（例如 "data.json"）
    * @param[in] key      字段名称
    * @param[in] value    要写入的字符串内容
    * @return 是否写入成功
    ***/
    static bool setString(std::string filename, std::string key, std::string value) {
        // 获取可写的完整路径
        // 数据持久化必须写入到 getWritablePath() 下。
        std::string writablePath = cocos2d::FileUtils::getInstance()->getWritablePath() + filename;

        rapidjson::Document doc;
        std::string contentStr;

        // 尝试读取现有文件内容
        // 优先从可写目录读取，如果不存在，再尝试从 Resources 读取（作为初始模版），如果都不存在则新建空对象
        if (cocos2d::FileUtils::getInstance()->isFileExist(writablePath)) {
            contentStr = cocos2d::FileUtils::getInstance()->getStringFromFile(writablePath);
        }
        else {
            // 如果可写目录没有，尝试读取包内资源作为初始数据
            std::string fullPath = cocos2d::FileUtils::getInstance()->fullPathForFilename(filename);
            if (!fullPath.empty()) {
                contentStr = cocos2d::FileUtils::getInstance()->getStringFromFile(fullPath);
            }
        }

        // 3. 解析 JSON
        if (contentStr.empty()) {
            doc.SetObject(); // 文件不存在或为空，初始化为空对象
        }
        else {
            doc.Parse(contentStr.c_str());
            // 如果解析出错或者原本不是Object类型（例如是数组），强制重置为空对象以保证后续逻辑
            if (doc.HasParseError() || !doc.IsObject()) {
                cocos2d::log("WriteJson Warning: Parse error or not an object, resetting %s", filename.c_str());
                doc.SetObject();
            }
        }

        // 4. 修改或添加字段
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        // 创建 Value 对象 (注意：需要使用 allocator 进行深拷贝，防止局部变量销毁导致指针悬空)
        rapidjson::Value jsonValue(value.c_str(), allocator);
        rapidjson::Value jsonKey(key.c_str(), allocator);

        if (doc.HasMember(key.c_str())) {
            // 如果字段已存在，直接修改
            doc[key.c_str()] = jsonValue;
        }
        else {
            // 如果字段不存在，添加新成员
            doc.AddMember(jsonKey, jsonValue, allocator);
        }

        // 5. 序列化 (将 Object 转回 String)
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        std::string outputStr = buffer.GetString();

        // 6. 写入文件
        // 使用标准 C++ 文件流写入，确保跨平台兼容性
        FILE* fp = fopen(writablePath.c_str(), "w");
        if (!fp) {
            cocos2d::log("WriteJson Error: Can not open file %s", writablePath.c_str());
            return false;
        }

        fputs(outputStr.c_str(), fp);
        fclose(fp);

        return true;
    }
};