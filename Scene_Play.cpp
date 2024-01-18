#include "Scene_Play.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "Action.h"

Scene_Play::Scene_Play(GameEngine* gameEngine, const std::string& levelPath)
	:Scene(gameEngine)
	, m_levelPath(levelPath)
{
	init(m_levelPath);
}

void Scene_Play::init(const std::string& levelPath)
{
	registerAction(sf::Keyboard::P, "PAUSE");
	registerAction(sf::Keyboard::Escape, "QUIT");
	registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE");
	registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");
	registerAction(sf::Keyboard::G, "TOGGLE_GRID");

	registerAction(sf::Keyboard::W, "JUMP");
	registerAction(sf::Keyboard::A, "LEFT");
	registerAction(sf::Keyboard::D, "RIGHT");
	registerAction(sf::Keyboard::Space, "SHOOT");

	m_gridText.setCharacterSize(12);
	m_gridText.setFont(m_game->assets().getFont("Arial"));

	m_livesText.setCharacterSize(20);
	m_livesText.setFont(m_game->assets().getFont("Megaman"));

	m_game->music().openFromFile("bin/audio/level.flac");
	m_game->music().play();
	m_game->music().setLoop(true);

	loadLevel(levelPath);
}

Vec2 Scene_Play::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity)
{
	float midX = gridX * m_gridSize.x + entity->getComponent<CAnimation>().animation.getSize().x/2.f;
	float midY = height() - gridY * m_gridSize.y - entity->getComponent<CAnimation>().animation.getSize().y / 2.f;
	return Vec2(midX, midY);
}

void Scene_Play::loadLevel(const std::string& filename)
{
	m_entityManager = EntityManager();

	std::ifstream fin(filename);
	std::string entityType="";
	while (fin >> entityType) 
	{
		if (entityType == "Tile" || entityType == "Dec")
		{
			std::string animationName;
			int gx, gy;

			fin >> animationName >> gx >> gy;

			auto tile = m_entityManager.addEntity(entityType);
			tile->addComponent<CAnimation>(m_game->assets().getAnimation(animationName), true);
			tile->addComponent<CTransform>(gridToMidPixel(gx, gy, tile));
			if (entityType == "Tile") tile->addComponent<CBoundingBox>(m_game->assets().getAnimation(animationName).getSize());
		}
		else if (entityType == "Player")
		{
			fin >> m_playerConfig.X >> m_playerConfig.Y >> m_playerConfig.CX >> m_playerConfig.CY >> m_playerConfig.SPEED >> m_playerConfig.JUMP >> m_playerConfig.MAXSPEED >> m_playerConfig.GRAVITY >> m_playerConfig.WEAPON;
			spawnPlayer();
		}
		else if (entityType == "Enemy")
		{
			std::string animationName;
			int gx, gy;
			float s;

			fin >> animationName >> gx >> gy >> s;

			auto enemy = m_entityManager.addEntity(entityType);

			enemy->addComponent<CAnimation>(m_game->assets().getAnimation("Goomba"), true);
			enemy->addComponent<CTransform>(gridToMidPixel(gx, gy, enemy));
			enemy->getComponent<CTransform>().velocity.x = s;
			enemy->addComponent<CBoundingBox>(m_game->assets().getAnimation(animationName).getSize());
		}
		else
		{
			std::cerr << "Unknown entity name in level file: " << filename << "\n";
		}
	}
}

void Scene_Play::spawnPlayer()
{
	m_player = m_entityManager.addEntity("Player");

	m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Stand"), true);
	m_player->addComponent<CTransform>(gridToMidPixel(m_playerConfig.X,m_playerConfig.Y,m_player));
	m_player->addComponent<CInput>();
	m_player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CX,m_playerConfig.CY));
	m_player->addComponent<CGravity>(m_playerConfig.GRAVITY);
	m_player->addComponent<CState>();
}

