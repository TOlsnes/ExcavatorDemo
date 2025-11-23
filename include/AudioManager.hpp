#ifndef AUDIOMANAGER_HPP
#define AUDIOMANAGER_HPP

#include <threepp/audio/Audio.hpp>
#include <memory>
#include <string>

class AudioManager {
public:
    explicit AudioManager(threepp::AudioListener& listener);
    
    // Load sound files
    void loadStartupSound(const std::string& path);
    void loadIdleSound(const std::string& path);
    void loadHydraulicsSound(const std::string& path);
    void loadSteamSound(const std::string& path);
    void loadDigSound(const std::string& path);
    void loadDumpSound(const std::string& path);
    void loadCollisionSound(const std::string& path);
    void loadCoinSound(const std::string& path);
    
    // Engine control
    void startEngine();  // Play startup then transition to idle
    void updateEngine(float dt);  // Call each frame to handle transitions
    void setIdleVolume(float volume);
    bool isStartupComplete() const { return startupComplete_; }
    
    // Hydraulics control (with volume hierarchy: boom > stick > bucket)
    void playHydraulics(float volume);  // For upward movement
    void playHydraulicsIfNotPlaying(float volume);  // Only start if not already playing
    void stopHydraulics();
    
    // Steam control (with volume hierarchy: boom > stick > bucket)
    void playSteam(float volume);  // For downward movement
    void playSteamIfNotPlaying(float volume);  // Only start if not already playing
    void stopSteam();
    
    // Gameplay sounds
    void playDig();
    void playDump();
    void playCollision();
    void playCoin();
    
    // Volume control
    void setMasterVolume(float volume);
    void setEffectsVolume(float volume);
    
private:
    threepp::AudioListener& listener_;
    
    std::unique_ptr<threepp::Audio> startupSound_;
    std::unique_ptr<threepp::Audio> idleSound_;
    std::unique_ptr<threepp::Audio> hydraulicsSound_;
    std::unique_ptr<threepp::Audio> steamSound_;
    std::unique_ptr<threepp::Audio> digSound_;
    std::unique_ptr<threepp::Audio> dumpSound_;
    std::unique_ptr<threepp::Audio> collisionSound_;
    std::unique_ptr<threepp::Audio> coinSound_;
    
    bool startupComplete_ = false;
    float effectsVolume_ = 1.0f;
};

#endif // AUDIOMANAGER_HPP
