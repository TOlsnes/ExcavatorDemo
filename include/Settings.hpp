#pragma once

namespace Settings {
    //---------------------------------------
    //----------Universal Constants----------
    //---------------------------------------
    constexpr float PI_ = 3.14159265359f;
    constexpr int MaxRocks = 50;
    constexpr float ExcavatorBaseRadius = 0.8f;
    constexpr float TrackWidth = 1.0f;
    constexpr float ParticleSpawnInterval = 0.05f;
    constexpr float SpeedThresholdForParticles = 0.3f;

    // Example tunable variables (can be changed at runtime)    
    inline float masterVolume = 1.0f;
    inline bool showCollisionDebug = false;
    // ...add more as needed

    //------------------------------------------
    //----------Track animation states----------
    //------------------------------------------
    inline float leftTrackSpeed_{0.f};   // meters per second
    inline float rightTrackSpeed_{0.f};  
    inline float leftTrackDist_{0.f};    // accumulated distance
    inline float rightTrackDist_{0.f};
    inline int leftTrackFrame_{0};       // 0,1,2 -->
    inline int rightTrackFrame_{0};
    inline float trackCircumference_{0.3f}; 

    //-----------------------------------------------
    //----------Excavator init joint angles----------
    //-----------------------------------------------
    inline float turretYaw_{0.f};
    inline float boomAngle_{0.f};
    inline float stickAngle_{0.f};
    inline float bucketAngle_{0.f};

    //------------------------------------------
    //----------Excavator joint limits----------
    //------------------------------------------
    // Stops joints in the boom, stick and bucket from rotating in unrealistic ways
    inline float boomMin_{-0.2f}; // radians
    inline float boomMax_{0.2f};
    inline float stickMin_{-0.5f};
    inline float stickMax_{1.2f};
    inline float bucketMin_{0.0f};
    inline float bucketMax_{0.5f};

    //-------------------------------------------------
    //----------Excavator debugging variables----------
    //-------------------------------------------------
    inline bool stickUseMaxEnd_{false}; // for testing joint limits
    inline bool bucketUseMaxEnd_{false};
    inline float stickNudgeZ_{0.f}; // small offset to tweak position
    inline float bucketNudgeZ_{0.f};

    inline bool bucketLoaded_{false}; // whether bucket has material (for dig/dump)

    //------------------------------------------------
    //----------Excavator movement variables----------
    //------------------------------------------------
    inline float targetLeftTrackSpeed_{0.f};
    inline float targetRightTrackSpeed_{0.f};
    inline float acceleration_{1.5f};      
    inline float deceleration_{3.0f};     
    inline float baseYaw_{0.f}; //(radians, around vertical)
    inline float trackWidth_{1.0f}; // distance between tracks
    inline float baseRadius_{0.8f}; // approximate radius for base/tracks for obstacle collision

    //---------------------------------------------
    //----------Excavator particle system----------
    //---------------------------------------------
    inline float particleSpawnTimer_{0.f};
    inline constexpr float particleSpawnInterval_{0.05f}; // spawn every 50ms when moving
    inline constexpr float speedThresholdForParticles_{0.3f}; // min speed to spawn particles

    //-----------------------------------------
    //----------Audio system settings----------
    //-----------------------------------------
    inline bool startupComplete_ = false;
    inline float effectsVolume_ = 1.0f;

    //----------------------------------
    //----------Coin variables----------
    //----------------------------------
    inline int collectedCount_ = 0;
    // Coin animation tuning (per-coin state lives in Coin now)
    inline float rotationSpeed_ = 1.5f; // slower spin
    inline float bobHeight_ = 0.4f;     // slightly lower amplitude
    inline float bobSpeed_ = 1.2f;      // much slower bob

    //----------------------------------------
    //----------Track mark variables----------
    //----------------------------------------
    inline float distanceAccumulator_ = 0.f;
    inline float spawnDistance_ = 0.6f; // distance between marks
    inline float lifetime_ = 3.f;       // seconds a mark persists
    inline float markWidth_ = 0.25f;    // default individual track imprint width 
    inline float markLength_ = 0.25f;   // default imprint length along travel 
    inline float trackSeparation_ = 1.0f; // distance between track centers
}
