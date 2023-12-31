#include "Scene/Map.hpp"
#include "Constants.hpp"

#include <algorithm>
#include <iostream>
#include <cassert>
#include <cmath>

[[maybe_unused]] void Map::printGrid()
{
	for (int y = 0; y < GRID_HEIGHT; ++y)
	{
		for (int x = 0; x < GRID_WIDTH; ++x)
			std::cout << (size_t)m_grid[x][y] << " ";
		std::cout << std::endl;
	}
}

bool Map::load(const std::string& filename)
{
	// reset
	GRID_WIDTH = GRID_HEIGHT = 0;
	m_grid.clear();
	m_vertices.clear();

	m_levelFilename = filename;

	std::ifstream file(filename);
	if (!file)
	{
		std::cerr << "Map load: can't open " << filename << std::endl;
		return false;
	}
	
	std::string line;

	// used in nested loops
	std::size_t i;
	int currentRowIndex = -1;

	// checks for file corruption. -1 if not initalized. otherwise value shouldn't change below
	int lineSize = -1;

	while (std::getline(file, line))
	{
		++currentRowIndex;  // counts number of rows

		for (i = 0; i < line.size(); ++i)   // trim whitespaces
		{
			if (line[i] == ' ')
				line.erase(i, 1);
		}

		if (lineSize == -1) // set up GRID_WIDTH automatically from file
		{
			lineSize = (int)line.size(); // new size, after trimming whitespaces
			GRID_WIDTH = lineSize;
			m_grid.resize(GRID_WIDTH);
		}

		// runtime assertion fails if level file is not properly set up
		assert((unsigned)lineSize == line.size());

		// reads file data and sets up m_grid procedurally
		for (i = 0; i < line.size(); ++i)
		{
			m_grid[i].reserve(currentRowIndex);
			m_grid[i].emplace_back(m_tilesMgr.getTileFromIndex(line[i]));
		}
	}

	// At this state, currentRowIndex represents the maximum index, so the vertical size-1 of the grid
	GRID_HEIGHT = currentRowIndex + 1;  // set up GRID_HEIGHT automatically from file

	m_virtualGround = 200;

	std::cout << GRID_WIDTH << "x" << GRID_HEIGHT << std::endl;

	// creates an optimized (smaller than m_grid) vertex array
	regenerateVertices();

	std::cout << "grid size: " << GRID_WIDTH * GRID_HEIGHT << std::endl;
	// std::cout << "vertex array size: " << m_vertices.size() << std::endl;


	return true;	// level loaded successfully
}

const std::string& Map::getLevelFilename() const
{
	return m_levelFilename;
}

