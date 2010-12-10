/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/siege/precache.hc,v 1.3 2007-02-07 17:01:21 sezero Exp $
 */

// called by worldspawn
void() W_Precache =
{
	precache_sound ("raven/kiltorch.wav");	// player torch dying
	precache_sound ("raven/littorch.wav");	// player torch being lit
	precache_sound ("raven/fire1.wav");    // player torch burning

	precache_sound ("rj/steve.wav");

	precache_sound ("weapons/ric1.wav");	// ricochet (used in c code)
	precache_sound ("weapons/ric2.wav");	// ricochet (used in c code)
	precache_sound ("weapons/ric3.wav");	// ricochet (used in c code)
	precache_sound ("weapons/tink1.wav");	// ricochet (used in c code)
	precache_sound ("weapons/r_exp3.wav");	//Still being used?
//	precache_sound ("items/inv2.wav");		//Quake sounds!

	precache_model ("gfx/puff.spr");
	precache_model ("models/ball.mdl");
	precache_model ("models/star.mdl");
};


//
// these are all of the lumps from the cached.ls files
//
void Precache_lmp (void)
{
	precache_file ("gfx/palette.lmp");
	precache_file ("gfx/colormap.lmp");
	precache_file ("gfx/player.lmp");
	precache_file ("gfx/tinttab.lmp");
	precache_file ("gfx/tinttab2.lmp");
	precache_file ("gfx/invpal.lmp");
	precache_file ("gfx/skin100.lmp");
	precache_file ("gfx/skin101.lmp");

	precache_file ("strings.txt");
	precache_file ("puzzles.txt");
	precache_file ("maplist.txt");

	precache_file ("gfx/menu/fontsize.lmp");
	precache_file ("gfx/menu/backtile.lmp");         
	precache_file ("gfx/menu/bigfont.lmp");        
	precache_file ("gfx/menu/bigfont2.lmp");        
	precache_file ("gfx/menu/conback.lmp");        
	precache_file ("gfx/menu/fontsize.lmp");           
	precache_file ("gfx/menu/help01.lmp");        
	precache_file ("gfx/menu/help02.lmp");        
	precache_file ("gfx/menu/help03.lmp");        
	precache_file ("gfx/menu/help04.lmp");        
	precache_file ("gfx/menu/help05.lmp");        
	precache_file ("gfx/menu/hplaque.lmp");         
	precache_file ("gfx/menu/load.lmp");         
	precache_file ("gfx/menu/loading.lmp");        
	precache_file ("gfx/menu/menudot1.lmp");         
	precache_file ("gfx/menu/menudot2.lmp");         
	precache_file ("gfx/menu/menudot3.lmp");         
	precache_file ("gfx/menu/menudot4.lmp");         
	precache_file ("gfx/menu/menudot5.lmp");         
	precache_file ("gfx/menu/menudot6.lmp");         
	precache_file ("gfx/menu/menudot7.lmp");         
	precache_file ("gfx/menu/menudot8.lmp");         
	precache_file ("gfx/menu/paused.lmp");        
	precache_file ("gfx/menu/save.lmp");         
	precache_file ("gfx/menu/skull0.lmp");           
	precache_file ("gfx/menu/skull1.lmp");           
	precache_file ("gfx/menu/skull10.lmp");           
	precache_file ("gfx/menu/skull11.lmp");           
	precache_file ("gfx/menu/skull12.lmp");          
	precache_file ("gfx/menu/skull13.lmp");           
	precache_file ("gfx/menu/skull14.lmp");           
	precache_file ("gfx/menu/skull15.lmp");           
	precache_file ("gfx/menu/skull16.lmp");           
	precache_file ("gfx/menu/skull17.lmp");           
	precache_file ("gfx/menu/skull2.lmp");           
	precache_file ("gfx/menu/skull3.lmp");           
	precache_file ("gfx/menu/skull4.lmp");           
	precache_file ("gfx/menu/skull5.lmp");           
	precache_file ("gfx/menu/skull6.lmp");           
	precache_file ("gfx/menu/skull7.lmp");           
	precache_file ("gfx/menu/skull8.lmp");           
	precache_file ("gfx/menu/skull9.lmp");           
	precache_file ("gfx/menu/title0.lmp");         
	precache_file ("gfx/menu/title1.lmp");         
	precache_file ("gfx/menu/title2.lmp");         
	precache_file ("gfx/menu/title3.lmp");         
	precache_file ("gfx/menu/title4.lmp");        
	precache_file ("gfx/menu/title5.lmp");        
	precache_file ("gfx/menu/title6.lmp");        
	precache_file ("gfx/menu/title7.lmp");
	precache_file ("gfx/menu/title8.lmp");
	
	precache_file ("gfx/box_tl.lmp");
	precache_file ("gfx/box_tm.lmp");
	precache_file ("gfx/box_tr.lmp");
	precache_file ("gfx/box_ml.lmp");
	precache_file ("gfx/box_mm.lmp");
	precache_file ("gfx/box_mm2.lmp");
	precache_file ("gfx/box_mr.lmp");
	precache_file ("gfx/box_bl.lmp");
	precache_file ("gfx/box_bm.lmp");
	precache_file ("gfx/box_br.lmp");

	precache_file ("gfx/cport1.lmp");
	precache_file ("gfx/cport2.lmp");
	precache_file ("gfx/cport3.lmp");
	precache_file ("gfx/cport4.lmp");

	precache_file ("gfx/menu/netp1.lmp");
	precache_file ("gfx/menu/netp2.lmp");
	precache_file ("gfx/menu/netp3.lmp");
	precache_file ("gfx/menu/netp4.lmp");
	precache_file ("gfx/menu/frame.lmp");

	// Interface graphics
	precache_file ("gfx/topbar1.lmp");
	precache_file ("gfx/topbar2.lmp");
	precache_file ("gfx/topbumpl.lmp");
	precache_file ("gfx/topbumpm.lmp");
	precache_file ("gfx/topbumpr.lmp");
	precache_file ("gfx/bmana.lmp");
	precache_file ("gfx/bmanacov.lmp");
	precache_file ("gfx/gmana.lmp");
	precache_file ("gfx/gmanacov.lmp");
	precache_file ("gfx/hpchain.lmp");
	precache_file ("gfx/hpgem.lmp");
	precache_file ("gfx/chnlcov.lmp");
	precache_file ("gfx/chnrcov.lmp");
	precache_file ("gfx/bmmana.lmp");
	precache_file ("gfx/gmmana.lmp");
	precache_file ("gfx/btmbar1.lmp");
	precache_file ("gfx/btmbar2.lmp");
	precache_file ("gfx/armor1.lmp");
	precache_file ("gfx/armor2.lmp");
	precache_file ("gfx/armor3.lmp");
	precache_file ("gfx/armor4.lmp");
	precache_file ("gfx/ring_f.lmp");
	precache_file ("gfx/ring_w.lmp");
	precache_file ("gfx/ring_t.lmp");
	precache_file ("gfx/ring_r.lmp");
	precache_file ("gfx/artisel.lmp");
	precache_file ("gfx/artinum0.lmp");
	precache_file ("gfx/artinum1.lmp");
	precache_file ("gfx/artinum2.lmp");
	precache_file ("gfx/artinum3.lmp");
	precache_file ("gfx/artinum4.lmp");
	precache_file ("gfx/artinum5.lmp");
	precache_file ("gfx/artinum6.lmp");
	precache_file ("gfx/artinum7.lmp");
	precache_file ("gfx/artinum8.lmp");
	precache_file ("gfx/artinum9.lmp");
	precache_file ("gfx/rngfly1.lmp");
	precache_file ("gfx/rngfly2.lmp");
	precache_file ("gfx/rngfly3.lmp");
	precache_file ("gfx/rngfly4.lmp");
	precache_file ("gfx/rngfly5.lmp");
	precache_file ("gfx/rngfly6.lmp");
	precache_file ("gfx/rngfly7.lmp");
	precache_file ("gfx/rngfly8.lmp");
	precache_file ("gfx/rngfly9.lmp");
	precache_file ("gfx/rngfly10.lmp");
	precache_file ("gfx/rngfly11.lmp");
	precache_file ("gfx/rngfly12.lmp");
	precache_file ("gfx/rngfly13.lmp");
	precache_file ("gfx/rngfly14.lmp");
	precache_file ("gfx/rngfly15.lmp");
	precache_file ("gfx/rngfly16.lmp");
	precache_file ("gfx/rngwtr1.lmp");
	precache_file ("gfx/rngwtr2.lmp");
	precache_file ("gfx/rngwtr3.lmp");
	precache_file ("gfx/rngwtr4.lmp");
	precache_file ("gfx/rngwtr5.lmp");
	precache_file ("gfx/rngwtr6.lmp");
	precache_file ("gfx/rngwtr7.lmp");
	precache_file ("gfx/rngwtr8.lmp");
	precache_file ("gfx/rngwtr9.lmp");
	precache_file ("gfx/rngwtr10.lmp");
	precache_file ("gfx/rngwtr11.lmp");
	precache_file ("gfx/rngwtr12.lmp");
	precache_file ("gfx/rngwtr13.lmp");
	precache_file ("gfx/rngwtr14.lmp");
	precache_file ("gfx/rngwtr15.lmp");
	precache_file ("gfx/rngwtr16.lmp");
	precache_file2 ("gfx/rngtrn1.lmp");
	precache_file2 ("gfx/rngtrn2.lmp");
	precache_file2 ("gfx/rngtrn3.lmp");
	precache_file2 ("gfx/rngtrn4.lmp");
	precache_file2 ("gfx/rngtrn5.lmp");
	precache_file2 ("gfx/rngtrn6.lmp");
	precache_file2 ("gfx/rngtrn7.lmp");
	precache_file2 ("gfx/rngtrn8.lmp");
	precache_file2 ("gfx/rngtrn9.lmp");
	precache_file2 ("gfx/rngtrn10.lmp");
	precache_file2 ("gfx/rngtrn11.lmp");
	precache_file2 ("gfx/rngtrn12.lmp");
	precache_file2 ("gfx/rngtrn13.lmp");
	precache_file2 ("gfx/rngtrn14.lmp");
	precache_file2 ("gfx/rngtrn15.lmp");
	precache_file2 ("gfx/rngtrn16.lmp");
/*	precache_file2 ("gfx/rngreg1.lmp");
	precache_file2 ("gfx/rngreg2.lmp");
	precache_file2 ("gfx/rngreg3.lmp");
	precache_file2 ("gfx/rngreg4.lmp");
	precache_file2 ("gfx/rngreg5.lmp");
	precache_file2 ("gfx/rngreg6.lmp");
	precache_file2 ("gfx/rngreg7.lmp");
	precache_file2 ("gfx/rngreg8.lmp");
	precache_file2 ("gfx/rngreg9.lmp");
	precache_file2 ("gfx/rngreg10.lmp");
	precache_file2 ("gfx/rngreg11.lmp");
	precache_file2 ("gfx/rngreg12.lmp");
	precache_file2 ("gfx/rngreg13.lmp");
	precache_file2 ("gfx/rngreg14.lmp");
	precache_file2 ("gfx/rngreg15.lmp");
	precache_file2 ("gfx/rngreg16.lmp");*/
	precache_file ("gfx/pwrbook1.lmp");
	precache_file ("gfx/pwrbook2.lmp");
	precache_file ("gfx/pwrbook3.lmp");
	precache_file ("gfx/pwrbook4.lmp");
	precache_file ("gfx/pwrbook5.lmp");
	precache_file ("gfx/pwrbook6.lmp");
	precache_file ("gfx/pwrbook7.lmp");
	precache_file ("gfx/pwrbook8.lmp");
	precache_file ("gfx/pwrbook9.lmp");
	precache_file ("gfx/pwrbook10.lmp");
	precache_file ("gfx/pwrbook11.lmp");
	precache_file ("gfx/pwrbook12.lmp");
	precache_file ("gfx/pwrbook13.lmp");
	precache_file ("gfx/pwrbook14.lmp");
	precache_file ("gfx/pwrbook15.lmp");
	precache_file ("gfx/pwrbook16.lmp");
	precache_file ("gfx/durhst1.lmp");
	precache_file ("gfx/durhst2.lmp");
	precache_file ("gfx/durhst3.lmp");
	precache_file ("gfx/durhst4.lmp");
	precache_file ("gfx/durhst5.lmp");
	precache_file ("gfx/durhst6.lmp");
	precache_file ("gfx/durhst7.lmp");
	precache_file ("gfx/durhst8.lmp");
	precache_file ("gfx/durhst9.lmp");
	precache_file ("gfx/durhst10.lmp");
	precache_file ("gfx/durhst11.lmp");
	precache_file ("gfx/durhst12.lmp");
	precache_file ("gfx/durhst13.lmp");
	precache_file ("gfx/durhst14.lmp");
	precache_file ("gfx/durhst15.lmp");
	precache_file ("gfx/durhst16.lmp");
	precache_file ("gfx/durshd1.lmp");
	precache_file ("gfx/durshd2.lmp");
	precache_file ("gfx/durshd3.lmp");
	precache_file ("gfx/durshd4.lmp");
	precache_file ("gfx/durshd5.lmp");
	precache_file ("gfx/durshd6.lmp");
	precache_file ("gfx/durshd7.lmp");
	precache_file ("gfx/durshd8.lmp");
	precache_file ("gfx/durshd9.lmp");
	precache_file ("gfx/durshd10.lmp");
	precache_file ("gfx/durshd11.lmp");
	precache_file ("gfx/durshd12.lmp");
	precache_file ("gfx/durshd13.lmp");
	precache_file ("gfx/durshd14.lmp");
	precache_file ("gfx/durshd15.lmp");
	precache_file ("gfx/durshd16.lmp");
	precache_file ("gfx/arti00.lmp");
	precache_file ("gfx/arti01.lmp");
	precache_file ("gfx/arti02.lmp");
	precache_file ("gfx/arti03.lmp");
	precache_file ("gfx/arti04.lmp");
	precache_file ("gfx/arti05.lmp");
	precache_file ("gfx/arti06.lmp");
	precache_file ("gfx/arti07.lmp");
	precache_file ("gfx/arti08.lmp");
	precache_file ("gfx/arti09.lmp");
	precache_file ("gfx/arti10.lmp");
	precache_file ("gfx/arti11.lmp");
	precache_file ("gfx/arti12.lmp");
	precache_file ("gfx/arti13.lmp");
	precache_file ("gfx/arti14.lmp");

	precache_file ("gfx/ringhlth.lmp");
	precache_file ("gfx/rhlthcvr.lmp");

	precache_file2 ("gfx/rhlthcv2.lmp");

	precache_file ("gfx/castle.lmp");
	precache_file2 ("gfx/meso.lmp");
	precache_file2 ("gfx/egypt.lmp");
	precache_file2 ("gfx/roman.lmp");

	precache_file2 ("gfx/end-1.lmp");
	precache_file2 ("gfx/end-2.lmp");
	precache_file2 ("gfx/end-3.lmp");

	precache_file ("gfx/defwin.lmp");
	precache_file ("gfx/attwin.lmp");
	precache_file ("gfx/attwin2.lmp");
/*
	precache_file ("");
	precache_file ("");
	precache_file ("");
	precache_file ("");
	precache_file ("");
	precache_file ("");
	precache_file ("");
*/

	// Puzzle Pieces
	precache_file2("models/puzzle/staff.mdl");
	precache_file2("models/puzzle/e1.mdl");
	precache_file2("models/puzzle/e2.mdl");
	precache_file2("models/puzzle/e3.mdl");
	precache_file2("models/puzzle/e4.mdl");
	precache_file2("models/puzzle/e5.mdl");
	precache_file2("models/puzzle/e6.mdl");
	precache_file2("models/puzzle/m1.mdl");
	precache_file2("models/puzzle/m2.mdl");
	precache_file2("models/puzzle/m3.mdl");
	precache_file2("models/puzzle/m4.mdl");
	precache_file2("models/puzzle/m5.mdl");
	precache_file2("models/puzzle/s1.mdl");
	precache_file2("models/puzzle/s2.mdl");
	precache_file2("models/puzzle/scepter.mdl");
	precache_file("models/puzzle/keep1.mdl");
	precache_file("models/puzzle/keep2.mdl");
	precache_file("models/puzzle/keep3.mdl");
	precache_file2("models/puzzle/ra1.mdl");
	precache_file2("models/puzzle/r1.mdl");
	precache_file2("models/puzzle/r2.mdl");
	precache_file2("models/puzzle/r3.mdl");
	precache_file2("models/puzzle/r4.mdl");
	precache_file2("models/puzzle/r5.mdl");
	precache_file2("models/puzzle/r6.mdl");
	precache_file2("models/puzzle/r7.mdl");
	precache_file2("models/puzzle/r8.mdl");
	precache_file3("models/puzzle/trkey.mdl");
	precache_file("models/puzzle/cskey.mdl");
	precache_file3("models/puzzle/amult.mdl");
	precache_file3("models/puzzle/glass.mdl");
	precache_file3("models/puzzle/lens.mdl");
	precache_file3("models/puzzle/shovl.mdl");
	precache_file3("models/puzzle/scrol.mdl");
	precache_file2("models/puzzle/e4.mdl");
	precache_file2("models/puzzle/e5.mdl");
	precache_file2("models/puzzle/e6.mdl");
	precache_file3("models/puzzle/sand.mdl");
	precache_file2("models/puzzle/air.mdl");
	precache_file2("models/puzzle/earth.mdl");
	precache_file2("models/puzzle/water.mdl");
	precache_file2("models/puzzle/fire.mdl");
	precache_file2("models/puzzle/scept.mdl");
	precache_file2("models/puzzle/eyeh.mdl");
	precache_file2("models/puzzle/clueb.mdl");
	precache_file2("models/puzzle/lcrwn.mdl");
	precache_file2("models/puzzle/ucrwn.mdl");
	precache_file("models/puzzle/mithl.mdl");
	precache_file3("models/puzzle/stkey.mdl");
	precache_file3("models/puzzle/takey.mdl");
	precache_file2("models/puzzle/ankey.mdl");
	precache_file2("models/puzzle/stime.mdl");
	precache_file2("models/puzzle/speye.mdl");
	precache_file2("models/puzzle/rcjem.mdl");
	precache_file2("models/puzzle/music.mdl");
	precache_file2("models/puzzle/silver.mdl");
	precache_file2("models/puzzle/cross.mdl");
	precache_file2("models/puzzle/holycrss.mdl");
	precache_file2("models/puzzle/gsphere.mdl");
	precache_file2("models/puzzle/prybar.mdl");
	precache_file2("models/puzzle/soul.mdl");
	precache_file2("models/puzzle/mage.mdl");
	precache_file2("models/puzzle/rakey.mdl");
	precache_file2("models/puzzle/h-book.mdl");


	precache_file2("gfx/puzzle/staff.lmp");
	precache_file2("gfx/puzzle/e1.lmp");
	precache_file2("gfx/puzzle/e2.lmp");
	precache_file2("gfx/puzzle/e3.lmp");
	precache_file2("gfx/puzzle/e4.lmp");
	precache_file2("gfx/puzzle/e5.lmp");
	precache_file2("gfx/puzzle/e6.lmp");
	precache_file2("gfx/puzzle/m1.lmp");
	precache_file2("gfx/puzzle/m2.lmp");
	precache_file2("gfx/puzzle/m3.lmp");
	precache_file2("gfx/puzzle/m4.lmp");
	precache_file2("gfx/puzzle/m5.lmp");
	precache_file2("gfx/puzzle/s1.lmp");
	precache_file2("gfx/puzzle/s2.lmp");
	precache_file2("gfx/puzzle/scepter.lmp");
	precache_file("gfx/puzzle/keep1.lmp");
	precache_file("gfx/puzzle/keep2.lmp");
	precache_file("gfx/puzzle/keep3.lmp");
	precache_file2("gfx/puzzle/ra1.lmp");
	precache_file2("gfx/puzzle/r1.lmp");
	precache_file2("gfx/puzzle/r2.lmp");
	precache_file2("gfx/puzzle/r3.lmp");
	precache_file2("gfx/puzzle/r4.lmp");
	precache_file2("gfx/puzzle/r5.lmp");
	precache_file2("gfx/puzzle/r6.lmp");
	precache_file2("gfx/puzzle/r7.lmp");
	precache_file2("gfx/puzzle/r8.lmp");
	precache_file3("gfx/puzzle/trkey.lmp");
	precache_file("gfx/puzzle/cskey.lmp");
	precache_file3("gfx/puzzle/amult.lmp");
	precache_file3("gfx/puzzle/glass.lmp");
	precache_file3("gfx/puzzle/lens.lmp");
	precache_file3("gfx/puzzle/shovl.lmp");
	precache_file3("gfx/puzzle/scrol.lmp");
	precache_file2("gfx/puzzle/e4.lmp");
	precache_file2("gfx/puzzle/e5.lmp");
	precache_file2("gfx/puzzle/e6.lmp");
	precache_file3("gfx/puzzle/sand.lmp");
	precache_file2("gfx/puzzle/air.lmp");
	precache_file2("gfx/puzzle/earth.lmp");
	precache_file2("gfx/puzzle/water.lmp");
	precache_file2("gfx/puzzle/fire.lmp");
	precache_file2("gfx/puzzle/scept.lmp");
	precache_file2("gfx/puzzle/eyeh.lmp");
	precache_file2("gfx/puzzle/clueb.lmp");
	precache_file2("gfx/puzzle/lcrwn.lmp");
	precache_file2("gfx/puzzle/ucrwn.lmp");
	precache_file("gfx/puzzle/mithl.lmp");
	precache_file3("gfx/puzzle/stkey.lmp");
	precache_file3("gfx/puzzle/takey.lmp");
	precache_file2("gfx/puzzle/ankey.lmp");
	precache_file2("gfx/puzzle/stime.lmp");
	precache_file2("gfx/puzzle/speye.lmp");
	precache_file2("gfx/puzzle/rcjem.lmp");
	precache_file2("gfx/puzzle/music.lmp");
	precache_file2("gfx/puzzle/silver.lmp");
	precache_file2("gfx/puzzle/cross.lmp");
	precache_file2("gfx/puzzle/holycrss.lmp");
	precache_file2("gfx/puzzle/gsphere.lmp");
	precache_file2("gfx/puzzle/prybar.lmp");
	precache_file2("gfx/puzzle/soul.lmp");
	precache_file2("gfx/puzzle/mage.lmp");
	precache_file2("gfx/puzzle/rakey.lmp");
	precache_file2("gfx/puzzle/h-book.lmp");

	//RIDER_INIT
    precache_file3 ("models/boss/shaft.mdl");
    precache_file3 ("models/boss/circle.mdl");
    precache_file3 ("models/boss/star.mdl");
//	precache_sound3 ("famine/flashdie.wav");

	//EIDOLON
	precache_file2 ("models/boss/smaleido.mdl");
	precache_file2 ("models/boss/bigeido.mdl");
	precache_file2 ("models/eidoball.mdl");
	precache_file2 ("models/eidoflam.spr");
	precache_file2 ("models/glowball.mdl");
	precache_file2 ("models/boss/chaosorb.mdl");
	precache_sound3 ("eidolon/roar.wav");
	precache_sound3 ("eidolon/pain.wav");	//Hurt
//	precache_sound3 ("eidolon/death.wav");	//Dies- long and agonizing
//	precache_sound3 ("eidolon/fakedie.wav");//1st death- fake
//	precache_sound3 ("eidolon/spell.wav");	//Spell attack (tracking globes)
//	precache_sound3 ("eidolon/stomp.wav");	//Hot-steppin'
//	precache_sound3 ("eidolon/fireball.wav");	//Launching Nasty fireballs
//	precache_sound3 ("eidolon/flamstrt.wav");	//
//	precache_sound3 ("eidolon/flambrth.wav");	//
//	precache_sound3 ("eidolon/flamend.wav");	//
	precache_sound3 ("eidolon/growl.wav");		//
//	precache_sound3 ("eidolon/chrgstrt.wav"); 	//Orb starts recharging Eido
//	precache_sound3 ("eidolon/orbhurt.wav");	//Orb gets hit
//	precache_sound3 ("eidolon/orbxpld.wav");	//Orb gets destroyed
//	precache_sound3 ("eidolon/orbpulse.wav");	//Orb pulsating

	//PESTILENCE
	precache_file2 ("models/boss/boar.mdl");
	precache_file2 ("models/boss/pstrider.mdl");
	precache_file2 ("models/boss/hive.mdl");
	precache_file2 ("models/boss/swarm.mdl");
    precache_file2 ("models/boss/shaft.mdl");
	precache_file2 ("models/pestshot.mdl");
	precache_file2 ("sound/pest/snort.wav");
	precache_file2 ("sound/pest/clop1.wav");
	precache_file2 ("sound/pest/clop2.wav");
	precache_file2 ("sound/pest/clop3.wav");
	precache_file2 ("sound/pest/gallop.wav");
	precache_file2 ("sound/pest/sight.wav");
	precache_file2 ("sound/pest/sting1.wav");
	precache_file2 ("sound/pest/sting2.wav");
	precache_file2 ("sound/pest/sting3.wav");
	precache_file2 ("sound/pest/buzz.wav");
	precache_file2 ("sound/pest/hivehit.wav");
	precache_file2 ("sound/pest/xbowfire.wav");
	precache_file2 ("sound/pest/xbowhit.wav");
	precache_file2 ("sound/pest/die.wav");
	precache_file2 ("sound/pest/charge.wav");
	precache_file2 ("sound/pest/laugh.wav");
	precache_file2 ("sound/pest/snort2.wav");

	// Famine Rider
	precache_file3 ("models/boss/famhorse.mdl");
	precache_file3 ("models/boss/famrider.mdl");
	precache_file3 ("models/famshot.mdl");
	precache_file3 ("sound/famine/die.wav");
	precache_file3 ("sound/famine/laugh.wav");
	precache_file3 ("sound/famine/whinny.wav");
	precache_file3 ("sound/famine/pull.wav");
	precache_file3 ("sound/famine/shot.wav");
	precache_file3 ("sound/famine/snort.wav");
	precache_file3 ("sound/famine/clop1.wav");
	precache_file3 ("sound/famine/clop2.wav");
	precache_file3 ("sound/famine/clop3.wav");
	precache_file3 ("sound/misc/null.wav");
	precache_file3 ("sound/raven/blast.wav");
	precache_file3 ("sound/skullwiz/blinkout.wav");
	precache_file3 ("sound/skullwiz/blinkin.wav");

	// War Rider
	precache_file2 ("models/boss/warhorse.mdl");
	precache_file2 ("models/boss/warrider.mdl");
    precache_file2 ("models/boss/waraxe.mdl");
	precache_file2 ("sound/war/fire_big.wav");
	precache_file2 ("sound/war/die.wav");
	precache_file2 ("sound/war/laugh.wav");
	precache_file2 ("sound/war/laugh_sm.wav");
	precache_file2 ("sound/war/fire.wav");
	precache_file2 ("sound/war/whinny.wav");
	precache_file2 ("sound/war/whinbig.wav");
	precache_file2 ("sound/boss/wartrot1.wav");
	precache_file2 ("sound/boss/wartrot2.wav");
	precache_file2 ("sound/boss/wartrot3.wav");

	// Death Rider
	precache_file2 ("models/boss/dthhorse.mdl");
	precache_file2 ("models/boss/dthrider.mdl");
	precache_file2 ("models/famshot.mdl");
    precache_file2 ("models/boss/bone1.mdl");
    precache_file2 ("models/boss/bone2.mdl");
    precache_file2 ("models/boss/bone3.mdl");
    precache_file2 ("models/boss/bone4.mdl");
    precache_file2 ("models/boss/bone5.mdl");
    precache_file2 ("models/boss/bone6.mdl");
	precache_file2 ("models/mumshot.mdl");
	precache_file2 ("models/booberry.mdl");
	precache_file2 ("sound/mummy/mislfire.wav");
	precache_file2 ("sound/eidolon/flamend.wav");
	precache_file2 ("sound/misc/fburn_bg.wav");
	precache_file2 ("sound/death/fout.wav");
	precache_file2 ("sound/death/dthdie.wav");
	precache_file2 ("sound/death/dthfire.wav");
	precache_file2 ("sound/death/victory.wav");
	precache_file2 ("sound/death/dthlaugh.wav");
	precache_file2 ("sound/death/clop.wav");
	precache_file2 ("sound/death/clop1.wav");
	precache_file2 ("sound/death/clop2.wav");
	precache_file2 ("sound/death/clop3.wav");
	precache_file2 ("sound/death/shot.wav");
	precache_file2 ("sound/ambience/moan1.wav");
	precache_file2 ("sound/ambience/moan2.wav");
	precache_file2 ("sound/ambience/moan3.wav");
}


