#include "Animation.h"
#include<cmath>

Animation::Animation() {}

Animation::Animation(const std::string& name, const sf::Texture& t)
	:Animation(name, t, 1, 0) 
{}

Animation::Animation(const std::string& name, const sf::Texture& t, size_t frameCount, size_t speed)
	:m_name(name),
	m_sprite(t),
	m_frameCount(frameCount),
	m_currentFrame(0),
	m_speed(speed)
{
	m_size = Vec2((float)t.getSize().x / frameCount, (float)t.getSize().y);
	m_sprite.setOrigin(m_size.x / 2.f, m_size.y / 2.f);
	m_sprite.setTextureRect(sf::IntRect((int)std::floor(m_currentFrame) * (int)m_size.x, 0, (int)m_size.x, (int)m_size.y));
}

//updates the animation to show next frame, depending on its speed
//animation loops when it reaches the end
void Animation::update()
{
	if (m_frameCount <= 1) return;
	//add the speed variable to curremt frame
	m_currentFrame++;
	//TODO 1-> calculate the correct frame of animation to play based on currentframe and speed
	size_t frame = m_currentFrame / m_speed % m_frameCount;
	//     2-> set the texture rect 
	m_sprite.setTextureRect(sf::IntRect((int) frame * m_size.x, 0, (int)m_size.x, (int) m_size.y));
}

const Vec2& Animation::getSize() const {
	return m_size;
}

const std::string& Animation::getName() const {
	return m_name;
}

sf::Sprite& Animation::getSprite(){
	return m_sprite;
}

bool Animation::hasEnded() const {
	return m_currentFrame >= m_frameCount * m_speed;
}