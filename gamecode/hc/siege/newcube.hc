float cube_distance = 400;

void spawn_replacement_key (vector spot)
{
entity newkey;
	newkey=spawn();
	newkey.puzzle_id = g_keyname;
	newkey.model = g_keymdl;
	newkey.flags2 = FL2_REPLACEMENT;//don't spawn another tracker
	newkey.spawnflags(+)8;
	newkey.classname="puzzle_piece";
	setorigin(newkey,spot);
	newkey.think = puzzle_piece;
	thinktime newkey : 0;
}

entity key_tracker_vigil ()
{
entity find_key,found, zombie_keyman;
	//first look for a player with it
	if(!g_keyname)
		g_keyname = "cskey";
	find_key=find(world,puzzle_inv1,g_keyname);
	if(find_key!=world)
		if(find_key.netname=="")
		{//dropped client still has it
			dprint("Error!  Person with no name has the key!!!\n");
			zombie_keyman = find_key;
			find_key=world;
		}

	if(find_key==world)
	{//No one is carrying it, look for key itself
		if(!g_keymdl)
			g_keymdl = "models/puzzle/cskey.mdl";
		find_key=find(world,model,g_keymdl);
		if(find_key==world)
		{//key not on map
			if(self.classname!="cube_of_force")
			{
				if(zombie_keyman!=world)
				{//take from zombie, make not solid and spawn new key
					dprint("zombie has only key!  Spawning replacement at: ");
					if(pointcontents(zombie_keyman.origin)!=CONTENT_SOLID)
					{
						dprintv("%s\n",zombie_keyman.origin);
						spawn_replacement_key(zombie_keyman.origin);
					}
					else
					{
						dprintv("%s\n",self.origin);
						spawn_replacement_key(self.origin);
					}
					WriteByte(MSG_ALL,SVC_NONEHASKEY);
					zombie_keyman.puzzle_inv1="";
					zombie_keyman.solid=SOLID_NOT;
		//			zombie_keyman.flags(-)FL_CLIENT;
		//			remove(zombie_keyman);
					find_key=find(world,model,g_keymdl);
					if(!find_key)
						dprint("What the FUCK?  Just made key and can't find it!\n");
				}
				else
				{//no one has it and it's not there- wtf?!
					dprintv("Key was LOST!  Spawning replacement at %s\n",self.origin);
					spawn_replacement_key(self.origin);
					WriteByte(MSG_ALL,SVC_NONEHASKEY);
				}
			}
		}
		else if(pointcontents(find_key.origin)==CONTENT_SOLID)
		{
			if(self.classname!="cube_of_force")
			{
				setorigin(find_key,self.origin);
				WriteByte(MSG_ALL,SVC_NONEHASKEY);
			}
		}
		else if(self.classname!="cube_of_force")//super hacky!!!
		{
			found=find(world,netname,"door");
			while(found)
			{
				if(overlapped(found,find_key))
				{
					dprintv("Key inside a door, moving it to %s\n",self.origin);
					setorigin(find_key,self.origin);
					found=world;
				}
				else
					found=find(found,netname,"door");
			}
		}
	}

	if(self.classname=="cube_of_force")
		return find_key;
	else
	{
		self.think=key_tracker_vigil;
		thinktime self : 1;//Check every second
	}
}

void spawn_key_tracker ()
{
entity keytracker;
	keytracker=spawn();
	setorigin(keytracker,self.origin);
	keytracker.think = key_tracker_vigil;
	thinktime keytracker: 1;
}

void CubeDie(void)
{
	stopSound(self,0);
	self.owner.artifact_flags(-)self.artifact_flags;
	remove(self);
}

//Key Finder
void cube_point_to_key ()
{//fixme - doesn't always work
entity find_key;
vector org,key_spot;
	if(self.lifetime<time)
	{
		dprint("Cube timed out, removing...\n");
		self.th_die();
		return;
	}

	find_key=key_tracker_vigil();

	if(find_key.classname=="player")
		if(find_key==self.owner||find_key.siege_team==self.owner.siege_team)
		{
			sprint(self.owner,PRINT_HIGH,find_key.netname);
			sprint(self.owner,PRINT_HIGH,", your teammate, has the key!\n");
			centerprint(self.owner,"Your teammates have the key!\n");
			self.th_die();
			return;
		}
	
	if(!find_key)
	{
		if(self.t_width<time)
		{
			centerprint(self.owner,"Cube searching for key...\n");
			self.t_width = time + 1;
		}
	}
	else
	{
		if(self.t_width<time)
		{
			if(find_key.classname=="player")
			{//what if he stuck around?  Wasn't removed?
				sprint(self.owner,PRINT_HIGH,find_key.netname);
				sprint(self.owner,PRINT_HIGH," has the key");
			}
			else
				sprint(self.owner,PRINT_HIGH,"Throne Key is this way");

			if(pointcontents(find_key.origin)==CONTENT_WATER)
				sprint(self.owner,PRINT_HIGH," and is under water!\n");
			else
				sprint(self.owner,PRINT_HIGH,"!\n");
			self.t_width = time + 3;
		}
		key_spot=(find_key.absmin+find_key.absmax)*0.5;
		org=self.owner.origin +self.owner.proj_ofs;//Not view- may block view!
		key_spot=normalize(key_spot-org);
		traceline(org,org+key_spot*128,FALSE,self.owner);
		setorigin(self,trace_endpos);
	}

	self.think=cube_point_to_key;
	thinktime self : 0.05;
}

void UseKeyFinder(void)
{
	entity cube;

	if(self.artifact_flags & AFL_CUBE_RIGHT)
		return;//one cube at a time

	cube = spawn();

	cube.owner = self;
	cube.solid = SOLID_NOT;
	cube.movetype = MOVETYPE_NOCLIP;//MOVETYPE_FLY;
	cube.flags (+) FL_FLY | FL_NOTARGET;
	setorigin (cube, cube.owner.origin);
	setmodel (cube, "models/a_cube.mdl");
	setsize (cube, '-5 -5 -5', '5 5 5');		

	cube.classname = "cube_of_force";

	self.artifact_flags (+) AFL_CUBE_RIGHT;
	cube.artifact_flags (+) AFL_CUBE_RIGHT;
	cube.think=cube_point_to_key;
	cube.th_die=CubeDie;

	thinktime cube : 0;
	cube.lifetime = time + 60;

	cube.effects(+)EF_LIGHT;
	cube.drawflags (+) MLS_ABSLIGHT;

	cube.abslight = .3;

	self.cnt_cubeofforce -= 1;
}
