/*
 * $Header: /HexenWorld/Siege/paladin.hc 3     5/25/98 1:39p Mgummelt $
 */

/*
==============================================================================

Q:\art\models\players\paladin\newfinal\paladin.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\players\paladin\newfinal
$origin 0 0 0
$base BASE skin
$skin skin
$skin SKIN2
$skin skin3
$flags 0

//
$frame attgnt1      attgnt2      attgnt3      attgnt4      attgnt5      
$frame attgnt6      attgnt7      attgnt8      attgnt9      attgnt10     
$frame attgnt11     

//
$frame attstf1      attstf2      attstf3      attstf4      

//
$frame attswd1      attswd2      attswd3      attswd4      attswd5      
$frame attswd6      attswd7      attswd8      attswd9      attswd10     
$frame attswd11     attswd12     

//
$frame crouch1      crouch2      crouch3      crouch4      crouch5      
$frame crouch6      crouch7      crouch8      crouch9      crouch10     
$frame crouch11     crouch12     crouch13     crouch14     crouch15     
$frame crouch16     crouch17     crouch18     crouch19     crouch20     

//
$frame death1       death2       death3       death4       death5       
$frame death6       death7       death8       death9       death10      
$frame death11      death12      death13      death14      death15      
$frame death16      death17      death18      death19      death20      

//
$frame decap1       decap2       decap3       decap4       decap5       
$frame decap6       decap7       decap8       decap9       decap10      
$frame decap11      decap12      decap13      decap14      decap15      
$frame decap16      decap17      decap18      decap19      decap20      
$frame decap21      decap22      decap23      decap24      decap25      
$frame decap26      decap27      decap28      

//
$frame flygnt1      flygnt2      flygnt3      flygnt4      flygnt5      
$frame flygnt6      flygnt7      flygnt8      flygnt9      flygnt10     
$frame flygnt11     flygnt12     flygnt13     flygnt14     flygnt15     

//
$frame flystf1      flystf2      flystf3      flystf4      flystf5      
$frame flystf6      flystf7      flystf8      flystf9      flystf10     
$frame flystf11     flystf12     flystf13     flystf14     flystf15     

//
$frame flyswd1      flyswd2      flyswd3      flyswd4      flyswd5      
$frame flyswd6      flyswd7      flyswd8      flyswd9      flyswd10     
$frame flyswd11     flyswd12     flyswd13     flyswd14     flyswd15     

//
$frame jump1        jump2        jump3        jump4        jump5        
$frame jump6        jump7        jump8        jump9        jump10       
$frame jump11       jump12       

//
$frame paingnt1     paingnt2     paingnt3     paingnt4     paingnt5     
$frame paingnt6     paingnt7     

//
$frame painstf1     painstf2     painstf3     painstf4     painstf5     
$frame painstf6     painstf7     

//
$frame painswd1     painswd2     painswd3     painswd4     painswd5     
$frame painswd6     painswd7     

//
$frame rungnt1      rungnt2      rungnt3      rungnt4      rungnt5      
$frame rungnt6      rungnt7      rungnt8      rungnt9      rungnt10     
$frame rungnt11     rungnt12     

//
$frame runstf1      runstf2      runstf3      runstf4      runstf5      
$frame runstf6      runstf7      runstf8      runstf9      runstf10     
$frame runstf11     runstf12     

//
$frame runswd1      runswd2      runswd3      runswd4      runswd5      
$frame runswd6      runswd7      runswd8      runswd9      runswd10     
$frame runswd11     runswd12     

//
$frame stdgnt1      stdgnt2      stdgnt3      stdgnt4      stdgnt5      
$frame stdgnt6      stdgnt7      stdgnt8      stdgnt9      stdgnt10     
$frame stdgnt11     stdgnt12     stdgnt13     

//
$frame stdstf1      stdstf2      stdstf3      stdstf4      stdstf5      
$frame stdstf6      stdstf7      stdstf8      stdstf9      stdstf10     
$frame stdstf11     stdstf12     stdstf13     

//
$frame stdswd1      stdswd2      stdswd3      stdswd4      stdswd5      
$frame stdswd6      stdswd7      stdswd8      stdswd9      stdswd10     
$frame stdswd11     stdswd12     stdswd13     




/*--------------------------
ACTUAL (UNIQUE TO CLASS) PLAYER CODE
----------------------------*/
void() player_paladin_run;
void() player_paladin_crouch_stand;
void() player_paladin_crouch_move;
void() player_paladin_stand;
		
