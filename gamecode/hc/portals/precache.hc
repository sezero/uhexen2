/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/portals/precache.hc,v 1.1.1.1 2004-11-29 11:34:58 sezero Exp $
 */

// called by worldspawn
void() W_Precache =
{
	precache_file ("rj/steve.wav");
	precache_file ("weapons/ric1.wav");	// ricochet (used in c code)
	precache_file ("weapons/ric2.wav");	// ricochet (used in c code)
	precache_file ("weapons/ric3.wav");	// ricochet (used in c code)
	precache_file ("weapons/tink1.wav");	// ricochet (used in c code)
	precache_file ("weapons/r_exp3.wav");	//Still being used?

	//why are these here?
	precache_model4 ("models/ball.mdl");	//magicmis, icemace
	precache_model ("models/star.mdl");	//lightwp, magicmis
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
	precache_file ("gfx/skin100.lmp");	//global stone skins
	precache_file ("gfx/skin101.lmp");

//	precache_file ("strings.txt");
//	precache_file ("infolist.txt");	//keep these out of the pak for easy localization
	precache_file ("puzzles.txt");

	precache_file ("gfx/menu/fontsize.lmp");
	precache_file ("gfx/menu/backtile.lmp");         
	precache_file ("gfx/menu/bigfont.lmp");        
	precache_file ("gfx/menu/bigfont2.lmp");        
	precache_file ("gfx/menu/conback.lmp");        
	precache_file4 ("gfx/menu/conchars.lmp");		//updated for mission pack
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

	//player faces
	precache_file ("gfx/cport1.lmp");
	precache_file3 ("gfx/cport2.lmp");
	precache_file3 ("gfx/cport3.lmp");
	precache_file ("gfx/cport4.lmp");
	precache_file4 ("gfx/cport5.lmp");

	//player colors
	precache_file ("gfx/menu/netp1.lmp");
	precache_file3 ("gfx/menu/netp2.lmp");
	precache_file3 ("gfx/menu/netp3.lmp");
	precache_file ("gfx/menu/netp4.lmp");
	precache_file ("gfx/menu/frame.lmp");
	precache_file4 ("gfx/menu/netp5.lmp");

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

	//intermission gfx
	precache_file ("gfx/castle.lmp");
	precache_file2 ("gfx/meso.lmp");
	precache_file2 ("gfx/egypt.lmp");
	precache_file2 ("gfx/roman.lmp");

	precache_file2 ("gfx/end-1.lmp");
	precache_file2 ("gfx/end-2.lmp");
	precache_file2 ("gfx/end-3.lmp");


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
//End screens
	precache_file4 ("gfx/mpmid.lmp");
	precache_file4 ("gfx/mpend.lmp");
//MISSION PACK PUZZLE PIECES
	precache_file4 ("gfx/puzzle/bell.lmp");
	precache_file4 ("gfx/puzzle/chalice.lmp");
	precache_file4 ("gfx/puzzle/crosskey.lmp");
	precache_file4 ("gfx/puzzle/dagger.lmp");
	precache_file4 ("gfx/puzzle/dtongue.lmp");
	precache_file4 ("gfx/puzzle/dukeseal.lmp");
	precache_file4 ("gfx/puzzle/goldgem.lmp");
	precache_file4 ("gfx/puzzle/greengem.lmp");
	precache_file4 ("gfx/puzzle/headkey.lmp");
	precache_file4 ("gfx/puzzle/incense.lmp");
	precache_file4 ("gfx/puzzle/lbudda.lmp");
	precache_file4 ("gfx/puzzle/magekey.lmp");
	precache_file4 ("gfx/puzzle/tienkey.lmp");
	precache_file4 ("gfx/puzzle/mask.lmp");
	precache_file4 ("gfx/puzzle/orb.lmp");
	precache_file4 ("gfx/puzzle/orb2.lmp");
	precache_file4 ("gfx/puzzle/prayer2.lmp");
	precache_file4 ("gfx/puzzle/redgem.lmp");
	precache_file4 ("gfx/puzzle/scepter.lmp");
	precache_file4 ("gfx/puzzle/scroll.lmp");
	precache_file4 ("gfx/puzzle/stonet.lmp");
	precache_file4 ("gfx/puzzle/tbtkey.lmp");
	precache_file4 ("gfx/puzzle/tbtmask.lmp");
	precache_file4 ("gfx/puzzle/vial.lmp");

	precache_file4 ("models/puzzle/bell.mdl");
	precache_file4 ("models/puzzle/chalice.mdl");
	precache_file4 ("models/puzzle/crosskey.mdl");
	precache_file4 ("models/puzzle/dagger.mdl");
	precache_file4 ("models/puzzle/dtongue.mdl");
	precache_file4 ("models/puzzle/dukeseal.mdl");
	precache_file4 ("models/puzzle/goldgem.mdl");
	precache_file4 ("models/puzzle/greengem.mdl");
	precache_file4 ("models/puzzle/headkey.mdl");
	precache_file4 ("models/puzzle/incense.mdl");
	precache_file4 ("models/puzzle/lbudda.mdl");
	precache_file4 ("models/puzzle/magekey.mdl");
	precache_file4 ("models/puzzle/tienkey.mdl");
	precache_file4 ("models/puzzle/mask.mdl");
	precache_file4 ("models/puzzle/orb.mdl");
	precache_file4 ("models/puzzle/orb2.mdl");
	precache_file4 ("models/puzzle/prayer.mdl");
	precache_file4 ("models/puzzle/prayer2.mdl");
	precache_file4 ("models/puzzle/redgem.mdl");
	precache_file4 ("models/puzzle/scepter.mdl");
	precache_file4 ("models/puzzle/scroll.mdl");
	precache_file4 ("models/puzzle/stonet.mdl");
	precache_file4 ("models/puzzle/tbtkey.mdl");
	precache_file4 ("models/puzzle/tbtmask.mdl");
	precache_file4 ("models/puzzle/vial.mdl");
//NOTE: Add any train models, designer-spoecified sounds & models, 
//		Anything used in C that isn't precached, anything we're
//		replacing that was in the old .pak files

}


