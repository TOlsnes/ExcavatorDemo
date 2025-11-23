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
        src/InputManager.cpp
        src/World.cpp
        src/Renderer.cpp
        src/Car.cpp
        src/Excavator.cpp
        src/ObjectSpawner.cpp
        src/CollisionWorld.cpp
        src/ParticleSystem.cpp
        src/DigZone.cpp
        src/DumpZone.cpp
        src/Gate.cpp
        src/AudioManager.cpp
        src/Coin.cpp
        src/CoinManager.cpp
        src/TrackMarkManager.cpp
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