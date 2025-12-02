cmake_minimum_required(VERSION 3.16)
project(blocks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
set(THREEPP_BUILD_TESTS OFF)
set(THREEPP_BUILD_EXAMPLES OFF)
set(THREEPP_WITH_IMGUI ON)
FetchContent_Declare(
        threepp
        GIT_REPOSITORY https://github.com/markaren/threepp.git
        GIT_TAG ada259b72435e380c8e3def429fc8001233b1a8f
)
FetchContent_MakeAvailable(threepp)

# Catch2 for testing
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.1
)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

# Dear ImGui
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.91.0
)
FetchContent_GetProperties(imgui)
if (NOT imgui_POPULATED)
    FetchContent_MakeAvailable(imgui)
    add_library(imgui STATIC
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
    target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
    # threepp already links glfw & glad; link glfw for backend
    target_link_libraries(imgui PUBLIC glfw)
endif()

add_executable(main
        main_excavator_example.cpp
        # Visualization
        src/Visualization/Renderer.cpp
        src/Visualization/ParticleSystem.cpp
        src/Visualization/TrackMarkManager.cpp
        src/Visualization/World.cpp
        # Logic
        src/Logic/InputManager.cpp
        src/Logic/Excavator.cpp
        src/Logic/ObjectSpawner.cpp
        src/Logic/CollisionWorld.cpp
        src/Logic/DigZone.cpp
        src/Logic/DumpZone.cpp
        src/Logic/AudioManager.cpp
        src/Logic/Coin.cpp
        src/Logic/CoinManager.cpp
)

target_include_directories(main
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include        
        $<TARGET_PROPERTY:threepp::threepp,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(main PRIVATE threepp::threepp)
target_link_libraries(main PRIVATE imgui)
target_compile_definitions(main PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)

if (MSVC)
    target_compile_definitions(main PRIVATE NOMINMAX)
endif()

# ========================================
# Tests
# ========================================
enable_testing()

# Create a library for testable components (excludes main())
add_library(blocks_lib STATIC
        # Visualization
        src/Visualization/Renderer.cpp
        src/Visualization/ParticleSystem.cpp
        src/Visualization/TrackMarkManager.cpp
        src/Visualization/World.cpp
        # Logic
        src/Logic/InputManager.cpp
        src/Logic/Excavator.cpp
        src/Logic/ObjectSpawner.cpp
        src/Logic/CollisionWorld.cpp
        src/Logic/DigZone.cpp
        src/Logic/DumpZone.cpp
        src/Logic/AudioManager.cpp
        src/Logic/Coin.cpp
        src/Logic/CoinManager.cpp
)

target_include_directories(blocks_lib
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include        
        $<TARGET_PROPERTY:threepp::threepp,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(blocks_lib PUBLIC threepp::threepp imgui)
target_compile_definitions(blocks_lib PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)

if (MSVC)
    target_compile_definitions(blocks_lib PUBLIC NOMINMAX)
endif()

# Test executable
add_executable(blocks_tests
        tests/test_main.cpp
        tests/test_collision.cpp
        tests/test_particle.cpp
        tests/test_coin.cpp
)

target_link_libraries(blocks_tests PRIVATE blocks_lib Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(blocks_tests)