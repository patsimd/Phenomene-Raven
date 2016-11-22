#include "Weapon_RocketLauncher.h"
#include "../Raven_Bot.h"
#include "misc/Cgdi.h"
#include "../Raven_Game.h"
#include "../Raven_Map.h"
#include "../lua/Raven_Scriptor.h"
#include "fuzzy/FuzzyOperators.h"


//--------------------------- ctor --------------------------------------------
//-----------------------------------------------------------------------------
RocketLauncher::RocketLauncher(Raven_Bot*   owner):

                      Raven_Weapon(type_rocket_launcher,
                                   script->GetInt("RocketLauncher_DefaultRounds"),
                                   script->GetInt("RocketLauncher_MaxRoundsCarried"),
                                   script->GetDouble("RocketLauncher_FiringFreq"),
                                   script->GetDouble("RocketLauncher_IdealRange"),
                                   script->GetDouble("Rocket_MaxSpeed"),
                                   owner)
{
    //setup the vertex buffer
  const int NumWeaponVerts = 8;
  const Vector2D weapon[NumWeaponVerts] = {Vector2D(0, -3),
                                           Vector2D(6, -3),
                                           Vector2D(6, -1),
                                           Vector2D(15, -1),
                                           Vector2D(15, 1),
                                           Vector2D(6, 1),
                                           Vector2D(6, 3),
                                           Vector2D(0, 3)
                                           };
  for (int vtx=0; vtx<NumWeaponVerts; ++vtx)
  {
    m_vecWeaponVB.push_back(weapon[vtx]);
  }

  //setup the fuzzy module
  InitializeFuzzyModule();

}


//------------------------------ ShootAt --------------------------------------
//-----------------------------------------------------------------------------
inline void RocketLauncher::ShootAt(Vector2D pos)
{ 
  if (NumRoundsRemaining() > 0 && isReadyForNextShot())
  {
    //fire off a rocket!
    m_pOwner->GetWorld()->AddRocket(m_pOwner, pos);

    m_iNumRoundsLeft--;

    UpdateTimeWeaponIsNextAvailable();

    //add a trigger to the game so that the other bots can hear this shot
    //(provided they are within range)
    m_pOwner->GetWorld()->GetMap()->AddSoundTrigger(m_pOwner, script->GetDouble("RocketLauncher_SoundRange"));
  }
}

//---------------------------- Desirability -----------------------------------
//
//-----------------------------------------------------------------------------
double RocketLauncher::GetDesirability(double DistToTarget)
{
  if (m_iNumRoundsLeft == 0)
  {
    m_dLastDesirabilityScore = 0;
  }
  else
  {
    //fuzzify distance and amount of ammo
    m_FuzzyModule.Fuzzify("DistToTarget", DistToTarget);
    m_FuzzyModule.Fuzzify("AmmoStatus", (double)m_iNumRoundsLeft);

    m_dLastDesirabilityScore = m_FuzzyModule.DeFuzzify("Desirability", FuzzyModule::max_av);
  }

  return m_dLastDesirabilityScore;
}

//-------------------------  InitializeFuzzyModule ----------------------------
//
//  set up some fuzzy variables and rules
//-----------------------------------------------------------------------------
void RocketLauncher::InitializeFuzzyModule()
{
  FuzzyVariable& DistToTarget = m_FuzzyModule.CreateFLV("DistToTarget");

  FzSet& Target_Melee = DistToTarget.AddLeftShoulderSet("Target_Melee", 0, 25, 50);
  FzSet& Target_Close = DistToTarget.AddTriangularSet("Target_Close", 40, 85, 100);
  FzSet& Target_Medium = DistToTarget.AddTriangularSet("Target_Medium", 90, 125, 150);
  FzSet& Target_Far = DistToTarget.AddTriangularSet("Target_Far", 135, 175, 200);
  FzSet& Target_Too_Far = DistToTarget.AddRightShoulderSet("Target_Too_Far", 190, 325, 1000);

  FuzzyVariable& Desirability = m_FuzzyModule.CreateFLV("Desirability");
  FzSet& VeryDesirable = Desirability.AddRightShoulderSet("VeryDesirable", 70, 85, 100);
  FzSet& Desirable = Desirability.AddTriangularSet("Desirable", 55, 70, 85);
  FzSet& PrettyDesirable = Desirability.AddTriangularSet("PrettyDesirable", 30, 45, 60);
  FzSet& Undesirable = Desirability.AddTriangularSet("Undesirable", 15, 30, 45);
  FzSet& NoWay = Desirability.AddLeftShoulderSet("NoWay", 0, 15, 30);

  FuzzyVariable& AmmoStatus = m_FuzzyModule.CreateFLV("AmmoStatus");
  FzSet& Ammo_Loads = AmmoStatus.AddRightShoulderSet("Ammo_Loads", 50, 75, 100);
  FzSet& Ammo_High = AmmoStatus.AddTriangularSet("Ammo_High", 30, 40, 60);
  FzSet& Ammo_Okay = AmmoStatus.AddTriangularSet("Ammo_Okay", 20, 30, 40);
  FzSet& Ammo_Lowish = AmmoStatus.AddTriangularSet("Ammo_Lowish", 2, 10, 20);
  FzSet& Ammo_Low = AmmoStatus.AddTriangularSet("Ammo_Low", 0, 0, 10);


  m_FuzzyModule.AddRule(FzAND(Target_Melee, Ammo_Loads), NoWay);
  m_FuzzyModule.AddRule(FzAND(Target_Melee, Ammo_High), NoWay);
  m_FuzzyModule.AddRule(FzAND(Target_Melee, Ammo_Okay), NoWay);
  m_FuzzyModule.AddRule(FzAND(Target_Melee, Ammo_Lowish), NoWay);
  m_FuzzyModule.AddRule(FzAND(Target_Melee, Ammo_Low), NoWay);

  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Loads), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_High), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Okay), NoWay);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Lowish), NoWay);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Low), NoWay);

  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Loads), VeryDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_High), VeryDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Okay), VeryDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Lowish), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Low), PrettyDesirable);

  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Loads), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_High), PrettyDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Okay), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Lowish), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Low), NoWay);

  m_FuzzyModule.AddRule(FzAND(Target_Too_Far, Ammo_Loads), PrettyDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Too_Far, Ammo_High), PrettyDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Too_Far, Ammo_Okay), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Too_Far, Ammo_Lowish), NoWay);
  m_FuzzyModule.AddRule(FzAND(Target_Too_Far, Ammo_Low), NoWay);
}


//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
void RocketLauncher::Render()
{
    m_vecWeaponVBTrans = WorldTransform(m_vecWeaponVB,
                                   m_pOwner->Pos(),
                                   m_pOwner->Facing(),
                                   m_pOwner->Facing().Perp(),
                                   m_pOwner->Scale());

  gdi->RedPen();

  gdi->ClosedShape(m_vecWeaponVBTrans);
}