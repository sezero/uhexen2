/*
	draw.h
	these are the only functions outside the refresh
	allowed to touch the vid buffer

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/draw.h,v 1.3 2004-12-12 14:14:42 sezero Exp $
*/


#define MAX_DISC 18

extern	qpic_t		*draw_disc[MAX_DISC]; // also used on sbar

void Draw_Init (void);
void Draw_Character (int x, int y, unsigned int num);
void Draw_DebugChar (char num);
void Draw_Pic (int x, int y, qpic_t *pic);
void Draw_IntermissionPic (qpic_t *pic);
void Draw_PicCropped(int x, int y, qpic_t *pic);
void Draw_TransPic (int x, int y, qpic_t *pic);
void Draw_TransPicCropped(int x, int y, qpic_t *pic);
void Draw_TransPicTranslate (int x, int y, qpic_t *pic, byte *translation);
void Draw_ConsoleBackground (int lines);
void Draw_BeginDisc (void);
void Draw_EndDisc (void);
void Draw_TileClear (int x, int y, int w, int h);
void Draw_Fill (int x, int y, int w, int h, int c);
void Draw_FadeScreen (void);
void Draw_String (int x, int y, char *str);
void Draw_SmallCharacter(int x, int y, int num);
void Draw_SmallString(int x, int y, char *str);
qpic_t *Draw_PicFromWad (char *name);
qpic_t *Draw_CachePic (char *path);
qpic_t *Draw_CachePicNoTrans (char *path);
qpic_t *Draw_CachePicResize (char *path, int targetWidth, int targetHeight);

/*
 * $Log: not supported by cvs2svn $
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
