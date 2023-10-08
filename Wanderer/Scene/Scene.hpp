#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>


class Scene
{
public:
	explicit Scene(sf::RenderWindow* window)
        : m_window(window)
    {
        std::cerr << "Creating a new scene (" << this << ')' << std::endl;
    }
	virtual ~Scene()
    {
        std::cerr << "Destroying a scene (" << this << ')' << std::endl;
    }

	virtual void handleEvent(const sf::Event& event) = 0;
	virtual void checkInput() = 0;
	virtual void update(float dt) = 0;
	virtual void draw(sf::RenderTarget& target) = 0;

protected:
	sf::RenderWindow* m_window;
};

