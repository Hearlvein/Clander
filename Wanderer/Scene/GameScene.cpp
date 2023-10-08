#include "Editor/MapEditor.hpp"
#include "Scene/GameScene.hpp"
#include "Constants.hpp"
#include "Utility/util.hpp"

#include <iostream>
#include <cassert>
#include <cmath>

#include <imgui-SFML.h>
#include <imgui.h>
#include <nlohmann/json.hpp>


GameScene::GameScene(sf::RenderWindow* window)
	: Scene(window)
	, m_map(m_tilesMgr)
{
	// ------------------- textures (resource: loaded once) -------------------
	m_tileset.loadFromFile(TEXTURES_PATH + "tileset.png");
	m_backgroundTexture.loadFromFile(TEXTURES_PATH + "background.png");

	// TODO: level.txt is some kind of default level to load. Should be customizable (levelData.json)
	loadLevel(LEVELS_PATH + "parkour");	// load map and entities


	// Setting up player health box in the update function updateHealthBox()
	m_PHB.setFillColor(sf::Color::Red);
	m_layers["game_gui"].addObject(&m_PHBOutline);
	m_layers["game_gui"].addObject(&m_PHB);

	// TODO: debug purpose
	m_mapEditor = new MapEditor(*this, m_tilesMgr);
}

GameScene::~GameScene()
{
	delete m_mapEditor;
	destroyEntities();
}

void GameScene::loadLevel(const std::string& levelFilename)
{
	// reset
	m_layers["backgroundLayer"].clear();
	m_layers["mapLayer"].clear();
	m_layers["mobsLayer"].clear();
	m_layers["playerLayer"].clear();
	destroyEntities();
	m_tilesMgr.clearTiles();
	// TODO: dirty: can't reset position because background changed as the player is (initially) centered (placeCameraOnPlayer)
	// m_background.resetPosition();

	// intialize

	// background
	m_background.getSprite()->setTexture(m_backgroundTexture);
	m_layers["backgroundLayer"].addObject(m_background.getDrawable());

	// Tiles
	// Important: load tiles before map because map uses the tiles (obviously!)
	m_tilesMgr.loadTiles(TEXTURES_PATH + "tilesData.json");	// Setting up pointers before creating map

	// Map
	m_map.setTexture(m_tileset);
	m_map.load(levelFilename + "/map.txt");

	// Entities
	loadEntities(levelFilename + "/entities.json");
	m_layers["mapLayer"].addObject(&m_map);

	// reset camera
	m_window->setView(m_window->getDefaultView());

	// Player should be registered after loading
	assert(m_player);
}

void GameScene::destroyEntities()
{
	m_player = nullptr;
	m_enemies.clear();

	while (!m_entities.empty())
	{
		delete m_entities.back();
		m_entities.pop_back();
	}
}

void GameScene::setReadEvents(bool read)
{
	m_readEvents = read;
}

bool GameScene::getReadEvents() const
{
	return m_readEvents;
}

void GameScene::updateHealthBox(float dt)
{
	float sw = m_window->getView().getSize().x;
	float sh = m_window->getView().getSize().y;
	sf::Vector2f top_left = m_window->mapPixelToCoords({ 0, 0 });

	float obw = sw * 0.8f;
	float obh = sh * 0.05f;
	float obx = top_left.x + (sw - obw) / 2;
	float oby = top_left.y + sh * 0.9f;
	m_PHBOutline.setSize({ obw, obh });
	m_PHBOutline.setPosition(obx, oby);
	m_PHBOutline.setOutlineColor(sf::Color::Magenta);
	m_PHBOutline.setOutlineThickness(3.f);
	m_PHBOutline.setFillColor(sf::Color::Transparent);

	float padding = 2.f;
	m_PHB.setPosition(obx + padding, oby + padding);

	if (m_PHBUpdateWidth)
	{
		std::cout << "Updating PHB!" << std::endl;

		float PHBMaxWidth = obw - 2 * padding;
		if (m_player->getHp() == m_player->getMaxHp())
		{
			// Set up full health bar for first time
			m_PHB.setSize({ PHBMaxWidth, obh - 2 * padding });
			m_PHBUpdateWidth = false;
		}
		else
		{
			// Adjusted width
			float ratio = (float)m_player->getHp() / (float)m_player->getMaxHp();
			float targetWidth = ratio * PHBMaxWidth;
			float currentWidth = m_PHB.getSize().x;
			float dx = m_PHBVelocity * dt * sign(targetWidth - currentWidth);

			float newWidth = currentWidth + dx;
			if (sign(targetWidth - newWidth) != sign(targetWidth - currentWidth))	// Go past the target
			{
				m_PHB.setSize({ targetWidth, obh - 2 * padding });
				m_PHBUpdateWidth = false;	// End animation
			}
			else
				m_PHB.setSize({ newWidth, obh - 2 * padding });
		}
	}
}

