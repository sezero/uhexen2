void() SmallExplosion =
{
        particle (self.origin, '0 0 10', 242, 5);
        self.movetype = MOVETYPE_NONE;
        self.velocity = '0 0 0';
        self.touch = SUB_Null;
        self.solid = SOLID_NOT;
        sound(self,CHAN_AUTO,"weapons/r_exp3.wav",0.25,ATTN_NORM);
        remove(self);
};

void() CB_BoltStick;
void(float offset) W_FireFlame;
void()BlowUp=
{
	if(self.mass<2.5)
	{
		self.v_angle_x=random()*360;
		self.v_angle_y=random()*360;
		self.v_angle_z=random()*360;
		self.scale=self.mass;
//		W_FireFlame(random()*360);
//		W_FireFlame(random()*360);
	    T_RadiusDamage (self, self.owner, self.mass*100, world);
		self.mass += 0.1;
		self.think=BlowUp;
		self.nextthink=time;//+0.1;
	}
	else
	{
		self.think=SUB_Remove;
		self.nextthink=time;
	}
};

void() SprayFire=
{
local entity fireballblast;
	sound(self,CHAN_AUTO,"weapons/fbfire.wav",1,ATTN_NORM);
	fireballblast=spawn();
	fireballblast.movetype=MOVETYPE_NOCLIP;
	fireballblast.owner=self.owner;
	fireballblast.classname="fireballblast";
	fireballblast.solid=SOLID_NOT;
	fireballblast.drawflags +=MLS_ABSLIGHT+SCALE_TYPE_UNIFORM+SCALE_ORIGIN_CENTER;
	fireballblast.abslight= 1;
	fireballblast.scale=0.1;
	setmodel(fireballblast,"progs/blast.mdl");
	setsize(fireballblast,'0 0 0','0 0 0');
	setorigin(fireballblast,self.origin);
	fireballblast.effects=EF_BRIGHTLIGHT;
	fireballblast.mass=0.1;
	fireballblast.avelocity='50 50 50';
	fireballblast.think=BlowUp;
	fireballblast.nextthink=time;
};

void() GrenadeExplode2 =
{
		if(self.classname=="stickmine")
				SprayFire();
	    T_RadiusDamage (self, self.owner, self.mass, world);

		if(self.small)
                SmallExplosion();
        else
        {
			WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
			WriteByte (MSG_BROADCAST, TE_EXPLOSION);
			WriteCoord (MSG_BROADCAST, self.origin_x);
			WriteCoord (MSG_BROADCAST, self.origin_y);
			WriteCoord (MSG_BROADCAST, self.origin_z);

			BecomeExplosion ();
        }
};

void() MultipleExplode;

void() GrenadeTouch2 =
{
	if (other == self.owner)
		return;         // don't explode on owner
    if(other.owner==self.owner&&other.classname==self.classname&&self.classname=="minigrenade")
        return;

//	if (other.takedamage == DAMAGE_YES)
	if (other.takedamage)
	{
                if(self.classname=="multigrenade")
                        MultipleExplode();
                else GrenadeExplode2();
		return;
	}
	sound (self, CHAN_WEAPON, "idweapons/tink1.wav", 1, ATTN_NORM);  // bounce sound
	if (self.velocity == '0 0 0')
		self.avelocity = '0 0 0';
};

void() StickMineTouch =
{
        if(other==self.owner)
                return;
local vector stickdir;
        sound(self, CHAN_WEAPON, "idweapons/pkup.wav", 1, ATTN_NORM);
		if(other.takedamage)
		{
//			makevectors(self.angles);
			stickdir=other.origin+normalize(self.origin-other.origin)*12;
//Modify height for shorter or taller models like imp, golem, spider, etc.
			if(other.classname=="player")
//Put it right below view of player
				stickdir_z=other.origin_z+other.view_ofs_z - 5;
			else if(other.classname=="monster_spider")
				stickdir_z=(self.origin_z+(other.origin_z+other.size_z*0.2)*3)*0.25;
			else stickdir_z=(self.origin_z+(other.origin_z+other.size_z*0.6)*3)*0.25;
			setorigin(self,stickdir);
			T_Damage(other,self,self.owner,3);
			SpawnPuff(self.origin+v_forward*8,'0 0 0'-v_forward*24,10,other);
		}
		else
			SpawnPuff(self.origin+v_forward*8,'0 0 0'-v_forward*24,10,world);

        self.velocity='0 0 0';
        self.movetype=MOVETYPE_NOCLIP;
        self.solid=SOLID_NOT;
        self.touch=SUB_Null;
        self.wait=time + 2;
		self.health=other.health;
        if(other.movetype)
        {
                self.enemy=other;
                self.view_ofs=(self.origin-other.origin);
                self.o_angle=(self.angles-self.enemy.angles);
				self.think=CB_BoltStick;
                self.nextthink=time;
        }
        else
        {
				self.movetype=MOVETYPE_NONE;
				self.think=GrenadeExplode2;
                self.nextthink=time + 2;
        }
};

void() Use_StickMine =
{
		self.attack_finished=time + 0.2;
        makevectors(self.v_angle);
//sound
        local entity missile;
                missile=spawn();
                missile.owner=self;
                missile.classname="stickmine";
                missile.movetype=MOVETYPE_BOUNCE;
                missile.solid=SOLID_BBOX;
//                missile.takedamage=DAMAGE_YES;
//                missile.health=10;
//                missile.th_die=GrenadeExplode2;
                missile.touch=StickMineTouch;
                missile.mass=50;

                missile.velocity=normalize(v_forward)*700 +v_up*200;
				missile.avelocity_x=random()*600 - 300;
				missile.avelocity_y=random()*600 - 300;
				missile.avelocity_z=random()*600 - 300;

                setmodel(missile,"progs/stikgren.mdl");
                setsize(missile,'0 0 0','0 0 0');
                setorigin(missile,self.origin+v_forward*16+'0 0 16');
                missile.think=GrenadeExplode2;
                missile.nextthink=time + 10;
};

void ParticleTesterThink(void)
{
	if(attck_cnt==-1)
		remove(self);
	particle2(self.origin+'0 0 64', '0 0 0','0 0 0',rint(random()*255+1),attck_cnt,10);
	self.think=ParticleTesterThink;
	self.nextthink=time + 0.1;
}

void() ParticleTester=
{
     local entity missile;
				attck_cnt=0;
				self.lifetime=TRUE;
                missile=spawn();
                missile.owner=self;
                missile.movetype=MOVETYPE_BOUNCE;
                missile.solid=SOLID_BBOX;
                missile.velocity=normalize(v_forward)*300;
                setmodel(missile,"progs/stikgren.mdl");
                setsize(missile,'0 0 0','0 0 0');
                setorigin(missile,self.origin+v_forward*16+'0 0 16');
                missile.think=ParticleTesterThink;
                missile.nextthink=time;
};

