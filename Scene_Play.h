#pragma once

#include"Scene.h"
#include<map>
#include<memory>

#include "EntityManager.h"

class Scene_Play : public Scene
{
	struct PlayerConfig
	{
		float X, Y, CX, CY, SPEED, MAXSPEED, JUMP, GRAVITY;
		std::string WEAPON;
	};

protected:
	std::shared_ptr<Entity> m_player;
	std::string m_levelPath;
	PlayerConfig m_playerConfig;
	bool m_drawTextures = true;
	bool m_drawCollision = false;
	bool m_drawGrid = false;
	int m_lives = 3;
	const Vec2 m_gridSize = { 64,64 };
	sf::Text m_gridText;
	sf::Text m_livesText;

	void init(const std::string& levelPath);

	void loadLevel(const std::string& filename);
	Vec2 gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity);

	void spawnPlayer();
	void spawnBullet(std::shared_ptr<Entity> entity);

	void update();
	void sDoAction(const Action& action);
	void sMovement();
	void sCollision();
	void sLifespan();
	void sAnimation();
	void sRender();

	void onEnd();

public:
	Scene_Play(GameEngine* gameEngine, const std::string& levelPath);
};