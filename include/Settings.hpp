#pragma once

namespace Settings {
    //---------------------------------------
    //----------Universal Constants----------
    //---------------------------------------
    constexpr float PI = 3.14159265359f;
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
    float leftTrackSpeed_{0.f};   // meters per second
    float rightTrackSpeed_{0.f};  
    float leftTrackDist_{0.f};    // accumulated distance
    float rightTrackDist_{0.f};
    int leftTrackFrame_{0};       // 0,1,2 -->
    int rightTrackFrame_{0};
    float trackCircumference_{0.3f}; 

    //-----------------------------------------------
    //----------Excavator init joint angles----------
    //-----------------------------------------------
    float turretYaw_{0.f};
    float boomAngle_{0.f};
    float stickAngle_{0.f};
    float bucketAngle_{0.f};

    //------------------------------------------
    //----------Excavator joint limits----------
    //------------------------------------------
    // Stops joints in the boom, stick and bucket from rotating in unrealistic ways
    float boomMin_{-0.2f}; // radians
    float boomMax_{0.2f};
    float stickMin_{-0.5f};
    float stickMax_{1.2f};
    float bucketMin_{0.0f};
    float bucketMax_{0.5f};

    //-------------------------------------------------
    //----------Excavator debugging variables----------
    //-------------------------------------------------
    bool stickUseMaxEnd_{false}; // for testing joint limits
    bool bucketUseMaxEnd_{false};
    float stickNudgeZ_{0.f}; // small offset to tweak position
    float bucketNudgeZ_{0.f};

    bool bucketLoaded_{false}; // whether bucket has material (for dig/dump)

    //------------------------------------------------
    //----------Excavator movement variables----------
    //------------------------------------------------
    float targetLeftTrackSpeed_{0.f};
    float targetRightTrackSpeed_{0.f};
    float acceleration_{1.5f};      
    float deceleration_{3.0f};     
    float baseYaw_{0.f}; //(radians, around vertical)
    float trackWidth_{1.0f}; // distance between tracks
    float baseRadius_{0.8f}; // approximate radius for base/tracks for obstacle collision

    //---------------------------------------------
    //----------Excavator particle system----------
    //---------------------------------------------
    float particleSpawnTimer_{0.f};
    const float particleSpawnInterval_{0.05f}; // spawn every 50ms when moving
    const float speedThresholdForParticles_{0.3f}; // min speed to spawn particles

    //-----------------------------------------
    //----------Audio system settings----------
    //-----------------------------------------
    bool startupComplete_ = false;
    float effectsVolume_ = 1.0f;

    //----------------------------------
    //----------Coin variables----------
    //----------------------------------
    int collectedCount_ = 0;
    bool collected_ = false;
    float rotationSpeed_ = 2.0f;
    float bobHeight_ = 0.5f;
    float bobSpeed_ = 3.0f;
    float time_ = 0.0f;

    //----------------------------------------
    //----------Track mark variables----------
    //----------------------------------------
    float distanceAccumulator_ = 0.f;
    float spawnDistance_ = 0.6f; // distance between marks
    float lifetime_ = 3.f;       // seconds a mark persists
    float markWidth_ = 0.25f;    // default individual track imprint width 
    float markLength_ = 0.25f;   // default imprint length along travel 
    float trackSeparation_ = 1.0f; // distance between track centers
}
