#pragma once

#include "Entity/GameObject.hpp"
#include "Entity/YState.hpp"
#include "Entity/WalkingState.hpp"
#include "Entity/Direction.hpp"

class MovingGameObject : public virtual GameObject
{
public:
	MovingGameObject() = default;
	~MovingGameObject() override = default;

	void update(float dt) override = 0;
	const sf::Vector2f& getMovement() const { return m_movement; }

protected:
	sf::Vector2f m_movement;
};