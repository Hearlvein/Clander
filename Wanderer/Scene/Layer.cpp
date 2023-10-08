#include <iostream>
#include "Layer.hpp"

void Layer::clear()
{
	m_drawables.clear();
}

void Layer::addObject(const sf::Drawable* drawable)
{
//	std::cerr << "Adding (" << drawable << ") to some layer" << std::endl;
	m_drawables.push_back(drawable);
}

void Layer::removeObject(const sf::Drawable* drawableToRemove)
{
	for (auto it = m_drawables.begin(); it != m_drawables.end(); ++it)
	{
		if (*it == drawableToRemove)
		{
			m_drawables.erase(it);
			break;
		}
	}
}

void Layer::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
	for (const sf::Drawable* drawable : m_drawables)
		rt.draw(*drawable, states);
}