void GameScene::handleEvent(const sf::Event& event)
{
	if (!m_readEvents)
		return;

	if (event.type == sf::Event::Closed)
	{
		m_window->close();
		return;
	}
	else if (event.type == sf::Event::Resized)
	{
		sf::FloatRect visibleArea(0.f, 0.f, (float)event.size.width, (float)event.size.height);
		m_window->setView(sf::View(visibleArea));
	}

	// Mouse coordinates in world coordinates
	else if (event.type == sf::Event::KeyPressed)
	{
		if (event.key.code == sf::Keyboard::W)
		{
			auto mousePosition = m_window->mapPixelToCoords(sf::Mouse::getPosition(*m_window));
			std::cout << "mouse position: " << mousePosition.x << " ; " << mousePosition.y << std::endl;
		}
		else if (event.key.code == sf::Keyboard::E)
		{
			if (m_mapEditor)
			{
				delete m_mapEditor;
				m_mapEditor = nullptr;
			}
			else
				m_mapEditor = new MapEditor(*this, m_tilesMgr);
		}
	}


	if (m_mapEditor)
	{
		m_mapEditor->handleMapWindowEvent(event);
	}
}

void GameScene::checkInput()
{
	if (m_mapEditor)
		m_mapEditor->handleInputs();

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		__debugbreak();

	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::H))
	{
		m_player->setHp(100);
		m_PHBUpdateWidth = true;
	}

	if (!m_readEvents)
		return;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
	{
		m_player->setFacing(Direction::Right);
		m_player->setWalkingState(WalkingState::Beginning);
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
	{
		m_player->setFacing(Direction::Left);
		m_player->setWalkingState(WalkingState::Beginning);
	}
	else
		m_player->setWalkingState(WalkingState::End);


	// Check for jump
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		m_player->setYState(YState::Jumping);

	// Check for ladder
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && m_map.touchingTile(m_player->getHitbox(), Tile::Property::Ladder))
	{
		m_player->setYState(YState::Climbing);
		m_player->setClimbingDirection(Direction::Up);
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && m_map.touchingTile(m_player->getHitbox(), Tile::Property::Ladder))
	{
		m_player->setYState(YState::Climbing);
		m_player->setClimbingDirection(Direction::Down);
	}
	else if (m_map.touchingTile(m_player->getHitbox(), Tile::Property::Ladder))
		m_player->setClimbingDirection(Direction::None);
}

void GameScene::update(float dt)
{	 
	m_lastDt = dt;

	// Enemy collision
	if (!m_player->isInvicible())
	{
		for (auto enemy : m_enemies)
		{
			if (boxesOverlapping(enemy->getHitbox(), m_player->getHitbox()))
			{
				if (m_player->isAlive())
				{
					m_player->takeDamage(20);
					m_player->setIsInvicible(true, 1.f);

					// Animation request
					m_PHBUpdateWidth = true;
				}
			}
		}
	}

	// internal player update
	m_player->update(dt);

	// external player update
	updateClimbingState(*m_player);
	moveEntity(*m_player);

	// enemies update
	for (auto enemy : m_enemies)
	{
		enemy->update(dt);
		// no updateClimbingState because entities can't climb ladders
		moveEnemy(*enemy);
	}

	updateCamera();
	updateHealthBox(dt);
}