//**********************************************
// ***************  Raven sounds
//**********************************************
void Precache_wav (void)
{
	precache_sound ("raven/kiltorch.wav");	// player torch dying
	precache_sound ("raven/littorch.wav");	// player torch being lit
	precache_sound ("raven/fire1.wav");    // player torch burning

//Miscellaneous
	precache_sound ("misc/drip.wav");		//Ambience
	precache_sound ("misc/bshatter.wav");	//Burnt thing breaking
	precache_sound ("misc/sshatter.wav");	//Stone thing breaking
	precache_sound ("misc/pushmetl.wav");	//Pushing different materials- metal
	precache_sound ("misc/pushwood.wav");	//Wood
	precache_sound ("misc/pushston.wav");	//Stone
	precache_sound ("misc/fout.wav");		//Fire doused
	precache_sound ("misc/fburn_sm.wav");	//Small fire burning
	precache_sound ("misc/fburn_md.wav");	//Medium fire buring
	precache_sound ("misc/fburn_bg.wav");	//Big ol' blaze!
	precache_sound ("misc/decomp.wav");		//Decomposing sound- actually used for blood squirts
	precache_sound ("misc/camera.wav");		//View through camera
	precache_sound ("misc/hith2o.wav");		// thing landing in water
	precache_sound ("misc/lighthit.wav");	//Something hit by lightning
	precache_sound ("misc/teleprt1.wav");	// teleport sounds- teleport coin
	precache_sound ("misc/teleprt2.wav");
	precache_sound ("misc/teleprt3.wav");
	precache_sound ("misc/teleprt4.wav");
	precache_sound ("misc/teleprt5.wav");
	precache_sound ("misc/comm.wav");		// communication
	precache_sound ("raven/squawk.wav");	//Ambient raven
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
	precache_sound4 ("fx/bonebrk.wav");		// bones breaking

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
	precache_sound ("golem/gbfire.wav");	//force cube
	precache_sound4 ("misc/cubehum.wav");	//force cube
	precache_sound4 ("misc/Beep1.wav");	//force cube
	//Summoning Stone
	precache_sound3 ("imp/upbig.wav");
	precache_sound3 ("imp/diebig.wav");
	precache_sound3 ("imp/swoophit.wav");
	precache_sound3 ("imp/swoopbig.wav");
	precache_sound3 ("imp/flybig.wav");
	precache_sound3 ("imp/fireball.wav");
	precache_sound3 ("imp/shard.wav");
	precache_sound  ("hydra/turn-s.wav");

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
	precache_sound ("player/slimbrn1.wav");	// player enter slime
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

//PALADIN SHARED SOUNDS
//Weapon sounds
	//Gauntlets
	precache_sound ("weapons/gaunt1.wav");		//SHARED!
	precache_sound ("weapons/gauntht1.wav");	//SHARED!	
	precache_sound ("weapons/gauntht2.wav");	//SHARED!	
	//Vorpal Sword
	precache_sound ("weapons/vorpswng.wav");	//SHARED! Vorpal sword swinging
	precache_sound ("weapons/vorpht2.wav");		//SHARED! Vorpal sword hitting something it cannot da

	precache_sound ("weapons/vorpturn.wav");	//SHARED! Vorpal Sword - weapon 2
	//Axe
	precache_sound ("paladin/axric1.wav");		//SHARED! Double Headed Axe - weapon 3
	precache_sound ("paladin/axblade.wav");		//SHARED!

//ASSASSIN
//Weapon sounds
	//Grenades
	precache_sound ("assassin/gbounce.wav");	//SHARED! (Bouncers)
	//Scarab Staff
	precache_sound ("assassin/chntear.wav");	//SHARED! (Imp)
	precache_sound ("misc/pulse.wav");			//SHARED! (Eidolon) Fully charged staff

//REGISTERED======================================================

//Shared sounds
	precache_sound3 ("raven/soul.wav");			// noise the soul sphere makes

//FX
	precache_sound ("fx/quake.wav");	

//CRUSADER
	//Warhammer
	precache_sound3 ("crusader/lghtn1.wav");	//SHARED! (weather)
	precache_sound3 ("crusader/lghtn2.wav");	//SHARED! (weather)	
	precache_sound3 ("raven/lightng1.wav");		//SHARED! (weather)
	//Ice Staff
	precache_sound3 ("crusader/icehit.wav");	//SHARED? (ice shatter)
	precache_sound3 ("crusader/frozen.wav");	//SHARED! (Monster unfreezing)	
	precache_sound3 ("misc/icestatx.wav");		//SHARED! Ice statue breaking
	//Meteor Staff
	precache_sound3 ("misc/rubble.wav");		//SHARED chunks, Meteor bits fall, stoned player bits fall (from Medusa)
	//Sunstaff
	precache_sound3 ("crusader/sunhum.wav");	//SHARED! (weather)
	precache_sound3 ("crusader/sunhit.wav");	//SHARED! (weather)

//NECROMANCER
	//Magic Missiles
	precache_sound3 ("necro/mmfire.wav");		//SHARED! (Eidolon)
}


//**********************************************
// ***************  Raven models
//**********************************************