void Scene_Play::spawnBullet(std::shared_ptr<Entity> entity)
{
	if (!m_player->getComponent<CInput>().canShoot) return;

	auto bullet = m_entityManager.addEntity("Bullet");

	bullet->addComponent<CAnimation>(m_game->assets().getAnimation("Buster"),true);
	bullet->addComponent<CTransform>(entity->getComponent<CTransform>().pos);

	if(entity->getComponent<CTransform>().scale.x==1) bullet->getComponent<CTransform>().velocity.x = 10;
	else bullet->getComponent<CTransform>().velocity.x = -10;
	
	bullet->addComponent<CBoundingBox>(m_game->assets().getAnimation("Buster").getSize());
	bullet->addComponent<CLifespan>(45, m_currentFrame);
}

void Scene_Play::update()
{
	if (!m_paused) 
	{
		m_entityManager.update();

		sMovement();
		sCollision();
		sLifespan();
		sAnimation();
	}
	sRender();
}

void Scene_Play::sMovement()
{
	auto& playerInput = m_player->getComponent<CInput>();
	Vec2 playerVelocity(0.f, m_player->getComponent<CTransform>().velocity.y);
	m_player->getComponent<CState>().run = false;
	
	if (playerInput.left)
	{
		playerVelocity.x -= m_playerConfig.SPEED;
		m_player->getComponent<CTransform>().scale.x = -1;
		m_player->getComponent<CState>().run = true;
	}
	if (playerInput.right)
	{
		playerVelocity.x += m_playerConfig.SPEED;
		m_player->getComponent<CTransform>().scale.x = 1;
		m_player->getComponent<CState>().run = true;
	}
	if (playerInput.jump && !m_player->getComponent<CState>().air)
	{
		playerVelocity.y = m_playerConfig.JUMP;
	}

	m_player->getComponent<CTransform>().velocity = playerVelocity;

	for (auto e : m_entityManager.getEntities())
	{
		e->getComponent<CTransform>().prevPos = e->getComponent<CTransform>().pos;

		if (e->hasComponent<CGravity>()) 
		{
			e->getComponent<CTransform>().velocity.y += e->getComponent<CGravity>().gravity;

			if (std::abs(playerVelocity.y) > m_playerConfig.MAXSPEED) playerVelocity.y = m_playerConfig.MAXSPEED;
			if (std::abs(playerVelocity.x) > m_playerConfig.MAXSPEED) playerVelocity.x = m_playerConfig.MAXSPEED;
		}

		e->getComponent<CTransform>().pos += e->getComponent<CTransform>().velocity;
	}
}

void Scene_Play::sLifespan()
{
	for (auto e : m_entityManager.getEntities())
	{
		if (e->hasComponent<CLifespan>()) 
		{
			e->getComponent<CLifespan>().lifespan--;
			if (e->getComponent<CLifespan>().lifespan <= 0) e->destroy();
		}
	}
}

