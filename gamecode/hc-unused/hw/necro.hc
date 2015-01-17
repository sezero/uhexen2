/*
 * $Header: /HexenWorld/HCode/necro.hc 1     2/04/98 1:59p Rjohnson $
 */

/*
==============================================================================

Q:\art\models\players\necroman\final\necro.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\players\necroman\final
$origin 0 0 0
$base BASE skin
$skin skin
$flags 0

//
$frame cwalk1      cwalk2      cwalk3      cwalk4      cwalk5      
$frame cwalk6      cwalk7      cwalk8      cwalk9      cwalk10     
$frame cwalk11     cwalk12     cwalk13     cwalk14     cwalk15     
$frame cwalk16     cwalk17     cwalk18     cwalk19     cwalk20     

//
$frame die1      die2      die3      die4      die5      
$frame die6      die7      die8      die9      die10     
$frame die11     die12     die13     die14     die15     
$frame die16     die17     die18     die19     die20     

//
$frame behead1       behead2       behead3       behead4       behead5       
$frame behead6       behead7       behead8       behead9       behead10      
$frame behead11      behead12      behead13      behead14      behead15      
$frame behead16      behead17      behead18      behead19      behead20      

//
$frame Hatak1       Hatak2       Hatak3       Hatak4       Hatak5       
$frame Hatak6       Hatak7       Hatak8       

//
$frame Hfly1        Hfly2        Hfly3        Hfly4        Hfly5        
$frame Hfly6        Hfly7        Hfly8        Hfly9        Hfly10       
$frame Hfly11       Hfly12       Hfly13       Hfly14       

//
$frame Hpain1       Hpain2       Hpain3       Hpain4       Hpain5       
$frame Hpain6       Hpain7       Hpain8       

//
$frame Hrun1        Hrun2        Hrun3        Hrun4        Hrun5        
$frame Hrun6        Hrun7        Hrun8        Hrun9        Hrun10       
$frame Hrun11       Hrun12       

//
$frame Hwait1       Hwait2       Hwait3       Hwait4       Hwait5       
$frame Hwait6       Hwait7       Hwait8       Hwait9       Hwait10      
$frame Hwait11      Hwait12      

//
$frame Siatak1      Siatak2      Siatak3      Siatak4      Siatak5      
$frame Siatak6      Siatak7      Siatak8      Siatak9      Siatak10     
$frame Siatak11     Siatak12     

//
$frame Sifly1       Sifly2       Sifly3       Sifly4       Sifly5       
$frame Sifly6       Sifly7       Sifly8       Sifly9       Sifly10      
$frame Sifly11      Sifly12      Sifly13      Sifly14      

//
$frame Sipain1      Sipain2      Sipain3      Sipain4      Sipain5      
$frame Sipain6      Sipain7      Sipain8      

//
$frame Sirun1       Sirun2       Sirun3       Sirun4       Sirun5       
$frame Sirun6       Sirun7       Sirun8       Sirun9       Sirun10      
$frame Sirun11      Sirun12      

//
$frame Siwait1      Siwait2      Siwait3      Siwait4      Siwait5      
$frame Siwait6      Siwait7      Siwait8      Siwait9      Siwait10     
$frame Siwait11     Siwait12     

//
$frame Statak1      Statak2      Statak3      Statak4      Statak5      
$frame Statak6      Statak7      Statak8      

//
$frame Stfly1       Stfly2       Stfly3       Stfly4       Stfly5       
$frame Stfly6       Stfly7       Stfly8       Stfly9       Stfly10      
$frame Stfly11      Stfly12      Stfly13      Stfly14      

//
$frame Stpain1      Stpain2      Stpain3      Stpain4      Stpain5      
$frame Stpain6      Stpain7      Stpain8      

//
$frame Strun1       Strun2       Strun3       Strun4       Strun5       
$frame Strun6       Strun7       Strun8       Strun9       Strun10      
$frame Strun11      Strun12      

//
$frame Stwait1      Stwait2      Stwait3      Stwait4      Stwait5      
$frame Stwait6      Stwait7      Stwait8      Stwait9      Stwait10     
$frame Stwait11     Stwait12     
 

/*--------------------------
ACTUAL (UNIQUE TO CLASS) PLAYER CODE
----------------------------*/
void() player_necromancer_run;
void() player_necromancer_crouch_stand;
void() player_necromancer_crouch_move;
void() player_necromancer_stand;
		
void() player_necromancer_swim =
{
	if(self.viewentity==self)
		self.th_weapon();
	if(self.waterlevel<3)
		if (self.velocity_x || self.velocity_y)
			self.think=self.th_run;
		else
			self.think=self.th_stand;
};

void() player_necromancer_sickle_swim =[++$Sifly1..$Sifly14]
{
	player_necromancer_swim();
};

void() player_necromancer_setstaff_swim =[++$Stfly1..$Stfly14]
{
	player_necromancer_swim();
};

void() player_necromancer_spell_swim =[++$Hfly1..$Hfly14]
{
	player_necromancer_swim();
};

void()	player_necromancer_fly =
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

void() player_necromancer_sickle_fly =[++$Sifly1..$Sifly14]
{
	player_necromancer_fly();
};

void() player_necromancer_setstaff_fly =[++$Stfly1..$Stfly14]
{
	player_necromancer_fly();
};

void() player_necromancer_spell_fly =[++$Hfly1..$Hfly14]
{
	player_necromancer_fly();
};

