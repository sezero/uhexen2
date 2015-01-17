void MakeOpaque (void)
{
	entity checking;

	checking=world;
	checking=nextent(checking);
	while(checking!=world)
	{
		if(checking.drawflags&DRF_TRANSLUCENT&&(
			checking.classname=="door"||
			checking.classname=="door_rotating"||
			checking.classname=="secret door"||
			checking.classname=="pushable brush"))
			checking.drawflags(-)DRF_TRANSLUCENT;
		checking=nextent(checking);
	}
	remove(self);
}

void FindDoorsAndStuff (void)
{
	entity checking;

	checking=world;
	checking=nextent(checking);
	while(checking!=world)
	{
		if(!checking.drawflags&DRF_TRANSLUCENT&&(
			checking.classname=="door"||
			checking.classname=="door_rotating"||
			checking.classname=="secret door"||
			checking.classname=="pushable brush"))
			checking.drawflags(+)DRF_TRANSLUCENT;
		checking=nextent(checking);
	}
	self.think=MakeOpaque;
	thinktime self : 15;
}

void Supes (void)
{
	newmis=spawn();
	newmis.owner=self;
	newmis.solid=SOLID_NOT;
	newmis.movetype=MOVETYPE_FLYMISSILE;
	newmis.think=FindDoorsAndStuff;
	thinktime newmis : 0;
	setorigin(newmis,self.origin+self.proj_ofs);
}

