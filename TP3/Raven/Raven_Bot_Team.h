#pragma once
#include "Raven_Bot.h"
class Raven_Bot_Team : public Raven_Bot
{
private:
	Raven_Bot_Team* leader;
public:
	Raven_Bot_Team(Raven_Game* world, Vector2D pos, Raven_Bot_Team* leader = nullptr);
	~Raven_Bot_Team();

	virtual void SetMyColor();

	virtual Raven_Bot* GetLeader(){ return leader; };
	virtual bool getTeam() const { return true; }
	virtual void SetDead();
};