void Precache_misc (void)
{
//REMOVE!!!
	precache_model("models/akarrow.mdl");//Mummy, archer, pstboar

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
	precache_model4("models/clshard1.mdl");	//updated need in new pak
	precache_model4("models/clshard2.mdl");
	precache_model4("models/clshard3.mdl");
	precache_model4("models/clshard4.mdl");
	precache_model("models/hay1.mdl");
	precache_model("models/hay2.mdl");
	precache_model("models/hay3.mdl");
	precache_model("models/shard.mdl");	//shard model for ice, rock, ashes
	precache_model("models/schunk1.mdl");
	precache_model("models/schunk2.mdl");
	precache_model("models/schunk3.mdl");
	precache_model("models/schunk4.mdl");

//ARTIFACTS
	precache_model("models/a_shbost.mdl");
	precache_model("models/a_hboost.mdl");
	precache_model("models/a_torch.mdl");
	//precache_model("models/torch.mdl");
	precache_model("models/a_blast.mdl");
	precache_model("models/a_mboost.mdl");
	precache_model("models/a_telprt.mdl");
	precache_model("models/a_tome.mdl");
	precache_model("models/a_summon.mdl");
//	precache_model("models/a_mine.mdl");
	precache_model("models/a_glyph.mdl");
	precache_model("models/a_haste.mdl");
	precache_model("models/a_poly.mdl");
//	precache_model("models/a_mirror.mdl");
	precache_model("models/a_cube.mdl");
	precache_model("models/a_invinc.mdl");
//	precache_model("models/a_growth.mdl");
//	precache_model("models/a_xray.mdl");
	precache_model("models/a_invis.mdl");
	precache_model("models/cube.mdl");
	precache_model("models/ringft.mdl");
	precache_model("models/glyphwir.mdl");	//Tripwire version of glyph

	//Lambinator
	precache_model("models/sheep.mdl");
	precache_model("models/snout.mdl");
	//Summoning Stone
	precache_model4 ("models/imp.mdl");//converted for MP
	precache_model3 ("models/h_imp.mdl");//empty for now
	precache_model3 ("models/shardice.mdl");
	precache_model ("models/fireball.mdl");//imp, dthhorse,ai,fireball.hc

//ITEMS
	precache_model("models/i_bracer.mdl");	// Armor
	precache_model("models/i_bplate.mdl");
	precache_model("models/i_helmet.mdl");
	precache_model("models/i_amulet.mdl");
	precache_model ("models/i_gmana.mdl");	// Instant Mana
	precache_model ("models/i_bmana.mdl");
	precache_model ("models/i_btmana.mdl");
	precache_model ("models/i_hboost.mdl");	// Instant Health
	
	precache_model ("models/bag.mdl");	// Our version of a backpack

//TE_STREAM models
	// TE_STREAM_SUNSTAFF1 / TE_STREAM_SUNSTAFF2
	precache_model3("models/stsunsf1.mdl");	//Sunbeam and ball models
	precache_model3("models/stsunsf2.mdl");	//(weather_sunbeam,sunstaff)
	precache_model3("models/stsunsf3.mdl");
	precache_model3("models/stsunsf4.mdl");
	precache_model3("models/stsunsf5.mdl");

	// TE_STREAM_LIGHNING
	precache_model3	("models/stlghtng.mdl");	//Lightning- (warhammer, eidolon, lightwp)
	precache_model4	("models/stltng2.mdl");	//Lightning- (warhammer, eidolon, lightwp)

	// TE_STREAM_COLORBEAM
	precache_model3("models/stclrbm.mdl");	//Colored beams of light (weather,golem,fangel,buddha)

	precache_model("models/glyph.mdl");		//Non-artifact flagged glyph

//Miscellaneous Shared
	precache_model3 ("models/test.mdl");	//For testing

	// CE_LSHOCK
	precache_model("models/vorpshok.mdl");	//Vorpal sword & lightning hit 

	precache_model4 ("models/playrbox.mdl");
}


void Precache_Male (void)
{
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
}

void Precache_Paladin (void)
{
//PALADIN
	precache_model4 ("models/paladin.mdl");//converted for MP
	precache_model ("models/h_pal.mdl");
	//Gauntlets
	precache_model4("models/gauntlet.mdl");//converted for MP	// Paladin Weapons
	//Axe
	precache_model4("models/axe.mdl");//converted for MP
	precache_model("models/axblade.mdl");
	precache_model("models/axtail.mdl");
	//Vorpal Sword
	precache_model4("models/vorpal.mdl");//converted for MP
	precache_model("models/vorpswip.mdl");
	precache_model("models/vorpshot.mdl");
	precache_model("models/vorpshk2.mdl");
	//Purifier
	precache_model4("models/purifier.mdl");//converted for MP
	precache_model("models/purfir1.mdl");	//Purifier flame
	precache_model("models/drgnball.mdl");	//Purifier fireball, take 2
	precache_model ("models/blast.mdl");	//Delayed fireball
	precache_model("models/xplod29.spr");	//eidolon and purifier

//SOUNDS
//Body sounds
	Precache_Male ();
	precache_sound ("paladin/devine.wav");		// Devine Intervention
//Weapon sounds
	//Vorpal Sword
	precache_sound ("weapons/vorpht1.wav");		// Vorpal sword hitting something it can damage

	precache_sound ("weapons/vorpblst.wav");
	precache_sound ("weapons/vorppwr.wav");
	//Axe
	precache_sound ("paladin/axgen.wav");		
	precache_sound ("paladin/axgenpr.wav");		
	//Purifier
	precache_sound ("paladin/purfire.wav");	
	precache_sound ("paladin/purfireb.wav");	
	precache_model("models/ring.mdl");		//Smoke ring
	//Glyph: delayed fireball
	precache_sound ("weapons/fbfire.wav");		//Delayed fireball explosion sound	
}

void Precache_Crusader (void)
{
//CRUSADER
	precache_model4("models/crusader.mdl");//converted for MP
	precache_model3 ("models/h_cru.mdl");
	//Warhammer
	precache_model4("models/warhamer.mdl");//converted for MP
	precache_model3("models/hamthrow.mdl");
	//Ice Staff
	precache_model4("models/icestaff.mdl");//converted for MP
	precache_model3("models/iceshot1.mdl");
	precache_model3("models/iceshot2.mdl");
	precache_model("models/stice.mdl");		//For blizzard  (used by crusader only)
	//Meteor Staff
	precache_model3("models/meteor.mdl");
	precache_model3("models/tempmetr.mdl");//temp- meteor projectile
	precache_model4("models/tornato.mdl");//converted for MP
	precache_model4("models/funnal.mdl");//converted for MP
	//Sunstaff
	precache_model3("models/sunstaff.mdl");
	//Crusader
	precache_model3 ("models/goodsphr.mdl");//Smiting Sphere
	precache_model3 ("models/cross.mdl");
//SOUNDS
//Body Sounds
	Precache_Male ();
	//Ice Staff
	precache_sound3 ("crusader/icewall.wav");	
	precache_sound3 ("crusader/icefire.wav");	
	precache_sound3 ("misc/tink.wav");				//Ice shots bounce
	precache_sound3 ("crusader/blizfire.wav");	
	precache_sound3 ("crusader/blizzard.wav");	
	//Meteor Staff
	precache_sound3 ("crusader/metfire.wav");	
	precache_sound3 ("crusader/torngo.wav");	
	precache_sound3 ("crusader/tornado.wav");	
	//Sunstaff
	precache_sound3 ("crusader/sunstart.wav");	
}

