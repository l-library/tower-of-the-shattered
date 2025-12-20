#include "Skills.h"
#include "Player.h"

void Skills::update(float dt)
{
	if (_currentCooldown > 0)
		_currentCooldown -= dt;
}