void GameScene::updateClimbingState(MovingCharacter& entity)
{
	if (entity.getYState() == YState::Climbing && !m_map.touchingTile(m_player->getHitbox(), Tile::Property::Ladder))
		entity.setYState(YState::Falling);
}

void GameScene::loadEntities(const std::string& entitiesFilename)
{
	std::ifstream stream(entitiesFilename);
	if (stream)
	{
		auto registerEntity = [&](const std::string& entityType, float x, float y)
		{
			if (entityType == "player")
			{
				// Allocation
				assert(!m_player);	// m_player should be initialized once per level loading
				m_player = new Player();
				m_entities.push_front(m_player);

				m_player->setPosition(x, y);
				m_player->setTexture(m_tileset);
				m_layers["playerLayer"].addObject(m_player);
			}
			else if (entityType == "enemy")
			{
				// Allocation
				auto* enemy = new Enemy();
				m_enemies.push_back(enemy);
				m_entities.push_back(enemy);

				enemy->setPosition(x, y);
				enemy->setTexture(m_tileset);
				m_layers["mobsLayer"].addObject(enemy);
			}
			else
				std::cerr << "!!! Calling loadEntities with name type=" << entityType << "!!!" << std::endl;
		};

		nlohmann::json data;
		stream >> data;

		// Loading the player
		registerEntity("player", data["player"][0], data["player"][1]);

		// Loading the entities
		for (const auto& enemy : data["enemies"])
			registerEntity("enemy", enemy[0], enemy[1]);
	}
	else
	{
		std::cerr << "Failed to open: " << entitiesFilename << std::endl;
	}
}

void GameScene::moveEntity(MovingCharacter& entity, bool* xCollision)
{
	Box hitbox = entity.getHitbox();

	float dx = entity.getMovement().x;
	float dy = entity.getMovement().y;

	// Y checking
	if (dy != 0.f)
	{
		if (std::abs(dy) >= TILE_SIZEf)
		{
			//std::cout << "dy=" << dy;
			dy = (TILE_SIZEf - 1.f) * ((dy < 0) ? -1.f : 1.f);	// maximum dy with care of sign
			//std::cout << " -> " << dy << std::endl;
		}

		if (!m_map.touchingTile({hitbox.x, hitbox.y+dy, hitbox.w, hitbox.h}, Tile::Property::Solid))
			hitbox.y += dy;
		else
		{
			if (entity.getYState() == YState::Falling)
			{
				hitbox.y = std::ceil((hitbox.y + hitbox.h - 1.f) / TILE_SIZEf) * TILE_SIZEf - hitbox.h;
				entity.setYState(YState::Grounded);
			}
			else if (entity.getYState() == YState::Jumping)
			{
				//std::cout << "block ahead keeping from jumping higher" << std::endl;
				entity.setYState(YState::Falling);
			}
		}
	}

	// X checking
	if (dx != 0.f)
	{
		if (std::abs(dx) >= TILE_SIZEf)
		{
			//std::cout << "dx=" << dx;
			dx = (TILE_SIZEf - 1.f) * ((dx < 0) ? -1.f : 1.f);	// maximum dx with care of sign
			//std::cout << " -> " << dx << std::endl;
		}

		if (!m_map.touchingTile({hitbox.x+dx, hitbox.y, hitbox.w, hitbox.h}, Tile::Property::Solid))
			hitbox.x += dx;
		else
		{
			if (dx > 0)
				hitbox.x = std::ceil((hitbox.x + hitbox.w - 1.f) / TILE_SIZEf) * TILE_SIZEf - hitbox.w;
			else
				hitbox.x = std::floor(hitbox.x / TILE_SIZEf) * TILE_SIZEf;

			if (xCollision)
				*xCollision = true;
		}

		if (entity.getYState() == YState::Grounded && !m_map.touchingTile({ hitbox.x, hitbox.y + 2.f, hitbox.w, hitbox.h }, Tile::Property::Solid))
		{
			//std::cerr << "moving aside made the player fall" << std::endl;
			entity.setYState(YState::Falling);
		}
	}

	entity.setPosition(hitbox.x, hitbox.y);
}