//**********************************************
// ***************  Raven sounds
//**********************************************
void Precache_wav (void)
{
//DEMO

//Miscellaneous
	precache_sound ("misc/drip.wav");		//Ambience
//	precache_sound ("misc/bshatter.wav");	//Burnt thing breaking
	precache_sound ("misc/sshatter.wav");	//Stone thing breaking
	precache_sound ("misc/pushmetl.wav");	//Pushing different materials- metal
	precache_sound ("misc/pushwood.wav");	//Wood
	precache_sound ("misc/pushston.wav");	//Stone
	precache_sound ("misc/fout.wav");		//Fire doused
	precache_sound ("misc/fburn_sm.wav");	//Small fire burning
	precache_sound ("misc/fburn_md.wav");	//Medium fire buring
	precache_sound ("misc/fburn_bg.wav");	//Big ol' blaze!
	precache_sound ("misc/decomp.wav");		//Decomposing sound- actually used for blood squirts
//	precache_sound ("misc/camera.wav");		//View through camera
	precache_sound ("misc/hith2o.wav");		// thing landing in water
	precache_sound ("misc/lighthit.wav");	//Something hit by lightning
	precache_sound ("misc/teleprt1.wav");	// teleport sounds- teleport coin
	precache_sound ("misc/teleprt2.wav");
	precache_sound ("misc/teleprt3.wav");
	precache_sound ("misc/teleprt4.wav");
	precache_sound ("misc/teleprt5.wav");
	precache_sound ("misc/comm.wav");		// communication
	precache_sound ("misc/talk.wav");		// communication
//	precache_sound ("raven/squawk.wav");	//Ambient raven
	precache_sound ("raven/blast.wav");	
	precache_sound ("doors/baddoor.wav");   // Bad attempt to open a door
	precache_sound ("buttons/switch04.wav");	// used by the trip mine

//FX
	precache_sound ("fx/glassbrk.wav");		// Glass breaking
	precache_sound ("fx/woodbrk.wav");		// Wood breaking
	precache_sound ("fx/wallbrk.wav");		// Stone breaking
	precache_sound ("fx/metalbrk.wav");		// Metal breaking
	precache_sound ("fx/claybrk.wav");		// Clay pot breaking
	precache_sound ("fx/leafbrk.wav");		// Leaves breaking (bush or tree)
	precache_sound ("fx/clothbrk.wav");		// Cloth breaking (rug)
	precache_sound ("fx/thngland.wav");		// landing thud
	precache_sound ("misc/null.wav");		// null sound to stop other sounds from playing

//ITEMS and ARTIFACTS
	precache_sound ("weapons/ammopkup.wav");// Backpack pick up
	precache_sound ("weapons/weappkup.wav");// weapon pick up
	precache_sound ("items/artpkup.wav");	//Artifact pick up
	precache_sound ("misc/sheep1.wav");		//In case someone uses Lambinator
	precache_sound ("misc/sheep2.wav");
	precache_sound ("misc/sheep3.wav");
	precache_sound ("misc/sheepfly.wav");	//Sheep launched from catapult!!!
	precache_sound ("spider/bite.wav");		//Sheep bite
	precache_sound ("items/itempkup.wav");
	precache_sound ("items/itmspawn.wav");		// item respawn sound
	precache_sound ("items/ringpkup.wav");		// Picking up a ring
	precache_sound ("items/artpkup.wav");
	precache_sound ("items/armrpkup.wav");
	precache_sound ("misc/warning.wav");   //glyph about to explode
//	precache_sound ("crusader/sunhum.wav");	
	//Summoning Stone
	precache_sound ("imp/upbig.wav");
	precache_sound ("imp/diebig.wav");
	precache_sound ("imp/swoophit.wav");
	precache_sound ("imp/swoopbig.wav");
	precache_sound ("imp/flybig.wav");
	precache_sound ("imp/fireball.wav");
	precache_sound ("imp/shard.wav");
	precache_sound5 ("hydra/turn-s.wav");

//SHARED PLAYER SOUNDS
	//General body/physics sounds
	precache_sound ("raven/outwater.wav");		// leaving water sound
	precache_sound ("raven/inh2o.wav");			// player in water
	precache_sound ("raven/inlava.wav");		// player enter lava
	precache_sound ("player/h2ojmp.wav");		// player jumping into water
	precache_sound ("player/swim1.wav");		// swimming
	precache_sound ("player/swim2.wav");		// swimming
	precache_sound ("player/land.wav");	
	//Player pain/death sounds
	precache_sound ("player/leave.wav");		// leaving deathmatch
	precache_sound ("player/telefrag.wav");		// telefrag death
	precache_sound ("player/decap.wav");		// Decapitation sound
	precache_sound ("player/megagib.wav");		//Really nasty explosive death
	precache_sound ("player/gib1.wav");			// player gib sound
	precache_sound ("player/gib2.wav");			// player gib sound
//	precache_sound ("player/slimbrn1.wav");	// player enter slime
	//General weapon sounds
	precache_sound ("misc/whoosh.wav");			//Throwing grenades, swing weapons, etc.
	precache_sound ("weapons/unsheath.wav");	//Unsheath bladed weapon
	precache_sound ("weapons/slash.wav");		//bladed weapon cuts flesh
	precache_sound ("weapons/met2flsh.wav");	//Metal impact sounds
	precache_sound ("weapons/met2wd.wav");	
	precache_sound ("weapons/met2stn.wav");	
	precache_sound ("weapons/met2met.wav");	
	precache_sound ("weapons/expsmall.wav");	//Small explosion
	precache_sound ("weapons/explode.wav");		//Normal explosion
	precache_sound ("weapons/exphuge.wav");		//SHA-BOOOOMMM!!!
	precache_sound ("weapons/hithurt2.wav");	// Damaging non-flesh with a melee weapon
	precache_sound ("weapons/hitwall.wav");		// Hitting (not damaging) a wall with a melee weapon

//PALADIN
//Body sounds
	precache_sound ("player/paljmp.wav");		// player jump
	precache_sound ("player/pallnd.wav");		// player hurt when landing
	precache_sound ("player/paldieh2.wav");		// player dying in water
	precache_sound ("player/paldie1.wav");		// player death 1
	precache_sound ("player/paldie2.wav");		// player death 2
	precache_sound ("player/palpain1.wav");		// player pain 1
	precache_sound ("player/palpain2.wav");		// player pain 2
	precache_sound ("player/palgasp1.wav");		// little air
	precache_sound ("player/palgasp2.wav");		// no air
	precache_sound ("player/paldrown.wav");		// he's drowning
	precache_sound ("paladin/devine.wav");		// Devine Intervention
//Weapon sounds
	//Gauntlets
	precache_sound ("weapons/gaunt1.wav");		
	precache_sound ("weapons/gauntht1.wav");	
	precache_sound ("weapons/gauntht2.wav");	
	//Vorpal Sword
	precache_sound ("weapons/vorpswng.wav");	// Vorpal sword swinging
	precache_sound ("weapons/vorpht1.wav");		// Vorpal sword hitting something it can damage
	precache_sound ("weapons/vorpht2.wav");		// Vorpal sword hitting something it cannot da

	precache_sound ("weapons/vorpturn.wav");	// Vorpal Sword - weapon 2
	precache_sound ("weapons/vorpblst.wav");
	precache_sound ("weapons/vorppwr.wav");
	//Axe
	precache_sound ("paladin/axric1.wav");		// Double Headed Axe - weapon 3
	precache_sound ("paladin/axgen.wav");		
	precache_sound ("paladin/axgenpr.wav");		
	precache_sound ("paladin/axblade.wav");		
	//Purifier
//	precache_sound ("paladin/purfire.wav");	
//	precache_sound ("paladin/purfireb.wav");	
	//Glyph: delayed fireball
	precache_sound ("weapons/fbfire.wav");		//Delayed fireball explosion sound	


//ASSASSIN
//Body sounds
	precache_sound ("player/assjmp.wav");		// player jump
	precache_sound ("player/asslnd.wav");		// player hurt when landing
	precache_sound ("player/assdieh2.wav");		// player dying in water
	precache_sound ("player/assdie1.wav");		// player death 1
	precache_sound ("player/assdie2.wav");		// player death 2
	precache_sound ("player/asspain1.wav");		// player pain 1
	precache_sound ("player/asspain2.wav");		// player pain 2
	precache_sound ("player/assgasp1.wav");		// little air
	precache_sound ("player/assgasp2.wav");		// no air
	precache_sound ("player/assdrown.wav");		// she's drowning
//Weapon sounds
	//Crossbow
	precache_sound ("assassin/arrowfly.wav");	
	precache_sound ("assassin/arr2flsh.wav");	
	precache_sound ("assassin/arr2wood.wav");	
	precache_sound ("assassin/arrowbrk.wav");
	precache_sound ("assassin/firefblt.wav");	
	precache_sound ("assassin/firebolt.wav");	
	//Grenades
	precache_sound ("assassin/gbounce.wav");	
	//Scarab Staff
//	precache_sound ("assassin/build.wav");	
//	precache_sound ("assassin/pincer.wav");	
	precache_sound ("assassin/chntear.wav");	
//	precache_sound ("assassin/chn2flsh.wav");	
	precache_sound ("assassin/chain.wav");	
//	precache_sound ("assassin/clink.wav");	
//	precache_sound ("assassin/scarab.wav");	
//	precache_sound ("assassin/scrbfly.wav");	
//	precache_sound ("assassin/spin.wav");	
//	precache_sound ("assassin/core.wav");	
	precache_sound ("misc/pulse.wav");			//Invincibility

//REGISTERED======================================================

//Shared sounds
	precache_sound ("raven/soul.wav");			// noise the soul sphere makes

//FX
	precache_sound ("fx/quake.wav");	

//CRUSADER
	//Warhammer
	precache_sound ("crusader/lghtn1.wav");	
	precache_sound ("crusader/lghtn2.wav");	
	precache_sound ("raven/lightng1.wav");
	//Ice Staff
	precache_sound ("crusader/icehit.wav");	
//	precache_sound ("crusader/icewall.wav");	
//	precache_sound ("crusader/icefire.wav");	
//	precache_sound ("misc/tink.wav");				//Ice shots bounce
//	precache_sound ("crusader/blizfire.wav");	
//	precache_sound ("crusader/blizzard.wav");	
	precache_sound ("crusader/frozen.wav");	
//	precache_sound ("misc/icestatx.wav");	//Ice statue breaking
	//Meteor Staff
//	precache_sound ("crusader/metfire.wav");	
	precache_sound ("misc/rubble.wav");			//Meteor bits fall, stoned player bits fall (from Medusa)
//	precache_sound ("crusader/torngo.wav");	
//	precache_sound ("crusader/tornado.wav");	
	//Sunstaff
//	precache_sound ("crusader/sunstart.wav");	
	precache_sound ("crusader/sunhit.wav");	

//NECROMANCER
	//Sickle
	precache_sound ("weapons/drain.wav");
	//Magic Missiles
	precache_sound ("necro/mmfire.wav");	
	//Bone Shards
//	precache_sound ("necro/bonefpow.wav");	
//	precache_sound ("necro/bonefnrm.wav");	
//	precache_sound ("necro/bonephit.wav");	
//	precache_sound ("necro/bonenhit.wav");	
//	precache_sound ("necro/bonenwal.wav");	
	//Raven Staff
//	precache_sound ("raven/ravengo.wav");		
//	precache_sound ("raven/squawk2.wav");		
//	precache_sound ("raven/death.wav");
//	precache_sound ("raven/rfire1.wav");
//	precache_sound ("raven/rfire2.wav");
//	precache_sound ("raven/split.wav");
	precache_sound2 ("eidolon/roar.wav");
	precache_sound2 ("eidolon/pain.wav");		//Hurt
	precache_sound2 ("eidolon/death.wav");		//Dies- long and agonizing
	precache_sound2 ("eidolon/growl.wav");		//
	precache_sound2 ("yakman/slam.wav");
	precache_sound2 ("yakman/grunt.wav");
	precache_sound2 ("eidolon/stomp.wav");		//Hot-steppin'
//WIN
	precache_sound ("misc/winner.wav");	
	precache_sound ("misc/victory.wav");
	precache_sound ("misc/smite.wav");		//Bad boy!
}

