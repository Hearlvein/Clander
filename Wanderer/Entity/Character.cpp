#include "Entity/Character.hpp"
#include "Utility/util.hpp"

void Character::update(float dt)
{
	if (m_isInvicible)
	{
		m_invicibilityTime += dt;
		if (m_invicibilityTime >= m_invicibilityMaxTime)
			setIsInvicible(false);
	}

	/*if (!isAlive())
		std::cout << "character died" << std::endl;*/

	AnimatedGameObject::increaseElapsedTime(dt);
}

void Character::setHp(unsigned int hp)
{
	m_hp = (hp > m_maxHp) ? m_maxHp : hp;
}

const unsigned int Character::getHp() const
{
	return m_hp;
}

const unsigned int Character::getMaxHp() const
{
	return m_maxHp;
}

void Character::takeDamage(unsigned int amount)
{
	if (!m_isInvicible)
	{
		if (amount >= m_hp)
			m_hp = 0;
		else
			m_hp -= amount;
	}
}

bool Character::isInvicible() const
{
	return m_isInvicible;
}

void Character::setIsInvicible(bool isInvicible, float time)
{
	if (m_isInvicible == isInvicible || !isAlive())
		return;

	m_isInvicible = isInvicible;

	if (m_isInvicible)
	{
		m_invicibilityMaxTime = time;
		m_invicibilityTime = 0.f;
		std::cout << "invicible" << std::endl;
	}
	else
	{
		std::cout << "no more invicible" << std::endl;
	}
}

bool Character::isAlive() const
{
	return m_hp > 0;
}