//REGISTERED=======================================================

void Precache_Necromancer (void)
{
//NECROMANCER
	precache_model4("models/necro.mdl");//converted for MP
	precache_model3 ("models/h_nec.mdl");
	//Sickle
	precache_model4("models/sickle.mdl");//converted for MP		// Necromancer Weapons
	//Magic Missiles
	precache_model3("models/spllbook.mdl");
	precache_model3("models/handfx.mdl");
	//Bone Shards
//	precache_model3("models/bonefx.mdl");
	precache_model3("models/boneshot.mdl");
	precache_model3("models/boneshrd.mdl");
	precache_model3("models/bonelump.mdl");
	//Raven Staff
	precache_model4("models/ravenstf.mdl");//converted for MP
	precache_model3("models/vindsht1.mdl");
	precache_model3("models/ravproj.mdl");
	precache_model3("models/birdmsl2.mdl");
	//Necromancer
	precache_model3 ("models/soulball.mdl");//Soul sphere
	precache_model3 ("models/soulskul.mdl");
	precache_model3 ("models/birdmisl.mdl");
//SOUNDS
//Body Sounds
	Precache_Male ();
	//Sickle
	precache_sound3 ("weapons/drain.wav");
	//Bone Shards
	precache_sound3 ("necro/bonefpow.wav");	
	precache_sound3 ("necro/bonefnrm.wav");	
	precache_sound3 ("necro/bonephit.wav");	
	precache_sound3 ("necro/bonenhit.wav");	
	precache_sound3 ("necro/bonenwal.wav");	
	//Raven Staff
	precache_sound3 ("raven/ravengo.wav");		
	precache_sound3 ("raven/squawk2.wav");		
	precache_sound3 ("raven/death.wav");
	precache_sound3 ("raven/rfire1.wav");
	precache_sound3 ("raven/rfire2.wav");
	precache_sound3 ("raven/split.wav");
}

void Precache_Female (void)
{
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
}

void Precache_Assassin (void)
{
//ASSASSIN
	precache_model4 ("models/assassin.mdl");//converted for MP
	precache_model ("models/h_ass.mdl");
	//Punch Dagger
	precache_model4("models/punchdgr.mdl");//converted for MP
	//Crossbow
	precache_model4("models/crossbow.mdl");//converted for MP
	precache_model ("models/arrow.mdl");
	precache_model ("models/arrowhit.mdl");
	precache_model ("models/flaming.mdl");
	precache_model ("models/NFarrow.mdl");
	//Grenades
	precache_model4("models/v_assgr.mdl");//converted for MP
	precache_model("models/assgren.mdl");
	//Scarab Staff
	precache_model4("models/scarabst.mdl");//converted for MP
	precache_model("models/scrbstp1.mdl");
	precache_model("models/scrbpbdy.mdl");
	precache_model("models/scrbpwng.mdl");
	precache_model ("models/twspike.mdl");	//Trip wire spike
	// TE_STREAM_CHAIN
	precache_model("models/stchain.mdl");	//Chain- also for Scarab staff (assasin only)
//SOUNDS
//ASSASSIN
//Body sounds
	Precache_Female();
//Weapon sounds
	//Crossbow
	precache_sound ("assassin/arrowfly.wav");	
	precache_sound ("assassin/arr2flsh.wav");	
	precache_sound ("assassin/arr2wood.wav");	
	precache_sound ("assassin/arrowbrk.wav");
	precache_sound ("assassin/firefblt.wav");	
	precache_sound ("assassin/firebolt.wav");	
	//Scarab Staff
	precache_sound ("assassin/build.wav");	
	precache_sound ("assassin/pincer.wav");	
	precache_sound ("assassin/chn2flsh.wav");	
	precache_sound ("assassin/chain.wav");	
//	precache_sound ("assassin/clink.wav");	
	precache_sound ("assassin/scarab.wav");	
	precache_sound ("assassin/scrbfly.wav");	
	precache_sound ("assassin/spin.wav");	
	precache_sound ("assassin/core.wav");	
	precache_sound ("misc/pulse.wav");			//Fully charged staff

}

void Precache_Demoness (void)
{
//SUCCUBUS
	precache_model4 ("models/h_suc.mdl");
	precache_model4 ("models/succubus.mdl");
//Weapon 1
	precache_model4("models/sucwp1.mdl");
	precache_model4("models/sucwp1p.mdl");
	precache_model4("models/xplsn_1.spr");
//	precache_model4("models/xplsn_2.spr");
//	precache_model4("models/xplsn_3.spr");
	precache_model("models/spark.spr");
//Weapon 2
	precache_model4("models/sucwp2.mdl");
	precache_model4("models/sucwp2p.mdl");
	precache_model4("models/muzzle1.spr");
	precache_model4("models/axplsn_1.spr");
	precache_model4("models/axplsn_2.spr");
//	precache_model4("models/axplsn_3.spr");
//	precache_model4("models/axplsn_4.spr");
	precache_model4("models/axplsn_5.spr");
//Weapon 3
	precache_model4("models/sucwp3.mdl");
	precache_model4("models/flamestr.spr"); //demoness
	precache_model4("models/pow.spr");      //demoness
	precache_model4("models/firewal1.spr"); //demoness
	precache_model4("models/firewal2.spr");
	precache_model4("models/firewal3.spr");
	precache_model4("models/fboom.spr");
	precache_model4("models/firewal5.spr");
	precache_model4("models/firewal4.spr");

//Weapon 4
	precache_model4("models/sucwp4.mdl");
	precache_model4("models/lball.mdl");
	precache_model4("models/Bluexp3.spr");
//	precache_model4("models/glowball.mdl");


//SOUNDS
//Body sounds
	Precache_Female();
	precache_sound4 ("succubus/fwoomp.wav");	// Slow fall

//Weapon 1
	precache_sound4 ("succubus/brnbounce.wav");
	precache_sound4 ("succubus/brnfire.wav");
	precache_sound4 ("succubus/brnhit.wav");
	precache_sound4 ("succubus/brnwall.wav");
//Weapon 2
	precache_sound4 ("succubus/acidfire.wav");
	precache_sound4 ("succubus/acidpfir.wav");
	precache_sound4 ("succubus/acidhit.wav");
	precache_sound4 ("succubus/blobexpl.wav");
	precache_sound4 ("succubus/dropfizz.wav");
//Weapon 3
	//precache_sound ("mummy/mislfire.wav");
	//precache_sound ("eidolon/flamend.wav");
	//precache_sound ("misc/fburn_bg.wav");
	//precache_sound ("death/fout.wav");
//	precache_sound4 ("succubus/firecirc.wav");
	precache_sound4 ("succubus/flamend.wav");
	precache_sound4 ("succubus/flamstrt.wav");
	precache_sound4 ("succubus/flampow.wav");
	
//Weapon 4
	precache_sound4 ("succubus/firelght.wav");
	precache_sound4 ("succubus/firelbal.wav");
	precache_sound4 ("succubus/firelpow.wav");
	precache_sound3 ("crusader/lghtn2.wav");
	precache_sound4 ("succubus/buzz.wav");
	precache_sound4 ("succubus/buzz2.wav");
//Glyph
	precache_sound4 ("succubus/gasss.wav");
	precache_sound4 ("succubus/hisss.wav");
	precache_sound4 ("succubus/endhisss.wav");
}

