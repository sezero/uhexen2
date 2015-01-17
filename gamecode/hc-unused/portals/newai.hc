/*
 * $Header: /H3MP/hcode/newai.hc 16    10/28/97 1:01p Mgummelt $
 */

void AI_Decision_Dead()
{
	if (self.health>-80)
	{	// Newly dead
		if ((self.ai_state != AI_DEAD) && (self.ai_state != AI_DEAD_TWITCH))				
		{
			self.ai_new_state = AI_DEAD;
		}
		else  // He's dead Jim
		{
			if (self.ai_poss_states & AI_DEAD_TWITCH )
			{
				self.ai_new_state = AI_DEAD_TWITCH;
				self.ai_state = AI_DECIDE;  // So states will change
			}
			else
				self.ai_new_state = AI_DEAD;
		}

		self.ai_duration = time + 15;	// Wait 15 more seconds before vanishing
	}
	else
		self.ai_new_state = AI_DEAD_GIB;

}



/*
=============
AI_face_enemy

Turn in place until within an angle to launch an attack
=============
*/
float AI_face_enemy ()
{

	self.ideal_yaw = vectoyaw(self.enemy.origin - self.origin);

	ChangeYaw ();

	if (FacingIdeal())  // Ready to go get em
		return TRUE;

	return FALSE;
}

/*
=============
AI_infront

returns 1 if the entity is in front (in sight) of self
=============
*/
float AI_infront (entity targ)
{
	local vector	vec;
	local float		dot;
	
	makevectors (self.angles);
	vec = normalize (targ.origin - self.origin);
	dot = vec * v_forward;
	
	if ( dot > 0.3)
		return TRUE;

	return FALSE;
}

/*
=============
AI_visionblocked

returns 1 if the entity is visible to self
=============
*/
float AI_visionblocked (entity targ)
{
	local vector	spot1, spot2;
	
	spot1 = self.origin + self.view_ofs;
	spot2 = targ.origin + targ.view_ofs;
	traceline (spot1, spot2, TRUE, self);	// see through other monsters
	
	if (trace_inopen && trace_inwater)
		return FALSE;			// sight line crossed contents

	if (trace_fraction == 1)
		return TRUE;

	return FALSE;
}

/*
=============
AI_calc_range

returns the range catagorization of an entity reletive to self
0	melee range, will become hostile even if back is turned
1	visibility and infront, or visibility and show hostile
2	infront and show hostile
3	only triggered by damage
=============
*/
float AI_calc_range (entity targ)
{
	local vector	spot1, spot2;
	local float		rnge;	

	spot1 = self.origin + self.view_ofs;
	spot2 = targ.origin + targ.view_ofs;
	
	rnge = vlen (spot1 - spot2);

	if (rnge < 50)
		return RANGE_MELEE;

	if (rnge < 400)	// Was 250
		return RANGE_NEAR;

	if (rnge < 1000)	// Was 800
		return RANGE_MID;

	return RANGE_FAR;
}

/*
===========
AI_TargetSearch

Self is currently not attacking anything, so try to find a target

Returns TRUE if an enemy was sighted

When a player fires a missile, the point of impact becomes a fakeplayer so
that monsters that see the impact will respond as if they had seen the
player.

To avoid spending too much time, only a single client (or fakeclient) is
checked each frame.  This means multi player games will have slightly
slower noticing monsters.
============
*/
entity AI_TargetSearch ()
{
	local entity	client;
	local float		r;


// FIXME: need to get sight_entity stuff working 

// spawnflags & 3 is a big hack, because zombie crucified used the first
// spawn flag prior to the ambush flag, and I forgot about it, so the second
// spawn flag works as well
/*	if (sight_entity_time >= time - 0.1 && !(self.spawnflags & 3) )
	{
		client = sight_entity;
		if (client.enemy == self.enemy)
		{
			return;
		}
	}
	else
	{*/
		client = checkclient ();		
		if (!client)
		{
			return FALSE;	// current check entity isn't in PVS
		}
//	}

	if (client.classname != "player")
		return FALSE;

	if (client.flags & FL_NOTARGET)
		return FALSE;

	if (client.items & IT_INVISIBILITY)
		return FALSE;

	return client;
}

