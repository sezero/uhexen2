$frame idle1	idle2	idle3	idle4	idle5
$frame idle6	idle7	idle8	idle9	idle10
$frame idle11	idle12	idle13	idle14	idle15
$frame idle16	idle17	idle18	idle19	idle20
$frame idle21	idle22	idle23	idle24	idle25
$frame idle26	idle27	idle28	idle29	idle30

$frame speak1	speak2	speak3	speak4	speak5
$frame speak6	speak7	speak8	speak9	speak10
$frame speak11	speak12	speak13	speak14	speak15
$frame speak16	speak17	speak18	speak19	speak20

/*

Talkin' Heads

Heads that can speak to you- triggered by events.  Can
play a series of sound files mixed in with delays.  Designer
needs to be able to script the list of sound files in order
with a specified delay for each.  Also needs to support
multiple triggerable events.  (Trigger Relay Event?)

PROBLEM: Need to be able to have entity fields that are
arrays of 256 floats (max number of sound events) so that
their values can be set by the designer in the map file.

NOTA BENE:  talking heads must not use any of these fields... (in struct)
		float splash_time;	    // When to generate the next splash
		float camera_time;      //
		float weaponframe_cnt;  //
		float attack_cnt;       // Shows which attack animation can be used
		float ring_regen_time;	// When to add the next point of health
		float ring_flight_time;	// When to update ring of flight health
		float ring_water_time;	// When to update ring of waterbreathing health
		float ring_turning_time;// When to update ring of turning health
		float super_damage;		// Player does this much more damage (Like Crusader with Special Ability #2)
		float super_damage_low; // Flag the super damage is low
		float puzzles_cheat;	// Allows player past puzzle triggers
		float camptime;			// Amount of time player has been motionless
		float crouch_time;		// Next time player should run crouch subroutine
		float crouch_stuck;		// If set this means the player has released the crouch key in an area too small to uncrouch in
		float divine_time;		// Amount of time flash happens in divine intervention
		float act_state;		// Anim info
		float raven_cnt;		// Number of raven's this guys has in the world
		float newclass;			// If doing a quick class change
		float fangel_SaveFrame;
		float fangel_Count;
		float shoot_cnt;
		float shoot_time;	//	Time of last shot
		float z_movement;
		float z_duration;
		float drop_time;
		float spell_angle;
		float hydra_FloatTo;
		float hydra_chargeTime;
		float spiderType;			// SPIDER_? types
		float spiderActiveCount;	// Tallies "activity"
		float spiderGoPause;		// Active/pause threshold
		float spiderPauseLength;	// Pause duration in frames
		float spiderPauseCount;		// Tallies paused frames
		float scorpionType;			// SCORPION_? types
		float scorpionRest;			// Resting state counter
		float scorpionWalkCount;	// Counts walking frames
		float golemSlideCounter;
		float golemBeamDelay;
		float golemBeamOff1;
		float golemBeamOff2;
		float impType;				// IMP_? types
		float parts_gone;
		float mummy_state;
		float mummy_state_time;
		float artifact_respawn;		// Should respawn?
		float artifact_ignore_owner_time;
		float artifact_ignore_time;
		float next_path_1;
		float next_path_2;
		float next_path_3;
		float next_path_4;
		float path_id;
		float next_path_5;
		float next_path_6;
		float rt_chance;
		float rider_gallop_mode;
		float rider_last_y_change;
		float rider_y_change;
		float rider_death_speed;
		float rider_path_distance;
		float rider_move_adjustment;
		float waraxe_offset;
		float waraxe_horizontal;
		float waraxe_track_inc;
		float waraxe_track_limit;
		float waraxe_max_speed;
		float waraxe_max_height;
		float wrq_effect_id;
		float wrq_radius;
		float wrq_count;
		float beam_angle_a;
		float beam_angle_b;
		float beam_max_scale;
		float beam_direction;
		float beam_speed;
		float z_modifier;	
		float last_health; // Used by bell entity
		float idealpitch;
		float pitchdowntime;
		float searchtime;	// Amount of time bird has been searching
		float next_action;	// Next time to take action
		float searchtime;	// When search was first started
		float damage_max; // Amount of damage each raven can do before it has to leave
		float fish_speed;
		float fish_leader_count;
		float exploderadius;
		float scream_time;
		float attack_cnt;
		float beginframe;
		float sound_time;
		float shot_cnt;   // Number of shots the force cube has shot
*/

