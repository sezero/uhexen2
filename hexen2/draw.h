/*
	draw.h
	these are the only functions outside the refresh
	allowed to touch the vid buffer

	$Id: draw.h,v 1.19 2007-02-06 12:23:37 sezero Exp $
*/

#ifndef __HX2_DRAW_H
#define __HX2_DRAW_H

#define MAX_DISC 18

void Draw_Init (void);
void Draw_ReInit (void);
void Draw_Character (int x, int y, unsigned int num);
void Draw_Pic (int x, int y, qpic_t *pic);
void Draw_AlphaPic (int x, int y, qpic_t *pic, float alpha);
void Draw_IntermissionPic (qpic_t *pic);
void Draw_PicCropped (int x, int y, qpic_t *pic);
void Draw_SubPic (int x, int y, qpic_t *pic, int srcx, int srcy, int width, int height);
void Draw_SubPicCropped (int x, int y, int h, qpic_t *pic);
void Draw_TransPic (int x, int y, qpic_t *pic);
void Draw_TransPicCropped (int x, int y, qpic_t *pic);
#ifndef GLQUAKE
void Draw_TransPicTranslate (int x, int y, qpic_t *pic, byte *translation);
#else
void Draw_TransPicTranslate (int x, int y, qpic_t *pic, byte *translation, int p_class, int top, int bottom);
#endif
void Draw_ConsoleBackground (int lines);
void Draw_ChangeConsize (void);
void Draw_Crosshair (void);
#ifndef GLQUAKE
void Draw_BeginDisc (void);
void Draw_EndDisc (void);
#endif
void Draw_TileClear (int x, int y, int w, int h);
void Draw_Fill (int x, int y, int w, int h, int c);
void Draw_FadeScreen (void);
void Draw_String (int x, int y, char *str);
void Draw_SmallCharacter (int x, int y, int num);
void Draw_SmallString (int x, int y, char *str);
void Draw_RedString (int x, int y, char *str);
qpic_t *Draw_PicFromWad (char *name);
qpic_t *Draw_CachePic (const char *path);
qpic_t *Draw_CachePicNoTrans (const char *path);
qpic_t *Draw_CachePicResize (const char *path, int targetWidth, int targetHeight);

// game/engine name to draw on the console
#define GAME_MOD_NAME		ENGINE_NAME
#define ENGINE_WATERMARK	GAME_MOD_NAME " " STRINGIFY(ENGINE_VERSION) " (" VERSION_PLATFORM ")"

#endif	/* __HX2_DRAW_H */

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.18  2007/02/02 18:33:37  sezero
 * draw.h, draw.c, gl_draw.c: small whitespace/readability cleanup
 *
 * Revision 1.17  2006/09/24 17:28:42  sezero
 * protected all headers against multiple inclusion
 *
 * Revision 1.16  2006/08/14 06:42:29  sezero
 * introduced Draw_ReInit() to be called when a gamedir change occurs.
 * reloads the textures which are loaded back at the init phase. fixes
 * hexenworld's wrong charset and conback display problem upon gamedir
 * changes.
 *
 * Revision 1.15  2006/08/14 06:07:35  sezero
 * exported Draw_AlphaPic() for possible future users
 *
 * Revision 1.14  2006/07/27 13:46:52  sezero
 * made scaling of the effective console size (the -conwidth commandline
 * argument) adjustable from the menu system, using the std_modes array
 * for the size to be emulated. made the user's choice to be remembered
 * via the config. deprecated the -conheight argument and used the same
 * width/height ratio as in the actual resolution.
 *
 * Revision 1.13  2006/07/18 08:38:20  sezero
 * made draw_disc static. unlike quake, it isn't shared with sbar.
 *
 * Revision 1.12  2006/05/19 11:32:54  sezero
 * misc clean-up
 *
 * Revision 1.11  2005/12/11 11:53:12  sezero
 * added menu.c arguments to gl version of Draw_TransPicTranslate, and
 * macroized M_DrawTransPicTranslate accordingly. this synchronizes h2
 * and h2w versions of gl_draw.c
 *
 * Revision 1.10  2005/10/25 20:04:17  sezero
 * static functions part-1: started making local functions static,
 * killing nested externs, const vars clean-up.
 *
 * Revision 1.9  2005/10/25 19:59:44  sezero
 * added a prototype for Draw_Crosshair to draw.h
 *
 * Revision 1.8  2005/10/25 19:58:33  sezero
 * killed Draw_DebugChar
 *
 * Revision 1.7  2005/10/25 17:14:22  sezero
 * added a STRINGIFY macro. unified version macros. simplified version
 * printing. simplified and enhanced version watermark print onto console
 * background. added HoT lines to the quit menu (shameless plug)
 *
 * Revision 1.6  2005/07/16 23:35:19  sezero
 * added transparent sbar of hexenworld to hexen2 for software mode.
 * style fixes in draw.c, draw.h, sbar.c, sbar.h. tiny synchronization.
 *
 * Revision 1.5  2005/06/07 20:30:49  sezero
 * More syncing: software version draw.c between hexen2/hexenworld
 *
 * Revision 1.4  2005/05/21 17:32:03  sezero
 * disabled the rotating skull annoyance in GL mode (used to
 * cause problems with voodoo1/mesa6 when using gamma tricks)
 *
 * Revision 1.3  2004/12/12 14:14:42  sezero
 * style changes to our liking
 *
 * Revision 1.2  2004/11/29 12:17:46  sezero
 * draw fullscreen intermission pics. borrowed from Pa3PyX sources.
 *
 * Revision 1.1.1.1  2004/11/28 00:02:40  sezero
 * Initial import of AoT 1.2.0 code
 *
 * 5     8/20/97 2:05p Rjohnson
 * fix for internationalization
 * 
 * 4     6/15/97 7:44p Rjohnson
 * Added new pause and loading graphics
 * 
 * 3     2/19/97 11:25a Rjohnson
 * Id Updates
 */