void Map::setTile(int x, int y, TileId newTile, bool* newColLeft, bool* newRowTop)
{
	if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) // no increase, maybe decrease
	{
		// testing property first for optimization
		if (m_grid[x][y] == newTile)   // same tile
			return;

		m_grid[x][y] = newTile;

		if (getTileProperty(x, y) == Tile::Property::Void)  // try to reduce on deletion
		{
			// ----- Becareful functions: move entities and cam -----
			*newColLeft = removeEmptyColsLeft();
			*newRowTop = removeEmptyRowsTop();
			// ------------------------------------------------------


			removeEmptyColsRight();
			removeEmptyRowsBottom();
		}

		regenerateVertices();

		// std::cout << "vertex array size: " << m_vertices.size() << std::endl;
	}
	else if (newTile != m_tilesMgr.getDefaultTile())  // modify grid dimensions
	{
		// Adding bottom-right
		if (x >= GRID_WIDTH && y >= GRID_HEIGHT)
		{
			std::cout << "adding bottom right" << std::endl;

			addColsRight(x - GRID_WIDTH + 1);
			addRowsBottom(y - GRID_HEIGHT + 1);

			setTile(0, 0, newTile);
		}

		// Adding bottom only
		else if (x >= 0 && x < GRID_WIDTH && y >= GRID_HEIGHT)
		{
			std::cout << "adding bottom only" << std::endl;

			// need to add more rows
			addRowsBottom(y - GRID_HEIGHT + 1);

			setTile(x, y, newTile);
		}

		// Adding right only
		else if (x >= GRID_WIDTH && y >= 0 && y < GRID_HEIGHT)  // increase GRID_WIDTH (adding right)
		{
			std::cout << "adding right only" << std::endl;

			// need to add more cols
			addColsRight(x - GRID_WIDTH + 1);

			setTile(x, y, newTile);
		}

		// Adding top-left
		else if (x < 0 && y < 0)
		{
			std::cout << "adding top-left" << std::endl;

			*newRowTop = true;
			*newColLeft = true;

			addColsLeft(-x);
			addRowsTop(-y);

			setTile(0, 0, newTile);
		}

		// Adding top only
		else if (x >= 0 && x < GRID_WIDTH && y < 0)
		{
			std::cout << "adding top only" << std::endl;

			*newRowTop = true;

			addRowsTop(-y);

			setTile(x, 0, newTile);
		}

		// Adding left only
		else if (x < 0 && y >= 0 && y < GRID_HEIGHT)
		{
			std::cout << "adding left only" << std::endl;

			*newColLeft = true;

			addColsLeft(-x);

			setTile(0, y, newTile);
		}

		// Adding top-right
		else if (x >= GRID_WIDTH && y < 0)
		{
			std::cout << "Adding top right" << std::endl;

			*newRowTop = true;

			addRowsTop(-y);
			addColsRight(x - GRID_WIDTH + 1);

			setTile(x, 0, newTile);
		}

		// Adding bottom-left
		else if (x < 0 && y >= GRID_HEIGHT)
		{
			std::cout << "Adding bottom-left" << std::endl;

			*newColLeft = true;

			addColsLeft(-x);
			addRowsBottom(y - GRID_HEIGHT + 1);

			setTile(0, y, newTile);
		}

		else
			std::cerr << "unhandled insertion: (x=" << x << "; y=" << y << ")" << std::endl;
	}

	else
		std::cerr << "setTile: undef ref at " << x << ";" << y << std::endl;
}

void Map::addRowsBottom(std::size_t n)
{
	if (n == 0)
		return;

	GRID_HEIGHT += n;
	for (auto& col : m_grid)
	{
		col.reserve(GRID_HEIGHT);
		col.insert(col.end(), n, m_tilesMgr.getDefaultTile());
	}
}

void Map::addRowsTop(std::size_t number)
{
	for (auto& col : m_grid)
		col.insert(col.begin(), number, m_tilesMgr.getDefaultTile());
	GRID_HEIGHT += number;
}

void Map::addColsRight(std::size_t n)
{
	if (n == 0)
		return;

	GRID_WIDTH += n;
	m_grid.reserve(GRID_WIDTH);
	m_grid.insert(m_grid.end(), n, std::vector<TileId>(GRID_HEIGHT, m_tilesMgr.getDefaultTile()));
}

void Map::addColsLeft(std::size_t number)
{
	m_grid.insert(m_grid.begin(), number, std::vector<TileId>(GRID_HEIGHT, m_tilesMgr.getDefaultTile()));
	GRID_WIDTH += number;
}

void Map::removeEmptyColsRight()
{
	auto countVoid = [&]() -> int
	{
		if (!m_grid.empty())
			return std::count(m_grid.back().begin(), m_grid.back().end(), m_tilesMgr.getDefaultTile());
		return -1;
	};

	int count = countVoid();
	while (count == GRID_HEIGHT)
	{
		// delete col
		m_grid.pop_back();
		--GRID_WIDTH;

		count = countVoid();
	}
}

bool Map::removeEmptyColsLeft()
{
	auto countVoid = [&]() -> int
	{
		if (!m_grid.empty())
			return std::count(m_grid.front().begin(), m_grid.front().end(), m_tilesMgr.getDefaultTile());
		return -1;
	};

	int previousGridWidth = GRID_WIDTH;	// to check if one dim is modified

	int count = countVoid();
	while (count == GRID_HEIGHT)
	{
		// delete col
		m_grid.erase(m_grid.begin());
		--GRID_WIDTH;

		count = countVoid();
	}

	return GRID_WIDTH != previousGridWidth;
}

void Map::removeEmptyRowsBottom()
{
	auto countVoid = [&]() -> int
	{
		int c = 0;
		for (auto& col : m_grid)
		{
			if (col.back() == m_tilesMgr.getDefaultTile())
				++c;
		}
		return c;
	};

	int count = countVoid();
	while (count == GRID_WIDTH)
	{
		// delete row
		for (auto& col : m_grid)
			col.pop_back();
		--GRID_HEIGHT;

		count = countVoid();
	}
}

