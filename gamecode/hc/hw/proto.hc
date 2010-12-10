
//**************************************************************************
//**
//** proto.hc
//**
//** $Header: /cvsroot/uhexen2/gamecode/hc/hw/proto.hc,v 1.1.1.1 2004-11-29 11:28:51 sezero Exp $
//**
//**************************************************************************

// combat.hc

void T_Damage(entity targ, entity inflictor, entity attacker, float damage);
//float T_Heal(entity e, float healamount, float ignore);
float CanDamage(entity targ, entity inflictor);

// subs.hc

void SUB_CalcMove(vector tdest, float tspeed, void() func);
void SUB_CalcAngleMove(vector destangle, float tspeed, void() func);
void SUB_CalcMoveDone(void);
void SUB_CalcAngleMoveDone(void);
void SUB_Null(void);
void SUB_UseTargets(void);
void SUB_Remove(void);

void AwardExperience(entity ToEnt, entity FromEnt, float Amount);

void() SetModelAndThinks;
void stats_NewPlayer(entity e);

void()shock_think;
void(vector org)spawnshockball;
void(entity light_targ)lightstyle_change;
void(entity targ, entity attacker,entity inflictor, float damage, vector dir) Killed;

//void()player_paladin_gauntlet_attack;
//void()player_paladin_swaxe_attack;
//void()player_paladin_staff_attack;
//void()player_paladin_crouch_move;

//void()player_assassin_hands_attack;
//void()player_assassin_xbow_attack;
//void()player_assassin_staff_attack;
//void()player_assassin_crouch_move;

// quake.hc

void(float richter) MonsterQuake;
void() MultiExplode;
void(entity targ, entity attacker, entity inflictor) ClientObituary;
void(entity loser,float number)drop_level;
void()DarkExplosion;
float GetImpactType (entity impacted);

float()FindMonsterTarget;
void()MonsterCheckContents;
float visibility_good (entity targ,float chance_mod);
float()CheckJump;
void()W_SetCurrentWeapon;
void(float explodetype)BecomeExplosion; 
void()player_frames;
void Create_Missile (entity spawner,vector spot1,vector spot2,string missilemodel, 
	string missilename,float missileskin,float missilespeed,void() touchfunct);
void() corpseblink;
void(vector org, vector vel, float damage,entity victim)SpawnPuff;
//void spawn_ghost (entity attacker);
void precache_archer();
void precache_spider();
vector aim_adjust (entity targ);
void()respawn;
void(vector dir)GibPlayer;
void stats_NewClass(entity e);
void(float damage,entity victim) spawn_touchpuff;
void W_SetCurrentAmmo (void);
void ToggleChaseCam (entity voyeur);
void PlayerTouch (void);
void()GrenadeTouch2;
void SmallExplosion();
void(vector p1, vector p2, entity from, float damage,string type) LightningDamage;
void player_pain (entity attacker,float total_damage);
void remove_invincibility(entity loser);
entity SelectSpawnPoint(void);
void teleport_coin_run (void);