void Scene_Play::sCollision()
{
	auto& playerPos = m_player->getComponent<CTransform>().pos;
	auto& playerVelo = m_player->getComponent<CTransform>().velocity;
	auto& playerState = m_player->getComponent<CState>();

	playerState.air = true;
	playerState.stand = false;
	
	//bullet collisons
	for (auto bullet : m_entityManager.getEntities("Bullet"))
	{
		//bullet tile
		for (auto tile : m_entityManager.getEntities("Tile"))
		{
			Vec2 overlap = Physics::GetOverlap(bullet, tile);
			if (overlap.x > 0 && overlap.y > 0) 
			{
				bullet->destroy();

				if (tile->getComponent<CAnimation>().animation.getName() == "Brick") 
				{
					tile->destroy();

					auto boom = m_entityManager.addEntity("Boom");
					boom->addComponent<CAnimation>(m_game->assets().getAnimation("Explosion"), false);
					boom->addComponent<CTransform>(tile->getComponent<CTransform>().pos);
				}

				break;
			}
		}
		//bullet enemy
		for (auto enemy : m_entityManager.getEntities("Enemy"))
		{
			Vec2 overlap = Physics::GetOverlap(bullet, enemy);
			if (overlap.x > 0 && overlap.y > 0)
			{
				bullet->destroy();
				enemy->destroy();

				auto boom = m_entityManager.addEntity("Boom");
				boom->addComponent<CAnimation>(m_game->assets().getAnimation("Explosion"), false);
				boom->addComponent<CTransform>(enemy->getComponent<CTransform>().pos);

				break;
			}
		}
	}

	//enemy tile
	for (auto enemy : m_entityManager.getEntities("Enemy"))
	{
		for (auto tile : m_entityManager.getEntities("Tile"))
		{
			Vec2 overlap = Physics::GetOverlap(enemy, tile);
			if (overlap.x > 0 && overlap.y > 0)
			{
				auto& tilePos = tile->getComponent<CTransform>().pos;
				auto& prevEnemyPos = enemy->getComponent<CTransform>().prevPos;
				auto& currEnemyPos = enemy->getComponent<CTransform>().pos;
				//from left
				if (prevEnemyPos.x < tilePos.x)
				{
					currEnemyPos.x -= overlap.x;
				}
				//comes from right
				else
				{
					currEnemyPos.x += overlap.x;
				}
				enemy->getComponent<CTransform>().velocity.x *= -1;
				break;
			}
		}
	}

	//player tile 
	for (auto tile : m_entityManager.getEntities("Tile"))
	{
		Vec2 overlap = Physics::GetOverlap(m_player, tile);

		//current overlap
		if (overlap.x > 0 && overlap.y > 0)
		{
			auto& prevPlayerPos = m_player->getComponent<CTransform>().prevPos;
			auto& currPlayerPos = m_player->getComponent<CTransform>().pos;
			auto& tilePos 		= tile->getComponent<CTransform>().pos;
			Vec2 prevOverlap = Physics::GetPreviousOverlap(m_player, tile);

			if (tile->getComponent<CAnimation>().animation.getName() == "Pole") {
				onEnd();
			}

			//prev vertical overlap only
			if (prevOverlap.x > 0 && prevOverlap.y == 0)
			{
				//comes from above
				if (prevPlayerPos.y < tilePos.y)
				{
					currPlayerPos.y -= overlap.y;
					playerState.stand = true;
					playerState.air = false;
				}
				//comes from below
				else
				{
					currPlayerPos.y += overlap.y;

					auto& tileAnimation = tile->getComponent<CAnimation>().animation;

					if (tileAnimation.getName() == "Brick") 
					{
						tile->destroy();

						auto boom = m_entityManager.addEntity("Boom");
						boom->addComponent<CAnimation>(m_game->assets().getAnimation("Explosion"), false);
						boom->addComponent<CTransform>(tilePos);
					}
					else if (tileAnimation.getName() == "Question")
					{
						tile->addComponent<CAnimation>(m_game->assets().getAnimation("Question2"),true);

						auto coin = m_entityManager.addEntity("Coin");
						coin->addComponent<CAnimation>(m_game->assets().getAnimation("Coin"),false);
						coin->addComponent<CTransform>(Vec2(tilePos.x, tilePos.y - m_gridSize.y));
					}
				}
				m_player->getComponent<CTransform>().velocity.y = 0;
			}
			//prev hori overlap only
			else if (prevOverlap.x==0 && prevOverlap.y > 0)
			{
				//comes from left
				if (prevPlayerPos.x < tilePos.x)
				{
					currPlayerPos.x -= overlap.x;
				}
				//comes from right
				else
				{
					currPlayerPos.x += overlap.x;
				}
				playerVelo.x = 0;
			}
			//no prev overlap ie player came diagonally
			else
			{
				//first do x movement and resolve
				float temp = currPlayerPos.y; //save new y position
				currPlayerPos.y = prevPlayerPos.y;

				for (auto tile : m_entityManager.getEntities("Tile"))
				{
					Vec2 overlap = Physics::GetOverlap(m_player, tile);
					if (overlap.x > 0 && overlap.y > 0) 
					{
						currPlayerPos.x = prevPlayerPos.x;
						break;
					}
				}

				//do y movement 
				currPlayerPos.y = temp;

				for (auto tile : m_entityManager.getEntities("Tile"))
				{
					Vec2 overlap = Physics::GetOverlap(m_player, tile);

					if (overlap.x > 0 && overlap.y > 0) 
					{
						currPlayerPos.y = prevPlayerPos.y;
						if (prevPlayerPos.y < tile->getComponent<CTransform>().pos.y)
						{
							playerState.stand = true;
							playerState.air = false;
						}
						break;
					}
				}
			}
		}
	}

	//player enemy 
	for (auto enemy : m_entityManager.getEntities("Enemy"))
	{
		Vec2 overlap = Physics::GetOverlap(m_player, enemy);
		//current overlap
		if (overlap.x > 0 && overlap.y > 0)
		{
			if (playerVelo.y > 0)
			{
				enemy->destroy();

				auto boom = m_entityManager.addEntity("Boom");
				boom->addComponent<CAnimation>(m_game->assets().getAnimation("Explosion"), false);
				boom->addComponent<CTransform>(enemy->getComponent<CTransform>().pos);

				playerVelo.y = -m_playerConfig.MAXSPEED/1.5f;
			}
			else
			{
				m_lives--;
				if (!m_lives) onEnd();
				m_player->destroy();
				spawnPlayer();
			}	
			break;
		}
	}
	
	//check if player fell down a hole (y>height())
	if (playerPos.y > height())
	{
		m_lives--;
		if (!m_lives) onEnd();
		m_player->destroy();
		spawnPlayer();
	}

	//dont let player walk off the left of map
	if (playerPos.x < m_player->getComponent<CBoundingBox>().halfSize.x)
	{
		playerPos.x = m_player->getComponent<CBoundingBox>().halfSize.x;
	}

}

