#pragma once

#include "Scene/Tile.hpp"

#include <nlohmann/json.hpp>
#include <fstream>
#include <list>

typedef size_t TileId;


class TilesManager
{
public:
	void loadTiles(const std::string& filename)
	{
		std::ifstream stream(filename);
		if (stream)
		{
			nlohmann::json data;
			stream >> data;
			std::string usedIndexes;

			// Browse tiles
			for (auto& tileData : data["tiles"])
			{
				m_tiles.push_back({
					(std::string)tileData["name"],
					(char)((std::string)tileData["index"]).front(),
					sf::Vector2f(tileData["texCoords"][0], tileData["texCoords"][1]),
					parseTileProperty(tileData["property"])
				});

				char currentIndex = m_tiles.back().indexInFile;
				if (usedIndexes.find(currentIndex), usedIndexes.cbegin(), usedIndexes.cend() != usedIndexes.cend())
					std::cerr << "Index " << currentIndex << " was already used!" << std::endl;
				usedIndexes += currentIndex;
			}

			// '.' character usually represents void
			m_defaultTile = getTileFromIndex('.');
		}
		else
			std::cerr << "Couldn't open stream to load tiles" << std::endl;
	}

	void clearTiles()
	{
		m_tiles.clear();
	}


	[[nodiscard]] Tile getTileFromId(TileId id) const { return m_tiles[id]; }
	[[nodiscard]] Tile::Property getTilePropertyFromId(TileId id) const {
		return getTileFromId(id).property;
	}
	[[nodiscard]] const sf::Vector2<float> getTileTexCoordsFromId(TileId id) const {
		return getTileFromId(id).texCoords;
	}

	[[nodiscard]] TileId getTileFromIndex(char index) const
	{
		for (size_t i = 0; i < m_tiles.size(); ++i)
		{
			if (m_tiles[i].indexInFile == index)
				return (TileId)i;
		}

		std::cerr << "Critical error: can't get tile from index(" << index << ")\n" << std::flush;
		return (TileId)0;
	}
	[[nodiscard]] TileId getDefaultTile() const { return m_defaultTile; }

	[[nodiscard]] std::vector<Tile> getTiles() const { return m_tiles; }

	static Tile::Property parseTileProperty(const std::string& str) {
		if      (str == "Void")   return Tile::Property::Void;
		else if (str == "Solid")  return Tile::Property::Solid;
		else if (str == "Ladder") return Tile::Property::Ladder;
		else {
			std::cerr << "Couldn't get tile property from: \'" << str << "\'" << std::endl;
			return Tile::Property::Void;
		}
	}

private:
	std::vector<Tile> m_tiles;
	TileId m_defaultTile = 0;	// See loadTiles()
};
