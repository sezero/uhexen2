/*
 * $Header: /H3MP/hcode/assassin.hc 38    10/28/97 1:00p Mgummelt $
 */

/*
==============================================================================

Q:\art\models\players\assassin\newfinal\assassin.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\players\assassin\newfinal
$origin 0 0 0
$base BASE Skin
$skin Skin
$flags 0

//
$frame attdag1      attdag2      attdag3      attdag4      attdag5      
$frame attdag6      attdag7      attdag8      attdag9      attdag10     
$frame attdag11     

//
$frame attstf1      attstf2      attstf3      attstf4      

//
$frame attxbw1      attxbw2      attxbw3      attxbw4      

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
$frame flydag1      flydag2      flydag3      flydag4      flydag5      
$frame flydag6      flydag7      flydag8      flydag9      flydag10     
$frame flydag11     flydag12     flydag13     flydag14     flydag15     

//
$frame flystf1      flystf2      flystf3      flystf4      flystf5      
$frame flystf6      flystf7      flystf8      flystf9      flystf10     
$frame flystf11     flystf12     flystf13     flystf14     flystf15     

//
$frame flyxbw1      flyxbw2      flyxbw3      flyxbw4      flyxbw5      
$frame flyxbw6      flyxbw7      flyxbw8      flyxbw9      flyxbw10     
$frame flyxbw11     flyxbw12     flyxbw13     flyxbw14     flyxbw15     

//
$frame jump1        jump2        jump3        jump4        jump5        
$frame jump6        jump7        jump8        jump9        jump10       
$frame jump11       jump12       

//
$frame paindag1     paindag2     paindag3     paindag4     paindag5     
$frame paindag6     paindag7     

//
$frame painstf1     painstf2     painstf3     painstf4     painstf5     
$frame painstf6     painstf7     

//
$frame painxbw1     painxbw2     painxbw3     painxbw4     painxbw5     
$frame painxbw6     painxbw7     

//
$frame rundag1      rundag2      rundag3      rundag4      rundag5      
$frame rundag6      rundag7      rundag8      rundag9      rundag10     
$frame rundag11     rundag12     

//
$frame runstf1      runstf2      runstf3      runstf4      runstf5      
$frame runstf6      runstf7      runstf8      runstf9      runstf10     
$frame runstf11     runstf12     

//
$frame runxbw1      runxbw2      runxbw3      runxbw4      runxbw5      
$frame runxbw6      runxbw7      runxbw8      runxbw9      runxbw10     
$frame runxbw11     runxbw12     

//
$frame stddag1      stddag2      stddag3      stddag4      stddag5      
$frame stddag6      stddag7      stddag8      stddag9      stddag10     
$frame stddag11     stddag12     stddag13     

//
$frame stdstf1      stdstf2      stdstf3      stdstf4      stdstf5      
$frame stdstf6      stdstf7      stdstf8      stdstf9      stdstf10     
$frame stdstf11     stdstf12     stdstf13     

//
$frame stdxbw1      stdxbw2      stdxbw3      stdxbw4      stdxbw5      
$frame stdxbw6      stdxbw7      stdxbw8      stdxbw9      stdxbw10     
$frame stdxbw11     stdxbw12     stdxbw13     

/*--------------------------
ACTUAL (UNIQUE TO CLASS) PLAYER CODE
----------------------------*/
void() player_assassin_run;
void() player_assassin_crouch_stand;
void() player_assassin_crouch_move;
void() player_assassin_stand;
		
float test_array [2] =
{
	1, 2
};

void()	player_assassin_jump=[++ test_array[1] .. test_array[2] ]
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

void() player_assassin_swim =
{
	if(self.viewentity==self)
		self.th_weapon();
	if(self.waterlevel<3)
		if (self.velocity_x || self.velocity_y)
			self.think=self.th_run;
		else
			self.think=self.th_stand;
};

void() player_assassin_hands_swim =[++$flydag1..$flydag15]
{
	player_assassin_swim();
};

void() player_assassin_staff_swim =[++$flystf1..$flystf15]
{
	player_assassin_swim();
};

void() player_assassin_xbow_swim =[++$flyxbw1..$flyxbw15]
{
	player_assassin_swim();
};

void()	player_assassin_fly =
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

void() player_assassin_hands_fly =[++$flydag1..$flydag15]
{
	player_assassin_fly();
};

void() player_assassin_staff_fly =[++$flystf1..$flystf15]
{
	player_assassin_fly();
};

void() player_assassin_xbow_fly =[++$flyxbw1..$flyxbw15]
{
	player_assassin_fly();
};

void()	player_assassin_stand =
{
	if(self.viewentity==self)
		self.th_weapon();
	if(self.hull==HULL_CROUCH)
		self.think=player_assassin_crouch_stand;
	else if(self.waterlevel>2)
		self.think=self.th_swim;
	else if(self.movetype==MOVETYPE_FLY)
		self.think=self.th_fly;
	else if (self.velocity_x || self.velocity_y)
		self.think=self.th_run;
};

void() player_assassin_hands_stand =[++$stddag1..$stddag13]
{
	player_assassin_stand();
};

