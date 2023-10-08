#include "MapEditor.hpp"
#include "Constants.hpp"
#include <imgui-SFML.h>
#include <imgui.h>
#include <iomanip>


MapEditor::MapEditor(GameScene& gs, TilesManager& tm)
	: m_gs(gs)
	, m_tilesMgr(tm)
{
	loadTileVignettes();
	loadEntityVignettes();

	updateSelectedTile(m_tilesMgr.getTileFromIndex('a'));
}

void MapEditor::loadTileVignettes()
{
	const auto& tiles = m_tilesMgr.getTiles();
	m_tilesPerRow = tiles.size();
	int iterator_distance = 0;
	for (const auto& tile : tiles)
	{
		m_tilesSprites.emplace_back(
			m_gs.m_tileset,
			sf::IntRect(
				(int)tile.texCoords.x,
				(int)tile.texCoords.y,
				TILE_SIZEi,
				TILE_SIZEi
			)
		)
		.setPosition((float)(iterator_distance) * TILE_SIZEf, 0.f);
		++iterator_distance;
	}
}

void MapEditor::loadEntityVignettes()
{
	std::ifstream stream(TEXTURES_PATH + "entitiesData.json");
	if (stream)
	{
		nlohmann::json data;
		stream >> data;

		for (const auto& entity : data["entities"])
		{
			m_entitiesVignettes.push_back({
				entity["name"],
				sf::Sprite(m_gs.m_tileset,
				           {
					           entity["texBox"][0],
					           entity["texBox"][1],
					           entity["texBox"][2],
					           entity["texBox"][3]
						   })
			});
		}
	}
	else
		std::cerr << "Failed to open entities stream in map editor !" << std::endl;
}

void MapEditor::render()
{
	// Update map window read events state
	if (ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered())
		m_gs.setReadEvents(false);
	else
		m_gs.setReadEvents(true);

	renderHover();


	// ---------------- ImGUI stuff ----------------
	using namespace ImGui;

	Begin("Map Editor");

	Text("Save or load level");
	InputText("Level filename", m_levelFilenameBuffer, 64);
	if (Button("Save level"))
		saveLevel(LEVELS_PATH + m_levelFilenameBuffer);
	SameLine();
	if (Button("Load level"))
		m_gs.loadLevel(LEVELS_PATH + (const std::string)m_levelFilenameBuffer);


	Text("Tiles window");
	int imguiIds = 0;
	for (TileId i = 0; i < m_tilesSprites.size(); ++i) {
		PushID(imguiIds++);
		if (ImageButton(m_tilesSprites[i])) {
			m_selectingTile = true;
			m_selectedTile = i;
			updateHover();
		}
		PopID();

		if (i+1 < m_tilesSprites.size())
			SameLine(0.f, 0.f);
	}

	for (size_t i = 0; i < m_entitiesVignettes.size(); ++i) {
		PushID(imguiIds++);
		if (ImageButton(m_entitiesVignettes[i].sprite)) {
			m_selectingTile = false;
			m_selectedEntity = i;
			updateHover();
		}
		PopID();

		if (i+1 < m_entitiesVignettes.size())
			SameLine(0.f, 0.f);
	}

	Checkbox("Cam on player", &(m_gs.m_cameraOnPlayer));

	End();	// Map editor
}

