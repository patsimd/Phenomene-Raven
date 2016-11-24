#ifndef GOAL_FOLLOWDODGE_H
#define GOAL_FOLLOWDODGE_H
#pragma warning (disable:4786)

#include "Goals/Goal_Composite.h"
#include "Raven_Goal_Types.h"
#include "../Raven_Bot.h"
#include "../navigation/Raven_PathPlanner.h"
#include "../navigation/PathEdge.h"



class Goal_FollowDodge : public Goal_Composite<Raven_Bot>
{
private:

	//a local copy of the path returned by the path planner
	std::list<PathEdge>  m_Path;

	Vector2D    m_vStrafeTarget;

	bool        m_bClockwise;

	Vector2D  GetStrafeTarget()const;

public:

	Goal_FollowDodge(Raven_Bot* pBot, std::list<PathEdge> path);

	//the usual suspects
	void Activate();
	int Process();
	void Render();
	void Terminate(){}
};

#endif