//Succubus precache shit
void Precache_Demoness (void)
{
//SUCCUBUS
	precache_model5 ("models/h_suc.mdl");
	precache_model5 ("models/succubus.mdl");
//Weapon 1
	precache_model5("models/sucwp1.mdl");
	precache_model5("models/sucwp1p.mdl");
	precache_model5("models/xplsn_1.spr");
//	precache_model5("models/xplsn_2.spr");
//	precache_model5("models/xplsn_3.spr");
	precache_model5("models/spark.spr");
//Weapon 2
//	precache_model5("models/sucwp2.mdl");
	precache_model5("models/sucwp2p.mdl");
//	precache_model5("models/muzzle1.spr");
//	precache_model5("models/axplsn_1.spr");
//	precache_model5("models/axplsn_2.spr");
//	precache_model5("models/axplsn_3.spr");
//	precache_model5("models/axplsn_4.spr");
//	precache_model5("models/axplsn_5.spr");
//Weapon 3
	precache_model5("models/sucwp3.mdl");
//	precache_model5("models/flamestr.spr"); //demoness
//	precache_model5("models/pow.spr");      //demoness
	precache_model5("models/firewal1.spr"); //demoness
	precache_model5("models/firewal2.spr");
	precache_model5("models/firewal3.spr");
//	precache_model5("models/fboom.spr");
	precache_model5("models/firewal5.spr");
	precache_model5("models/firewal4.spr");

//Weapon 4
//	precache_model5("models/sucwp4.mdl");
//	precache_model5("models/lball.mdl");
//	precache_model5("models/Bluexp3.spr");
//	precache_model5	("models/stltng2.mdl");	//Lightning- (warhammer, eidolon, lightwp)


//SOUNDS
//Body sounds
	precache_sound5 ("succubus/fwoomp.wav");	// Slow fall

//Weapon 1
	precache_sound5 ("succubus/brnbounce.wav");
	precache_sound5 ("succubus/brnfire.wav");
	precache_sound5 ("succubus/brnhit.wav");
	precache_sound5 ("succubus/brnwall.wav");
//Weapon 2
//	precache_sound5 ("succubus/acidfire.wav");
//	precache_sound5 ("succubus/acidpfir.wav");
//	precache_sound5 ("succubus/acidhit.wav");
//	precache_sound5 ("succubus/blobexpl.wav");
//	precache_sound5 ("succubus/dropfizz.wav");
//Weapon 3
	//precache_sound ("mummy/mislfire.wav");
	//precache_sound ("eidolon/flamend.wav");
	//precache_sound ("misc/fburn_bg.wav");
	//precache_sound ("death/fout.wav");
//	precache_sound5 ("succubus/firecirc.wav");
//	precache_sound5 ("succubus/flamend.wav");
//	precache_sound5 ("succubus/flamstrt.wav");
//	precache_sound5 ("succubus/flampow.wav");
	
//Weapon 4
//	precache_sound5 ("succubus/firelght.wav");
//	precache_sound5 ("succubus/firelbal.wav");
//	precache_sound5 ("succubus/firelpow.wav");
	precache_sound5 ("crusader/lghtn2.wav");
//	precache_sound5 ("succubus/buzz.wav");
//	precache_sound5 ("succubus/buzz2.wav");
//Glyph
//	precache_sound5 ("succubus/gasss.wav");
//	precache_sound5 ("succubus/hisss.wav");
//	precache_sound5 ("succubus/endhisss.wav");

}