void MapEditor::renderHover()
{
	if (!m_gs.getReadEvents())
		return;

	auto& w = *(m_gs.m_window);

	sf::Vector2i mouse = sf::Mouse::getPosition(w);
	// Mouse should be in the window to enable hovering
	if (mouse.x >= 0 &&
		mouse.y >= 0 &&
		(unsigned)mouse.x < w.getSize().x &&
		(unsigned)mouse.y < w.getSize().y)
	{
		sf::Vector2f coords = w.mapPixelToCoords(mouse);

		if (m_selectingTile)
		{
			sf::Vector2f floored(
				std::floor(coords.x / TILE_SIZEi) * TILE_SIZEi,
				std::floor(coords.y / TILE_SIZEi) * TILE_SIZEi
			);
			m_hover.setPosition(floored);
		}
		else
		{
			m_hover.setPosition(coords - sf::Vector2f{
				m_hover.getLocalBounds().width / 2,
				m_hover.getLocalBounds().height / 2
			});
		}

		w.draw(m_hover);
	}
}
void MapEditor::handleInputs()
{
	// ------ map window inputs below ------

	if (ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered())
	{
		// BREAKING THE FUNCTION!
		return;
	}

	// Pre-requisites: window active (with pointer inside) and edit delay respected
	if (!m_gs.m_window->hasFocus())
	{
		// BREAKING THE FUNCTION!
		return;
	}

	// mouse management
	char mouseCode = -1;	// unhandled value
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		mouseCode = 0;
	else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
		mouseCode = 1;
	else
	{
		// BREAKING THE FUNCTION!
		return;
	}

	if (m_selectingTile && m_editTileTimer.getElapsedTime() > m_editTileDelay)
	{
		placeOrRemoveTile(mouseCode);
		m_editTileTimer.restart();
	}
	else if (!m_selectingTile && m_editEntityTimer.getElapsedTime() > m_editEntityDelay)
	{
		placeOrRemoveEntity(mouseCode);
		m_editEntityTimer.restart();
	}
}

void MapEditor::handleMapWindowEvent(const sf::Event& event)
{
	// Middle mouse button selection
	if (event.type == sf::Event::MouseButtonPressed &&
		event.mouseButton.button == sf::Mouse::Middle)
	{
		sf::Vector2f worldCoords = m_gs.m_window->mapPixelToCoords(
			sf::Mouse::getPosition(*m_gs.m_window), m_gs.m_window->getView());

		sf::Vector2i mouseTileCoords{
			(int)std::floor(worldCoords.x / TILE_SIZEi),
			(int)std::floor(worldCoords.y / TILE_SIZEi)
		};

		updateSelectedTile(m_gs.m_map.getTile(mouseTileCoords.x, mouseTileCoords.y));
	}
}

void MapEditor::saveLevel(const std::string& levelFilename) const
{
	// Saving map
	std::ofstream mapSaveStream(levelFilename + "/map.txt");
	if (mapSaveStream)
	{
		for (int y = 0; y < m_gs.m_map.GRID_HEIGHT; ++y)
		{
			for (int x = 0; x < m_gs.m_map.GRID_WIDTH; ++x)
				mapSaveStream << m_gs.m_map.getTileIndex(x, y) << " ";

			if (y + 1 < m_gs.m_map.GRID_HEIGHT)
				mapSaveStream << '\n';
		}
	}
	else
		std::cerr << "couldn't create map save file stream!" << std::endl;


	// Saving entities
	std::ofstream entitiesSaveStream(levelFilename + "/entities.json");
	if (entitiesSaveStream)
	{
		nlohmann::json data;

		const auto& player = *(m_gs.m_player);
		data["player"] = { player.getPosition().x, player.getPosition().y };

		data["enemies"] = {};
		for (const Enemy* enemy : m_gs.m_enemies)
		{
			std::vector<float> vec_pos = { enemy->getPosition().x, enemy->getPosition().y };
			data["enemies"].emplace_back(std::move(vec_pos));
		}

		entitiesSaveStream << std::setw(4) << data;
	}
	else
		std::cerr << "couldn't create entities save file stream!" << std::endl;
}

void MapEditor::placeOrRemoveTile(int mouseCode)
{
	sf::Vector2f worldCoords = m_gs.m_window->mapPixelToCoords(
	sf::Mouse::getPosition(*m_gs.m_window), m_gs.m_window->getView());

	sf::Vector2i mouseTileCoords{
		(int)std::floor(worldCoords.x / TILE_SIZEi),
		(int)std::floor(worldCoords.y / TILE_SIZEi)
	};

	// To compare previous and after states
	sf::Vector2i previousGridSize{
		m_gs.m_map.GRID_WIDTH,
		m_gs.m_map.GRID_HEIGHT,
	};

	// Hint to know how to translate entities and view
	bool newColLeft = false;
	bool newRowTop = false;

	TileId newTile = (mouseCode == 0)
		? m_tilesMgr.getDefaultTile()
		: m_selectedTile;
	m_gs.m_map.setTile(mouseTileCoords.x, mouseTileCoords.y, newTile, &newColLeft, &newRowTop);

	// Translate entities and view if needed
	sf::Vector2f translation;
	if (newColLeft)	translation.x = static_cast<float>(m_gs.m_map.GRID_WIDTH  - previousGridSize.x) * TILE_SIZEi;
	if (newRowTop)	translation.y = static_cast<float>(m_gs.m_map.GRID_HEIGHT - previousGridSize.y) * TILE_SIZEi;

	if (translation != sf::Vector2f())
	{
		std::cout << "translating" << std::endl;

		for (GameObject* entity : m_gs.m_entities)
			entity->move(translation);

		m_gs.moveCamera(translation);
	}
}