/*
Voice filename and length arrays.
*/
void()talkhead_speak_init;

float voice_slot (float vs)
{
	switch(vs)
	{
		case 1:
			return self.voice1;
			break;
		case 2:
			return self.voice2;
			break;
		case 3:
			return self.voice3;
			break;
		case 4:
			return self.voice4;
			break;
		case 5:
			return self.voice5;
			break;
		case 6:
			return self.voice6;
			break;
		case 7:
			return self.voice7;
			break;
		case 8:
			return self.voice8;
			break;
		case 9:
			return self.voice9;
			break;
		case 10:
			return self.voice10;
			break;
		default:
			dprint("Voice Slot out of limits!\n");
			return 0;
			break;
	}
}

float delay_slot (float ds)
{
	switch(ds)
	{
		case 1:
			return self.delay1;
			break;
		case 2:
			return self.delay2;
			break;
		case 3:
			return self.delay3;
			break;
		case 4:
			return self.delay4;
			break;
		case 5:
			return self.delay5;
			break;
		case 6:
			return self.delay6;
			break;
		case 7:
			return self.delay7;
			break;
		case 8:
			return self.delay8;
			break;
		case 9:
			return self.delay9;
			break;
		case 10:
			return self.delay10;
			break;
		default:
			dprint("Delay Slot out of limits!\n");
			return 0;
			break;
	}
}

string voice_filename (float sf_index) 
{//Can't have an array of strings
	switch(sf_index)
	{
		case 1:
			return "t_heads/hey.wav";			//"Hey!"
			break;
		case 2:
			return "t_heads/you.wav";			//"You!"
			break;
		case 3:
			return "t_heads/overhere.wav";		//"Over here!"
			break;
		case 4:
			return "t_heads/halt1.wav";
			break;
		case 5:
			return "t_heads/chat7b.wav";
			break;
		default:
			return "t_heads/default.wav";
			break;
	}
}

float voice_length [51] =
{//Need to know how long sound is to stop mouth movement and start delay
	0,	//Null first sound
	2.610,	//hey!
	2.659,	//you!
	2.522,	//over here!
	0.530,	//halt!
	2.500,	//DeutcheSprach
	0.523,	//default
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1,	
	1
};

void talkhead_idle () [++$idle1 .. $idle30] 
{
	if(self.frame==$idle1)
		dprint("T_head idle\n");
}

void obj_talkinghead ()
{
float cnter, voicenumber;
string voicestring;
	if(self.health>0)
		self.takedamage=TRUE;
	self.classname="obj_talkinghead";
	self.thingtype=THINGTYPE_FLESH;
	self.solid=SOLID_BBOX;
	self.movetype=MOVETYPE_NONE;//?
	self.use=talkhead_speak_init;
	self.th_die=chunk_death;//death sound?  Scream?
	
	cnter=1;
	while(voice_slot(cnter)>0&&cnter<11)
	{//Precache all the sounds
		dprint("t_head precaching\n");
		voicenumber=voice_slot(cnter);
		voicestring=voice_filename(voicenumber);
		precache_sound(voicestring);
		cnter+=1;
	}
	dprint("T_head done precaching\n");
	
//do they have idle anims?
	self.think=talkhead_idle;
	thinktime self : self.wait;
}

