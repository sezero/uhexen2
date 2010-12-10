/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/ph_eido.hc,v 1.1.1.1 2004-11-29 11:34:20 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\Eidolon\ph_eido.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\Eidolon
$origin 0 0 0
$base BASE SKIN
$skin SKIN
$flags 0

//
$frame death1       death2       death3       death4       death5       
$frame death6       death7       death8       death9       death10      
$frame death11      death12      death13      death14      death15      
$frame death16      death17      death18      death19      death20      
$frame death21      death22      death23      death24      death25      
$frame death26      death27      death28      death29      death30      
$frame death31      death32      death33      death34      death35      
$frame death36      death37      death38      death39      death40      
$frame death41      death42      death43      death44      death45      
$frame death46      death47      death48      death49      death50      
$frame death51      death52      death53      death54      death55      
$frame death56      death57      death58      death59      death60      
$frame death61      death62      death63      death64      death65      
$frame death66      death67      death68      death69      death70      
$frame death71      death72      death73      death74      death75      
$frame death76      death77      death78      death79      death80      
$frame death81      death82      death83      death84      death85      
$frame death86      death87      death88      death89      death90      
$frame death91      death92      death93      death94      death95      
$frame death96      death97      death98      death99      death100
$frame death101     death102     death103     death104     death105

//
$frame howl1        howl2        howl3        howl4        howl5        
$frame howl6        howl7        howl8        howl9        howl10       
$frame howl11       howl12       howl13       howl14       howl15       
$frame howl16       howl17       howl18       howl19       howl20       
$frame howl21       howl22       howl23       howl24       howl25       
$frame howl26       howl27       howl28       howl29       howl30       
$frame howl31       howl32       howl33       howl34       howl35       
$frame howl36       howl37       howl38       howl39       howl40       
$frame howl41       howl42       howl43       howl44       howl45       
$frame howl46       howl47       howl48       howl49       howl50       
$frame howl51       howl52       howl53       howl54       howl55       
$frame howl56       howl57       howl58       howl59       howl60       

//
$frame painA1       painA2       painA3       painA4       painA5       
$frame painA6       painA7       painA8       painA9       

//
$frame wait1        wait2        wait3        wait4        wait5        
$frame wait6        wait7        wait8        wait9        wait10       
$frame wait11       wait12       wait13       wait14       wait15       
$frame wait16       

//==================================================================

void()ph_eidolon_pain;
void ph_eidolon_wait () [++ $wait1 .. $wait16]
{
}

void ph_eidolon_roar () [++ $howl1 .. $howl60]
{
	if(self.frame==$howl60)
	{
		self.th_pain = ph_eidolon_pain;
		self.think=self.th_stand;
		self.use=self.th_die;
	}
	thinktime self : 0.1;
}

void ph_eidolon_ready_roar()
{
	self.th_pain=SUB_Null;
	sound(self,CHAN_VOICE,"eidolon/roar.wav",1,ATTN_NONE);
	self.frame=$howl1;
	self.think=ph_eidolon_roar;
	thinktime self : 0;
}

void ph_eidolon_pain () [++ $painA1 .. $painA9]
{
	if(random()<0.95)
		return;

	self.health = self.max_health=100000000;
	if(self.frame==$painA1)
		sound(self,CHAN_VOICE,"eidolon/pain.wav",1,ATTN_NONE);
	else if(self.frame==$painA9)
		self.think=self.th_stand;
}

void ph_eidolon_die () [++ $death1 .. $death105]
{//FIXME: death frames, thud when hit ground
	if(self.frame>=$death1)
		self.th_pain=SUB_Null;
	if(self.frame==$death85)
		setsize(self,'-24 -24 0','24 24 64');
	if(self.frame==$death8)
		sound(self,CHAN_AUTO,"eidolon/death.wav",1,ATTN_NONE);
	else if(self.frame==$death19
		||self.frame==$death26
		||self.frame==$death40
		||self.frame==$death50
		||self.frame==$death61
		||self.frame==$death78
		||self.frame==$death90
		||self.frame==$death100
		||self.frame==$death105)
		sound(self,CHAN_BODY,"eidolon/stomp.wav",1,ATTN_NONE);
	else if(self.frame==$death95)
		MonsterQuake(500);

	if(self.frame==$death105)
		self.nextthink=-1;
	else
		thinktime self : 0.1;
}

/*QUAKED monster_eidolon_weakling (1 0 0) (-100 -100 0) (100 100 666)
Not quite the big bad ugly boss guy

1st use- howl

2nd use - long dramatic death!
-------------------------FIELDS-------------------------
--------------------------------------------------------

*/
void monster_eidolon_weakling(void)
{
	precache_model4 ("models/ph_eido.mdl");
	precache_sound4 ("eidolon/roar.wav");
	precache_sound4 ("eidolon/death.wav");		//Dies- long and agonizing
	precache_sound4 ("eidolon/stomp.wav");		//Hot-steppin'
	precache_sound4 ("eidolon/pain.wav");		//Hurt

	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_STEP;
	self.takedamage=DAMAGE_YES;
	self.monsterclass=CLASS_FINAL_BOSS;
	self.flags(+)FL_MONSTER;
	self.flags2(+)FL_ALIVE;
	self.thingtype=THINGTYPE_FLESH;

	setmodel (self, "models/ph_eido.mdl");
	self.scale=2.5;
	self.skin = 0;
	setsize (self, '-16 -16 0'*1.3333333*self.scale, '16 16 384'*1.3333333*self.scale);
	self.mass=2000*1.34*self.scale;
	self.hull=HULL_POINT;
	self.proj_ofs=self.view_ofs='0 0 200';
	self.frame=$howl1;
	self.use=ph_eidolon_ready_roar;

	self.hull=HULL_GOLEM;
	self.health = self.max_health=100000000;
	self.yaw_speed = 10;

	self.th_stand = ph_eidolon_wait;
	self.th_die = ph_eidolon_die;
	self.th_pain = ph_eidolon_pain;

	self.drawflags (+) SCALE_ORIGIN_BOTTOM;
	self.touch=SUB_Null;
}
