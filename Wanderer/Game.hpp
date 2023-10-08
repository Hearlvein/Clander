#pragma once

#include <SFML/Graphics.hpp>
#include "Scene/SceneManager.hpp"
#include "Utility/debug.hpp"

class Game
{
public:
	Game()
    {
        openLog();
        m_sceneManager.run();
        closeLog();
    }
	~Game() = default;

private:
	SceneManager m_sceneManager;
};

