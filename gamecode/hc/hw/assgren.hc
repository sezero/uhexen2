/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/hw/assgren.hc,v 1.1.1.1 2004-11-29 11:24:58 sezero Exp $
* Grenade Throw, Assassin. 
*/

/*
==============================================================================

Q:\art\models\weapons\grenades\final\assgr.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\weapons\grenades\final
$origin 0 0 0
$base BASE skin
$skin skin
$flags 0

$frame select1      select2      select3      select4      select5      
$frame select6      

$frame idle

$frame throw1      throw2      throw3      throw4      throw5      
$frame throw6      throw7      throw8      throw9      throw10     
$frame throw11     throw12

void grenade_trail ()
{
	if(self.lifetime<time)
	{
	    self.think=self.th_die;
		thinktime self : 0;
	}
	else
	{
//		particle4(self.origin,0.5,284,PARTICLETYPE_SLOWGRAV,3);
		thinktime self : 0.05;
	}
}

void() ThrowMiniGrenade =
{
		self.greenmana-=6;
        makevectors(self.v_angle);
		sound(self,CHAN_WEAPON,"misc/whoosh.wav",1,ATTN_NORM);
entity missile;
        missile=spawn();
        missile.owner=self;
        missile.classname="minigrenade";
        missile.movetype=MOVETYPE_BOUNCE;
        missile.solid=SOLID_BBOX;
		missile.takedamage=DAMAGE_YES;
		missile.health=3;
		missile.th_die=DarkExplosion;
        missile.touch=GrenadeTouch2;
        missile.dmg=100;
//		missile.lifetime=time+2;

		missile.o_angle = self.origin+self.proj_ofs+v_forward*8+v_right*8;

		missile.speed=500+self.weaponframe_cnt*10;
//        missile.velocity=(normalize(v_forward)+'0 0 .4')*missile.speed;
//UQ method
		if(self.v_angle_x)
			missile.velocity = v_forward*missile.speed + v_up * 200 + crandom()*v_right*10 + crandom()*v_up*10;
		else
		{
			missile.velocity = aim(self, missile.o_angle,1000);
	        missile.velocity = missile.velocity * missile.speed;
            missile.velocity_z = 200;
		}

		missile.angles = vectoangles(missile.velocity);
		missile.avelocity=randomv('-300 -300 -300','300 300 300');

        setmodel(missile,"models/assgren.mdl");
		missile.scale=0.77;
        setsize(missile,'0 0 0','0 0 0');
        setorigin(missile,missile.o_angle);
		missile.think=missile.th_die;//grenade_trail;
		thinktime missile : 2;//0;
};


void AssGrenExplosion ()
{
	entity ignore;

//	if(self.controller.classname=="multigrenade")
//		sound(self.controller,CHAN_BODY,"weapons/explode.wav",1,ATTN_NORM);
//	else
//		sound(self,CHAN_AUTO,"weapons/explode.wav",1,ATTN_NORM);
	ignore=world;

	T_RadiusDamage (self, self.owner, self.dmg, ignore);

//	BecomeExplosion(FALSE);
	remove (self);
}

void() SuperGrenadeExplode =
{
		entity missile;
		float attack_counter,number_explosions;

		self.takedamage=DAMAGE_NO;
		self.th_die = SUB_Null;

        attack_counter=0;        
        number_explosions=rint(random(3,6));
        while(attack_counter<number_explosions)
        {
			attack_counter+=1;
//			if(random()<0.5)
				missile=spawn_temp();
//			else
//				missile=spawn();
            missile.owner=self.owner;
            missile.classname="minigrenade";
			if(self.classname=="multigrenade")
			{
				missile.controller=self;
				if(self.cnt)
					self.cnt=FALSE;
				else
					self.cnt=TRUE;
			}
            if((self.classname=="multigrenade"&&self.cnt)||(self.classname!="multigrenade"&&self.movetype==MOVETYPE_BOUNCE))
            {
                missile.movetype=MOVETYPE_BOUNCE;
                missile.velocity_x=random(-300,300);
                missile.velocity_y=random(-300,300);
                missile.velocity_z=random(50,150);
            }
            else
            {
                missile.movetype=MOVETYPE_FLYMISSILE;
                missile.velocity_x=random(-40,40);
                missile.velocity_y=random(-40,40);
	            missile.velocity_z=random(150,300);
            }
            missile.dmg=self.dmg=self.dmg*(0.7+random(0.2));
			if(missile.dmg<70)
				missile.dmg=70;
			missile.solid=SOLID_NOT;
//            setmodel(missile,"models/null.spr");
			setsize(missile,'0 0 0','0 0 0');
            setorigin(missile,self.origin);
			if(missile.dmg>70)
				missile.think=SuperGrenadeExplode;
            else 
				missile.think=AssGrenExplosion;
            thinktime missile : random(0.1,0.6);
			missile.effects (+) EF_NODRAW;
        }
		self.dmg*=2;
		if(self.classname=="multigrenade")
		{
			//testing: put back in damage!!!!!!
			WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
			WriteByte (MSG_MULTICAST, TE_BIGGRENADE);
			WriteCoord (MSG_MULTICAST, self.origin_x);
			WriteCoord (MSG_MULTICAST, self.origin_y);
			WriteCoord (MSG_MULTICAST, self.origin_z);
			multicast(self.origin,MULTICAST_PHS_R);
//			remove(self);

			if(random()<0.3)
				MonsterQuake(200);
			MultiExplode();
		}
		else
			AssGrenExplosion();
			


};

void() ThrowMultiGrenade =
{//FIXME: too many t_rad's?
entity missile;
        makevectors(self.v_angle);
		self.greenmana-=15;
		sound(self,CHAN_WEAPON,"misc/whoosh.wav",1,ATTN_NORM);
        missile=spawn();
		missile.frags=TRUE;
        missile.owner=self;
        missile.classname="multigrenade";
        missile.movetype=MOVETYPE_BOUNCE;
        missile.solid=SOLID_BBOX;
		missile.takedamage=DAMAGE_YES;
		missile.health=3;
		missile.th_die=SuperGrenadeExplode;
        missile.touch=GrenadeTouch2;
        missile.dmg=250;//simulates max level for now

		missile.o_angle = self.origin+self.proj_ofs+v_forward*8+v_right*8;

		missile.speed=500+self.weaponframe_cnt*10;
//        missile.velocity=(normalize(v_forward)+'0 0 .4')*missile.speed;
//UQ method
		if(self.v_angle_x)
			missile.velocity = v_forward*missile.speed + v_up * 200 + crandom()*v_right*10 + crandom()*v_up*10;
		else
		{
			missile.velocity = aim(self, missile.o_angle,1000);
	        missile.velocity = missile.velocity * missile.speed;
            missile.velocity_z = 200;
		}

		missile.angles = vectoangles(missile.velocity);
		missile.avelocity=randomv('-300 -300 -300','300 300 300');

        setmodel(missile,"models/assgren.mdl");
		missile.scale=2;
		setsize (missile, '0 0 0', '0 0 0');
        setorigin(missile,missile.o_angle);

		missile.lifetime=time+2;
        missile.think=grenade_trail;
        thinktime missile : 0;
};



void()grenade_select;
void()grenade_throw;
void grenade_idle(void)
{
	self.th_weapon=grenade_idle;
	self.weaponframe=$idle;
}

void grenade_reload (void)
{
	self.th_weapon=grenade_reload;
	self.wfs = advanceweaponframe($select1,$select6);
	self.weaponmodel = "models/v_assgr.mdl";
	if (self.wfs==WF_CYCLE_WRAPPED)
		grenade_idle();
}

void grenade_throw (void)
{
	self.th_weapon=grenade_throw;
	self.wfs = advanceweaponframe($throw1,$throw12);
	if(self.button0&&self.weaponframe==$throw5)
	{
		self.weaponframe=$throw4;
		if(self.weaponframe_cnt<50)
			self.weaponframe_cnt+=1;
	}
	else if(self.weaponframe==$throw10)
	{
		if(self.artifact_active&ART_TOMEOFPOWER)
		{
			ThrowMultiGrenade();
			self.attack_finished=time + 2;
		}
		else
		{
			ThrowMiniGrenade();
			self.attack_finished=time+0.3;
		}
		self.weaponframe_cnt=0;
	}
	else if (self.wfs==WF_CYCLE_WRAPPED)
		grenade_reload();
}

void grenade_select (void)
{
//selection sound?
	self.th_weapon=grenade_select;
	self.wfs = advanceweaponframe($select1,$select6);
	self.weaponmodel = "models/v_assgr.mdl";
	if (self.wfs==WF_CYCLE_WRAPPED)
	{
		self.attack_finished = time - 1;
		grenade_idle();
	}
}

void grenade_deselect (void)
{
//selection sound?
	self.th_weapon=grenade_deselect;
	self.wfs = advanceweaponframe($select6,$select1);
	if (self.wfs==WF_CYCLE_WRAPPED)
		W_SetCurrentAmmo();
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2001/11/09 17:05:00  theoddone33
 * Inital import
 *
 * 
 * 12    4/02/98 2:08p Mgummelt
 * Testing damage modification on grenades
 * 
 * 11    3/25/98 3:37p Ssengele
 * increased amt of mana used by grenades to make it more difficult to
 * simply blanket-attack w/ em.
 * 
 * 10    3/24/98 6:06p Ssengele
 * exploding grenades, xbolts, tripmines can't go into infinite loops
 * anymore; sheep impulse has 10 sec delay; scarab staff using union in
 * non-dangerous way (tho that frankly dint seem to cause any probs)
 * 
 * 9     3/05/98 12:04a Ssengele
 * just updated a comment--wOOO.
 * 
 * 8     2/24/98 6:30p Ssengele
 * grenade smoke is somewhat acceptable now. but you can't see it yet. so
 * there.
 * 
 * 7     2/17/98 3:39p Ssengele
 * my very first multicast. oboy!
 * 
 * 6     2/17/98 1:49p Ssengele
 * big grenade is pretty damn net-friendly now, and fully functional too
 * (ooh, still need to switch the msg_broadcast to a multicast).
 * 
 * 5     2/16/98 3:16p Ssengele
 * just added a comment about the boffo grenade's tempent message, so i
 * don't forget to change it.
 * 
 * 4     2/16/98 3:11p Ssengele
 * first step in putting the sounds & damage back in for ultra
 * grenade--still need to make these server-spawned tempents not be sent
 * to client (will leave them in to avoid mucking with damage system), and
 * make the sounds not be played on server.
 * 
 * 3     2/13/98 11:58a Ssengele
 * first pass at speeding up supergrenade
 * 
 * 2     2/12/98 2:37p Ssengele
 * gave explosion structure rudimentary usefulness--prolly should add a
 * frameFunc and removeFunc, maybe need to add angles?
 * 
 * 1     2/04/98 1:59p Rjohnson
 * 
 * 48    9/03/97 7:30a Mgummelt
 * 
 * 47    8/30/97 11:53p Mgummelt
 * 
 * 46    8/26/97 6:00p Mgummelt
 * 
 * 45    8/26/97 7:38a Mgummelt
 * 
 * 44    8/19/97 12:57p Mgummelt
 * 
 * 43    8/13/97 5:46p Mgummelt
 * 
 * 42    8/08/97 6:22p Mgummelt
 * 
 * 41    7/30/97 8:26p Mgummelt
 * 
 * 40    7/24/97 11:38a Mgummelt
 * 
 * 39    7/17/97 6:53p Mgummelt
 * 
 * 38    7/16/97 6:06p Mgummelt
 * 
 * 37    7/15/97 9:19p Mgummelt
 * 
 * 36    7/15/97 8:30p Mgummelt
 * 
 * 35    7/14/97 9:30p Mgummelt
 * 
 * 34    7/10/97 7:21p Mgummelt
 * 
 * 33    7/09/97 6:31p Mgummelt
 * 
 * 32    7/02/97 6:35p Mgummelt
 * 
 * 31    7/01/97 3:30p Mgummelt
 * 
 * 30    7/01/97 2:21p Mgummelt
 * 
 * 29    6/30/97 5:37p Mgummelt
 * 
 * 28    6/18/97 8:14p Mgummelt
 * 
 * 27    6/18/97 6:58p Mgummelt
 * 
 * 26    6/18/97 6:21p Mgummelt
 * 
 * 25    6/18/97 6:19p Mgummelt
 * 
 * 24    6/18/97 5:56p Mgummelt
 * 
 * 23    6/18/97 4:00p Mgummelt
 * 
 * 22    6/16/97 2:21p Mgummelt
 * 
 * 21    6/16/97 2:20p Mgummelt
 * 
 * 20    6/05/97 9:29a Rlove
 * Weapons now have deselect animations
 * 
 */