//**********************************************
// ***************  Raven sprites
//**********************************************
void Precache_spr (void)
{//ALPHABETIZED!
	precache_model ("models/s_light.spr");	// id's sphere light	(used by plats and lights still)
//FIXME: Which of these aren't being used anymore?
	precache_model("models/bg_expld.spr");
//	precache_model("models/bldspot1.spr");
//	precache_model("models/bldspot2.spr");
//	precache_model("models/bldspot3.spr");
	precache_model("models/bldspot4.spr");
	precache_model("models/bluflash.spr");
	precache_model("models/bonexpld.spr");
	precache_model("models/bspark.spr");
	precache_model("models/rcloud.spr");

	precache_model("models/fcircle.spr");//mummy and purifier
	precache_model("models/fl_expld.spr");//dthhorse and BecomeImp(Summon)

	precache_model("models/gen_expl.spr");
	precache_model ("models/ghost.spr");
//	precache_model ("gfx/glass.spr");	//create sprite chunks not used
	precache_model("models/grnsmk1.spr");
	precache_model ("models/gryspt.spr");
	precache_model("models/grysmk1.spr");
	precache_model ("models/gspark.spr");

	precache_model("models/icehit.spr");	//Ice mace shot impact
	
	precache_model ("models/medhit.spr");
	precache_model ("models/mezzoref.spr");
	precache_model ("models/mm_expld.spr");

	precache_model ("models/null.spr");		//nothing there

	precache_model("models/polymrph.spr");

	precache_model("models/redsmk1.spr");
	precache_model ("models/redspt.spr");
	precache_model("models/rspark.spr");

//	precache_model ("gfx/s_blast.spr");		//not used anywhere
	precache_model ("models/s_bubble.spr");	// drowning bubbles
//	precache_model ("models/s_explod.spr");	// old ogre
	precache_model ("models/s_light.spr");
	//CE_SM_BLUE_FLASH(CreateLittleBlueFlash)
	precache_model("models/sm_blue.spr");
	precache_model("models/sm_expld.spr");
	precache_model("models/sm_white.spr");
//	precache_model("gfx/smoke1.spr");	//not used anywhere
	precache_model("models/spark.spr");
//	precache_model("models/spark0.spr");	//not used anywhere
//	precache_model ("gfx/stone.spr");	//sprite chunks not used

	//CE_TELESMK1
	precache_model("models/telesmk1.spr");
	precache_model("models/telesmk2.spr");
	precache_model("models/telesmk3.spr");

	precache_model("models/whtsmk1.spr");
//	precache_model("models/wsplash.spr");

	precache_model("models/xbowexpl.spr");	//Crossbow explosion
	precache_model("models/xpspblue.spr");

	precache_model("models/yr_flsh.spr");
	precache_model3("models/rcloud.spr");
}

