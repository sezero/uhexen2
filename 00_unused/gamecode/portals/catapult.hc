void movechain_target (void)
{
	if(self.target)
	{
	entity found;
		found=find(world,targetname,self.target);
		if(found==self)
		{
			found=nextent(self);
			found=find(found,targetname,self.target);
		}
		self.movechain=found;
		found.flags(+)FL_MOVECHAIN_ANGLE;
	}
	else
	{
		dprint(self.classname);
		dprint(" has no target set\n");
	}
	self.nextthink=-1;
}

void BSP_stop (void)
{
	self.velocity='0 0 0';
	self.think=BSP_stop;
	self.nextthink=time - 1;
}

void BSP_push (void)
{
vector pos,dir;
	if(other.absmin_z>self.absmin_z+4||vlen(other.velocity)<150)
		return;
	dir=normalize(other.velocity+self.velocity);
	dir_z=0;
	pos=dir*10+self.origin;
	SUB_CalcMove(pos,10,BSP_stop);
}

/*QUAKED obj_stairs (0 .5 .8) ?
*/
void obj_stairs (void)
{
	if(!self.thingtype)
		self.thingtype=THINGTYPE_WOOD;
	if(self.health)
	{
		self.takedamage=DAMAGE_NO_GRENADE;
		self.th_die=chunk_death;
	}

	self.flags(+)FL_PUSH;
	self.solid=SOLID_BSP;
	self.movetype=MOVETYPE_PUSH;
	self.touch=BSP_push;

	setmodel (self, self.model);
	setsize(self,self.mins,self.maxs);
	setorigin(self,self.origin);
}

/*QUAKED obj_bridge (0 .5 .8) ?
*/
void obj_bridge (void)
{
	if(!self.thingtype)
		self.thingtype=THINGTYPE_WOOD;
	if(self.health)
	{
		self.takedamage=DAMAGE_NO_GRENADE;
		self.th_die=chunk_death;
	}

	self.solid=SOLID_SLIDEBOX;
	self.movetype=MOVETYPE_PUSHPULL;
	self.touch=obj_push;

	setmodel (self, self.model);
	setsize(self,self.mins,self.maxs);
	setorigin(self,self.origin);
}
void() catapult_button_turn=
{
};
void() catapult_button_think=
{
};