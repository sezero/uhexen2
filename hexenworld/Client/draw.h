/*
	draw.h
	these are the only functions outside the refresh
	allowed to touch the vid buffer

	$Id: draw.h,v 1.8 2005-10-25 19:58:33 sezero Exp $
*/


#define MAX_DISC 18

extern	qpic_t		*draw_disc[MAX_DISC]; // also used on sbar

void Draw_Init (void);
void Draw_Character (int x, int y, unsigned int num);
void Draw_Pic (int x, int y, qpic_t *pic);
void Draw_IntermissionPic (qpic_t *pic);
void Draw_PicCropped(int x, int y, qpic_t *pic);
void Draw_SubPic(int x, int y, qpic_t *pic, int srcx, int srcy, int width, int height);
void Draw_SubPicCropped(int x, int y, int h, qpic_t *pic);
void Draw_TransPic (int x, int y, qpic_t *pic);
void Draw_TransPicCropped(int x, int y, qpic_t *pic);
void Draw_TransPicTranslate (int x, int y, qpic_t *pic, byte *translation);
void Draw_ConsoleBackground (int lines);
#ifndef GLQUAKE
void Draw_BeginDisc (void);
void Draw_EndDisc (void);
#endif
void Draw_TileClear (int x, int y, int w, int h);
void Draw_Fill (int x, int y, int w, int h, int c);
void Draw_FadeScreen (void);
void Draw_String (int x, int y, char *str);
void Draw_SmallCharacter(int x, int y, int num);
void Draw_SmallString(int x, int y, char *str);
void Draw_RedString (int x, int y, char *str);
qpic_t *Draw_PicFromWad (char *name);
qpic_t *Draw_CachePic (char *path);
qpic_t *Draw_CachePicNoTrans (char *path);
qpic_t *Draw_CachePicResize (char *path, int targetWidth, int targetHeight);

// game/engine name to draw on the console
#if defined(H2MP)
#define GAME_MOD_NAME		"H2mp"
#else
#define GAME_MOD_NAME		ENGINE_NAME
#endif
#define ENGINE_WATERMARK	GAME_MOD_NAME " " STRINGIFY(ENGINE_VERSION) " (" VERSION_PLATFORM ")"