void()	player_necromancer_stand =
{
	if(self.viewentity==self)
		self.th_weapon();
	if(self.hull==HULL_CROUCH)
		self.think=player_necromancer_crouch_stand;
	else if(self.waterlevel>2)
		self.think=self.th_swim;
	else if(self.movetype==MOVETYPE_FLY)
		self.think=self.th_fly;
	else if (self.velocity_x || self.velocity_y)
		self.think=self.th_run;
};

void() player_necromancer_sickle_stand =[++$Siwait1..$Siwait12]
{
	player_necromancer_stand();
};

void() player_necromancer_setstaff_stand =[++$Stwait1..$Stwait12]
{
	player_necromancer_stand();
};

void() player_necromancer_spell_stand =[++$Hwait1..$Hwait12]
{
	player_necromancer_stand();
};

void()	player_necromancer_run =
{
	if(self.viewentity==self)
		self.th_weapon();
	if(self.hull==HULL_CROUCH)
		self.think=player_necromancer_crouch_move;
	else if(self.waterlevel>2)
		self.think=self.th_swim;
	else if(self.movetype==MOVETYPE_FLY)
		self.think=self.th_fly;
	else if (!self.velocity_x && !self.velocity_y)
		self.think=self.th_stand;
};

void() player_necromancer_sickle_run =[++$Sirun1..$Sirun12]
{
	player_necromancer_run();
};

void() player_necromancer_setstaff_run =[++$Strun1..$Strun12]
{
	player_necromancer_run();
};

void() player_necromancer_spell_run =[++$Hrun1..$Hrun12]
{
	player_necromancer_run();
};

void()	player_necromancer_crouch_stand =
{
	if(self.viewentity==self)
		self.th_weapon();
	if(self.frame>$cwalk20 || self.frame<$cwalk1)
		self.frame=$cwalk1;
	if(self.movetype==MOVETYPE_FLY)
		self.think=self.th_fly;
	else if(self.hull==HULL_PLAYER)
		self.think=self.th_stand;
	else if (self.velocity_x || self.velocity_y)
		self.think=player_necromancer_crouch_move;
	thinktime self : HX_FRAME_TIME;
};

void()	player_necromancer_crouch_move =[++$cwalk1..$cwalk20]
{
	if(self.viewentity==self)
		self.th_weapon();
	if(self.movetype==MOVETYPE_FLY)
		self.think=player_necromancer_fly;
	else if(self.hull==HULL_PLAYER)
		self.think=self.th_run;
	else if (!self.velocity_x && !self.velocity_y)
		self.think=player_necromancer_crouch_stand;
};

void()	player_necromancer_attack=
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

void()	player_necromancer_sickle_attack=[++$Siatak1..$Siatak12]
{
	player_necromancer_attack();
};

void()	player_necromancer_spell_attack=[++$Hatak1..$Hatak8]
{
	player_necromancer_attack();
};

void()	player_necromancer_setstaff_attack=[++$Statak1..$Statak8]
{
	player_necromancer_attack();
};

void()	player_necromancer_pain=
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

void() player_necromancer_sickle_pain =[++$Sipain1..$Sipain8]
{
	if(self.frame==$Sipain1)
		PainSound();
	player_necromancer_pain();
};

void() player_necromancer_setstaff_pain =[++$Stpain1..$Stpain8]
{
	if(self.frame==$Stpain1)
		PainSound();
	player_necromancer_pain();
};

void() player_necromancer_spell_pain =[++$Hpain1..$Hpain8]
{
	if(self.frame==$Hpain1)
		PainSound();
	player_necromancer_pain();
};

void()	player_necromancer_die1=[++$die1..$die10]
{
	if(cycle_wrapped)
	{
		self.frame=$die20;
		self.think=PlayerDead;
	}
};

void()	player_necromancer_die2=[++$die1..$die10]
{
	if(cycle_wrapped)
	{
		self.frame=$die20;
		self.think=PlayerDead;
	}
};

void()	player_necromancer_behead =
{
	self.level=$behead1;
	self.dmg=$behead20;
	self.cnt=0;
	player_behead();
};

void Nec_Change_Weapon (void)
{
	if(self.weapon==IT_WEAPON1)
	{
		self.th_stand=player_necromancer_sickle_stand;
		self.th_missile=sickle_decide_attack;
		self.th_run=player_necromancer_sickle_run;
		self.th_pain=player_necromancer_sickle_pain;
		self.th_swim=player_necromancer_sickle_swim;
		self.th_fly=player_necromancer_sickle_fly;
	}
	else if(self.weapon==IT_WEAPON4)
	{
		self.th_stand=player_necromancer_setstaff_stand;
		self.th_missile=setstaff_decide_attack;
		self.th_run=player_necromancer_setstaff_run;
		self.th_pain=player_necromancer_setstaff_pain;
		self.th_swim=player_necromancer_setstaff_swim;
		self.th_fly=player_necromancer_setstaff_fly;
	}
	else
	{
		self.th_stand=player_necromancer_spell_stand;
		if(self.weapon==IT_WEAPON2)
			self.th_missile=Nec_Mis_Attack;
		else
			self.th_missile=Nec_Bon_Attack;
		self.th_run=player_necromancer_spell_run;
		self.th_pain=player_necromancer_spell_pain;
		self.th_swim=player_necromancer_spell_swim;
		self.th_fly=player_necromancer_spell_fly;
	}
	if(self.hull!=HULL_CROUCH)
		self.think=self.th_stand;
}