/*
===========
AI_Target

Some preliminary target decisions
- if just hurt go after the attacker
- if enemy is the world look to see if you can find someone else
============
*/entity AI_Target ()
{
	entity targ;

	// Monster was just hurt - go after the attacker
	if (self.just_hurt)
		targ = self.enemy;   // Set in T-Damage

	else if (self.enemy==world)	// No enemies currently
		targ=AI_TargetSearch ();   // Look for a player to kill

	else					// Go after the current enemy
		targ = self.enemy;			

	return targ;
}

void AI_MeleeAttack ()
{
	AI_face_enemy();   // Turn to face the enemy		
}


void AI_MissileAttack ()
{		
	AI_face_enemy();   // Turn to face the enemy
}


void AI_Stand ()
{
	
}

void AI_Pain ()
{
	AI_face_enemy();   // Turn to face the enemy	
}

void AI_Walk ()
{
	movetogoal (self.move_distance);
}

void AI_Wander ()
{
	movetogoal (self.move_distance);
}


/*
===========
AI_Charge

Charge the enemy - unless monster is too close to enemy, then attack

============
*/
void AI_Charge ()
{
	float rnge;

	rnge = AI_calc_range (self.enemy);

	if (rnge==RANGE_MELEE)	// Too close - attack enemy
		self.ai_new_state=AI_DECIDE;
	else
		movetogoal (self.move_distance);
}

/*
===========
AI_Dead_Gib

Monster is majorly dead so make him explode

============
*/
void AI_Dead_Gib ()
{
	chunk_death();

	remove(self);
}


/*
===========
AI_Dead

Monster is dead - in a very real and legally binding sense

============
*/
// FIXME:  We need to come up with a nice way of getting rid of dead bodies
void AI_Dead ()
{
	if (self.ai_duration <= time)
		remove(self);
}


/*
===========
AI_TurnLook

Turn and look for an enemy that is behind monster

============
*/
void AI_TurnLook ()
{
	self.angles_y += 4;
}


/*
===========
AI_Change_State

Set up the variables for the new AI state
This has the final say in the enemy

============
*/
void AI_Change_State(entity targ)
{
	self.ai_state_func();	// Change monster variables to match the new state

	// Set up the info for the new state
	if (self.ai_new_state == AI_STAND)
	{
		self.ai_move_func = AI_Stand;
		self.goalentity = self.pathentity;		
		self.enemy = world;
	}	
	else if (self.ai_new_state == AI_WALK)
	{
		self.ai_move_func = AI_Walk;
		self.goalentity = self.pathentity;					
		self.enemy = world;
	}
	else if (self.ai_new_state==AI_CHARGE)
	{
		self.ai_move_func = AI_Charge;
		self.goalentity = targ;					
		self.enemy = targ;
	}
	else if (self.ai_new_state==AI_MISSILE_ATTACK)
	{
		self.ai_move_func = AI_MissileAttack;
		self.goalentity = targ;					
		self.enemy = targ;
		self.ai_duration = time + 9999;		// This action must run it course before changing
	}
	else if (self.ai_new_state==AI_MISSILE_REATTACK)
	{
		self.ai_move_func = AI_MissileAttack;
		self.goalentity = targ;					
		self.enemy = targ;
		self.ai_duration = time + 9999;		// This action must run it course before changing
	}
	else if (self.ai_new_state==AI_MELEE_ATTACK)
	{
		self.ai_move_func = AI_MeleeAttack;
		self.goalentity = targ;					
		self.enemy = targ;
		self.ai_duration = time + 9999;		// This action must run it course before changing
	}
	else if ((self.ai_new_state==AI_PAIN) || (self.ai_new_state==AI_PAIN_CLOSE) || (self.ai_new_state==AI_PAIN_FAR))
	{
		self.ai_move_func = AI_Pain;
		self.goalentity = self.enemy;				
		self.ai_duration = time + 9999;		// This action must run it course before changing
	}
	else if (self.ai_new_state==AI_DEAD)
	{
		self.ai_move_func = AI_Dead;
//		self.goalentity = world;					
//		self.enemy = world;
		self.ai_duration = time + 15;		// Body will lay there for a while before disappearing
	}
	else if (self.ai_new_state==AI_DEAD_TWITCH)
	{
		self.ai_move_func = AI_Dead;
		self.goalentity = world;					
		self.enemy = world;
	}
	else if (self.ai_new_state==AI_DEAD_GIB)
	{
		self.ai_move_func = AI_Dead_Gib;
		self.goalentity = world;					
		self.enemy = world;
	}
	else if (self.ai_new_state==AI_TURNLOOK)
	{
		self.ai_move_func = AI_TurnLook;
		self.goalentity = world;					
		self.enemy = world;
	}
	else if (self.ai_new_state==AI_WANDER)
	{
		self.ai_move_func = AI_Wander;
		self.goalentity = world;					
		self.enemy = world;
	}
	else
	{
		self.ai_move_func = AI_Walk;
		self.goalentity = self.pathentity;					
	}

	self.ai_state = self.ai_new_state;
}

