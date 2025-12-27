#include "AudioManager.h"

USING_NS_CC;

AudioManager* AudioManager::_instance = nullptr;

AudioManager* AudioManager::getInstance() {
    if (!_instance) {
        _instance = new AudioManager();
    }
    return _instance;
}

AudioManager::AudioManager() : _bgmID(-1), _bgmVolume(0.5f), _effectVolume(1.0f) {
}

void AudioManager::playBGM(const std::string& filename, bool loop) {
    // 如果当前有BGM在播放，先停止
    if (_bgmID != -1) {
        AudioEngine::stop(_bgmID);
    }

    // 播放新的BGM
    _bgmID = AudioEngine::play2d(filename, loop, _bgmVolume);

    // 设置播放结束的回调
    if (!loop) {
        AudioEngine::setFinishCallback(_bgmID, [&](int id, const std::string& filePath) {
            _bgmID = -1;
            });
    }
}

int AudioManager::playEffect(const std::string& filename, bool loop, float pitch, float pan, float gain) {
    // gain * _effectVolume 实现了全局音效音量控制
    return AudioEngine::play2d(filename, loop, _effectVolume * gain);
}

void AudioManager::stopBGM() {
    if (_bgmID != -1) {
        AudioEngine::stop(_bgmID);
        // 如果设置了 FinishCallback，调用 stop 通常不会触发它，
        // 但为了逻辑严谨，我们必须将 ID 重置，
        // 这样 Intro 的回调函数里的 if (_bgmID == currentIntroID) 判断就会失败。
        _bgmID = -1;
    }
}

void AudioManager::pauseBGM() {
    if (_bgmID != -1) {
        AudioEngine::pause(_bgmID);
    }
}

void AudioManager::resumeBGM() {
    if (_bgmID != -1) {
        AudioEngine::resume(_bgmID);
    }
}

void AudioManager::stopAll() {
    AudioEngine::stopAll();
    _bgmID = -1;
}

void AudioManager::setBGMVolume(float volume) {
    _bgmVolume = volume;
    if (_bgmID != -1) {
        AudioEngine::setVolume(_bgmID, _bgmVolume);
    }
}

void AudioManager::setEffectsVolume(float volume) {
    _effectVolume = volume;
}

void AudioManager::setEffectsVolumeById(int effect_id, float volume) {
    AudioEngine::setVolume(effect_id, volume);
}

void AudioManager::preload(const std::string& filename) {
    AudioEngine::preload(filename);
}

void AudioManager::stopById(int audio_id){
    AudioEngine::stop(audio_id);
}

void AudioManager::playIntroLoopBGM(const std::string& introFile, const std::string& loopFile) {
    // 1. 先停止当前正在播放的BGM
    stopBGM();

    // 2. 关键：预加载两个文件，确保无缝衔接
    // 注意：preload 是异步的，但在本地文件较小的情况下通常很快。
    // 如果文件很大，建议在场景进入前（Loading界面）就统一preload。
    AudioEngine::preload(introFile);
    AudioEngine::preload(loopFile);

    // 3. 播放前奏 (不循环)
    _bgmID = AudioEngine::play2d(introFile, false, _bgmVolume);

    // 4. 设置播放结束的回调
    // 使用 lambda 表达式捕获 this 指针和 loopFile 路径
    // 注意：这里捕获当前的 _bgmID (introID) 用于校验
    int currentIntroID = _bgmID;

    AudioEngine::setFinishCallback(_bgmID, [this, loopFile, currentIntroID](int id, const std::string& filePath) {

        // 安全检查：
        // 只有当当前的 _bgmID 仍然等于当初播放 Intro 的 ID 时，才播放 Loop。
        // 如果在前奏播放期间用户调用了 stopBGM()，_bgmID 会变成 -1，这里就不会执行。
        // 如果用户切歌了，_bgmID 会变成新的 ID，这里也不会执行。
        if (_bgmID == currentIntroID) {
            _bgmID = AudioEngine::play2d(loopFile, true, _bgmVolume);
        }
        });
}