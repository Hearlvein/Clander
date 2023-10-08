#pragma once

#include "Entity/AnimatedGameObject.hpp"

class Character : public virtual AnimatedGameObject
{
public:
	virtual void update(float dt);
	void setHp(unsigned int hp);
	const unsigned int getHp() const;
	const unsigned int getMaxHp() const;
	void takeDamage(unsigned int amount);
	bool isInvicible() const;
	void setIsInvicible(bool isInvicible, float time = 1.f);
	bool isAlive() const;
	
private:
	unsigned int m_hp = 100;
	const unsigned int m_maxHp = 100;
	bool m_isInvicible = false;
	float m_invicibilityTime;
	float m_invicibilityMaxTime;
};

