cmake_minimum_required(VERSION 3.21)
project(Clander)

set(CMAKE_CXX_STANDARD 17)
set(SFML_DIR C:/dev/SFML-2.5.1-mingw32-7.3.0/lib/cmake/SFML)
set(SOURCE_FILES
        Wanderer/main.cpp
        Wanderer/Entity/Character.cpp
        Wanderer/Entity/Enemy.cpp
        Wanderer/Entity/GameObject.cpp
        Wanderer/Entity/MovingCharacter.cpp
        Wanderer/Scene/GameScene.cpp
        Wanderer/Scene/SceneManager.cpp
        Wanderer/Scene/Layer.cpp
        Wanderer/Scene/Map.cpp
        Wanderer/Editor/MapEditor.cpp
        Wanderer/Utility/Box.cpp
        Wanderer/Utility/debug.cpp
        Wanderer/Utility/util.cpp)

add_library(imgui STATIC
        C:/dev/Clander/vendor/imgui/imgui.cpp
        C:/dev/Clander/vendor/imgui/imgui_draw.cpp
        C:/dev/Clander/vendor/imgui/imgui_demo.cpp
        C:/dev/Clander/vendor/imgui/imgui_widgets.cpp)

add_library(imgui-sfml STATIC
        C:/dev/imgui-sfml/imgui-SFML.cpp)

include_directories(
        C:/dev/Clander/Wanderer
        C:/dev/SFML-2.5.1-mingw32-7.3.0/include
        C:/dev/imgui
        C:/dev/imgui-sfml
        C:/dev/nlohmann)
find_package(SFML COMPONENTS graphics window system REQUIRED)

add_executable(Clander ${SOURCE_FILES})
target_link_libraries(Clander imgui imgui-sfml sfml-graphics sfml-window sfml-system opengl32)
