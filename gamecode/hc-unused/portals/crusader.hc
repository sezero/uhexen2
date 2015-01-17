/*
 * $Header: /H3MP/hcode/crusader.hc 13    10/28/97 1:00p Mgummelt $
 */

/*
==============================================================================

Q:\art\models\players\crusader\final\crusader.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\players\crusader\final
$origin 0 0 0
$base BASE skin
$skin skin
$flags 0

//
$frame crouch1      crouch2      crouch3      crouch4      crouch5      
$frame crouch6      crouch7      crouch8      crouch9      crouch10     
$frame crouch11     crouch12     crouch13     crouch14     crouch15     
$frame crouch16     crouch17     crouch18     crouch19     crouch20     

//
$frame decap1       decap2       decap3       decap4       decap5       
$frame decap6       decap7       decap8       decap9       decap10      
$frame decap11      decap12      decap13      decap14      decap15      
$frame decap16      decap17      decap18      decap19      decap20      
$frame decap21      decap22      decap23      decap24      decap25      
$frame decap26      decap27      decap28      

//
$frame die1         die2         die3         die4         die5         
$frame die6         die7         die8         die9         die10        
$frame die11        die12        die13        die14        die15        
$frame die16        die17        die18        die19        die20        

//
$frame FINLPOLY     

//
$frame HmrFly1      HmrFly2      HmrFly3      HmrFly4      HmrFly5      
$frame HmrFly6      HmrFly7      HmrFly8      HmrFly9      HmrFly10     
$frame HmrFly11     HmrFly12     HmrFly13     HmrFly14     HmrFly15     

//
$frame HmrHit1      HmrHit2      HmrHit3      HmrHit4      HmrHit5      
$frame HmrHit6      HmrHit7      HmrHit8      HmrHit9      HmrHit10     

//
$frame hmrPain1     hmrPain2     hmrPain3     hmrPain4     hmrPain5     
$frame hmrPain6     hmrPain7     hmrPain8     

//
$frame HmrStn1      HmrStn2      HmrStn3      HmrStn4      HmrStn5      
$frame HmrStn6      HmrStn7      HmrStn8      HmrStn9      HmrStn10     
$frame HmrStn11     HmrStn12     HmrStn13     

//
$frame HmrRun1       HmrRun2       HmrRun3       HmrRun4       HmrRun5       
$frame HmrRun6       HmrRun7       HmrRun8       HmrRun9       HmrRun10      
$frame HmrRun11      HmrRun12      

//
$frame IceFly1      IceFly2      IceFly3      IceFly4      IceFly5      
$frame IceFly6      IceFly7      IceFly8      IceFly9      IceFly10     
$frame IceFly11     IceFly12     IceFly13     IceFly14     IceFly15     

//
$frame IcePain1     IcePain2     IcePain3     IcePain4     IcePain5     
$frame IcePain6     IcePain7     IcePain8     

//
$frame IceRun1      IceRun2      IceRun3      IceRun4      IceRun5      
$frame IceRun6      IceRun7      IceRun8      IceRun9      IceRun10     
$frame IceRun11     IceRun12     

//
$frame IceShot1     IceShot2     IceShot3     IceShot4     

//
$frame IceStn1      IceStn2      IceStn3      IceStn4      IceStn5      
$frame IceStn6      IceStn7      IceStn8      IceStn9      IceStn10     
$frame IceStn11     IceStn12     IceStn13     

//
$frame jump1        jump2        jump3        jump4        jump5        
$frame jump6        jump7        jump8        jump9        jump10       
$frame jump11       jump12       jump13       

//
$frame SunFly1      SunFly2      SunFly3      SunFly4      SunFly5      
$frame SunFly6      SunFly7      SunFly8      SunFly9      SunFly10     
$frame SunFly11     SunFly12     SunFly13     SunFly14     

//
$frame SunPain1     SunPain2     SunPain3     SunPain4     SunPain5     
$frame SunPain6     SunPain7     SunPain8     

//
$frame SunRun1      SunRun2      SunRun3      SunRun4      SunRun5      
$frame SunRun6      SunRun7      SunRun8      SunRun9      SunRun10     
$frame SunRun11     SunRun12     

//
$frame SunShot1     SunShot2     SunShot3     SunShot4     SunShot5     

//
$frame SunStn1      SunStn2      SunStn3      SunStn4      SunStn5      
$frame SunStn6      SunStn7      SunStn8      SunStn9      SunStn10     
$frame SunStn11     SunStn12     SunStn13     

/*--------------------------
ACTUAL (UNIQUE TO CLASS) PLAYER CODE
----------------------------*/
void() player_crusader_run;
void() player_crusader_crouch_stand;
void() player_crusader_crouch_move;
void() player_crusader_stand;
		
void()	player_crusader_jump=[++$jump1..$jump13]//FIX
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

void() player_crusader_swim =
{
	if(self.viewentity==self)
		self.th_weapon();
	if(self.waterlevel<3)
		if (self.velocity_x || self.velocity_y)
			self.think=self.th_run;
		else
			self.think=self.th_stand;
};

void() player_crusader_hammer_swim =[++$HmrFly1..$HmrFly15]
{
	player_crusader_swim();
};

void() player_crusader_sunstaff_swim =[++$SunFly1..$SunFly14]
{
	player_crusader_swim();
};