//**********************************************
// ***************  Id models
//**********************************************
void Precache_Id_mdl (void)
{
//REMOVE!!!!
	precache_model ("models/s_light.spr");	// sphere light
}


//**********************************************
// ***************  Raven models
//**********************************************
void Precache_mdl (void)
{
//REMOVE!!!
//	precache_model("models/akarrow.mdl");//Mummy, archer, pstboar
//	precache_model("models/dthball.mdl");//Goes in dthfire.hc

//MISC
	precache_model ("models/teleport.mdl");	//Teleport model
	precache_model("models/xhair.mdl");		//Ballista- REPLACE!!!
	precache_model ("models/spike.mdl");
//CHUNKS
	precache_model("models/shard1.mdl");
	precache_model("models/shard2.mdl");
	precache_model("models/shard3.mdl");
	precache_model("models/shard4.mdl");
	precache_model("models/shard5.mdl");
	precache_model("models/splnter1.mdl");
	precache_model("models/splnter2.mdl");
	precache_model("models/splnter3.mdl");
	precache_model("models/splnter4.mdl");
	precache_model("models/metlchk1.mdl");
	precache_model("models/metlchk2.mdl");
	precache_model("models/metlchk3.mdl");
	precache_model("models/metlchk4.mdl");
	precache_model("models/leafchk1.mdl");
	precache_model("models/leafchk2.mdl");
	precache_model("models/leafchk3.mdl");
	precache_model("models/clthchk1.mdl");
	precache_model("models/clthchk2.mdl");
	precache_model("models/clthchk3.mdl");
	precache_model("models/flesh1.mdl");
	precache_model("models/flesh2.mdl");
	precache_model("models/flesh3.mdl");
	precache_model("models/brains.mdl");
	precache_model("models/clshard1.mdl");
	precache_model("models/clshard2.mdl");
	precache_model("models/clshard3.mdl");
	precache_model("models/clshard4.mdl");
	precache_model("models/hay1.mdl");
	precache_model("models/hay2.mdl");
	precache_model("models/hay3.mdl");
	precache_model("models/shard.mdl");	//shard model for ice, rock, ashes

//ARTIFACTS
//	precache_model("models/a_shbost.mdl");
//	precache_model("models/a_hboost.mdl");
	precache_model("models/a_torch.mdl");
//	precache_model("models/a_blast.mdl");
//	precache_model("models/a_mboost.mdl");
	precache_model("models/a_telprt.mdl");
//	precache_model("models/a_tome.mdl");
//	precache_model("models/a_summon.mdl");
//	precache_model("models/a_mine.mdl");
	precache_model("models/a_glyph.mdl");
//	precache_model("models/a_haste.mdl");
//	precache_model("models/a_poly.mdl");
//	precache_model("models/a_mirror.mdl");
	precache_model("models/a_cube.mdl");
	precache_model("models/a_invinc.mdl");
//	precache_model("models/a_growth.mdl");
//	precache_model("models/a_xray.mdl");
	precache_model("models/a_invis.mdl");
	precache_model("models/cube.mdl");
//	precache_model("models/ringft.mdl");

	//Lambinator
	precache_model("models/sheep.mdl");
	precache_model("models/snout.mdl");
	//Summoning Stone
	precache_model ("models/imp.mdl");
	precache_model ("models/h_imp.mdl");//empty for now
	precache_model ("models/shardice.mdl");
	precache_model ("models/fireball.mdl");

//ITEMS
	precache_model("models/i_bracer.mdl");	// Armor
	precache_model("models/i_bplate.mdl");
	precache_model("models/i_helmet.mdl");
	precache_model("models/i_amulet.mdl");
//	precache_model ("models/i_gmana.mdl");	// Instant Mana
//	precache_model ("models/i_bmana.mdl");
//	precache_model ("models/i_btmana.mdl");
//	precache_model ("models/i_hboost.mdl");	// Instant Health
	precache_model ("models/bag.mdl");	// Our version of a backpack

//TE_STREAM models
	// TE_STREAM_SUNSTAFF1 / TE_STREAM_SUNSTAFF2
//	precache_model("models/stsunsf1.mdl");	//Sunbeam and ball models
//	precache_model("models/stsunsf2.mdl");
//	precache_model("models/stsunsf3.mdl");
//	precache_model("models/stsunsf4.mdl");
//	precache_model("models/stsunsf5.mdl");
	// TE_STREAM_LIGHNING
	precache_model	("models/stlghtng.mdl");	//Lightning- also warhammer
	// TE_STREAM_CHAIN
	precache_model("models/stchain.mdl");	//Chain- also for Scarab staff
	// TE_STREAM_COLORBEAM
//	precache_model("models/stclrbm.mdl");	//Colored beams of light
	// TE_STREAM_ICECHUNKS
//	precache_model("models/stice.mdl");		//For blizzard
	// TE_STREAM_GAZE
//	precache_model("models/stmedgaz.mdl");	//Medusa's gaze
	// TE_STREAM_FAMINE
//	precache_model("models/fambeam.mdl");	//Famine's beam attack

//GLYPHS
	precache_model("models/glyph.mdl");		//Non-artifact flagged glyph
	//Paladin
	precache_model ("models/blast.mdl");	//Delayed fireball
	//Assassin
	precache_model("models/glyphwir.mdl");	//Tripwire version of glyph
	precache_model ("models/twspike.mdl");	//Trip wire spike

//PALADIN
	precache_model ("models/paladin.mdl");
	precache_model ("models/h_pal.mdl");
	//Gauntlets
	precache_model("models/gauntlet.mdl");		// Paladin Weapons
	//Axe
	precache_model("models/axe.mdl");
	precache_model("models/axblade.mdl");
	precache_model("models/axtail.mdl");
	//Vorpal Sword
	precache_model("models/vorpal.mdl");
	precache_model("models/vorpswip.mdl");
	precache_model("models/vorpshot.mdl");
	precache_model("models/vorpshok.mdl");	//Vorpal sword & lightning hit
	precache_model("models/vorpshk2.mdl");
	//Purifier
//	precache_model("models/purifier.mdl");
//	precache_model("models/purfir1.mdl");	//Purifier flame
//	precache_model("models/drgnball.mdl");	//Purifier fireball, take 2
//	precache_model("models/ring.mdl");		//Smoke ring

//ASSASSIN
	precache_model ("models/assassin.mdl");
	precache_model ("models/h_ass.mdl");
	precache_model ("models/h_fangel.mdl");//Temp head model for Assassin
	//Punch Dagger
	precache_model("models/punchdgr.mdl");
	//Crossbow
//	precache_model("models/xbow2.mdl");
//	precache_model("models/w_l3_c4.mdl");
	precache_model("models/lbow.mdl");
	precache_model("models/arrows.mdl");
	precache_model("models/crossbow.mdl");
	precache_model ("models/arrow.mdl");
	precache_model ("models/arrowhit.mdl");
	precache_model ("models/flaming.mdl");
	precache_model ("models/NFarrow.mdl");
	//Grenades
	precache_model("models/v_assgr.mdl");
//	precache_model("models/assgren.mdl");
	//Scarab Staff
//	precache_model("models/scarabst.mdl");
//	precache_model("models/scrbstp1.mdl");
//	precache_model("models/scrbpbdy.mdl");
//	precache_model("models/scrbpwng.mdl");


//REGISTERED=======================================================

//NECROMANCER
	precache_model("models/necro.mdl");
	precache_model ("models/h_nec.mdl");
	//Sickle
	precache_model("models/sickle.mdl");		// Necromancer Weapons
	//Magic Missiles
	precache_model("models/spllbook.mdl");
	precache_model("models/handfx.mdl");
	//Bone Shards
//	precache_model("models/bonefx.mdl");
//	precache_model("models/boneshot.mdl");
//	precache_model("models/boneshrd.mdl");
//	precache_model("models/bonelump.mdl");
	//Raven Staff
//	precache_model("models/ravenstf.mdl");
//	precache_model("models/vindsht1.mdl");
//	precache_model("models/ravproj.mdl");
//	precache_model("models/birdmsl2.mdl");

//CRUSADER
	precache_model("models/crusader.mdl");
	precache_model ("models/h_cru.mdl");
	//Warhammer
	precache_model("models/warhamer.mdl");
	precache_model("models/hamthrow.mdl");
	//Ice Staff
	precache_model("models/icestaff.mdl");
//	precache_model("models/iceshot1.mdl");
//	precache_model("models/iceshot2.mdl");
	//Meteor Staff
//	precache_model("models/meteor.mdl");
//	precache_model("models/tempmetr.mdl");//temp- meteor projectile
//	precache_model("models/tornato.mdl");
//	precache_model("models/funnal.mdl");
	//Sunstaff
//	precache_model("models/sunstaff.mdl");

//SPECIAL ABILITIES
	//Necromancer
	precache_model ("models/soulball.mdl");//Soul sphere
	precache_model ("models/soulskul.mdl");
	//Crusader
	precache_model ("models/goodsphr.mdl");//Smiting Sphere
	precache_model ("models/cross.mdl");

//	precache_model ("models/birdmisl.mdl");
//Miscellaneous Shared
	precache_model ("models/test.mdl");	//For testing

//SIEGE
	precache_model ("models/hank.mdl");
	precache_model ("models/h_hank.mdl");
	precache_model("models/ripple.mdl");
	precache_sound("misc/combust.wav");
//Become beast
	precache_model2 ("models/yakman.mdl");
	precache_model ("models/god.mdl");		//The big guy
}