/*
===========
AI_Decision_Melee

At MELEE distance from the enemy - decide what state to take

============
*/
entity AI_Decision_Melee(float in_sight,float in_front,entity targ)
{
	float chance;
	entity newtarg;

	if (self.just_hurt)
	{	// What types of pain can it have???
		
		if (self.ai_poss_states & AI_PAIN_CLOSE )
			self.ai_new_state = AI_PAIN_CLOSE;
		else
			self.ai_new_state = AI_PAIN;

		self.just_hurt = FALSE;
		newtarg = targ;
	}
	else
	{
		chance = random();	//  90% chance of a melee attack
							//  20% chance of wandering
							// if health is below 10 there is an 
							//	90% chance he will throw a missile (he's almost dead so take him out with a bang)
							//  20% chance of wandering

		if (chance < 0.90)
		{
			if (self.health > 10)
				self.ai_new_state = AI_MELEE_ATTACK;
			else
				self.ai_new_state = AI_MISSILE_ATTACK;

			self.ai_state = AI_DECIDE;  // make it so states change
		}
		else
		{
			self.ai_new_state = AI_WANDER;
			self.angles_y += random() * 40;
			self.ai_duration = time + (random() * 2) + 2;
		}
		newtarg = targ;
	}	

	return newtarg;
}
/*
===========
AI_Decision_Near

At NEAR distance from the enemy - decide what state to take

============
*/
entity AI_Decision_Near(float in_sight,float in_front,entity targ)
{
	float chance;
	entity newtarg;

	if (self.just_hurt)
	{
		if (self.ai_poss_states & AI_PAIN_CLOSE )
			self.ai_new_state = AI_PAIN_CLOSE;
		else
			self.ai_new_state = AI_PAIN;
		self.just_hurt = FALSE;
		newtarg = targ;
	}
	else
	{
		chance = random();	//  100% chance of a charging

		if (chance <= 1)   // Yes I know, I will add other states soon
		{
			self.ai_new_state = AI_CHARGE;
			self.ai_state = AI_DECIDE;  // make it so states change
		}
		else
		{
			self.ai_new_state = AI_WANDER;
			self.angles_y += random() * 40;
			self.ai_duration = time + (random() * 2) + 2;
		}
		newtarg = targ;
	}	

	return newtarg;
}

