#pragma once

#include "Entity/MovingCharacter.hpp"
#include "Constants.hpp"

class Player : public MovingCharacter
{
public:
	Player()
	{
		AnimatedGameObject::loadFromFile(ANIMATIONS_PATH + "player.txt");
		AnimatedGameObject::setCurrentAnimationName("right");
	}

	~Player() override = default;

	// virtual void update(float dt) override; // no need to overwrite for the moment
};