//**********************************************
// ***************  Raven sprites
//**********************************************
void Precache_spr (void)
{//ALPHABETIZED!
//FIXME: Which of these aren't being used anymore?
	precache_model("models/bg_expld.spr");
	precache_model("models/bldspot1.spr");
	precache_model("models/bldspot2.spr");
	precache_model("models/bldspot3.spr");
	precache_model("models/bldspot4.spr");
	precache_model("models/bluflash.spr");
	precache_model("models/bonexpld.spr");
	precache_model("models/bspark.spr");
	precache_model("models/rcloud.spr");

//	precache_model ("models/eidoflam.spr");

	precache_model("models/fcircle.spr");
	precache_model("models/fl_expld.spr");

	precache_model("models/gen_expl.spr");
//	precache_model ("models/ghost.spr");
	precache_model ("gfx/glass.spr");
	precache_model("models/grnsmk1.spr");
	precache_model ("models/gryspt.spr");
	precache_model("models/grysmk1.spr");
	precache_model ("models/gspark.spr");

//	precache_model("models/icehit.spr");	//Ice mace shot impact
	
//	precache_model ("models/medhit.spr");
	precache_model ("models/mezzoref.spr");
	precache_model ("models/mm_expld.spr");

	precache_model ("models/null.spr");		//nothing there

	precache_model("models/polymrph.spr");

	precache_model("models/redsmk1.spr");
	precache_model ("models/redspt.spr");
	precache_model("models/rspark.spr");

	precache_model ("gfx/s_blast.spr");
	precache_model ("models/s_bubble.spr");	// drowning bubbles
	precache_model ("models/s_explod.spr");	// Not sure, but it was in our directory
	precache_model ("models/s_light.spr");
	precache_model("models/sm_blue.spr");
	precache_model("models/sm_expld.spr");
	precache_model("models/sm_white.spr");
	precache_model("gfx/smoke1.spr");
	precache_model("models/spark.spr");
	precache_model("models/spark0.spr");
	precache_model ("gfx/stone.spr");

	precache_model("models/telesmk1.spr");
	precache_model("models/telesmk2.spr");
	precache_model("models/telesmk3.spr");

	precache_model("models/whtsmk1.spr");
	precache_model("models/wsplash.spr");

	precache_model("models/xbowexpl.spr");	//Crossbow explosion
//	precache_model("models/xplod29.spr");	//old Eidolon flames
	precache_model("models/xpspblue.spr");

	precache_model("models/yr_flsh.spr");
	precache_model3("models/rcloud.spr");
}

