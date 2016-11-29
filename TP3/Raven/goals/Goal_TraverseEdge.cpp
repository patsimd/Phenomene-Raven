#include "Goal_TraverseEdge.h"
#include "..\Raven_Bot.h"
#include "Raven_Goal_Types.h"
#include "..\Raven_SteeringBehaviors.h"
#include "time/CrudeTimer.h"
#include "..\constants.h"
#include "../navigation/Raven_PathPlanner.h"
#include "misc/cgdi.h"
#include "../lua/Raven_Scriptor.h"


#include "debug/DebugConsole.h"



//---------------------------- ctor -------------------------------------------
//-----------------------------------------------------------------------------
Goal_TraverseEdge::Goal_TraverseEdge(Raven_Bot* pBot,
                                     PathEdge   edge,
                                     bool       LastEdge):

                                Goal<Raven_Bot>(pBot, goal_traverse_edge),
                                m_Edge(edge),
                                m_dTimeExpected(0.0),
                                m_bLastEdgeInPath(LastEdge)
                                
{}

                            
                                             
//---------------------------- Activate -------------------------------------
//-----------------------------------------------------------------------------  
void Goal_TraverseEdge::Activate()
{
  m_iStatus = active;
  m_deviant = false;
  maxTime = Clock->GetTickCount() + 0.5;
  //the edge behavior flag may specify a type of movement that necessitates a 
  //change in the bot's max possible speed as it follows this edge
  switch(m_Edge.Behavior())
  {
    case NavGraphEdge::swim:
    {
      m_pOwner->SetMaxSpeed(script->GetDouble("Bot_MaxSwimmingSpeed"));
    }
   
    break;
   
    case NavGraphEdge::crawl:
    {
       m_pOwner->SetMaxSpeed(script->GetDouble("Bot_MaxCrawlingSpeed"));
    }
   
    break;
  }
  

  //record the time the bot starts this goal
  m_dStartTime = Clock->GetCurrentTime();   
  
  //calculate the expected time required to reach the this waypoint. This value
  //is used to determine if the bot becomes stuck 
  m_dTimeExpected = m_pOwner->CalculateTimeToReachPosition(m_Edge.Destination());
  
  //factor in a margin of error for any reactive behavior
  static const double MarginOfError = 15.0;

  m_dTimeExpected += MarginOfError;

  bool static m_bClockwise = true;
  Vector2D m_vStrafeTarget;
  if (!m_deviant){
	
	  if (m_bClockwise)
	  {
		  if (m_pOwner->canStepRight(m_vStrafeTarget))
		  {
			  m_pOwner->GetSteering()->SetTarget(m_vStrafeTarget - m_pOwner->Pos() + m_Edge.Destination());
			  m_deviant = true;
		  }
		  else
		  {
			  m_pOwner->GetSteering()->SetTarget(m_Edge.Destination());
			  //debug_con << "changing" << "";
			  m_bClockwise = !m_bClockwise;
			  m_iStatus = inactive;
		  }
	  }

	  else
	  {
		  if (m_pOwner->canStepLeft(m_vStrafeTarget))
		  {
			  m_pOwner->GetSteering()->SetTarget(m_vStrafeTarget - m_pOwner->Pos() + m_Edge.Destination());
			  m_deviant = true;
		  }
		  else
		  {
			  m_pOwner->GetSteering()->SetTarget(m_Edge.Destination());
			  // debug_con << "changing" << "";
			  m_bClockwise = !m_bClockwise;
			  m_iStatus = inactive;
		  }
	  }
  }
  else
  {
	  m_pOwner->GetSteering()->SetTarget(m_Edge.Destination());
  }


  //set the steering target
 // m_pOwner->GetSteering()->SetTarget(m_Edge.Destination());

  //Set the appropriate steering behavior. If this is the last edge in the path
  //the bot should arrive at the position it points to, else it should seek
  if (m_bLastEdgeInPath)
  {
     m_pOwner->GetSteering()->ArriveOn();
  }

  else
  {
    m_pOwner->GetSteering()->SeekOn();
  }
}



//------------------------------ Process --------------------------------------
//-----------------------------------------------------------------------------
int Goal_TraverseEdge::Process()
{
  //if status is inactive, call Activate()
  ActivateIfInactive();
  if (maxTime < Clock->GetTickCount())
  {
	  m_pOwner->GetSteering()->SetTarget(m_Edge.Destination());
  }
  
  //if the bot has become stuck return failure
  if (isStuck())
  {
  //  m_iStatus = failed;
  }
  
  //if the bot has reached the end of the edge return completed
  else
  { 
    if (m_pOwner->isAtPosition(m_Edge.Destination()))
    {
      m_iStatus = completed;
    }
  }

  return m_iStatus;
}

//--------------------------- isBotStuck --------------------------------------
//
//  returns true if the bot has taken longer than expected to reach the 
//  currently active waypoint
//-----------------------------------------------------------------------------
bool Goal_TraverseEdge::isStuck()const
{  
  double TimeTaken = Clock->GetCurrentTime() - m_dStartTime;

  if (TimeTaken > m_dTimeExpected)
  {
    debug_con << "BOT " << m_pOwner->ID() << " IS STUCK!!" << "";

    return true;
  }

  return false;
}


//---------------------------- Terminate --------------------------------------
//-----------------------------------------------------------------------------
void Goal_TraverseEdge::Terminate()
{
  //turn off steering behaviors.
  m_pOwner->GetSteering()->SeekOff();
  m_pOwner->GetSteering()->ArriveOff();

  //return max speed back to normal
  m_pOwner->SetMaxSpeed(script->GetDouble("Bot_MaxSpeed"));
}

//----------------------------- Render ----------------------------------------
//-----------------------------------------------------------------------------
void Goal_TraverseEdge::Render()
{
  if (m_iStatus == active)
  {
    gdi->BluePen();
    gdi->Line(m_pOwner->Pos(), m_Edge.Destination());
    gdi->GreenBrush();
    gdi->BlackPen();
    gdi->Circle(m_Edge.Destination(), 3);
  }
}