/*QUAKED obj_talkinghead1 (0.3 0.1 0.6) (-12 -12 -32) (12 12 0)
Note: Origin is at top of head where it hangs from ceiling

health = how many hp it has.  <=0 means it can't be shot and broken.  Still can be destroyed by a .killtarget field.

voice1 - voice10: "-1" = loop back at start. -# will go back to the positive value of that number.
delay1 - delay10: "-1" = stop forever, 0 = wait for next trigger, >0 = wait this long before next sound
loop = number of times to loop
NOTE: no next voice (0) and no next delay (0) will make it wait for a trigger event and start all over again from the beginning of the list

EXAMPLE:
voice1 = 1	//play sound 1
delay1 = 1	//wait 1 second
voice2 = 10	//play sound 10
delay2 = 3	//wait 1 second
voice3 = 17	//play sound 17
//!! NOTE: no value (0) for delay3 means to wait until it's triggered again to continue.  If there WERE no .voice[4] value(0), it would just start back at the beginning on the next triggering.
//a -1 value in a delay will make it not be triggerable again after this point

//Next triggering: 
voice4 = 22	//play sound 22
delay4 = 2	//wait 2 seconds
voice5 = 13	//play sound 13
delay5 = 2	//wait 2 seconds
voice6 = -2	//Tells it to go back to voice2
loop = 3		//repeat loop 3 times before moving on
delay6 = 1	//after last loop finished, wait 1 second and continue. A zero value here would make it wait for the next triggering to continue

voice7 = 4	//play sound 4
delay7 = -1	//Stop here and never trigger again

resultant sound sequence:
1
10
17
wait for trigger
22
13
10 (start first loop)
17
wait for trigger
22
13
10 (start second loop)
17
wait for trigger
22
13
10 (start third loop)
17
wait for trigger
22
13
4
stop forever
*/
void obj_talkinghead1 ()
{
	precache_model2("models/t_head1.mdl");
	setmodel(self,"models/t_head1.mdl");
	obj_talkinghead();
}

/*QUAKED obj_talkinghead2 (0.3 0.1 0.6) (-12 -12 -32) (12 12 0)
Note: Origin is at top of head where it hangs from ceiling

health = how many hp it has.  <=0 means it can't be shot and broken.  Still can be destroyed by a .killtarget field.

voice1 - voice10: "-1" = loop back at start. -# will go back to the positive value of that number.
delay1 - delay10: "-1" = stop forever, 0 = wait for next trigger, >0 = wait this long before next sound
loop = number of times to loop
NOTE: no next voice (0) and no next delay (0) will make it wait for a trigger event and start all over again from the beginning of the list

EXAMPLE:
voice1 = 1	//play sound 1
delay1 = 1	//wait 1 second
voice2 = 10	//play sound 10
delay2 = 3	//wait 1 second
voice3 = 17	//play sound 17
//!! NOTE: no value (0) for delay3 means to wait until it's triggered again to continue.  If there WERE no .voice[4] value(0), it would just start back at the beginning on the next triggering.
//a -1 value in a delay will make it not be triggerable again after this point

//Next triggering: 
voice4 = 22	//play sound 22
delay4 = 2	//wait 2 seconds
voice5 = 13	//play sound 13
delay5 = 2	//wait 2 seconds
voice6 = -2	//Tells it to go back to voice2
loop = 3		//repeat loop 3 times before moving on
delay6 = 1	//after last loop finished, wait 1 second and continue. A zero value here would make it wait for the next triggering to continue

voice7 = 4	//play sound 4
delay7 = -1	//Stop here and never trigger again

resultant sound sequence:
1
10
17
wait for trigger
22
13
10 (start first loop)
17
wait for trigger
22
13
10 (start second loop)
17
wait for trigger
22
13
10 (start third loop)
17
wait for trigger
22
13
4
stop forever
*/
void obj_talkinghead2 ()
{
	precache_model2("models/t_head2.mdl");
	setmodel(self,"models/t_head2.mdl");
	obj_talkinghead();
}