void() player_assassin_staff_stand =[++$stdstf1..$stdstf13]
{
	player_assassin_stand();
};

void() player_assassin_xbow_stand =[++$stdxbw1..$stdxbw13]
{
	player_assassin_stand();
};

void()	player_assassin_run =
{
	if(self.viewentity==self)
		self.th_weapon();
	if(self.hull==HULL_CROUCH)
		self.think=player_assassin_crouch_move;
	else if(self.waterlevel>2)
		self.think=self.th_swim;
	else if(self.movetype==MOVETYPE_FLY)
		self.think=self.th_fly;
	else if (!self.velocity_x && !self.velocity_y)
		self.think=self.th_stand;
};

void() player_assassin_hands_run =[++$rundag1..$rundag12]
{
	player_assassin_run();
};

void() player_assassin_staff_run =[++$runstf1..$runstf12]
{
	player_assassin_run();
};

void() player_assassin_xbow_run =[++$runxbw1..$runxbw12]
{
	player_assassin_run();
};

void()	player_assassin_crouch_stand =
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
		self.think=player_assassin_crouch_move;
	thinktime self : HX_FRAME_TIME;
};

void()	player_assassin_crouch_move =[++$crouch1..$crouch20]
{
	if(self.viewentity==self)
		self.th_weapon();
	if(self.movetype==MOVETYPE_FLY)
		self.think=player_assassin_fly;
	else if(self.hull==HULL_PLAYER)
		self.think=self.th_run;
	else if (!self.velocity_x && !self.velocity_y)
		self.think=player_assassin_crouch_stand;
};

void()	player_assassin_attack=
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

void()	player_assassin_hands_attack=[++$attdag1..$attdag11]
{
	player_assassin_attack();
};

void()	player_assassin_xbow_attack=[++$attxbw1..$attxbw4]
{
	player_assassin_attack();
};

void()	player_assassin_staff_attack=[++$attstf1..$attstf4]
{
	player_assassin_attack();
};

void()	player_assassin_pain=
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

void() player_assassin_hands_pain =[++$paindag1..$paindag7]
{
	if(self.frame==$paindag1)
		PainSound();
	player_assassin_pain();
};

void() player_assassin_staff_pain =[++$painstf1..$painstf7]
{
	if(self.frame==$painstf1)
		PainSound();
	player_assassin_pain();
};

void() player_assassin_xbow_pain =[++$painxbw1..$painxbw7]
{
	if(self.frame==$painxbw1)
		PainSound();
	player_assassin_pain();
};

void()	player_assassin_die1=[++$death1..$death20]
{
	if(cycle_wrapped)
	{
		self.frame=$death20;
		self.think=PlayerDead;
	}
};

void()	player_assassin_die2=[++$death1..$death20]
{
	if(cycle_wrapped)
	{
		self.frame=$death20;
		self.think=PlayerDead;
	}
};

void()	player_assassin_behead =
{
	self.level=$decap1;
	self.dmg=$decap28;
	self.cnt=0;
	player_behead();
};

void Ass_Change_Weapon (void)
{
	if(self.weapon==IT_WEAPON1||self.weapon==IT_WEAPON3)
	{
		self.th_stand=player_assassin_hands_stand;
		if(self.weapon==IT_WEAPON3)
			self.th_missile=grenade_throw;
		else
			self.th_missile=Ass_Pdgr_Fire;
		self.th_run=player_assassin_hands_run;
		self.th_pain=player_assassin_hands_pain;
		self.th_swim=player_assassin_hands_swim;
		self.th_fly=player_assassin_hands_fly;
	}
	else if(self.weapon==IT_WEAPON4)
	{
		self.th_stand=player_assassin_staff_stand;
		self.th_missile=ass_setstaff_fire;
		self.th_run=player_assassin_staff_run;
		self.th_pain=player_assassin_staff_pain;
		self.th_swim=player_assassin_staff_swim;
		self.th_fly=player_assassin_staff_fly;
	}
	else
	{
		self.th_stand=player_assassin_xbow_stand;
		self.th_missile=crossbow_fire;
		self.th_run=player_assassin_xbow_run;
		self.th_pain=player_assassin_xbow_pain;
		self.th_swim=player_assassin_xbow_swim;
		self.th_fly=player_assassin_xbow_fly;
	}
	if(self.hull!=HULL_CROUCH)
		self.think=self.th_stand;
}

/*
void assassin_spurt ()
{
	makevectors(self.angles);
	SpawnPuff (self.origin+'0 0 56'+v_forward*12, '0 0 35',5,self);
	thinktime self : 1;
}

void assassin_hurt(void)
{
	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_STEP;
	self.takedamage=DAMAGE_YES;
	self.flags2(+)FL_ALIVE;
	self.thingtype=THINGTYPE_FLESH;
	self.frame=$painstf4;
	self.colormap=187;
	setmodel (self, "models/assassin.mdl");

	setsize (self, '-16 -16 0', '16 16 200');
	self.hull=HULL_POINT;
	self.health = self.max_health=2000;
	self.mass = 2000;
	self.drawflags(+)MLS_ABSLIGHT;
	self.abslight=0.5;
}
*/