void()	player_paladin_jump=[++$jump1..$jump12]
{
	if(self.viewentity==self)
		self.th_weapon();
	if(cycle_wrapped)
	{
		if(!self.velocity_x && !self.velocity_y)
			self.think=self.th_stand;
		else
			self.think=self.th_run;
	}
};

void() player_paladin_swim =
{
	if(self.viewentity==self)
		self.th_weapon();
	if(self.waterlevel<3)
		if (self.velocity_x || self.velocity_y)
			self.think=self.th_run;
		else
			self.think=self.th_stand;
};

void() player_paladin_gauntlet_swim =[++$flygnt1..$flygnt15]
{
	player_paladin_swim();
};

void() player_paladin_staff_swim =[++$flystf1..$flystf15]
{
	player_paladin_swim();
};

void() player_paladin_swaxe_swim =[++$flyswd1..$flyswd15]
{
	player_paladin_swim();
};

void()	player_paladin_fly =
{
	if(self.viewentity==self)
		self.th_weapon();
	if(self.waterlevel>2)
		self.think=self.th_swim;
	else if(self.movetype!=MOVETYPE_FLY)
		if (self.velocity_x || self.velocity_y)
			self.think=self.th_run;
		else
			self.think=self.th_stand;
};

void() player_paladin_gauntlet_fly =[++$flygnt1..$flygnt15]
{
	player_paladin_fly();
};

void() player_paladin_staff_fly =[++$flystf1..$flystf15]
{
	player_paladin_fly();
};

void() player_paladin_swaxe_fly =[++$flyswd1..$flyswd15]
{
	player_paladin_fly();
};

void()	player_paladin_stand =
{
	if(self.viewentity==self)
		self.th_weapon();
	if(self.hull==HULL_CROUCH)
		self.think=player_paladin_crouch_stand;
	else if(self.waterlevel>2)
		self.think=self.th_swim;
	else if(self.movetype==MOVETYPE_FLY)
		self.think=self.th_fly;
	else if (self.velocity_x || self.velocity_y)
		self.think=self.th_run;
};

void() player_paladin_gauntlet_stand =[++$stdgnt1..$stdgnt13]
{
	player_paladin_stand();
};

void() player_paladin_staff_stand =[++$stdstf1..$stdstf13]
{
	player_paladin_stand();
};

void() player_paladin_swaxe_stand =[++$stdswd1..$stdswd13]
{
	player_paladin_stand();
};

void()	player_paladin_run =
{
	if(self.viewentity==self)
		self.th_weapon();
	if(self.hull==HULL_CROUCH)
		self.think=player_paladin_crouch_move;
	else if(self.waterlevel>2)
		self.think=self.th_swim;
	else if(self.movetype==MOVETYPE_FLY)
		self.think=self.th_fly;
	else if (!self.velocity_x && !self.velocity_y)
		self.think=self.th_stand;
};

void() player_paladin_gauntlet_run =[++$rungnt1..$rungnt12]
{
	player_paladin_run();
};

void() player_paladin_staff_run =[++$runstf1..$runstf12]
{
	player_paladin_run();
};

void() player_paladin_swaxe_run =[++$runswd1..$runswd12]
{
	player_paladin_run();
};

void()	player_paladin_crouch_stand =
{
	if(self.viewentity==self)
		self.th_weapon();
	if(self.frame>$crouch20 || self.frame<$crouch1)
		self.frame=$crouch1;
	if(self.movetype==MOVETYPE_FLY)
		self.think=self.th_fly;
	else if(self.hull==HULL_PLAYER)
		self.think=self.th_stand;
	else if (self.velocity_x || self.velocity_y)
		self.think=player_paladin_crouch_move;
	thinktime self : HX_FRAME_TIME;
};

void()	player_paladin_crouch_move =[++$crouch1..$crouch20]
{
	if(self.viewentity==self)
		self.th_weapon();
	if(self.movetype==MOVETYPE_FLY)
		self.think=player_paladin_fly;
	else if(self.hull==HULL_PLAYER)
		self.think=self.th_run;
	else if (!self.velocity_x && !self.velocity_y)
		self.think=player_paladin_crouch_stand;
};

