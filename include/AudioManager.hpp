#ifndef AUDIOMANAGER_HPP
#define AUDIOMANAGER_HPP

#include <threepp/audio/Audio.hpp>
#include <memory>
#include <string>
#include "Settings.hpp"

class AudioManager {
public:
    explicit AudioManager(threepp::AudioListener& listener);
    
    // sound files
    void loadStartupSound(const std::string& path);
    void loadIdleSound(const std::string& path);
    void loadHydraulicsSound(const std::string& path);
    void loadSteamSound(const std::string& path);
    void loadCoinSound(const std::string& path);
    
    // Engine 
    void startEngine();  
    void updateEngine(float dt);  // Call to update sounds
    void setIdleVolume(float volume);
    bool isStartupComplete() const { return Settings::startupComplete_; }
    
    // Hydraulics 
    void playHydraulics(float volume);  // For upward movement
    void playHydraulicsIfNotPlaying(float volume);  
    void stopHydraulics();
    
    // Steam control 
    void playSteam(float volume);  // For downward movement
    void playSteamIfNotPlaying(float volume); 
    void stopSteam();
    
    // Gameplay 
    void playCoin();
    // Maybe ni the future add dig, dump and collision

    // Volume 
    void setMasterVolume(float volume);
    void setEffectsVolume(float volume);
    
private:
    threepp::AudioListener& listener_;
    
    std::unique_ptr<threepp::Audio> startupSound_;
    std::unique_ptr<threepp::Audio> idleSound_;
    std::unique_ptr<threepp::Audio> hydraulicsSound_;
    std::unique_ptr<threepp::Audio> steamSound_;
    std::unique_ptr<threepp::Audio> coinSound_;
};

#endif // AUDIOMANAGER_HPP
