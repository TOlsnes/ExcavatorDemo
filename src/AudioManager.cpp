#include "AudioManager.hpp"
#include <iostream>

AudioManager::AudioManager(threepp::AudioListener& listener)
    : listener_(listener) {}

void AudioManager::loadStartupSound(const std::string& path) {
    try {
        startupSound_ = std::make_unique<threepp::Audio>(listener_, path);
        startupSound_->setLooping(false);
        startupSound_->setVolume(0.5f);
        std::cout << "Loaded startup sound: " << path << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load startup sound: " << e.what() << std::endl;
    }
}

void AudioManager::loadIdleSound(const std::string& path) {
    try {
        idleSound_ = std::make_unique<threepp::Audio>(listener_, path);
        idleSound_->setLooping(true);
        idleSound_->setVolume(0.2f);
        std::cout << "Loaded idle sound: " << path << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load idle sound: " << e.what() << std::endl;
    }
}

void AudioManager::loadHydraulicsSound(const std::string& path) {
    try {
        hydraulicsSound_ = std::make_unique<threepp::Audio>(listener_, path);
        hydraulicsSound_->setLooping(true);
        hydraulicsSound_->setVolume(0.4f);
        std::cout << "Loaded hydraulics sound: " << path << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load hydraulics sound: " << e.what() << std::endl;
    }
}

void AudioManager::loadSteamSound(const std::string& path) {
    try {
        steamSound_ = std::make_unique<threepp::Audio>(listener_, path);
        steamSound_->setLooping(true);
        steamSound_->setVolume(0.35f);
        std::cout << "Loaded steam sound: " << path << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load steam sound: " << e.what() << std::endl;
    }
}

void AudioManager::loadDigSound(const std::string& path) {
    try {
        digSound_ = std::make_unique<threepp::Audio>(listener_, path);
        digSound_->setVolume(0.5f * effectsVolume_);
        std::cout << "Loaded dig sound: " << path << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load dig sound: " << e.what() << std::endl;
    }
}

void AudioManager::loadDumpSound(const std::string& path) {
    try {
        dumpSound_ = std::make_unique<threepp::Audio>(listener_, path);
        dumpSound_->setVolume(0.6f * effectsVolume_);
        std::cout << "Loaded dump sound: " << path << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load dump sound: " << e.what() << std::endl;
    }
}

void AudioManager::loadCollisionSound(const std::string& path) {
    try {
        collisionSound_ = std::make_unique<threepp::Audio>(listener_, path);
        collisionSound_->setVolume(0.4f * effectsVolume_);
        std::cout << "Loaded collision sound: " << path << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load collision sound: " << e.what() << std::endl;
    }
}

void AudioManager::loadCoinSound(const std::string& path) {
    try {
        coinSound_ = std::make_unique<threepp::Audio>(listener_, path);
        coinSound_->setVolume(0.5f * effectsVolume_);
        std::cout << "Loaded coin sound: " << path << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load coin sound: " << e.what() << std::endl;
    }
}

void AudioManager::startEngine() {
    startupComplete_ = false;
    if (startupSound_) {
        startupSound_->play();
        std::cout << "[audio] Playing startup sound" << std::endl;
    }
}

void AudioManager::updateEngine(float dt) {
    // Transition from startup to idle when startup finishes
    if (!startupComplete_ && startupSound_) {
        if (!startupSound_->isPlaying()) {
            startupComplete_ = true;
            if (idleSound_ && !idleSound_->isPlaying()) {
                idleSound_->play();
                std::cout << "[audio] Transitioning to idle loop" << std::endl;
            }
        }
    }
}

void AudioManager::setIdleVolume(float volume) {
    if (idleSound_) {
        idleSound_->setVolume(volume);
    }
}

void AudioManager::playHydraulics(float volume) {
    if (hydraulicsSound_) {
        hydraulicsSound_->setVolume(volume);
        if (!hydraulicsSound_->isPlaying()) {
            hydraulicsSound_->play();
        }
    }
}

void AudioManager::playHydraulicsIfNotPlaying(float volume) {
    if (hydraulicsSound_ && !hydraulicsSound_->isPlaying()) {
        hydraulicsSound_->setVolume(volume);
        hydraulicsSound_->play();
    }
}

void AudioManager::stopHydraulics() {
    if (hydraulicsSound_ && hydraulicsSound_->isPlaying()) {
        hydraulicsSound_->stop();
    }
}

void AudioManager::playSteam(float volume) {
    if (steamSound_) {
        steamSound_->setVolume(volume);
        if (!steamSound_->isPlaying()) {
            steamSound_->play();
        }
    }
}

void AudioManager::playSteamIfNotPlaying(float volume) {
    if (steamSound_ && !steamSound_->isPlaying()) {
        steamSound_->setVolume(volume);
        steamSound_->play();
    }
}

void AudioManager::stopSteam() {
    if (steamSound_ && steamSound_->isPlaying()) {
        steamSound_->stop();
    }
}

void AudioManager::playDig() {
    if (digSound_) {
        if (digSound_->isPlaying()) {
            digSound_->stop();
        }
        digSound_->play();
    }
}

void AudioManager::playDump() {
    if (dumpSound_) {
        if (dumpSound_->isPlaying()) {
            dumpSound_->stop();
        }
        dumpSound_->play();
    }
}

void AudioManager::playCollision() {
    if (collisionSound_ && !collisionSound_->isPlaying()) {
        collisionSound_->play();
    }
}

void AudioManager::playCoin() {
    if (coinSound_) {
        if (coinSound_->isPlaying()) {
            coinSound_->stop();
        }
        coinSound_->play();
    }
}

void AudioManager::setMasterVolume(float volume) {
    listener_.setMasterVolume(volume);
}

void AudioManager::setEffectsVolume(float volume) {
    effectsVolume_ = volume;
    if (digSound_) digSound_->setVolume(0.5f * effectsVolume_);
    if (dumpSound_) dumpSound_->setVolume(0.6f * effectsVolume_);
    if (collisionSound_) collisionSound_->setVolume(0.4f * effectsVolume_);
    if (coinSound_) coinSound_->setVolume(0.5f * effectsVolume_);
}