bool Map::removeEmptyRowsTop()
{
	auto countVoid = [&]() -> int
	{
		int c = 0;
		for (auto& col : m_grid)
		{
			if (col.front() == m_tilesMgr.getDefaultTile())
				++c;
		}
		return c;
	};

	int previousGridHeight = GRID_HEIGHT;	// to check if one dim is modified

	int count = countVoid();
	while (count == GRID_WIDTH)
	{
		// delete row
		for (auto& col : m_grid)
			col.erase(col.begin());
		--GRID_HEIGHT;

		count = countVoid();
	}

	return GRID_HEIGHT != previousGridHeight;
}

char Map::getTileIndex(int x, int y) const
{
	if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT)
		return m_tilesMgr.getTileFromId(m_grid[x][y]).indexInFile;
	else
		return m_tilesMgr.getTileFromId(m_tilesMgr.getDefaultTile()).indexInFile;
}

TileId Map::getTile(int x, int y) const
{
	if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT)
		return m_grid[x][y];
	else
	{
		//std::cout << "getTileProperty: undef ref at " << x << ";" << y << std::endl;
		return m_tilesMgr.getDefaultTile();
	}
}

Tile::Property Map::getTileProperty(int x, int y) const
{
	return m_tilesMgr.getTilePropertyFromId(getTile(x, y));
}

bool Map::touchingTile(const Box& box, Tile::Property tileProperty) const
{
	// Hypothetical values
	int x_min = (int)std::floor(box.x / TILE_SIZEf);
	int y_min = (int)std::floor(box.y / TILE_SIZEf);
	int x_max = (int)std::ceil((box.x + box.w - 1) / TILE_SIZEf);
	int y_max = (int)std::ceil((box.y + box.h - 1) / TILE_SIZEf);

	// Smarting values
	if (x_min < 0)              x_min = 0;
	if (y_min < 0)              y_min = 0;
	if (x_max > GRID_WIDTH)     x_max = GRID_WIDTH;
	// commented to use virtual ground
	//if (y_max > GRID_HEIGHT)  y_max = GRID_HEIGHT;

	for (int i = x_min; i < x_max; ++i)
	{
		for (int j = y_min; j < y_max; ++j)
		{
			if (getTileProperty(i, j) == tileProperty)
				return true;
		}
	}

	if (y_max >= m_virtualGround)   // make the player unable to fall forever
	{
		std::cout << "Keeping the entity on a virtual ground" << std::endl;
		// @return true
		return true;
	}

	return false;
}

sf::Vector2f Map::getWorldSize() const
{
	return sf::Vector2f(static_cast<float>(GRID_WIDTH), static_cast<float>(GRID_HEIGHT)) * TILE_SIZEf;
}

void Map::regenerateVertices()
{
	m_vertices.clear();

	// creates an optimized (smaller than m_grid) vertex array
	for (int i = 0; i < GRID_WIDTH; ++i)
	{
		for (int j = 0; j < GRID_HEIGHT; ++j)
		{
			if (getTileProperty(i, j) != Tile::Property::Void)  // reduce vertices number
			{
				// set position & texture coords
				auto& texCoords = m_tilesMgr.getTileTexCoordsFromId(getTile(i, j));   // for read-ability

				m_vertices.emplace_back(
					sf::Vector2f(i * TILE_SIZEi,j * TILE_SIZEi),
					texCoords);
				m_vertices.emplace_back(
					sf::Vector2f((i + 1) * TILE_SIZEi,j * TILE_SIZEi),
					sf::Vector2f(texCoords.x + TILE_SIZEi, texCoords.y));
				m_vertices.emplace_back(
					sf::Vector2f((i + 1) * TILE_SIZEi, (j + 1) * TILE_SIZEi),
					sf::Vector2f(texCoords.x + TILE_SIZEi, texCoords.y + TILE_SIZEi));
				m_vertices.emplace_back(
					sf::Vector2f( i * TILE_SIZEi, (j + 1) * TILE_SIZEi),
					sf::Vector2f(texCoords.x, texCoords.y + TILE_SIZEi));
			}
		}
	}
}

void Map::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.texture = m_texture;
	if (!m_vertices.empty())
		target.draw(&m_vertices[0], m_vertices.size(), sf::Quads, states);
}
