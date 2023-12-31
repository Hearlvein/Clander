#pragma once

#include "Utility/Box.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>

// Holds a single animation for an AnimatedGameObject
class Animation
{
public:
	Animation() = default;
	~Animation() = default;

	// update
	void reset()	// reset m_elapsedTime and m_currentFrameIndex
	{
		m_frameChanged = true;
		m_elapsedTime = 0.f;
		m_currentFrameIndex = 0;
	}

	void increaseElapsedTime(float modifier)	// increase m_elapsedTime and update m_currentFrameIndex if needed
	{
		m_elapsedTime += modifier;
		// std::cout << "new elapsed time: " << m_elapsedTime << " on " << getName() << std::endl;
		std::size_t newCurrentFrameIndex = m_currentFrameIndex;
		
        if (m_elapsedTime > m_frameDurations[m_currentFrameIndex])
        {
            // change animation frame (frameCount = 1 => index = 0, index < frameCount)
            if (++newCurrentFrameIndex >= m_frameCount)
                newCurrentFrameIndex = 0;

            m_elapsedTime = 0.f;
        }

		if (newCurrentFrameIndex != m_currentFrameIndex)
		{
		    // texture optimization: need to change the current texture
		    // std::cout << "changing m_currentFrameIndex" << std::endl;
			m_currentFrameIndex = newCurrentFrameIndex;
			m_frameChanged = true;
		}
	}

	bool frameChanged()
	{
	    bool toReturn = m_frameChanged;
	    m_frameChanged = false;
		return toReturn;
	}

	// Getters for data
	[[nodiscard]] const std::string& getName() const { return m_name; }
	[[nodiscard]] size_t getCurrentFrameIndex() const { return m_currentFrameIndex; }
	[[nodiscard]] const std::vector<float>& getFrameDurations() const { return m_frameDurations; }
	[[nodiscard]] const std::vector<sf::IntRect>& getSubTextureCoords() const { return m_subTextureCoords; }
	[[nodiscard]] const std::vector<Box>& getHitboxCoords() const { return m_hitboxCoords; }

	// Getters for state
	[[nodiscard]] const sf::IntRect& getCurrentSubTextureCoords() const { return m_subTextureCoords[m_currentFrameIndex]; }
	[[nodiscard]] const Box& getCurrentRelativeHitboxCoords() const { return m_hitboxCoords[m_currentFrameIndex]; }

private:
	friend class AnimatedGameObject;	// for easy properties access

	// save animation state
	bool m_frameChanged = true;
	float m_elapsedTime = 0.f;
	size_t m_currentFrameIndex = 0;

	// reference data
	std::string m_name;
	size_t m_frameCount = 0;
	std::vector<float> m_frameDurations;
	std::vector<sf::IntRect> m_subTextureCoords;
	std::vector<Box> m_hitboxCoords;
};