/*
===========
AI_Decision_Mid

At MID distance from the enemy - decide what state to take

============
*/
entity AI_Decision_Mid(float in_sight,float in_front,entity targ)
{
	float chance;
	entity newtarg;

	// He was just hurt make him go through pain
	if (self.just_hurt)
	{
		if (self.ai_poss_states & AI_PAIN_CLOSE )
			self.ai_new_state = AI_PAIN_CLOSE;
		else
			self.ai_new_state = AI_PAIN;
		self.just_hurt = FALSE;
		newtarg = targ;
	}

	// No walls are blocking enemy and enemy's right in front of monster
	else if ((in_sight) && (in_front))
	{	// Can monster throw missiles
		if (self.ai_poss_states & AI_MISSILE_ATTACK )
		{
			chance = random();	// 70% chance of throwing a missile
								// 30% chance of charging
			if (chance <= 0.70)
			{
				//  Not missile attacking yet
				if ((self.ai_state != AI_MISSILE_ATTACK) && (self.ai_state != AI_MISSILE_REATTACK))
					self.ai_new_state = AI_MISSILE_ATTACK;
				else 
				{
					self.ai_new_state = AI_MISSILE_REATTACK;
					self.ai_state = AI_DECIDE;  // make it so reattacks can repeat
				}
			}
			else
			{
				self.ai_new_state = AI_CHARGE;		
				self.ai_duration = time + (random() * 2) + 1;
			}
		}
		else	// No missile attack so charge enemy
		{
			self.ai_new_state = AI_CHARGE;		
			self.ai_duration = time + (random() * 2) + 1;
		}
		newtarg = targ;
	}

	// Enemy is close but he is not right in front of monster
	// so stand and turn for a minute
	else if (in_sight)
	{
		chance = random();  // 80% chance he'll keep walking
							// 20% chance he'll turn looking for the player

		if ((chance <= 0.80) || (self.ai_state != AI_WALK))   // Keep walking
		{
			self.ai_new_state = AI_WALK;
			self.ai_duration = time + random() * 4;
		}
		else		// Stop and turn
		{
			self.ai_new_state = AI_TURNLOOK;
			self.ai_duration = time + random() + 1;
		}
		newtarg = world;
	}
	// A wall is in the way so continue do what you're doing
	else
	{
		if (self.ai_state == AI_DECIDE)
			self.ai_new_state = AI_STAND;
		else
		{
			self.ai_new_state = self.ai_state;
		}
		newtarg = world;		// Cause we haven't seen him yet
	}	

	return newtarg;
}

/*
===========
AI_Decision_Far

At FAR distance from the enemy - decide what state to take

============
*/
entity AI_Decision_Far(float in_sight,float in_front,entity targ)
{
	float chance;
	entity newtarg;

	// He was just hurt make him go through pain
	if (self.just_hurt)	// Just given pain
	{
		if (self.ai_poss_states & AI_PAIN_FAR )
			self.ai_new_state = AI_PAIN_FAR;
		else
			self.ai_new_state = AI_PAIN;

		self.just_hurt = FALSE;
		newtarg = targ;

	}

	// He just finished his pain now he should charge
	else if ((self.ai_state==AI_PAIN_FAR) || (self.ai_state==AI_PAIN))
	{
		chance = random();  // 45% chance he'll charge
							// 45% chance he'll fire a missile (if he can)
							// 10% chance of wandering to get away

		
		if (chance <= 0.10)
		{
			self.ai_new_state = AI_WANDER;		
			self.ai_duration = time + (random() * 2) + 2;
		}
		else if (chance <= 55)
		{
			if (self.ai_state & AI_MISSILE_ATTACK) 
			{
				self.ai_new_state = AI_MISSILE_ATTACK;		
				newtarg = targ;
			}
			else
			{
				self.ai_new_state = AI_CHARGE;		
				self.ai_duration = time + (random() * 4) + 4;
				newtarg = targ;
			}
		}
		else
		{
			self.ai_new_state = AI_CHARGE;		
			self.ai_duration = time + (random() * 4) + 4;
			newtarg = targ;
		}	
	}

	// Not in pain and everyone is too far away and monster has a path to walk
	else if (self.pathentity)
	{
		self.ai_new_state = AI_WALK;
		self.goalentity=self.pathentity;
		newtarg = world;
	}

	// Not in pain and everyone is too far away and no path to walk
	else
	{
		self.ai_new_state = AI_STAND;
		newtarg = world;
	}

	return newtarg;
}