void precache_archer()
{
	precache_model("models/archer.mdl");
	precache_model("models/archerhd.mdl");

	precache_model("models/gspark.spr");

	precache_sound ("archer/arrowg.wav");
	precache_sound ("archer/arrowr.wav");

	precache_sound ("archer/growl.wav");
	precache_sound ("archer/pain.wav");
	precache_sound ("archer/sight.wav");
	precache_sound ("archer/death.wav");
	precache_sound ("archer/draw.wav");
}

void precache_spider ()
{
	precache_model("models/spider.mdl");
	precache_model("models/sflesh1.mdl");
	precache_model("models/sflesh2.mdl");
	precache_model("models/sflesh3.mdl");

	precache_sound("spider/bite.wav");
	precache_sound("spider/pain.wav");
	precache_sound("spider/death.wav");
	precache_sound("spider/step1.wav");
	precache_sound("spider/step2.wav");
	precache_sound("spider/step3.wav");
}

void Precache_hw(void)
{
	// Maps
	precache_file5("maps/hwdm1.bsp");
	precache_file5("maps/hwdm2.bsp");
	precache_file5("maps/hwdm3.bsp");
	precache_file5("maps/hwdm4.bsp");
	precache_file5("maps/hwdm5.bsp");

//	precache_sound5 ("crusader/sunhuml.wav");		//Looping sunhum
//	precache_sound5 ("misc/cubehum.wav");			//force cube
//	precache_sound5 ("misc/Beep1.wav");				//force cube
//	precache_sound5 ("misc/pulsel.wav");			//Fully charged staff- loops
//	precache_sound5 ("golem/gbfire.wav");

	// Models
	precache_model5("models/assgren.mdl");
	precache_model5("models/newmmis.mdl");
	precache_model5 ("models/fstep.mdl");	//For testing

	precache_file5("sound/misc/talk.wav");

	// Taunts don't need to be precached as sounds
	precache_file5("sound/taunt/taunt001.wav");
	precache_file5("sound/taunt/taunt002.wav");
	precache_file5("sound/taunt/taunt003.wav");
	precache_file5("sound/taunt/taunt004.wav");
	precache_file5("sound/taunt/taunt005.wav");
	precache_file5("sound/taunt/taunt006.wav");
	precache_file5("sound/taunt/taunt007.wav");
//	precache_file5("sound/taunt/taunt008.wav");
	precache_file5("sound/taunt/taunt009.wav");
	precache_file5("sound/taunt/taunt010.wav");
	precache_file5("sound/taunt/taunt011.wav");
//	precache_file5("sound/taunt/taunt012.wav");
	precache_file5("sound/taunt/taunt013.wav");
	precache_file5("sound/taunt/taunt014.wav");
	precache_file5("sound/taunt/taunt015.wav");
	precache_file5("sound/taunt/taunt016.wav");
	precache_file5("sound/taunt/taunt017.wav");
	precache_file5("sound/taunt/taunt018.wav");
	precache_file5("sound/taunt/taunt019.wav");
	precache_file5("sound/taunt/taunt020.wav");
//	precache_file5("sound/taunt/taunt021.wav");
//	precache_file5("sound/taunt/taunt022.wav");
//	precache_file5("sound/taunt/taunt023.wav");
	precache_file5("sound/taunt/taunt024.wav");
	precache_file5("sound/taunt/taunt025.wav");
	precache_file5("sound/taunt/taunt026.wav");
	precache_file5("sound/taunt/taunt027.wav");
	precache_file5("sound/taunt/taunt028.wav");
//	precache_file5("sound/taunt/taunt029.wav");
	precache_file5("sound/taunt/taunt030.wav");
	precache_file5("sound/taunt/taunt031.wav");
	precache_file5("sound/taunt/taunt032.wav");
	precache_file5("sound/taunt/taunt033.wav");
	precache_file5("sound/taunt/taunt034.wav");
	precache_file5("sound/taunt/taunt035.wav");
	precache_file5("sound/taunt/taunt036.wav");

	// misc
	precache_file5 ("gfx/menu/conchars.lmp");        
}

