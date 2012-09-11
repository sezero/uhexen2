/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/precache.hc,v 1.3 2007-02-07 16:59:35 sezero Exp $
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
	precache_file ("gfx/cport2.lmp");
	precache_file ("gfx/cport3.lmp");
	precache_file ("gfx/cport4.lmp");
	precache_file4 ("gfx/cport5.lmp");

	//player colors
	precache_file ("gfx/menu/netp1.lmp");
	precache_file ("gfx/menu/netp2.lmp");
	precache_file ("gfx/menu/netp3.lmp");
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
	precache_sound ("raven/fire1.wav");	// player torch burning

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
	precache_sound ("doors/baddoor.wav");	// Bad attempt to open a door
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
	precache_sound ("misc/warning.wav");	//glyph about to explode
	precache_sound ("golem/gbfire.wav");	//force cube
	precache_sound4 ("misc/cubehum.wav");	//force cube
	precache_sound4 ("misc/Beep1.wav");	//force cube
	//Summoning Stone
	precache_sound ("imp/upbig.wav");
	precache_sound ("imp/diebig.wav");
	precache_sound ("imp/swoophit.wav");
	precache_sound ("imp/swoopbig.wav");
	precache_sound ("imp/flybig.wav");
	precache_sound ("imp/fireball.wav");
	precache_sound ("imp/shard.wav");
	precache_sound ("hydra/turn-s.wav");

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
	precache_sound ("raven/soul.wav");			// noise the soul sphere makes

//FX
	precache_sound ("fx/quake.wav");	

//CRUSADER
	//Warhammer
	precache_sound ("crusader/lghtn1.wav");		//SHARED! (weather)
	precache_sound ("crusader/lghtn2.wav");		//SHARED! (weather)
	precache_sound ("raven/lightng1.wav");		//SHARED! (weather)
	//Ice Staff
	precache_sound ("crusader/icehit.wav");		//SHARED? (ice shatter)
	precache_sound ("crusader/frozen.wav");		//SHARED! (Monster unfreezing)
	precache_sound ("misc/icestatx.wav");		//SHARED! Ice statue breaking
	//Meteor Staff
	precache_sound ("misc/rubble.wav");		//SHARED chunks, Meteor bits fall, stoned player bits fall (from Medusa)
	//Sunstaff
	precache_sound ("crusader/sunhum.wav");		//SHARED! (weather)
	precache_sound ("crusader/sunhit.wav");		//SHARED! (weather)

//NECROMANCER
	//Magic Missiles
	precache_sound ("necro/mmfire.wav");		//SHARED! (Eidolon)
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
	precache_model ("models/h_imp.mdl");//empty for now
	precache_model ("models/shardice.mdl");
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
	precache_model ("models/stsunsf1.mdl");	//Sunbeam and ball models
	precache_model ("models/stsunsf2.mdl");	//(weather_sunbeam,sunstaff)
	precache_model ("models/stsunsf3.mdl");
	precache_model ("models/stsunsf4.mdl");
	precache_model ("models/stsunsf5.mdl");

	// TE_STREAM_LIGHNING
	precache_model ("models/stlghtng.mdl");	//Lightning- (warhammer, eidolon, lightwp)
	precache_model4	("models/stltng2.mdl");	//Lightning- (warhammer, eidolon, lightwp)

	// TE_STREAM_COLORBEAM
	precache_model ("models/stclrbm.mdl");	//Colored beams of light (weather,golem,fangel,buddha)

	precache_model("models/glyph.mdl");		//Non-artifact flagged glyph

//Miscellaneous Shared
	precache_model ("models/test.mdl");	//For testing

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
	precache_model ("models/h_cru.mdl");
	//Warhammer
	precache_model4("models/warhamer.mdl");//converted for MP
	precache_model ("models/hamthrow.mdl");
	//Ice Staff
	precache_model4("models/icestaff.mdl");//converted for MP
	precache_model ("models/iceshot1.mdl");
	precache_model ("models/iceshot2.mdl");
	precache_model ("models/stice.mdl");		//For blizzard  (used by crusader only)
	//Meteor Staff
	precache_model ("models/meteor.mdl");
	precache_model ("models/tempmetr.mdl");//temp- meteor projectile
	precache_model4("models/tornato.mdl");//converted for MP
	precache_model4("models/funnal.mdl");//converted for MP
	//Sunstaff
	precache_model ("models/sunstaff.mdl");
	//Crusader
	precache_model ("models/goodsphr.mdl");//Smiting Sphere
	precache_model ("models/cross.mdl");
//SOUNDS
//Body Sounds
	Precache_Male ();
	//Ice Staff
	precache_sound ("crusader/icewall.wav");	
	precache_sound ("crusader/icefire.wav");	
	precache_sound ("misc/tink.wav");				//Ice shots bounce
	precache_sound ("crusader/blizfire.wav");	
	precache_sound ("crusader/blizzard.wav");	
	//Meteor Staff
	precache_sound ("crusader/metfire.wav");	
	precache_sound ("crusader/torngo.wav");	
	precache_sound ("crusader/tornado.wav");	
	//Sunstaff
	precache_sound ("crusader/sunstart.wav");	
}

//REGISTERED=======================================================

void Precache_Necromancer (void)
{
//NECROMANCER
	precache_model4("models/necro.mdl");//converted for MP
	precache_model ("models/h_nec.mdl");
	//Sickle
	precache_model4("models/sickle.mdl");//converted for MP		// Necromancer Weapons
	//Magic Missiles
	precache_model ("models/spllbook.mdl");
	precache_model ("models/handfx.mdl");
	//Bone Shards
//	precache_model ("models/bonefx.mdl");
	precache_model ("models/boneshot.mdl");
	precache_model ("models/boneshrd.mdl");
	precache_model ("models/bonelump.mdl");
	//Raven Staff
	precache_model4("models/ravenstf.mdl");//converted for MP
	precache_model ("models/vindsht1.mdl");
	precache_model ("models/ravproj.mdl");
	precache_model ("models/birdmsl2.mdl");
	//Necromancer
	precache_model ("models/soulball.mdl");//Soul sphere
	precache_model ("models/soulskul.mdl");
	precache_model ("models/birdmisl.mdl");
//SOUNDS
//Body Sounds
	Precache_Male ();
	//Sickle
	precache_sound ("weapons/drain.wav");
	//Bone Shards
	precache_sound ("necro/bonefpow.wav");	
	precache_sound ("necro/bonefnrm.wav");	
	precache_sound ("necro/bonephit.wav");	
	precache_sound ("necro/bonenhit.wav");	
	precache_sound ("necro/bonenwal.wav");	
	//Raven Staff
	precache_sound ("raven/ravengo.wav");		
	precache_sound ("raven/squawk2.wav");		
	precache_sound ("raven/death.wav");
	precache_sound ("raven/rfire1.wav");
	precache_sound ("raven/rfire2.wav");
	precache_sound ("raven/split.wav");
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
	precache_model4("models/flamestr.spr");	//demoness
	precache_model4("models/pow.spr");	//demoness
	precache_model4("models/firewal1.spr");	//demoness
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

