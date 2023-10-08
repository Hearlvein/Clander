#include "Utility/Box.hpp"

bool boxContains(const Box& box, const sf::Vector2f& point)
{
	return point.x >= box.x &&
		point.x <= box.x + box.w &&
		point.y >= box.y &&
		point.y <= box.y + box.h;
}

bool boxesOverlapping(const Box& lhs, const Box& rhs)
{
	return 	(lhs.x < rhs.x + rhs.w) &&
		(lhs.x + lhs.w > rhs.x) &&
		(lhs.y < rhs.y + rhs.h) &&
		(lhs.y + lhs.h > rhs.y);
}