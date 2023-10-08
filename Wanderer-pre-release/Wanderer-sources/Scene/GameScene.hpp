#pragma once

#include "Scene.hpp"
#include "Scene/Map.hpp"
#include "Entity/Player.hpp"
#include "Entity/Enemy.hpp"
#include "Scene/Layer.hpp"
#include "Scene/Background.hpp"
#include "Scene/TilesManager.hpp"

#include <list>
#include <memory>

class MapEditor;

/** The GameScene is composed of a map and entities, including the player **/
class GameScene : public Scene
{
public:
	explicit GameScene(sf::RenderWindow* window);
	~GameScene() override;

	void loadLevel(const std::string& levelFilename);

	// ----- Scene overwritten methods -----
	void handleEvent(const sf::Event& event) override;
	void checkInput() override;
	void update(float dt) override;
	void draw(sf::RenderTarget& target) override;
	
	// ----- Behavior -----
	void setReadEvents(bool read);
	bool getReadEvents() const;

	// ---- Gui management -----
	void updateHealthBox(float dt);

	// ----- Entity management -----
	void loadEntities(const std::string& entitiesFilename);
	void destroyEntities();
	void moveEntity(MovingCharacter& entity, bool* xCollision = nullptr);
	void updateClimbingState(MovingCharacter& entity);
	void moveEnemy(Enemy& enemy);

	// ----- Camera management -----
	void setCameraOnPlayer(bool v = true);
	void updateCamera();
	void placeCameraOnPlayer();
	void moveCamera(const sf::Vector2f& translation);
	void moveCameraIfMouseOnEdge();

private:
	friend class MapEditor;

	// Resources
	sf::Texture m_tileset;
	sf::Texture m_backgroundTexture;

	// Layers
	std::map<std::string, Layer> m_layers;
	Background m_background;
	Map m_map;

	// Entities storage
	std::list<GameObject*> m_entities;	// "ownership" of heap pointers
	Player* m_player = nullptr;
	std::list<Enemy*> m_enemies;

	// Gui
	// PHB = player health box
	sf::RectangleShape m_PHBOutline;
	sf::RectangleShape m_PHB;
	const float m_PHBVelocity = 300.f;
	bool m_PHBUpdateWidth = true;

	// Behavior
	bool m_readEvents = true;
	bool m_imguiEnabled = true;
	float m_lastDt = 0.f;
	bool m_cameraOnPlayer = true;
	const float m_screenPadding = 300.f;
	const unsigned int m_mouseScreenPadding = 50;

	TilesManager m_tilesMgr;
	mutable MapEditor* m_mapEditor = nullptr;
};