void() player_crusader_ice_swim =[++$IceFly1..$IceFly15]
{
	player_crusader_swim();
};

void()	player_crusader_fly =
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

void() player_crusader_hammer_fly =[++$HmrFly1..$HmrFly15]
{
	player_crusader_fly();
};

void() player_crusader_sunstaff_fly =[++$SunFly1..$SunFly14]
{
	player_crusader_fly();
};

void() player_crusader_ice_fly =[++$IceFly1..$IceFly15]
{
	player_crusader_fly();
};

void()	player_crusader_stand =
{
	if(self.viewentity==self)
		self.th_weapon();
	if(self.hull==HULL_CROUCH)
		self.think=player_crusader_crouch_stand;
	else if(self.waterlevel>2)
		self.think=self.th_swim;
	else if(self.movetype==MOVETYPE_FLY)
		self.think=self.th_fly;
	else if (self.velocity_x || self.velocity_y)
		self.think=self.th_run;
};

void() player_crusader_hammer_stand =[++$HmrStn1..$HmrStn13]
{
	player_crusader_stand();
};

void() player_crusader_sunstaff_stand =[++$SunStn1..$SunStn13]
{
	player_crusader_stand();
};

void() player_crusader_ice_stand =[++$IceStn1..$IceStn13]
{
	player_crusader_stand();
};

void()	player_crusader_run =
{
	if(self.viewentity==self)
		self.th_weapon();
	if(self.hull==HULL_CROUCH)
		self.think=player_crusader_crouch_move;
	else if(self.waterlevel>2)
		self.think=self.th_swim;
	else if(self.movetype==MOVETYPE_FLY)
		self.think=self.th_fly;
	else if (!self.velocity_x && !self.velocity_y)
		self.think=self.th_stand;
};

void() player_crusader_hammer_run =[++$HmrRun1..$HmrRun12]
{
	player_crusader_run();
};

void() player_crusader_sunstaff_run =[++$SunRun1..$SunRun12]
{
	player_crusader_run();
};

void() player_crusader_ice_run =[++$IceRun1..$IceRun12]
{
	player_crusader_run();
};

void()	player_crusader_crouch_stand =
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
		self.think=player_crusader_crouch_move;
	self.nextthink=time + HX_FRAME_TIME;
};

void()	player_crusader_crouch_move =[++$crouch1..$crouch20]
{
	if(self.viewentity==self)
		self.th_weapon();
	if(self.movetype==MOVETYPE_FLY)
		self.think=player_crusader_fly;
	else if(self.hull==HULL_PLAYER)
		self.think=self.th_run;
	else if (!self.velocity_x && !self.velocity_y)
		self.think=player_crusader_crouch_stand;
};

void()	player_crusader_attack=
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

void()	player_crusader_hammer_attack=[++$HmrHit1..$HmrHit10]
{
	player_crusader_attack();
};

void()	player_crusader_ice_attack=[++$IceShot1..$IceShot4]
{
	player_crusader_attack();
};

void()	player_crusader_sunstaff_attack=[++$SunShot1..$SunShot5]
{
	player_crusader_attack();
};

void()	player_crusader_pain=
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

void() player_crusader_hammer_pain =[++$HmrPain1..$HmrPain8]
{
	if(self.frame==$HmrPain1)
		PainSound();
	player_crusader_pain();
};

void() player_crusader_sunstaff_pain =[++$SunPain1..$SunPain8]
{
	if(self.frame==$SunPain1)
		PainSound();
	player_crusader_pain();
};

void() player_crusader_ice_pain =[++$IcePain1..$IcePain8]
{
	if(self.frame==$IcePain1)
		PainSound();
	player_crusader_pain();
};

void()	player_crusader_die1=[++$die1..$die20]
{
	if(cycle_wrapped)
	{
		self.frame=$die20;
		self.think=PlayerDead;
	}
};

void()	player_crusader_behead =
{
	self.level=$decap1;
	self.dmg=$decap28;
	self.cnt=0;
	player_behead();
};

void Cru_Change_Weapon (void)
{
	if(self.weapon==IT_WEAPON1)
	{
		self.th_stand=player_crusader_hammer_stand;
		self.th_missile=player_crusader_hammer_attack;
		self.th_run=player_crusader_hammer_run;
		self.th_pain=player_crusader_hammer_pain;
		self.th_swim=player_crusader_hammer_swim;
		self.th_fly=player_crusader_hammer_fly;
	}
	else if(self.weapon==IT_WEAPON2)
	{
		self.th_stand=player_crusader_ice_stand;
		self.th_missile=player_crusader_ice_attack;
		self.th_run=player_crusader_ice_run;
		self.th_pain=player_crusader_ice_pain;
		self.th_swim=player_crusader_ice_swim;
		self.th_fly=player_crusader_ice_fly;
	}
	else
	{
		self.th_stand=player_crusader_sunstaff_stand;
		self.th_missile=player_crusader_sunstaff_attack;
		self.th_run=player_crusader_sunstaff_run;
		self.th_pain=player_crusader_sunstaff_pain;
		self.th_swim=player_crusader_sunstaff_swim;
		self.th_fly=player_crusader_sunstaff_fly;
	}
	if(self.hull!=HULL_CROUCH)
		self.think=self.th_stand;
}
