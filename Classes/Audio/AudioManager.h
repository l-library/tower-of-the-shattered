/***
* 这个类用于管理整个游戏中的各个音效/音乐
* 这是一个单例类，使用时，请采用 AudioManager::getInstance()获取它的唯一实例
* 基于AudioEngine实现，每个音效都有其对应的Id，创建时保留id可以方便后续的管理
* 设置音量时，范围在 0.0f-1.0f 之间
**/

#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

#include "cocos2d.h"
#include "audio/include/AudioEngine.h"

class AudioManager {
public:
    static AudioManager* getInstance();

    // 播放背景音乐 (loop: 是否循环)
    void playBGM(const std::string& filename, bool loop = true);

    // 播放音效 (返回 audioID，用于单独控制)
    int playEffect(const std::string& filename, bool loop = false, float pitch = 1.0f, float pan = 0.0f, float gain = 1.0f);

    // 停止/暂停控制
    void stopBGM();
    void pauseBGM();
    void resumeBGM();
    void stopAll(); // 停止所有声音

    // 背景音乐音量设置 (0.0f - 1.0f)
    void setBGMVolume(float volume);
    // 所有音效音量设置(0.0f - 1.0f)
    void setEffectsVolume(float volume);
    // 根据音效ID设置音效音量 (0.0f - 1.0f)
    void setEffectsVolumeById(int effect_id, float volume);

    // 预加载资源 (防止第一次播放卡顿)
    void preload(const std::string& filename);

    // 根据id停止指定的音频
    void AudioManager::stopById(int audio_id);

    /***
     * 播放带前奏的背景音乐
     * introFile: 前奏文件（只播一次）
     * loopFile:  循环主体文件（无限循环）
    **/
    void playIntroLoopBGM(const std::string& introFile, const std::string& loopFile);

private:
    AudioManager();
    static AudioManager* _instance;

    int _bgmID;           // 当前背景音乐的ID
    float _bgmVolume;     // 记录BGM音量
    float _effectVolume;  // 记录音效音量
};

#endif // __AUDIO_MANAGER_H__