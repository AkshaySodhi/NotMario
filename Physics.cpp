#include "Physics.h"

Vec2 Physics::GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b) 
{
	if(!a->hasComponent<CBoundingBox>() || !b->hasComponent<CBoundingBox>()) return Vec2(0.f, 0.f);

	auto& posA = a->getComponent<CTransform>().pos;
	auto& posB = b->getComponent<CTransform>().pos;

	auto& halfSizeA = a->getComponent<CBoundingBox>().halfSize;
	auto& halfSizeB = b->getComponent<CBoundingBox>().halfSize;

	float horiOverlap = halfSizeA.x + halfSizeB.x - std::abs(posA.x - posB.x);
	float vertOverlap = halfSizeA.y + halfSizeB.y - std::abs(posA.y - posB.y);

	if (horiOverlap < 0) horiOverlap = 0;
	if (vertOverlap < 0) vertOverlap = 0;
	return Vec2(horiOverlap, vertOverlap);
}

Vec2 Physics::GetPreviousOverlap(std::shared_ptr<Entity>a, std::shared_ptr<Entity>b) 
{
	if (!a->hasComponent<CBoundingBox>() || !b->hasComponent<CBoundingBox>()) return Vec2(0.f, 0.f);

	auto& prevPosA = a->getComponent<CTransform>().prevPos;
	auto& prevPosB = b->getComponent<CTransform>().prevPos;

	auto& halfSizeA = a->getComponent<CBoundingBox>().halfSize;
	auto& halfSizeB = b->getComponent<CBoundingBox>().halfSize;

	float horiOverlap = halfSizeA.x + halfSizeB.x - std::abs(prevPosA.x - prevPosB.x);
	float vertOverlap = halfSizeA.y + halfSizeB.y - std::abs(prevPosA.y - prevPosB.y);
	
	if (horiOverlap < 0) horiOverlap = 0;
	if (vertOverlap < 0) vertOverlap = 0;
    return Vec2(horiOverlap, vertOverlap);
}