void precache_archer()
{
	if(self.netname=="monster_archer_ice")
	{
		precache_model4("models/archer2.mdl");
		precache_model4("models/akarrow2.mdl");
		precache_sound4 ("archer/arrowg2.wav");
		precache_sound4 ("archer/growl4.wav");
	}
	else
		precache_model("models/archer.mdl");

	precache_sound ("archer/arrowg.wav");
	precache_sound ("archer/arrowr.wav");

	precache_model("models/archerhd.mdl");

	precache_model("models/gspark.spr");

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
/*
 * $Log: not supported by cvs2svn $
 * 
 * 70    3/19/98 12:17a Mgummelt
 * last bug fixes
 * 
 * 69    3/16/98 3:52p Jmonroe
 * fixed info_masks for load/save changelevel
 * 
 * 68    3/15/98 2:22p Jmonroe
 * dammit!
 * 
 * 67    3/15/98 1:53p Jmonroe
 * dammit
 * 
 * 66    3/14/98 9:24p Mgummelt
 * 
 * 65    3/14/98 5:52p Mgummelt
 * changed precaches to precache 4 for old models converted for MP
 * 
 * 64    3/13/98 3:02a Mgummelt
 * 
 * 63    3/12/98 10:50p Jmonroe
 * 
 * 62    3/12/98 6:33p Jmonroe
 * fixed stupid hexen2 wrong precache for scepter
 * 
 * 61    3/12/98 4:02p Mgummelt
 * 
 * 60    3/10/98 2:25p Jweier
 * 
 * 59    3/10/98 10:56a Mgummelt
 * 
 * 58    3/10/98 10:55a Mgummelt
 * 
 * 57    3/05/98 5:12p Jmonroe
 * changed ball to precache4 to get it in the new pak
 * 
 * 56    3/05/98 2:38p Jmonroe
 * 
 * 55    3/05/98 11:00a Jmonroe
 * 
 * 54    3/03/98 4:45p Jmonroe
 * 
 * 53    3/03/98 4:36p Jmonroe
 * changed over to precache 4 to build my pak
 * 
 * 52    3/03/98 3:32p Jmonroe
 * added the pak builder cmds for the maps and midi
 * 
 * 51    3/02/98 1:18p Jmonroe
 * 
 * 50    3/02/98 12:41p Mgummelt
 * 
 * 49    2/28/98 1:15a Jweier
 * 
 * 48    2/27/98 11:52a Mgummelt
 * 
 * 47    2/26/98 2:04p Mgummelt
 * 
 * 46    2/25/98 6:10p Mgummelt
 * 
 * 45    2/20/98 2:36p Jmonroe
 * precaches the bonebrk sound
 * 
 * 44    2/19/98 12:05p Jmonroe
 * 
 * 43    2/18/98 6:10p Jmonroe
 * 
 * 42    2/18/98 6:02p Jmonroe
 * added cache4 functions, added puzzle piece cache_file4 cmds
 * 
 * 41    2/18/98 1:32p Jmonroe
 * 
 * 40    2/12/98 5:55p Jmonroe
 * remove unreferenced funcs
 * 
 * 39    2/12/98 2:48p Mgummelt
 * 
 * 38    2/11/98 12:57p Jmonroe
 * 
 * 37    2/11/98 11:37a Jmonroe
 * put paladin in his own precache section
 * 
 * 36    2/10/98 8:50p Jmonroe
 * 
 * 35    2/10/98 3:53p Jmonroe
 * smarter precache, removed unused sprites and funcs
 * 
 * 34    2/10/98 2:42p Jmonroe
 * improved precache some more
 * 
 * 33    2/09/98 5:11p Jweier
 * 
 * 32    2/08/98 3:09p Mgummelt
 * 
 * 31    2/07/98 1:28p Jweier
 * 
 * 30    2/06/98 3:47p Mgummelt
 * 
 * 29    2/05/98 11:21p Mgummelt
 * Making weaps network friendly
 * 
 * 28    2/03/98 3:51p Mgummelt
 * 
 * 27    2/03/98 10:56a Mgummelt
 * 
 * 26    2/02/98 1:20p Jmonroe
 * moved some more sprites to specific precache
 * 
 * 25    2/02/98 10:26a Mgummelt
 * 
 * 24    1/29/98 6:34p Mgummelt
 * 
 * 23    1/28/98 3:10p Mgummelt
 * 
 * 22    1/26/98 12:29p Mgummelt
 * 
 * 21    1/22/98 4:05p Mgummelt
 * 
 * 20    1/21/98 10:30a Jweier
 * 
 * 19    1/20/98 11:12a Mgummelt
 * 
 * 18    1/20/98 10:58a Mgummelt
 * 
 * 17    1/20/98 10:31a Mgummelt
 * 
 * 16    1/14/98 7:43p Mgummelt
 * 
 * 15    1/14/98 6:06p Mgummelt
 * 
 * 232   10/29/97 5:39p Jheitzman
 * 
 * 231   10/29/97 11:55a Mgummelt
 * 
 * 230   10/28/97 1:01p Mgummelt
 * Massive replacement, rewrote entire code... just kidding.  Added
 * support for 5th class.
 * 
 * 228   10/03/97 4:06p Rjohnson
 * Updates
 * 
 * 227   9/16/97 4:17p Rjohnson
 * Updates
 * 
 * 226   9/11/97 7:13p Rjohnson
 * Caching Updates
 * 
 * 225   9/09/97 2:31p Rjohnson
 * Updates
 * 
 * 224   9/03/97 4:09a Mgummelt
 * 
 * 223   9/03/97 2:32a Mgummelt
 * 
 * 222   9/02/97 1:58p Rlove
 * 
 * 221   9/01/97 6:42p Rlove
 * 
 * 220   9/01/97 4:31p Rjohnson
 * Stuff
 * 
 * 219   9/01/97 5:44a Jweier
 * 
 * 218   9/01/97 4:11a Rjohnson
 * Updates
 * 
 * 217   8/31/97 12:54p Jweier
 * 
 * 216   8/31/97 12:44p Rjohnson
 * Updates
 * 
 * 215   8/28/97 8:55p Mgummelt
 * 
 * 214   8/28/97 8:52p Rjohnson
 * Updates
 * 
 * 213   8/26/97 3:01p Rlove
 * 
 * 212   8/26/97 10:23a Rlove
 * 
 * 211   8/26/97 9:11a Rlove
 * 
 * 210   8/26/97 9:05a Rlove
 * 
 * 209   8/26/97 8:42a Mgummelt
 * 
 * 208   8/26/97 8:37a Mgummelt
 * 
 * 207   8/26/97 8:34a Rjohnson
 * Updates
 * 
 * 206   8/22/97 9:51p Mgummelt
 * 
 * 205   8/22/97 2:18p Jweier
 * 
 * 204   8/22/97 2:16p Rjohnson
 * Updates
 * 
 * 203   8/20/97 10:50p Rlove
 * Update
 * 
 * 202   8/20/97 2:58p Rjohnson
 * Change of precache for strings
 * 
 * 201   8/20/97 2:34p Rjohnson
 * Change for precache of fireball
 * 
 * 200   8/19/97 11:59p Rjohnson
 * Updates
 * 
 * 199   8/19/97 8:36p Rjohnson
 * Precache Update
 * 
 * 198   8/19/97 7:13p Rjohnson
 * Precache update
 * 
 * 197   8/18/97 4:47p Rlove
 * 
 * 196   8/18/97 4:14p Mgummelt
 * 
 * 195   8/18/97 3:01p Rjohnson
 * Fix for precaching
 * 
 * 194   8/18/97 11:16a Rjohnson
 * Added rankings
 * 
 * 193   8/17/97 11:47p Rjohnson
 * Added puzzle item
 * 
 * 192   8/17/97 1:37p Mgummelt
 * 
 * 191   8/17/97 12:22p Rjohnson
 * Fixed precache
 * 
 * 190   8/16/97 5:47p Mgummelt
 * 
 * 189   8/16/97 5:23p Mgummelt
 * 
 * 188   8/16/97 10:51a Rjohnson
 * Precache update
 * 
 * 187   8/15/97 8:10p Mgummelt
 * 
 * 186   8/15/97 5:29p Rjohnson
 * Updates
 * 
 * 185   8/15/97 3:00p Rjohnson
 * Updates
 * 
 * 184   8/14/97 7:48a Rlove
 * 
 * 183   8/13/97 3:39p Rlove
 * 
 * 182   8/13/97 1:48p Rlove
 * 
 * 181   8/13/97 12:55p Rlove
 * added a raven sound
 * 
 * 180   8/13/97 12:08p Mgummelt
 * 
 * 179   8/13/97 12:05p Mgummelt
 * 
 * 
 * 178   8/13/97 12:04p Rjohnson
 * Precache adjustments
 * 
 * 177   8/11/97 10:54a Rlove
 * 
 * 176   8/09/97 11:14a Rlove
 * 
 * 175   8/09/97 1:58a Mgummelt
 * 
 * 174   8/09/97 1:54a Mgummelt
 * 
 * 172   8/05/97 6:47p Mgummelt
 * 
 * 171   8/01/97 5:45p Rlove
 * 
 * 170   7/31/97 4:58p Mgummelt
 * 
 * 169   7/31/97 4:57p Mgummelt
 * 
 * 168   7/30/97 4:39p Mgummelt
 * 
 * 167   7/30/97 3:35p Mgummelt
 * 
 * 165   7/29/97 4:48p Rlove
 * 
 * 164   7/28/97 8:27p Mgummelt
 * 
 * 163   7/28/97 7:50p Mgummelt
 * 
 * 162   7/25/97 4:04p Mgummelt
 * 
 * 161   7/25/97 11:36a Rjohnson
 * 
 * 160   7/24/97 12:32p Mgummelt
 * 
 * 159   7/24/97 3:27a Mgummelt
 * 
 * 158   7/23/97 2:53p Rlove
 * 
 * 157   7/22/97 5:26p Bgokey
 * 
 * 156   7/22/97 11:24a Bgokey
 * 
 * 155   7/21/97 2:02p Bgokey
 * 
 * 154   7/21/97 1:28p Rlove
 * Added some puzzle models
 * 
 * 153   7/19/97 9:57p Mgummelt
 * 
 * 152   7/18/97 2:25p Rlove
 * 
 * 151   7/18/97 11:06a Mgummelt
 * 
 * 150   7/17/97 1:53p Rlove
 * 
 * 149   7/15/97 9:19p Mgummelt
 * 
 * 148   7/15/97 8:44p Mgummelt
 * 
 * 147   7/15/97 2:31p Mgummelt
 * 
 * 146   7/15/97 11:20a Mgummelt
 * 
 * 145   7/14/97 4:11p Mgummelt
 * 
 * 144   7/14/97 10:06a Rjohnson
 * Added more puzzle pieces
 * 
 * 143   7/11/97 12:34p Rjohnson
 * Added a puzzle precache, changed puzzle precaching, added a parameter
 * to the call back for client reentering a level
 * 
 * 142   7/09/97 3:23p Bgokey
 * 
 * 141   7/09/97 11:53a Mgummelt
 * 
 * 140   7/07/97 11:12a Mgummelt
 * 
 * 139   7/02/97 4:07p Mgummelt
 * 
 * 138   7/01/97 6:28p Rlove
 * 
 * 137   7/01/97 5:23p Rjohnson
 * More file caching
 * 
 * 136   7/01/97 5:17p Rjohnson
 * Removed water splashes
 * 
 * 135   7/01/97 5:06p Rlove
 * 
 * 134   7/01/97 3:55p Mgummelt
 * 
 * 133   7/01/97 3:50p Bgokey
 * 
 * 132   6/30/97 8:01p Mgummelt
 * 
 * 131   6/30/97 7:22p Rlove
 * 
 * 130   6/30/97 7:09p Rlove
 * More puzzle models
 * 
 * 129   6/30/97 4:52p Mgummelt
 * 
 * 128   6/28/97 6:32p Mgummelt
 * 
 * 127   6/27/97 5:37p Mgummelt
 * 
 * 126   6/27/97 3:40p Rlove
 * 
 * 125   6/27/97 2:37p Rlove
 * 
 * 124   6/27/97 10:44a Mgummelt
 * 
 * 123   6/27/97 10:18a Rlove
 * Monsters drop stuff on death
 * 
 * 122   6/26/97 11:58p Rjohnson
 * Added cube
 * 
 * 121   6/26/97 4:56p Rlove
 * 
 * 120   6/26/97 4:46p Rlove
 * 
 * 119   6/26/97 7:36a Rlove
 * Changed Vindictus to Ravenstaff
 * 
 * 118   6/25/97 3:00p Mgummelt
 * 
 * 117   6/25/97 10:08a Rlove
 * 
 * 116   6/19/97 5:55p Jweier
 * 
 * 115   6/19/97 12:10p Jweier
 * 
 * 114   6/16/97 4:05p Rlove
 * 
 * 113   6/16/97 2:55p Rlove
 * New explosion sounds added
 * 
 * 112   6/16/97 10:04a Rlove
 * 
 * 111   6/16/97 8:04a Rlove
 * Fixed null.wav error, also paladin weapons deplete mana now
 * 
 * 110   6/14/97 2:22p Rjohnson
 * Precached more puzzle pieces
 * 
 * 109   6/13/97 6:08p Rlove
 * 
 * 108   6/13/97 10:51a Rlove
 * Moved some precache code
 * 
 * 107   6/13/97 10:11a Rlove
 * Moved all message.hc to strings.hc
 * 
 * 106   6/12/97 4:32p Rlove
 * 
 * 105   6/12/97 12:13p Rlove
 * Archer arrows generate red or green sparks
 * 
 * 104   6/10/97 4:02p Rlove
 * 
 * 103   6/09/97 3:36p Rlove
 * 
 * 102   6/07/97 3:35p Rlove
 * Added water splash animation.  It ain't done yet.
 * 
 * 101   6/06/97 4:46p Rlove
 * Now using just the generic weapon artifacts.
 * 
 * 100   6/04/97 8:16p Mgummelt
 * 
 * 99    6/04/97 8:41a Rlove
 * New weapon artifact models
 * 
 * 98    6/03/97 9:00a Rlove
 * Added fx_smoke_generator entity
 * 
 * 97    6/03/97 7:59a Rlove
 * Change take_art.wav to artpkup.wav
 * 
 * 96    6/03/97 7:41a Rlove
 * Added ring of absorption model
 * 
 * 95    6/02/97 9:30a Rlove
 * Added haste blur to player model, changed chase camera - but will
 * change it back once testing is done
 * 
 * 94    5/30/97 3:46p Rlove
 * Added smoke ring
 * 
 * 93    5/30/97 12:01p Rlove
 * New blue explosion
 * 
 * 92    5/29/97 9:23p Mgummelt
 * 
 * 91    5/29/97 4:24p Rlove
 * Added axe tail model
 * 
 * 90    5/29/97 11:28a Mgummelt
 * 
 * 89    5/28/97 8:25p Mgummelt
 * 
 * 87    5/28/97 10:45a Rlove
 * Moved sprite effects to client side - smoke, explosions, and flashes.
 * 
 * 86    5/27/97 4:59p Rlove
 * Taking out Id sounds
 * 
 * 85    5/27/97 10:58a Rlove
 * Took out old Id sound files
 * 
 * 84    5/27/97 7:58a Rlove
 * New thingtypes of GreyStone,BrownStone, and Cloth.
 * 
 * 83    5/24/97 2:48p Rlove
 * Taking out old Id sounds
 * 
 * 82    5/23/97 4:17p Rlove
 * Getting rid of Quake sounds
 * 
 * 79    5/23/97 12:22p Bgokey
 * 
 * 78    5/23/97 11:56a Rlove
 * 
 * 77    5/22/97 5:18p Rlove
 * New door sounds
 * 
 * 76    5/22/97 10:28a Rlove
 * Added fire circle fx
 * 
 * 75    5/21/97 3:34p Rlove
 * New chunks
 * 
 * 74    5/21/97 1:25p Rlove
 * 
 * 73    5/20/97 9:32p Mgummelt
 * 
 * 72    5/19/97 11:36p Mgummelt
 * 
 * 71    5/19/97 12:43p Mgummelt
 * 
 * 70    5/19/97 12:07p Mgummelt
 * 
 * 69    5/19/97 12:01p Rlove
 * New sprites for axe
 * 
 * 68    5/19/97 10:56a Rlove
 * 
 * 67    5/19/97 8:58a Rlove
 * Adding sprites and such to the axe.
 * 
 * 66    5/16/97 1:52p Rlove
 * 
 * 65    5/15/97 8:28p Mgummelt
 * 
 * 64    5/15/97 6:34p Rjohnson
 * Code cleanup
 * 
 * 63    5/15/97 1:33p Rlove
 * 
 * 62    5/15/97 12:30a Mgummelt
 * 
 * 61    5/13/97 3:52p Mgummelt
 * 
 * 60    5/13/97 9:55a Rlove
 * 
 * 59    5/12/97 7:46a Rlove
 * For the bell animations
 * 
 * 58    5/09/97 7:43a Rlove
 * 
 * 57    5/07/97 4:09p Rlove
 * 
 * 56    5/07/97 11:03a Rlove
 * 
 * 55    5/06/97 5:18p Rjohnson
 * Added the precache stuff for the existing puzzle models
 * 
 * 54    5/06/97 9:12a Rlove
 * Added thingtype_leaves
 * 
 * 53    5/05/97 5:40p Rlove
 * 
 * 52    5/05/97 10:29a Mgummelt
 * 
 * 51    5/03/97 3:23p Rlove
 * 
 * 50    5/03/97 12:59p Rlove
 * 
 * 49    5/02/97 8:06p Mgummelt
 * 
 * 48    5/02/97 8:05a Rlove
 * 
 * 47    5/01/97 8:52p Mgummelt
 * 
 * 46    5/01/97 5:06p Rlove
 * New camera
 * 
 * 45    4/30/97 5:02p Mgummelt
 * 
 * 44    4/28/97 6:53p Mgummelt
 * 
 * 43    4/28/97 10:17a Rlove
 * New artifacts and items
 * 
 * 42    4/26/97 6:23p Mgummelt
 * 
 * 41    4/26/97 3:52p Mgummelt
 * 
 * 38    4/25/97 8:32p Mgummelt
 * 
 * 37    4/24/97 10:00p Rjohnson
 * Fixed problem with precache and spawning artifacts
 * 
 * 36    4/24/97 8:48p Mgummelt
 * 
 * 35    4/24/97 2:22p Mgummelt
 * 
 * 34    4/21/97 8:47p Mgummelt
 * 
 * 33    4/21/97 12:31p Mgummelt
 * 
 * 32    4/18/97 5:24p Mgummelt
 * 
 * 31    4/18/97 11:16a Rlove
 * Added smoke sprite
 * 
 * 30    4/18/97 7:01a Rlove
 * Added new gib models
 * 
 * 29    4/17/97 4:11p Mgummelt
 * 
 * 28    4/17/97 4:10p Mgummelt
 * 
 * 27    4/17/97 2:50p Mgummelt
 * 
 * 26    4/16/96 11:52p Mgummelt
 * 
 * 25    4/15/97 8:59a Rlove
 * Pulled out old Id wav
 * 
 * 24    4/14/97 3:29p Rlove
 * Precaching a wav
 * 
 * 23    4/14/96 2:36p Mgummelt
 * 
 * 22    4/13/96 4:05p Mgummelt
 * 
 * 21    4/13/96 3:30p Mgummelt
 * 
 * 20    4/12/96 8:56p Mgummelt
 * 
 * 19    4/12/97 3:20p Rlove
 * Fixed problem with assassin grenade model.
 * 
 * 18    4/12/96 9:01a Mgummelt
 * 
 * 17    4/11/97 7:33p Mgummelt
 * 
 * 16    4/11/97 7:30p Mgummelt
 * 
 * 15    4/11/96 1:04p Mgummelt
 * 
 * 14    4/11/97 12:38a Mgummelt
 * 
 * 13    4/10/96 3:29p Mgummelt
 * 
 * 12    4/10/97 2:14p Rlove
 * Some tweaking of gauntlets and vorpal sword.
 */
