/*===========================
Level listings for DM- seperate .hc file to make it easy to find.
Change the order to whatever you want and recompile the progs.dat.

MG
=============================*/
void FindDMLevel(void)
{
//FIXME: DO WE EVEN REACH HERE?? IF WE REALLY DO, THEN
//	 WE SHOULD USE MAP ROTATION LISTS HERE! -- O.S.

	serverflags (+) SFL_NEW_UNIT;

	nextmap = string_null;

	if(dmMode == DM_SIEGE)
	{//FIXME: Check a text file for the next map, else use Siege11?
		nextmap = "siege";
	}
	else
	if (cvar("registered") != 0 || cvar("oem") != 0)
	{//registered
		if (mapname == "demo1")
			nextmap = "demo2";
		else if (mapname == "demo2")
			nextmap = "demo3";
		else if (mapname == "demo3")
			nextmap = "village1";
		else if (mapname == "village1")
			nextmap = "village2";
		else if (mapname == "village2")
			nextmap = "village3";
		else if (mapname == "village3")
			nextmap = "village4";
		else if (mapname == "village4")
			nextmap = "village5";
		else if (mapname == "village5")
			nextmap = "rider1a";
		else if (mapname == "rider1a")
			nextmap = "demo1";

		else if (mapname == "meso1")
			nextmap = "meso2";
		else if (mapname == "meso2")
			nextmap = "meso3";
		else if (mapname == "meso3")
			nextmap = "meso4";
		else if (mapname == "meso4")
			nextmap = "meso5";
		else if (mapname == "meso5")
			nextmap = "meso6";
		else if (mapname == "meso6")
			nextmap = "meso8";
		else if (mapname == "meso8")
			nextmap = "meso9";
		else if (mapname == "meso9")
			nextmap = "meso1";

		else if (mapname == "egypt1")
			nextmap = "egypt2";
		else if (mapname == "egypt2")
			nextmap = "egypt3";
		else if (mapname == "egypt3")
			nextmap = "egypt4";
		else if (mapname == "egypt4")
			nextmap = "egypt5";
		else if (mapname == "egypt5")
			nextmap = "egypt6";
		else if (mapname == "egypt6")
			nextmap = "egypt7";
		else if (mapname == "egypt7")
			nextmap = "rider2c";
		else if (mapname == "rider2c")
			nextmap = "egypt1";

		else if (mapname == "romeric1")
			nextmap = "romeric2";
		else if (mapname == "romeric2")
			nextmap = "romeric3";
		else if (mapname == "romeric3")
			nextmap = "romeric4";
		else if (mapname == "romeric4")
			nextmap = "romeric5";
		else if (mapname == "romeric5")
			nextmap = "romeric6";
		else if (mapname == "romeric6")
			nextmap = "romeric7";
		else if (mapname == "romeric7")
			nextmap = "romeric1";

		else if (mapname == "cath")
			nextmap = "tower";
		else if (mapname == "tower")
			nextmap = "castle4";
		else if (mapname == "castle4")
			nextmap = "castle5";
		else if (mapname == "castle5")
			nextmap = "eidolon";
		else if (mapname == "eidolon")
			nextmap = "cath";

		else if (mapname == "ravdm1")
			nextmap = "ravdm2";
		else if (mapname == "ravdm2")
			nextmap = "ravdm3";
		else if (mapname == "ravdm3")
			nextmap = "ravdm4";
		else if (mapname == "ravdm4")
			nextmap = "ravdm5";
		else if (mapname == "ravdm5")
			nextmap = "hwdm1";
		else if (mapname == "hwdm1")
			nextmap = "hwdm2";
		else if (mapname == "hwdm2")
			nextmap = "hwdm3";
		else if (mapname == "hwdm3")
			nextmap = "hwdm4";
		else if (mapname == "hwdm4")
			nextmap = "hwdm5";
		else if (mapname == "hwdm5")
			nextmap = "ravdm1";
	}
	else
	{//demo
		if (mapname == "demo1")
			nextmap = "demo2";
		else if (mapname == "demo2")
			nextmap = "ravdm1";
		else if (mapname == "ravdm1")
			nextmap = "demo1";
	}
}
