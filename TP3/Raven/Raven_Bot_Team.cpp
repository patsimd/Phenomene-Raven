#include "Raven_Bot_Team.h"
#include "Raven_Bot.h"
#include "misc/Cgdi.h"
#include "misc/utils.h"
#include "2D/Transformations.h"
#include "2D/Geometry.h"
#include "lua/Raven_Scriptor.h"
#include "Raven_Game.h"
#include "navigation/Raven_PathPlanner.h"
#include "Raven_SteeringBehaviors.h"
#include "Raven_UserOptions.h"
#include "time/Regulator.h"
#include "Raven_WeaponSystem.h"
#include "Raven_SensoryMemory.h"

#include "Messaging/Telegram.h"
#include "Raven_Messages.h"
#include "Messaging/MessageDispatcher.h"

#include "goals/Raven_Goal_Types.h"
#include "goals/Goal_Think.h"


#include "Debug/DebugConsole.h"

class Raven_Game;

Raven_Bot_Team::Raven_Bot_Team(Raven_Game* world, Vector2D pos) :Raven_Bot(world, pos)
{

}

Raven_Bot_Team::~Raven_Bot_Team()
{
}

void Raven_Bot_Team::SetMyColor()
{
	gdi->RedPen();
}