/*QUAKED obj_talkinghead3 (0.3 0.1 0.6) (-12 -12 -32) (12 12 0)
Note: Origin is at top of head where it hangs from ceiling

health = how many hp it has.  <=0 means it can't be shot and broken.  Still can be destroyed by a .killtarget field.

voice1 - voice10: "-1" = loop back at start. -# will go back to the positive value of that number.
delay1 - delay10: "-1" = stop forever, 0 = wait for next trigger, >0 = wait this long before next sound
loop = number of times to loop
NOTE: no next voice (0) and no next delay (0) will make it wait for a trigger event and start all over again from the beginning of the list

EXAMPLE:
voice1 = 1	//play sound 1
delay1 = 1	//wait 1 second
voice2 = 10	//play sound 10
delay2 = 3	//wait 1 second
voice3 = 17	//play sound 17
//!! NOTE: no value (0) for delay3 means to wait until it's triggered again to continue.  If there WERE no .voice[4] value(0), it would just start back at the beginning on the next triggering.
//a -1 value in a delay will make it not be triggerable again after this point

//Next triggering: 
voice4 = 22	//play sound 22
delay4 = 2	//wait 2 seconds
voice5 = 13	//play sound 13
delay5 = 2	//wait 2 seconds
voice6 = -2	//Tells it to go back to voice2
loop = 3		//repeat loop 3 times before moving on
delay6 = 1	//after last loop finished, wait 1 second and continue. A zero value here would make it wait for the next triggering to continue

voice7 = 4	//play sound 4
delay7 = -1	//Stop here and never trigger again

resultant sound sequence:
1
10
17
wait for trigger
22
13
10 (start first loop)
17
wait for trigger
22
13
10 (start second loop)
17
wait for trigger
22
13
10 (start third loop)
17
wait for trigger
22
13
4
stop forever
*/
void obj_talkinghead3 ()
{
	precache_model2("models/t_head3.mdl");
	setmodel(self,"models/t_head3.mdl");
	obj_talkinghead();
}

void talkhead_speaking () [++ $speak1 .. $speak20]
{
	if(time>=self.wait)
		if(delay_slot(self.cnt)>0)
		{
			self.think=talkhead_speak_init;
			thinktime self : delay_slot(self.cnt);
		}
		else
		{//0 will make it wait to be triggered again- you can continue a list of words afterwards
			if(delay_slot(self.cnt)==-1)
				self.use=SUB_Null;//stop forever
			else if(voice_slot(self.cnt+1)==0)//No next voice!
				self.cnt=0;//Start over from beginning

			if(delay_slot(self.cnt)==0)
			{//Done witrh this trigger, go back to idle anim
				self.think=talkhead_idle;
				thinktime self : 0;
			}
			else
			{//Just pausing between words
				self.frame=$idle1;//Root frame
				self.think=SUB_Null;
				self.nextthink=-1;
			}
		}
}

void talkhead_speak_init ()
{//FIX ME: WHAT IF TALKING AND USED AGAIN- INTERRUPT?
	dprint("Being used!\n");
	self.cnt+=1;
	if(voice_slot(self.cnt)<0)//A loop value
		if(self.loop>0)//still have loops left to do
		{
			self.cnt=voice_slot(self.cnt) * -1;//go to loop voice value
			self.loop-=1;//subtract one loop
		}
		else
		{
			if(voice_slot(self.cnt+1)!=0)
				self.cnt+=1;//Go onto next sequence, finished all loops
			else
			{//No next voice, so wait to trigger again
				if(delay_slot(self.cnt)==-1)//Next delay is -1
					self.use=SUB_Null;//stop forever
				else
					self.cnt=0;//Start over from the beginning
				self.think=SUB_Null;//or go to idle anim?
				self.nextthink=-1;
				return;
			}
		}
	sound(self,CHAN_VOICE,voice_filename(voice_slot(self.cnt)),1,ATTN_NORM);
	self.think=talkhead_speaking;
	thinktime self : 0;
	self.wait=time+voice_length[voice_slot(self.cnt)];
}
