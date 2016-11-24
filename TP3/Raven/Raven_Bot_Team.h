#pragma once
#include "Raven_Bot.h"
class Raven_Bot_Team : public Raven_Bot
{
public:
	Raven_Bot_Team(Raven_Game* world, Vector2D pos);
	~Raven_Bot_Team();

	virtual void SetMyColor();
};