void Scene_Play::sDoAction(const Action& action)
{
	if (action.type() == "START")
	{
		if		(action.name() == "TOGGLE_TEXTURE")		{ m_drawTextures = !m_drawTextures; }
		else if (action.name() == "TOGGLE_COLLISION")	{ m_drawCollision = !m_drawCollision; }
		else if (action.name() == "TOGGLE_GRID")		{ m_drawGrid = !m_drawGrid; }
		else if (action.name() == "PAUSE")				{ setPaused(!m_paused); }
		else if (action.name() == "QUIT")				{ onEnd(); }
		else if (action.name() == "JUMP")				{ m_player->getComponent<CInput>().jump = true; }
		else if (action.name() == "LEFT")				{ m_player->getComponent<CInput>().left = true; }
		else if (action.name() == "RIGHT")				{ m_player->getComponent<CInput>().right = true; }
		else if (action.name() == "SHOOT")				{ spawnBullet(m_player); m_player->getComponent<CInput>().shoot = true; m_player->getComponent<CInput>().canShoot = false;  }

	}
	else if (action.type() == "END")
	{
		if		(action.name() == "JUMP")				{ m_player->getComponent<CInput>().jump = false; }
		else if (action.name() == "LEFT")				{ m_player->getComponent<CInput>().left = false; }
		else if (action.name() == "RIGHT")				{ m_player->getComponent<CInput>().right = false; }
		else if (action.name() == "SHOOT")				{ m_player->getComponent<CInput>().shoot = false; m_player->getComponent<CInput>().canShoot = true; }
	}
}

