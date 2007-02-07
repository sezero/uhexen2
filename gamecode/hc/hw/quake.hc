/*
=========================================
QUAKE.HC
MG

EarthQuakes!
=========================================
*/

void() ShakeRattleAndRoll =
{
entity head;
float inertia;
float richter;
float dist;
float rand,boost;
vector dir;

	head = findradius(self.origin, self.mass);
	richter=self.mass/100*(self.lifetime-time)/3;
	
//FIXME:  Add particle dust
	while (head)
	{
		if ((head.movetype==MOVETYPE_STEP||head.movetype==MOVETYPE_WALK||head.movetype==MOVETYPE_PUSHPULL)&&head.solid!=SOLID_BSP&&head.mass<500 && head != self.owner && ((head.classname != "monster_golem_bronze") &&(head.classname != "monster_golem_iron") && (head.classname != "monster_golem_stone")&& (head.classname != "monster_golem_crystal")))
		{	
//			bprint(head.classname);
			dist=vlen(self.origin-head.origin)/self.mass;
			if(!head.mass)
				inertia=1;
			else
				inertia=head.mass/10;
			if((!head.flags2&FL_ALIVE)&&head.takedamage&&head.health&&random()<0.2&&head!=self.owner)
				T_Damage(head,self,self.owner,richter*dist);
			if(head.flags&FL_ONGROUND)
			{
				if(head!=self.owner)
				{
					if(head.classname=="player")
					{
						head.punchangle_x=random()*10 - 5;    
						head.punchangle_y=random()*8 - 4;
						head.punchangle_z=random()*8 - 4;
					}
					else if(head.flags2&FL_ALIVE)
					{
						head.angles_y+=random()*20 - 10;
						rand=random()*6 - 3;//FIXME: Remember old angles
						if(head.angles_x+rand>30||head.angles_x+rand<-30)
							head.angles_x-=rand;
						else
							head.angles_x+=rand;
						rand=random()*6 - 3;
						if(head.angles_z+rand>30||head.angles_z+rand<-30)
							head.angles_z-=rand;
						else
							head.angles_z+=rand;
					}
					else
					{
						if(head.movetype!=MOVETYPE_BOUNCE)
						{
							head.oldmovetype=head.movetype;
							head.movetype=MOVETYPE_BOUNCE;
						}
						else if(!head.oldmovetype)
							head.oldmovetype=MOVETYPE_BOUNCE;
						head.avelocity_z= random(1,10);
						head.avelocity_y= random()*720 - 360;
						head.avelocity_x= random()*720 - 360;
					}
					boost= (random(100)+25)/inertia*richter*dist;
					if(boost>100)
						boost=100;
					head.velocity_z+=boost;
				}
				if(self.owner.classname=="monster_golem_bronze")
				{//Make Bronze G make you drift towards him
					dir=normalize(self.owner.origin-head.origin);
					dir_z=0;
					head.velocity=head.velocity+dir*((random()*50 - 25)/inertia*richter*dist);
				}
				else
				{
					head.velocity_y+= (random()*50 - 25)/inertia*richter*dist;
					head.velocity_x+= (random()*50 - 25)/inertia*richter*dist;
					head.flags(-)FL_ONGROUND;
				}
			}
			if(self.lifetime<time+0.2)
			{
				if(head.movetype!=head.oldmovetype&&head.movetype==MOVETYPE_BOUNCE)
					head.movetype=head.oldmovetype;
				if(head.classname!="player"&&(head.angles_x!=0||head.angles_y!=0))
				{
					//FIXME: make sure this doesn't affect their original angles... store this in their o_angle?
					head.angles_x=0;
					head.angles_z=0;
				}
			}
		}
		head = head.chain;
	}
	if(self.lifetime>time)
		self.think = ShakeRattleAndRoll;
	else
		self.think=SUB_Remove;
	self.nextthink = time + 0.1;
};

void (float richter) MonsterQuake =
{
	newmis=spawn();
	newmis.owner=self;
	newmis.solid=SOLID_NOT;
	newmis.movetype=MOVETYPE_NONE;
	newmis.classname="quake";
	newmis.think=ShakeRattleAndRoll;
	newmis.nextthink=time;
	newmis.mass=fabs(richter);
	newmis.lifetime=time + 3;
	setorigin(newmis,self.origin);
//FIXME:  Replace explosion with some other quake-start sound
	sound(newmis,CHAN_AUTO,"weapons/explode.wav",1,ATTN_NORM);
	sound(newmis,CHAN_AUTO,"fx/quake.wav",1,ATTN_NORM);
};