void MapEditor::placeOrRemoveEntity(int mouseCode)
{
	sf::Vector2f worldCoords = m_gs.m_window->mapPixelToCoords(
	sf::Mouse::getPosition(*m_gs.m_window), m_gs.m_window->getView());
	sf::Vector2f bias{
		m_hover.getLocalBounds().width / 2,
		m_hover.getLocalBounds().height / 2
	};

	const std::string& entityType = m_entitiesVignettes[m_selectedEntity].name;
	if (entityType == "player")
	{
		// Whatever the mouse code, we tp the player to mouse
		m_gs.m_player->setPosition(worldCoords - bias);
	}
	else if (entityType == "enemy")
	{
		if (mouseCode == 0)
		{
			GameObject* gameObjPtr = nullptr;
			// Delete Enemy* ptr
			for (auto it = m_gs.m_enemies.begin(); it != m_gs.m_enemies.end(); ++it)
			{
				const auto& box = (*it)->getHitbox();
				if (boxContains(box, worldCoords))
				{
					gameObjPtr = *it;
					std::cout << "Found entity to destroy (" << gameObjPtr << ')' << std::endl;
					m_gs.m_enemies.erase(it);
//					std::cout << "nb enemies: " << m_gs.m_enemies.size() << std::endl;
					break;
				}
			}

			if (!gameObjPtr)
			{
				std::cerr << "Didn't find any enemy to destroy at cursor position!" << std::endl;
				// BREAKING THE FUNCTION!
				return;
			}

			// Delete GameObject* ptr
			for (auto it = m_gs.m_entities.begin(); it != m_gs.m_entities.end(); ++it)
			{
				if (*it == gameObjPtr)
				{
					m_gs.m_entities.erase(it);
//					std::cout << "nb entities: " << m_gs.m_entities.size() << std::endl;
					break;
				}
			}

			// Delete Drawable* in the mob layer
			const sf::Drawable* drawablePtr = gameObjPtr;
//			std::cout << "The drawable-version ptr: " << drawablePtr << std::endl;
			m_gs.m_layers["mobsLayer"].removeObject(drawablePtr);

			delete gameObjPtr;
			// EXITING THE FUNCTION...
		}
		else if (mouseCode == 1)
		{
			auto* enemy = new Enemy();
			m_gs.m_enemies.push_back(enemy);
			m_gs.m_entities.push_back(enemy);

			enemy->setPosition(worldCoords - bias);
			enemy->setTexture(m_gs.m_tileset);
			m_gs.m_layers["mobsLayer"].addObject(enemy);
		}
	}
}

void MapEditor::updateSelectedTile(TileId id)
{
	m_selectedTile = id;
	updateHover();
}

void MapEditor::updateHover()
{
	if (m_selectingTile)
	{
		m_hover = sf::Sprite(
		m_gs.m_tileset,
			sf::IntRect(
				(int)m_tilesMgr.getTileFromId(m_selectedTile).texCoords.x,
				(int)m_tilesMgr.getTileFromId(m_selectedTile).texCoords.y,
				TILE_SIZEi,
				TILE_SIZEi
		));
	}
	else
	{
		// Copying sprite
		m_hover = m_entitiesVignettes[m_selectedEntity].sprite;
	}

	m_hover.setColor(sf::Color(255, 255, 255, 128));
}