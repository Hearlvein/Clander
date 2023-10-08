#pragma once

#include <SFML/Graphics.hpp>
#include <list>

class Layer : public sf::Drawable
{
public:
	Layer() = default;
	~Layer() override = default;

	void clear();
	void addObject(const sf::Drawable* drawable);
	void removeObject(const sf::Drawable* drawableToRemove);

private:
	void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;

	std::list<const sf::Drawable*> m_drawables;
};
