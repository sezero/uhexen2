/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/fight.hc,v 1.2 2007-02-07 16:59:32 sezero Exp $
 */



/*
 * A monster is in fight mode if it thinks it can effectively attack its enemy.
 * When it decides it can't attack, it goes into hunt mode.
 */


float anglemod(float v);
//void  ChooseTurn(vector dest);
void  ai_face();
float CheckMonsterAttack(float AttackType, float ChanceModifier);


float enemy_vis, enemy_infront, enemy_range;
float enemy_yaw;



float MAX_MELEE			= 1;
float MAX_MISSILE		= 2;
float MAX_BOTH			= 3;
float MAX_FAR_MELEE		= 4;
float MAX_SHORT_MISSILE = 8;





/*
 * CheckAttack() -- The player is in view, so decide to move or launch an
 *                  attack.  Returns FALSE if movement should continue.
 */

float CheckAttack()
{
vector	spot1, spot2;	
entity	targ;
float		chance;

	targ = self.enemy;
	
// see if any entities are in the way of the shot
	spot1 = self.origin + self.view_ofs;
	spot2 = (targ.absmin+targ.absmax)*0.5;

	traceline (spot1, spot2, FALSE, self);

	if(trace_ent.thingtype>=THINGTYPE_WEBS)
		traceline (trace_endpos, spot2, FALSE, trace_ent);

	if (trace_ent != targ)
		if(trace_ent.health>25||!trace_ent.takedamage||(trace_ent.flags&FL_MONSTER&&trace_ent.classname!="player_sheep"))
			return FALSE;//Don't have a clear shot, and don't want to shoot obstruction
			
//FIXME: check for translucent water?
	if (trace_inopen && trace_inwater)
		return FALSE;			// sight line crossed contents

	if (enemy_range == RANGE_MELEE)
	{	// melee attack
		if (self.th_melee)
		{
			self.th_melee ();
			return TRUE;
		}
	}

//FIXME: check for darkness, maybe won't fire, maybe aim will be off
	
// missile attack
	if (!self.th_missile)
		return FALSE;
		
	if (time < self.attack_finished)
		return FALSE;
		
	if (enemy_range == RANGE_FAR)
		return FALSE;
		
	if (enemy_range == RANGE_MELEE)
	{
		chance = 0.9;
		self.attack_finished = 0;
	}
	else if (enemy_range == RANGE_NEAR)
	{
		if (self.th_melee)
			chance = 0.2;
		else
			chance = 0.4;
	}
	else if (enemy_range == RANGE_MID)
	{
		if (self.th_melee)
			chance = 0.05;
		else
			chance = 0.1;
	}
	else
		chance = 0;

	if (random () < chance)
	{
		self.th_missile ();
		if(skill>=4)
			self.attack_finished=0;
		else
			SUB_AttackFinished (random(0,2));
		return TRUE;
	}

	return FALSE;
}


/*
 * ai_face() -- Keep facing the enemy.
 */

void ai_face()
{
	self.ideal_yaw = vectoyaw(self.goalentity.origin - self.origin);
//	self.ideal_yaw = vectoyaw(self.enemy.origin - self.origin);
	ChangeYaw();
}


/*
 * ai_charge() -- The monster is in a melee attack, so get as close as
 *                possible to .enemy.
 */

float visible(entity targ);
float infront(entity targ);
float range  (entity targ);

void ai_charge(float d)
{
	ai_face();	
	movetogoal(d);		// done in C code...
}

/*
void ai_charge_side()
{
	local	vector	dtemp;
	local	float	heading;
	
// aim to the left of the enemy for a flyby

//	self.ideal_yaw = vectoyaw(self.enemy.origin - self.origin);
	self.ideal_yaw = vectoyaw(self.goalentity.origin - self.origin);
	ChangeYaw();

	makevectors (self.angles);
	dtemp   = self.enemy.origin - 30*v_right;
	heading = vectoyaw(dtemp - self.origin);
	
	walkmove(heading, 20, FALSE);
}
*/

/*
 * ai_melee()
 */

void ai_melee()
{//Bad idea- doesn't care where around self player is!
vector	org1,org2;
float 	ldmg;

	if (!self.enemy)
		return;		// removed before stroke
		
	org1=self.origin+self.proj_ofs;
	org2=self.enemy.origin;

	if(vlen(org2-org1)>60)
		return;

	traceline(org1,org2,FALSE,self);
	if(trace_ent!=self.enemy)
	{
		org2=(self.enemy.absmin+self.enemy.absmax)*0.5;
		traceline(org1,org2,FALSE,self);
	}
		
	if(!trace_ent.takedamage)
		return;

	if(self.model=="models/spider.mdl")
		ldmg=random(self.scale*3);
	else
		ldmg = random(9);

	T_Damage (trace_ent, self, self, ldmg);
}


/*
 * ai_melee_side()
 */
/*
void ai_melee_side()
{
	local vector	delta;
	local float 	ldmg;

	if (!self.enemy)
		return;		// removed before stroke
		
	ai_charge_side();
	
	delta = self.enemy.origin - self.origin;

	if (vlen(delta) > 60)
		return;
	if (!CanDamage (self.enemy, self))
		return;
	ldmg = random(9);
	T_Damage (self.enemy, self, self, ldmg);
}
*/

