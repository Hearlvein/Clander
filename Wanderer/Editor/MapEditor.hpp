#pragma once

#include "Scene/Map.hpp"
#include "Entity/GameObject.hpp"
#include "Constants.hpp"
#include "Scene/GameScene.hpp"
#include "Scene/Tile.hpp"

#include <SFML/Graphics.hpp>
#include <fstream>
#include <cmath>
#include <map>

class Map;

class MapEditor
{
public:
	MapEditor(GameScene& gs, TilesManager& tm);
	~MapEditor() = default;

	void loadTileVignettes();
	void loadEntityVignettes();

	void render();
	void renderHover();

	void handleInputs();
	void handleMapWindowEvent(const sf::Event& event);
	void saveLevel(const std::string& levelFilename) const;
	void placeOrRemoveTile(int mouseCode);
	void placeOrRemoveEntity(int mouseCode);

	void updateSelectedTile(TileId id);
	void updateHover();

private:
	GameScene& m_gs;
	TilesManager& m_tilesMgr;

	std::size_t m_tilesPerRow;

	// Inventory placable
	std::vector<sf::Sprite> m_tilesSprites;
	struct EntityVignette
	{ std::string name; sf::Sprite sprite; };
	std::vector<EntityVignette> m_entitiesVignettes;

	// Selection vars
	bool m_selectingTile = true;
	TileId m_selectedTile = 0;
	size_t m_selectedEntity = 0;

	char m_levelFilenameBuffer[64] = "";
	sf::Sprite m_hover;

	sf::Time m_editTileDelay = sf::milliseconds(10);
	sf::Clock m_editTileTimer;	// prevent from spamming
	sf::Time m_editEntityDelay = sf::milliseconds(1000);
	sf::Clock m_editEntityTimer;	// prevent from spamming
};