void()	player_paladin_attack=
{
	if(self.viewentity==self)
		self.th_weapon();
	if(cycle_wrapped&&!self.button0)
	{
		if(!self.velocity_x && !self.velocity_y)
			self.think=self.th_stand;
		else
			self.think=self.th_run;
	}
};

void()	player_paladin_gauntlet_attack=[++$attgnt1..$attgnt11]
{
	player_paladin_attack();
};

void()	player_paladin_swaxe_attack=[++$attswd1..$attswd12]
{
	player_paladin_attack();
};

void()	player_paladin_staff_attack=[++$attstf1..$attstf4]
{
	player_paladin_attack();
};

void()	player_paladin_pain=
{
	if(self.viewentity==self)
		self.th_weapon();
	if(cycle_wrapped)
	{
		if(!self.velocity_x && !self.velocity_y)
			self.think=self.th_stand;
		else
			self.think=self.th_run;
	}
};

void() player_paladin_gauntlet_pain =[++$paingnt1..$paingnt7]
{
	if(self.frame==$paingnt1)
		PainSound();
	player_paladin_pain();
};

void() player_paladin_staff_pain =[++$painstf1..$painstf7]
{
	if(self.frame==$painstf1)
		PainSound();
	player_paladin_pain();
};

void() player_paladin_swaxe_pain =[++$painswd1..$painswd7]
{
	if(self.frame==$painswd1)
		PainSound();
	player_paladin_pain();
};

void()	player_paladin_die1=[++$death1..$death20]
{
	if(cycle_wrapped)
	{
		self.frame=$death20;
		self.think=PlayerDead;
	}
};

void()	player_paladin_die2=[++$death1..$death20]
{
	if(cycle_wrapped)
	{
		self.frame=$death20;
		self.think=PlayerDead;
	}
};

void()	player_paladin_behead =
{
	self.level=$decap1;
	self.dmg=$decap28;
	self.cnt=0;
	player_behead();
};

void Pal_Change_Weapon (void)
{
	if(self.weapon==IT_WEAPON1)
	{
		self.th_stand=player_paladin_gauntlet_stand;
		self.th_missile=pal_gauntlet_fire;
		self.th_run=player_paladin_gauntlet_run;
		self.th_pain=player_paladin_gauntlet_pain;
		self.th_swim=player_paladin_gauntlet_swim;
		self.th_fly=player_paladin_gauntlet_fly;
	}
	else if(self.weapon==IT_WEAPON4)
	{
		self.th_stand=player_paladin_staff_stand;
		self.th_missile=pal_purifier_fire;
		self.th_run=player_paladin_staff_run;
		self.th_pain=player_paladin_staff_pain;
		self.th_swim=player_paladin_staff_swim;
		self.th_fly=player_paladin_staff_fly;
	}
	else
	{
		self.th_stand=player_paladin_swaxe_stand;
		if(self.weapon==IT_WEAPON2)
			self.th_missile=pal_vorpal_fire;
		else
			self.th_missile=pal_axe_fire;
		self.th_run=player_paladin_swaxe_run;
		self.th_pain=player_paladin_swaxe_pain;
		self.th_swim=player_paladin_swaxe_swim;
		self.th_fly=player_paladin_swaxe_fly;
	}
	if(self.hull!=HULL_CROUCH)
		self.think=self.th_stand;
}

/*
void paladin_spurt ()
{
	SpawnPuff (self.origin+'0 0 56', '0 0 35',5,self);
	thinktime self : 0.1;
}
*/
/*QUAK-ED paladin_dead (1 0 0) (-16 -16 0) (16 16 56)
*/
/*
void paladin_dead(void)
{
	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_STEP;
	self.takedamage=DAMAGE_YES;
	self.flags2(+)FL_ALIVE;
	self.thingtype=THINGTYPE_FLESH;
	self.frame=$decap14;
	setmodel (self, "models/paladin.mdl");

	setsize (self, '-16 -16 0', '16 16 200');
	self.hull=HULL_POINT;
	self.health = self.max_health=2000;
	self.mass = 2000;
	self.drawflags(+)MLS_ABSLIGHT;
	self.abslight=0.5;
	self.think=paladin_spurt;
	thinktime self : 0.1;
}
*/