void Scene_Play::sAnimation()
{
	auto& playerState = m_player->getComponent<CState>();

	if (playerState.air) m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Air"), true);
	else if (playerState.run) 
	{
		if (m_player->getComponent<CAnimation>().animation.getName()!="Run") m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Run"), true);
	}
	else if (playerState.stand) m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Stand"), true);
	
	for (auto e : m_entityManager.getEntities())
	{
		if (e->hasComponent<CAnimation>())
		{
			e->getComponent<CAnimation>().animation.update();
			if (e->getComponent<CAnimation>().animation.hasEnded())
			{
				if(!e->getComponent<CAnimation>().repeat) e->destroy();
			}
		}
	}
}

void Scene_Play::onEnd()
{
	m_game->changeScene("MENU", std::make_shared<Scene_Menu>(m_game),true);
}

void Scene_Play::sRender()
{	
	if (!m_paused) { m_game->window().clear(sf::Color(100, 100, 255)); }
	else { m_game->window().clear(sf::Color(50, 50, 150)); }

	//set viewport of window to be centered on the player if its far enough right
	auto& pPos = m_player->getComponent<CTransform>().pos;
	float windowCenterX = std::max(m_game->window().getSize().x / 2.f, pPos.x);
	sf::View view = m_game->window().getView();
	view.setCenter(windowCenterX, m_game->window().getSize().y - view.getCenter().y);
	m_game->window().setView(view);

	if (m_drawTextures)
	{
		for (auto e : m_entityManager.getEntities())
		{
			auto& transform = e->getComponent<CTransform>();

			if (e->hasComponent<CAnimation>())
			{
				auto& animation = e->getComponent<CAnimation>().animation;
				animation.getSprite().setRotation(transform.angle);
				animation.getSprite().setPosition(transform.pos.x, transform.pos.y);
				animation.getSprite().setScale(transform.scale.x, transform.scale.y);
				m_game->window().draw(animation.getSprite());
			}
		}
	}

	m_livesText.setString("Lives remaining: " + std::to_string(m_lives));
	m_livesText.setPosition(windowCenterX - m_game->window().getSize().x / 2.f +10, 80);
	m_game->window().draw(m_livesText);

	/*if (m_drawCollision)
	{
		for (auto e : m_entityManager.getEntities())
		{
			if (e->hasComponent<CBoundingBox>())
			{
				auto& box = e->getComponent<CBoundingBox>();
				auto& transform = e->getComponent<CTransform>();
				sf::RectangleShape rect;
				rect.setSize(sf::Vector2f(box.size.x - 1, box.size.y));
				rect.setOrigin(sf::Vector2f(box.halfSize.x, box.halfSize.y));
				rect.setPosition(transform.pos.x, transform.pos.y);
				rect.setFillColor(sf::Color(0, 0, 0, 0));
				rect.setOutlineColor(sf::Color(255, 255, 255, 255));
				rect.setOutlineThickness(1);
				m_game->window().draw(rect);
			}
		}
	}*/

	/*if (m_drawGrid)
	{
		float leftX = m_game->window().getView().getCenter().x - width() / 2.f;
		float rightX = leftX + width() + m_gridSize.x;
		float nextGridX = leftX - ((int)leftX % (int)m_gridSize.x);

		for (float x = nextGridX; x < rightX; x += m_gridSize.x) {
			drawLine(Vec2(x, 0), Vec2(x, height()));
		}

		for (float y = 0; y < height(); y += m_gridSize.y) {
			drawLine(Vec2(leftX, height() - y), Vec2(rightX, height() - y));

			for (float x = nextGridX; x < rightX; x += m_gridSize.x)
			{
				std::string xCell = std::to_string((int)x / (int)m_gridSize.x);
				std::string yCell = std::to_string((int)y / (int)m_gridSize.y);
				m_gridText.setString("(" + xCell + "," + yCell + ")");
				m_gridText.setPosition(x + 3, height() - y - m_gridSize.y + 2);
				m_game->window().draw(m_gridText);
			}
		}
	}*/

	m_game->window().display();
}