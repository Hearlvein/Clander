#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>


struct Tile {
	// Describe how entities intereact with the tile
	enum class Property
	{
		Void = 0,
		Solid,
		Ladder
	};

	std::string name;		// Comprehensible name [optional]
	char indexInFile;		// In-file representation
	sf::Vector2f texCoords;	// Position in tileset
	Property property;		// Physical nature
};