/*
===========
AI_Decision_Tree

Have monster look for enemys

Based on range from enemy decide what state to go to

Place new state in ai_new_state

============
*/
void AI_Decision_Tree(float state_demanded)
{
	float rnge,in_sight,in_front;
	local entity	targ,finaltarg;

	if ((state_demanded==FALSE) || (self.ai_new_state == AI_DECIDE))  // Monster is not forcing a state
	{
		if ((self.ai_duration > time) && (self.ai_new_state != AI_DECIDE))   // Shouldn't change states just yet
			return;

		if (self.health > 0)  // Still alive
		{

			if (self.ai_new_state == AI_DECIDE)  // Reset time for the DECIDE state
				self.ai_duration = time - 1;

			targ=AI_Target();	// Find a target if there is one

			rnge = AI_calc_range (targ);		// Get target range

			in_sight=AI_visionblocked (targ);	// Is a wall between monster and target?

			in_front=AI_infront(targ);			// Is target in front of monster


			// Decide on a new state for the monster based on range to enemy
			if (rnge == RANGE_FAR)  
				finaltarg=AI_Decision_Far(in_sight,in_front,targ);

			else if (rnge == RANGE_MID)
				finaltarg=AI_Decision_Mid(in_sight,in_front,targ);

			else if (rnge == RANGE_NEAR)
				finaltarg=AI_Decision_Near(in_sight,in_front,targ);

			else if (rnge == RANGE_MELEE)   // He's very very close
				finaltarg=AI_Decision_Melee(in_sight,in_front,targ);
		}
		else 
			AI_Decision_Dead();			
	}
	else // Monster is forcing a state
		finaltarg=self.enemy;   // Keep it's current enemy


	if (self.ai_new_state!=self.ai_state) // Did we just change states??
		AI_Change_State(finaltarg);	

}


/*
===========
AI_Main

Main loop for monster AI

	- decide on state of monster
	- use monster's own AI function
	- use main movement function

============
*/
void AI_Main() 
{
	float demand_state;

	if (!comamode)	// Freeze monsters
	{
		self.ai_frame_time=0;

		demand_state=FALSE;

		do
		{
			AI_Decision_Tree(demand_state);	// Should monster change it's current state

			self.ai_self_func();	// AI particular to this type of monster (Archer_Walk  Archer_Stand...)

			// Did monster force a change in ai state? Change it before running ai_move_func
			if (self.ai_state != self.ai_new_state)  
				demand_state = TRUE;
			else
			{
				demand_state = FALSE;

				self.ai_move_func();   // General monster AI    AI_Walk, AI_Stand ...

				if (self.ai_state != self.ai_new_state)  // was a change in AI forced??
					demand_state = TRUE;
				else
					demand_state = FALSE;
			}

		} while (demand_state==TRUE);
	}
	else
		AI_TargetSearch ();

	if (!self.ai_frame_time)
		self.nextthink = time + HX_FRAME_TIME;
	else
	{
		self.nextthink = time + self.ai_frame_time;
	}
}

void AI_start ()
{

	self.origin_z += 1;	// raise off floor a bit
	droptofloor();
	
	if (!walkmove(0,0, FALSE))
	{
		dprint ("walkmonster in wall at: ");
		dprint (vtos(self.origin));
		dprint ("\n");
	}
	
	if (self.target)
	{
		self.goalentity = self.pathentity = find(world, targetname, self.target);
		self.ideal_yaw = vectoyaw(self.goalentity.origin - self.origin);
		if (!self.pathentity)
		{
			dprint ("Monster can't find target at ");
			dprint (vtos(self.origin));
			dprint ("\n");
		}
	}

	self.think = AI_Main;
// spread think times so they don't all happen at same time
	self.nextthink = self.nextthink + random()*0.5;


}

void AI_Land_Init ()
{

	self.takedamage = DAMAGE_YES;
	self.flags2+=FL_ALIVE;
	self.ideal_yaw = self.angles * '0 1 0';
	if (!self.yaw_speed)
		self.yaw_speed = 20;
	self.view_ofs = '0 0 25';
	self.use = monster_use;	
	self.flags = self.flags | FL_MONSTER;
	self.enemy = world;
	self.ai_state = AI_DECIDE;

// delay drop to floor to make sure all doors have been spawned
// spread think times so they don't all happen at same time
	self.nextthink = self.nextthink + random()*0.5;
	self.think = AI_start;
	total_monsters += 1;
}

