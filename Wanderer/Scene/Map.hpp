#pragma once

#include "TilesManager.hpp"
#include "Scene/Tile.hpp"
#include "Utility/Box.hpp"

#include <SFML/Graphics.hpp>
#include <functional>
#include <vector>
#include <string>

class Map : public sf::Drawable
{
public:
	explicit Map(TilesManager& mt) : m_tilesMgr(mt) {}
	~Map() override = default;

	[[maybe_unused]] void printGrid();

	void setTexture(const sf::Texture& texture) { m_texture = &texture; }
	bool load(const std::string& filename);
	[[nodiscard]] const std::string& getLevelFilename() const;

	void setTile(int x, int y, TileId newTile, bool* newColLeft = nullptr, bool* newRowTop = nullptr);

	void addColsRight	(std::size_t n);
	void addColsLeft	(std::size_t n);
	void addRowsBottom	(std::size_t n);
	void addRowsTop		(std::size_t n);

	void removeEmptyColsRight();
	bool removeEmptyColsLeft();
	void removeEmptyRowsBottom();
	bool removeEmptyRowsTop();

	[[nodiscard]] char getTileIndex(int x, int y) const;
	[[nodiscard]] TileId getTile(int x, int y) const;    // get tile from index
	[[nodiscard]] Tile::Property getTileProperty(int x, int y) const;

	[[nodiscard]] bool touchingTile(const Box& box, Tile::Property tileProperty) const;
	[[nodiscard]] sf::Vector2f getWorldSize() const;

private:
	friend class MapEditor;

	/** heavy internal method called everytime grid is modified **/
	void regenerateVertices();
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	// Grid
	std::vector< std::vector<TileId> > m_grid;	// Pointers shouldn't be invalidated, nor be nullptr
	int GRID_WIDTH = 0;
	int GRID_HEIGHT = 0;
	const sf::Vector2i m_minimumGridSize { 3, 3 };

	// Tiles list
	TilesManager& m_tilesMgr;	// Helper that boxContains all posible tiles grid  refers to

	// Graphics
	const sf::Texture* m_texture = nullptr;
	std::vector<sf::Vertex> m_vertices;

	// Behavior
	int m_virtualGround = -1;	// entities won't fall forever
	std::string m_levelFilename;
};