void GameScene::moveEnemy(Enemy& enemy)
{
	bool xCollision = false;
	moveEntity(enemy, &xCollision);
	if (xCollision)
		enemy.toggleFacing();
}

void GameScene::setCameraOnPlayer(bool value)
{
	m_cameraOnPlayer = value;
}

void GameScene::updateCamera()
{
	if (m_cameraOnPlayer)
		placeCameraOnPlayer();
#if 1
	else if (m_readEvents)
	{
		moveCameraIfMouseOnEdge();
	}
#endif
}

void GameScene::placeCameraOnPlayer()
{
	sf::View currentView = m_window->getView();

	auto& playerBox = m_player->getHitbox();
	sf::Vector2f playerCenter(playerBox.x + playerBox.w / 2, playerBox.y + playerBox.h / 2);
	sf::Vector2f vecCenter = playerCenter - currentView.getCenter();

	// if (distance(vecCenter) > m_screenPadding)
	// {
		currentView.move(vecCenter);
		m_background.move(currentView.getCenter() - m_window->getView().getCenter());
		m_window->setView(currentView);

	// }
}

void GameScene::moveCamera(const sf::Vector2f& translation)
{
	auto view = m_window->getView();
	view.move(translation);
	m_window->setView(view);
}

void GameScene::moveCameraIfMouseOnEdge()
{
	// Move camera if mouse pointer is close to the edge of the map window
		// Regardless if map window is active or not
	auto windowSize = m_window->getSize();
	auto mousePositionInWindow = sf::Mouse::getPosition(*m_window);

	// Unactive map window?
	if (!m_window->hasFocus())
		return;

	// Mouse outside window? (even if active)
	if (mousePositionInWindow.x < 0 ||
		mousePositionInWindow.y < 0 ||
		(unsigned)mousePositionInWindow.x > windowSize.x ||
		(unsigned)mousePositionInWindow.y > windowSize.y)
		return;

	sf::Vector2f translation;

	// Right
	if (windowSize.x - mousePositionInWindow.x < m_mouseScreenPadding)
		translation.x = 10.f;

	// Left
	else if ((unsigned)mousePositionInWindow.x < m_mouseScreenPadding)
		translation.x = -10.f;

	// Bottom
	if (windowSize.y - mousePositionInWindow.y < m_mouseScreenPadding)
		translation.y = 10.f;

	// Top
	else if ((unsigned)mousePositionInWindow.y < m_mouseScreenPadding)
		translation.y = -10.f;

	// A camera translation may be required
	if (translation != sf::Vector2f())
		moveCamera(translation);
}

void GameScene::draw(sf::RenderTarget& target)
{
	target.draw(m_layers["backgroundLayer"]);
	target.draw(m_layers["mapLayer"]);
	target.draw(m_layers["mobsLayer"]);
	target.draw(m_layers["playerLayer"]);
	target.draw(m_layers["game_gui"]);

	if (m_mapEditor)
		m_mapEditor->render();

	if (m_imguiEnabled)
	{
		ImGui::Text("Frame took %f FPS=%f", m_lastDt * 1000.f, 1 / m_lastDt);
		const sf::Vector2i mpos = sf::Mouse::getPosition(*m_window);
		std::ostringstream ss;
		ss << "Absolute cursor position: " << mpos.x << " ; " << mpos.y << '\n';
		auto top_left = m_window->mapPixelToCoords({ 0, 0 });
		ss << "Translated cursor position: "
            << top_left.x + static_cast<float>(mpos.x)
            << " ; "
            << top_left.y + static_cast<float>(mpos.y);
		ImGui::Text("%s", ss.str().c_str());
		//ImGui::ShowDemoWindow();
	}
}
