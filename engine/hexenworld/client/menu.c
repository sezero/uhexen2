/*
 * menu.c
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997-1998  Raven Software Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "quakedef.h"
#include "bgmusic.h"
#include "cdaudio.h"
#include "r_shared.h"

void (*vid_menudrawfn)(void);
void (*vid_menukeyfn)(int key);

enum m_state_e	m_state;

void M_Menu_Main_f (void);
	static void M_Menu_MultiPlayer_f (void);
		static void M_Menu_Setup_f (void);
	void M_Menu_Options_f (void);
		static void M_Menu_Keys_f (void);
		static void M_Menu_Video_f (void);
	static void M_Menu_Help_f (void);
	void M_Menu_Quit_f (void);

static void M_Main_Draw (void);
	static void M_MultiPlayer_Draw (void);
		static void M_Setup_Draw (void);
	static void M_Options_Draw (void);
		static void M_Keys_Draw (void);
		static void M_Video_Draw (void);
	static void M_Help_Draw (void);
	static void M_Quit_Draw (void);

static void M_Main_Key (int key);
	static void M_MultiPlayer_Key (int key);
		static void M_Menu_Connect_f (void);
		static void M_Setup_Key (int key);
	static void M_Options_Key (int key);
		static void M_Keys_Key (int key);
		static void M_Video_Key (int key);
	static void M_Help_Key (int key);
	static void M_Quit_Key (int key);

static qboolean	m_entersound;		// play after drawing a frame, so caching
					// won't disrupt the sound
static qboolean	m_recursiveDraw;

qboolean	menu_disabled_mouse = false;

static float TitlePercent = 0;
static float TitleTargetPercent = 1;
static float LogoPercent = 0;
static float LogoTargetPercent = 1;

static int	setup_class, which_class;

static const char	*message,*message2;
static double message_time;

static const char *ClassNames[MAX_PLAYER_CLASS] =
{
	"Paladin",
	"Crusader",
	"Necromancer",
	"Assassin",
	"Succubus",
	"Dwarf"
};

//=============================================================================
/* Support Routines */

/*
================
M_DrawCharacter

Draws one solid graphics character
================
*/
void M_DrawCharacter (int cx, int line, int num)
{
	Draw_Character ( cx + ((vid.width - 320)>>1), line, num);
}

void M_Print (int cx, int cy, const char *str)
{
	while (*str)
	{
		M_DrawCharacter (cx, cy, ((unsigned char)(*str))+256);
		str++;
		cx += 8;
	}
}

void M_PrintWhite (int cx, int cy, const char *str)
{
	while (*str)
	{
		M_DrawCharacter (cx, cy, (unsigned char)*str);
		str++;
		cx += 8;
	}
}

void M_DrawTransPic (int x, int y, qpic_t *pic)
{
	Draw_TransPic (x + ((vid.width - 320)>>1), y, pic);
}

void M_DrawPic (int x, int y, qpic_t *pic)
{
	Draw_Pic (x + ((vid.width - 320)>>1), y, pic);
}

static void M_DrawTransPicCropped (int x, int y, qpic_t *pic)
{
	Draw_TransPicCropped (x + ((vid.width - 320)>>1), y, pic);
}

static byte identityTable[256];
static byte translationTable[256];

static void M_BuildTranslationTable(int top, int bottom)
{
	int		j;
	byte	*dest, *source, *sourceA, *sourceB, *colorA, *colorB;

	for (j = 0; j < 256; j++)
		identityTable[j] = j;
	dest = translationTable;
	source = identityTable;
	memcpy (dest, source, 256);

	if (top > 10)
		top = 0;
	if (bottom > 10)
		bottom = 0;

	top -= 1;
	bottom -= 1;

	colorA = playerTranslation + 256 + color_offsets[(int)which_class-1];
	colorB = colorA + 256;
	sourceA = colorB + 256 + (top * 256);
	sourceB = colorB + 256 + (bottom * 256);
	for (j = 0; j < 256; j++, colorA++, colorB++, sourceA++, sourceB++)
	{
		if (top >= 0 && (*colorA != 255))
			dest[j] = source[*sourceA];
		if (bottom >= 0 && (*colorB != 255))
			dest[j] = source[*sourceB];
	}
}


void M_DrawTextBox (int x, int y, int width, int lines)
{
	qpic_t	*p;
	int		cx, cy;
	int		n;

	// draw left side
	cx = x;
	cy = y;
	p = Draw_CachePic ("gfx/box_tl.lmp");
	M_DrawTransPic (cx, cy, p);
	p = Draw_CachePic ("gfx/box_ml.lmp");
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		M_DrawTransPic (cx, cy, p);
	}
	p = Draw_CachePic ("gfx/box_bl.lmp");
	M_DrawTransPic (cx, cy+8, p);

	// draw middle
	cx += 8;
	while (width > 0)
	{
		cy = y;
		p = Draw_CachePic ("gfx/box_tm.lmp");
		M_DrawTransPic (cx, cy, p);
		p = Draw_CachePic ("gfx/box_mm.lmp");
		for (n = 0; n < lines; n++)
		{
			cy += 8;
			if (n == 1)
				p = Draw_CachePic ("gfx/box_mm2.lmp");
			M_DrawTransPic (cx, cy, p);
		}
		p = Draw_CachePic ("gfx/box_bm.lmp");
		M_DrawTransPic (cx, cy+8, p);
		width -= 2;
		cx += 16;
	}

	// draw right side
	cy = y;
	p = Draw_CachePic ("gfx/box_tr.lmp");
	M_DrawTransPic (cx, cy, p);
	p = Draw_CachePic ("gfx/box_mr.lmp");
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		M_DrawTransPic (cx, cy, p);
	}
	p = Draw_CachePic ("gfx/box_br.lmp");
	M_DrawTransPic (cx, cy+8, p);
}

//=============================================================================

static int m_save_demonum;

/*
================
M_ToggleMenu_f
================
*/
void M_ToggleMenu_f (void)
{
	keydest_t dest = Key_GetDest();

	m_entersound = true;

	if (dest & key_menu)
	{
		if (m_state != m_main)
		{
			LogoTargetPercent = TitleTargetPercent = 1;
			LogoPercent = TitlePercent = 0;
			M_Menu_Main_f ();
			return;
		}
		Key_SetDest (key_game);
		m_state = m_none;
		return;
	}
	if (dest == key_console && cls.state == ca_active)
	{
		Con_ToggleConsole_f ();
	}
	else
	{
		LogoTargetPercent = TitleTargetPercent = 1;
		LogoPercent = TitlePercent = 0;
		M_Menu_Main_f ();
	}
}

// Note: old version of demo has bigfont.lmp, not bigfont2.lmp
#define	BIGCHAR_FONT_FILE0	"gfx/menu/bigfont.lmp"
#define	BIGCHAR_FONT_FILE	"gfx/menu/bigfont2.lmp"
#define	BIGCHAR_WIDTH_FILE	"gfx/menu/fontsize.lmp"

static char	BigCharWidth[27][27];

static void M_BuildBigCharWidth (void)
{
	qpic_t		*p;
	byte		*source;
	int	ypos, xpos;
	int	numA, numB;
	int	biggestX, adjustment;
	char	After[20], Before[20];

	p = (qpic_t *)FS_LoadTempFile (BIGCHAR_FONT_FILE, NULL);
	if (!p) p = (qpic_t *)FS_LoadTempFile (BIGCHAR_FONT_FILE0, NULL);
	if (!p)
		Sys_Error ("Failed to load %s", BIGCHAR_FONT_FILE);
	SwapPic(p);

	for (numA = 0; numA < 27; numA++)
	{
		memset (After, 20, sizeof(After));
		source = p->data + ((numA % 8) * 20) + (numA / 8 * p->width * 20);
		biggestX = 0;

		for (ypos = 0; ypos < 19; ypos++)
		{
			for (xpos = 0; xpos < 19; xpos++, source++)
			{
				if (*source)
				{
					After[ypos] = xpos;
					if (xpos > biggestX)
						biggestX = xpos;
				}
			}
			source += (p->width - 19);
		}
		biggestX++;

		for (numB = 0; numB < 27; numB++)
		{
			memset (Before, 0, sizeof(Before));
			source = p->data + ((numB % 8) * 20) + (numB / 8 * p->width * 20);
			adjustment = 0;

			for (ypos = 0; ypos < 19; ypos++)
			{
				for (xpos = 0; xpos < 19; xpos++, source++)
				{
					if (!(*source))
					{
						Before[ypos]++;
					}
					else
						break;
				}
				source += (p->width - xpos);
			}

			while (1)
			{
				for (ypos = 0; ypos < 19; ypos++)
				{
					if (After[ypos] - Before[ypos] >= 15)
						break;
					Before[ypos]--;
				}
				if (ypos < 19)
					break;
				adjustment--;
			}
			BigCharWidth[numA][numB] = adjustment + biggestX;
		}
	}

	FS_CreatePath(FS_MakePath(FS_USERDIR, NULL, BIGCHAR_WIDTH_FILE));
	FS_WriteFile (BIGCHAR_WIDTH_FILE, BigCharWidth, sizeof(BigCharWidth));
}

static int M_DrawBigCharacter (int x, int y, int num, int numNext)
{
	int		add;

	if (num == ' ')
		return 32;

	if (num == '/')
		num = 26;
	else
		num -= 65;

	if (num < 0 || num >= 27)	// only a-z and /
		return 0;

	if (numNext == '/')
		numNext = 26;
	else
		numNext -= 65;

	Draw_BigCharacter (x, y, num);

	if (numNext < 0 || numNext >= 27)
		return 0;

	add = 0;
	if (num == (int)'C'-65 && numNext == (int)'P'-65)
		add = 3;

	return BigCharWidth[num][numNext] + add;
}

static void M_DrawBigString(int x, int y, const char *string)
{
	x += ((vid.width - 320)>>1);

	while (*string)
	{
		x += M_DrawBigCharacter(x, y, string[0], string[1]);
		++string;
	}
}


void ScrollTitle (const char *name)
{
	qpic_t			*p;
	float		delta;
	int		finaly;
	static const char	*LastName = "";
	static qboolean		CanSwitch = true;

	if (TitlePercent < TitleTargetPercent)
	{
		delta = ((TitleTargetPercent-TitlePercent)/0.5)*host_frametime;
		if (delta < 0.004)
		{
			delta = 0.004;
		}
		TitlePercent += delta;
		if (TitlePercent > TitleTargetPercent)
		{
			TitlePercent = TitleTargetPercent;
		}
	}
	else if (TitlePercent > TitleTargetPercent)
	{
		delta = ((TitlePercent-TitleTargetPercent)/0.15)*host_frametime;
		if (delta < 0.02)
		{
			delta = 0.02;
		}
		TitlePercent -= delta;
		if (TitlePercent <= TitleTargetPercent)
		{
			TitlePercent = TitleTargetPercent;
			CanSwitch = true;
		}
	}

	if (LogoPercent < LogoTargetPercent)
	{
/*		delta = ((LogoTargetPercent-LogoPercent)/1.1)*host_frametime;
		if (delta < 0.0015)
		{
			delta = 0.0015;
		}
*/
		delta = ((LogoTargetPercent-LogoPercent)/.15)*host_frametime;
		if (delta < 0.02)
		{
			delta = 0.02;
		}
		LogoPercent += delta;
		if (LogoPercent > LogoTargetPercent)
		{
			LogoPercent = LogoTargetPercent;
		}
	}

	if (q_strcasecmp(LastName,name) != 0 && TitleTargetPercent != 0)
		TitleTargetPercent = 0;

	if (CanSwitch)
	{
		LastName = name;
		CanSwitch = false;
		TitleTargetPercent = 1;
	}

	p = Draw_CachePic(LastName);
	finaly = ((float)p->height * TitlePercent) - p->height;
	M_DrawTransPicCropped( (320-p->width)/2, finaly , p);

	if (m_state != m_keys)
	{
		p = Draw_CachePic("gfx/menu/hplaque.lmp");
		finaly = ((float)p->height * LogoPercent) - p->height;
		M_DrawTransPicCropped(10, finaly, p);
	}
}


//=============================================================================
/* MAIN MENU */

static int	m_main_cursor;
#define	MAIN_ITEMS	4

static void BGM_RestartMusic(void);
static char	old_bgmtype[20];	// S.A


void M_Menu_Main_f (void)
{
	// Deactivate the mouse when the menus are drawn - S.A.
	menu_disabled_mouse = true;
	if (modestate == MS_WINDOWED)
		IN_DeactivateMouse ();

	if (!(Key_GetDest() & key_menu))
	{
		m_save_demonum = cls.demonum;
		cls.demonum = -1;
	}
	Key_SetDest (key_menu);
	m_state = m_main;
	m_entersound = true;
}


static void M_Main_Draw (void)
{
	int		f;

	ScrollTitle("gfx/menu/title0.lmp");
	M_DrawBigString (72, 60 + (0 * 20), "MULTIPLAYER");
	M_DrawBigString (72, 60 + (1 * 20), "OPTIONS");
	M_DrawBigString (72, 60 + (2 * 20), "HELP");
	M_DrawBigString (72, 60 + (3 * 20), "QUIT");

	f = (int)(realtime * 10)%8;
	M_DrawTransPic (43, 54 + m_main_cursor * 20,Draw_CachePic( va("gfx/menu/menudot%i.lmp", f+1 ) ) );
}


static void M_Main_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		// leaving the main menu, reactivate mouse - S.A.
		menu_disabled_mouse = false;
		IN_ActivateMouse ();
		// and check we haven't changed the music type
		if (old_bgmtype[0] != 0 && strcmp(old_bgmtype,bgmtype.string) != 0)
			BGM_RestartMusic ();
		old_bgmtype[0] = 0;
		Key_SetDest (key_game);
		m_state = m_none;
		cls.demonum = m_save_demonum;
		if (cls.demonum != -1 && !cls.demoplayback && cls.state == ca_disconnected)
			CL_NextDemo ();
		break;

	case K_DOWNARROW:
		S_LocalSound ("raven/menu1.wav");
		if (++m_main_cursor >= MAIN_ITEMS)
			m_main_cursor = 0;
		break;

	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
		if (--m_main_cursor < 0)
			m_main_cursor = MAIN_ITEMS - 1;
		break;

	case K_ENTER:
		m_entersound = true;

		switch (m_main_cursor)
		{
		case 0:
			M_Menu_MultiPlayer_f ();
			break;

		case 1:
			M_Menu_Options_f ();
			break;

		case 2:
			M_Menu_Help_f ();
			break;

		case 3:
			M_Menu_Quit_f ();
			break;
		}
	}
}


//=============================================================================
/* OPTIONS MENU */

#define	SLIDER_RANGE	10

enum
{
	OPT_CUSTOMIZE = 0,
	OPT_CONSOLE,
	OPT_DEFAULTS,
#ifdef GLQUAKE
	OPT_SCALE,
#endif
	OPT_SCRSIZE,
	OPT_GAMMA,
	OPT_MOUSESPEED,
	OPT_MUSICTYPE,
	OPT_MUSICVOL,
	OPT_SNDVOL,
	OPT_ALWAYRUN,
	OPT_INVMOUSE,
	OPT_ALWAYSMLOOK,
	OPT_USEMOUSE,
	OPT_CROSSHAIR,
#ifdef GLQUAKE
	OPT_OPENGL,
#endif
	OPT_VIDEO,
	OPTIONS_ITEMS
};

#ifdef GLQUAKE
// prototypes for the opengl menu
static void M_Menu_OpenGL_f (void);
static void M_OpenGL_Draw (void);
static void M_OpenGL_Key (int k);
#endif

static int	options_cursor;

void M_Menu_Options_f (void)
{
	Key_SetDest (key_menu);
	m_state = m_options;
	m_entersound = true;

	// get the current music type
	if (old_bgmtype[0] == 0)
		q_strlcpy(old_bgmtype, bgmtype.string, sizeof(old_bgmtype));
#if 0	// change to 1 if dont want to disable mouse in fullscreen
	if ((options_cursor == OPT_USEMOUSE) && (modestate != MS_WINDOWED))
		options_cursor = 0;
#endif
}


static void M_AdjustSliders (int dir)
{
	float	f;

	S_LocalSound ("raven/menu3.wav");

	switch (options_cursor)
	{
#ifdef GLQUAKE
	case OPT_SCALE: 	// scale slider S.A.
		VID_ChangeConsize(dir);
		break;
#endif
	case OPT_SCRSIZE:	// screen size
		Cvar_SetValue ("viewsize", scr_viewsize.integer + dir * 10);
		break;
	case OPT_GAMMA:		// gamma
		f = v_gamma.value - dir * 0.05;
		if (f < 0.5)	f = 0.5;
		else if (f > 1)	f = 1;
		Cvar_SetValue ("gamma", f);
		break;
	case OPT_MOUSESPEED:	// mouse speed
		f = sensitivity.value + dir * 0.5;
		if (f > 11)	f = 11;
		else if (f < 1)	f = 1;
		Cvar_SetValue ("sensitivity", f);
		break;
	case OPT_MUSICTYPE:	// bgm type
		if (q_strcasecmp(bgmtype.string,"midi") == 0)
		{
			if (bgm_extmusic.integer)
			{
			    if (dir < 0)
				Cvar_Set("bgm_extmusic","0");
			    else
				Cvar_Set("bgmtype","cd");
			}
			else
			{
			    if (dir < 0)
				Cvar_Set("bgmtype","none");
			    else
				Cvar_Set("bgm_extmusic","1");
			}
		}
		else if (q_strcasecmp(bgmtype.string,"cd") == 0)
		{
			if (dir < 0)
			{
				Cvar_Set("bgmtype","midi");
				Cvar_Set("bgm_extmusic","1");
			}
			else
			{
				Cvar_Set("bgmtype","none");
			}
		}
		else
		{
			if (dir < 0)
			{
				Cvar_Set("bgmtype","cd");
			}
			else
			{
				Cvar_Set("bgm_extmusic","0");
				Cvar_Set("bgmtype","midi");
			}
		}
		break;
	case OPT_MUSICVOL:	// music volume
		f = bgmvolume.value + dir * 0.1;
		if (f < 0)	f = 0;
		else if (f > 1)	f = 1;
		Cvar_SetValue ("bgmvolume", f);
		break;
	case OPT_SNDVOL:	// sfx volume
		f = sfxvolume.value + dir * 0.1;
		if (f < 0)	f = 0;
		else if (f > 1)	f = 1;
		Cvar_SetValue ("volume", f);
		break;

	case OPT_ALWAYRUN:	// always run
		if (cl_forwardspeed.value > 200)
		{
			Cvar_Set ("cl_forwardspeed", "200");
			Cvar_Set ("cl_backspeed", "200");
		}
		else
		{
			Cvar_Set ("cl_forwardspeed", "400");
			Cvar_Set ("cl_backspeed", "400");
		}
		break;

	case OPT_INVMOUSE:	// invert mouse
		Cvar_SetValue ("m_pitch", -m_pitch.value);
		break;

	case OPT_ALWAYSMLOOK:
		if (in_mlook.state & 1)
			Cbuf_AddText("-mlook");
		else
			Cbuf_AddText("+mlook");
		break;

	case OPT_CROSSHAIR:
		Cvar_Set ("crosshair", crosshair.integer ? "0" : "1");
		break;

	case OPT_USEMOUSE:	// _enable_mouse
		Cvar_Set ("_enable_mouse", _enable_mouse.integer ? "0" : "1");
		break;

	default:
		break;
	}
}


static void M_DrawSlider (int x, int y, float range)
{
	int	i;

	if (range < 0)
		range = 0;
	else if (range > 1)
		range = 1;
	M_DrawCharacter (x-8, y, 256);
	for (i = 0; i < SLIDER_RANGE; i++)
		M_DrawCharacter (x + i*8, y, 257);
	M_DrawCharacter (x + i*8, y, 258);
	M_DrawCharacter (x + (SLIDER_RANGE-1)*8 * range, y, 259);
}

void M_DrawCheckbox (int x, int y, int on)
{
	if (on)
		M_Print (x, y, "on");
	else
		M_Print (x, y, "off");
}

static void M_Options_Draw (void)
{
	float		r;

	menu_disabled_mouse = false;
	IN_ActivateMouse ();	// we entered the customization menu

	ScrollTitle("gfx/menu/title3.lmp");

//	we use 22 character option titles. the increment to
//	the x offset is: (22 - strlen(option_title)) * 8
//	r goes from 0 (left) to 1 (right)
	M_Print (16 + (4 * 8), 60 + 8*OPT_CUSTOMIZE,	"Customize controls");
	M_Print (16 + (9 * 8), 60 + 8*OPT_CONSOLE,	"Go to console");
	M_Print (16 + (5 * 8), 60 + 8*OPT_DEFAULTS,	"Reset to defaults");
#ifdef GLQUAKE
	M_Print (16 + (17 * 8), 60 + 8*OPT_SCALE,	"Scale");
	r = VID_ReportConsize();
	M_DrawSlider (220, 60 + 8*OPT_SCALE, (r-1)/2);
#endif
	M_Print (16 + (11 * 8), 60 + 8*OPT_SCRSIZE,	"Screen size");
	r = (scr_viewsize.value - 30.0) / (130 - 30);
	M_DrawSlider (220, 60 + 8*OPT_SCRSIZE, r);

	M_Print (16 + (12 * 8), 60 + 8*OPT_GAMMA,	"Brightness");
	r = (1.0 - v_gamma.value) / 0.5;
	M_DrawSlider (220, 60 + 8*OPT_GAMMA, r);

	M_Print (16 + (11 * 8), 60 + 8*OPT_MOUSESPEED,	"Mouse Speed");
	r = (sensitivity.value - 1) / 10;
	M_DrawSlider (220, 60 + 8*OPT_MOUSESPEED, r);

	M_Print (16 + (12 * 8), 60 + 8*OPT_MUSICTYPE,	"Music Type");
	if (q_strcasecmp(bgmtype.string, "midi") == 0)
	{
	    if (bgm_extmusic.integer)
		M_Print (220, 60 + 8*OPT_MUSICTYPE, "ALL CODECS");
	    else
		M_Print (220, 60 + 8*OPT_MUSICTYPE, "MIDI ONLY");
	}
	else if (q_strcasecmp(bgmtype.string, "cd") == 0)
		M_Print (220, 60 + 8*OPT_MUSICTYPE, "CD");
	else
		M_Print (220, 60 + 8*OPT_MUSICTYPE, "None");

	M_Print (16 + (10 * 8), 60 + 8*OPT_MUSICVOL,	"Music Volume");
	r = bgmvolume.value;
	M_DrawSlider (220, 60 + 8*OPT_MUSICVOL, r);

	M_Print (16 + (10 * 8), 60 + 8*OPT_SNDVOL,	"Sound Volume");
	r = sfxvolume.value;
	M_DrawSlider (220, 60 + 8*OPT_SNDVOL, r);

	M_Print (16 + (12 * 8), 60 + 8*OPT_ALWAYRUN,	"Always Run");
	M_DrawCheckbox (220, 60 + 8*OPT_ALWAYRUN, (cl_forwardspeed.value > 200));

	M_Print (16 + (10 * 8), 60 + 8*OPT_INVMOUSE,	"Invert Mouse");
	M_DrawCheckbox (220, 60 + 8*OPT_INVMOUSE, m_pitch.value < 0);

	M_Print (16 + (12 * 8), 60 + 8*OPT_ALWAYSMLOOK,	"Mouse Look");
	M_DrawCheckbox (220, 60 + 8*OPT_ALWAYSMLOOK, in_mlook.state & 1);

	M_Print (16 + (13 * 8), 60 + 8*OPT_USEMOUSE,	"Use Mouse");
	M_DrawCheckbox (220, 60 + 8*OPT_USEMOUSE, _enable_mouse.integer);

	M_Print (16 + (8 * 8), 60 + 8*OPT_CROSSHAIR,	"Show Crosshair");
	M_DrawCheckbox (220, 60 + 8*OPT_CROSSHAIR, crosshair.integer);

#ifdef GLQUAKE
	M_Print (16 + (7 * 8), 60 + 8*OPT_OPENGL,	"OpenGL Features");
#endif

	if (vid_menudrawfn)
		M_Print (16 + (11 * 8), 60 + 8*OPT_VIDEO,	"Video Modes");

	// cursor
	M_DrawCharacter (200, 60 + 8*options_cursor, 12 + ((int)(realtime*4) & 1));
}


static void M_Options_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
		M_Menu_Main_f ();
		break;

	case K_ENTER:
		m_entersound = true;
		switch (options_cursor)
		{
		case OPT_CUSTOMIZE:
			M_Menu_Keys_f ();
			break;
		case OPT_CONSOLE:
			//m_state = m_none;
			//Con_ToggleConsole_f ();
			Key_SetDest (key_game);
			m_state = m_none;
			break;
		case OPT_DEFAULTS:
			Cbuf_AddText ("exec default.cfg\n");
			break;
#ifdef GLQUAKE
		case OPT_OPENGL:
			M_Menu_OpenGL_f ();
			break;
#endif
		case OPT_VIDEO:
			M_Menu_Video_f ();
			break;
		default:
			M_AdjustSliders (1);
			break;
		}
		return;

	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
		options_cursor--;
		if (options_cursor < 0)
			options_cursor = OPTIONS_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("raven/menu1.wav");
		options_cursor++;
		if (options_cursor >= OPTIONS_ITEMS)
			options_cursor = 0;
		break;

	case K_LEFTARROW:
		M_AdjustSliders (-1);
		break;

	case K_RIGHTARROW:
		M_AdjustSliders (1);
		break;
	}
#if 0	// change to 1 if dont want to disable mouse in fullscreen
	if ((options_cursor == OPT_USEMOUSE) && (modestate != MS_WINDOWED))
	{
		if (k == K_UPARROW)
			options_cursor = OPT_USEMOUSE - 1;
		else
		{
			options_cursor = OPT_USEMOUSE + 1;
			if (options_cursor == OPTIONS_ITEMS)
				options_cursor = 0;
		}
	}
#endif
	if (options_cursor == OPT_VIDEO && vid_menudrawfn == NULL)
	{
		if (k == K_UPARROW)
			options_cursor = OPT_VIDEO - 1;
		else
			options_cursor = 0;
	}
}


//=============================================================================
/* OPENGL FEATURES MENU */

#ifdef GLQUAKE

enum
{
	OGL_MULTITEX,
	OGL_NPOT,
	OGL_PURGETEX,
	OGL_GLOW1,
	OGL_GLOW2,
	OGL_GLOW3,
	OGL_LIGHTMAPFMT,
	OGL_COLOREDLIGHT,
	OGL_COLOREDSTATIC,
	OGL_COLOREDDYNAMIC,
	OGL_COLOREDEXTRA,
	OGL_TEXFILTER,
	OGL_ANISOTROPY,
	OGL_SHADOWS,
	OGL_ITEMS
};

static const struct
{
	const char	*name;	// legible string value
	const char	*desc;	// description for user
	int		glenum;	// opengl enum
} lm_formats[] =
{
	{ "gl_luminance",	"gl_luminance (8 bit)",	GL_LUMINANCE	},
	{ "gl_rgba",		"gl_rgba (32 bit)",	GL_RGBA		},
	{ " ",			"Unknown value (?)",	0		}
};

#define	MAX_LMFORMATS	(sizeof(lm_formats) / sizeof(lm_formats[0]))

static int	tex_mode;
static int	lm_format;
static int	opengl_cursor;

static void M_Menu_OpenGL_f (void)
{
	Key_SetDest (key_menu);
	m_state = m_opengl;
	m_entersound = true;
}


static void M_OpenGL_Draw (void)
{
	int		i;

	ScrollTitle("gfx/menu/title3.lmp");
	M_PrintWhite (96, 72, "OpenGL Features:");

//	we use 22 character option titles. the increment to
//	the x offset is: (22 - strlen(option_title)) * 8

	M_Print (32 + (8 * 8), 90 + 8*OGL_MULTITEX,	"Multitexturing");
	if (gl_mtexable)
		M_DrawCheckbox (232, 90 + 8*OGL_MULTITEX, gl_multitexture.integer);
	else	M_Print (232, 90 + 8*OGL_MULTITEX, "Not found");

	M_Print (32 + (9 * 8), 90 + 8*OGL_NPOT, "NPOT textures");
	M_DrawCheckbox (232, 90 + 8*OGL_NPOT, gl_texture_NPOT.integer);

	M_Print (32 + (4 * 8), 90 + 8*OGL_PURGETEX,	"Purge map textures");
	M_DrawCheckbox (232, 90 + 8*OGL_PURGETEX, gl_purge_maptex.integer);

	M_Print (32 + (9 * 8), 90 + 8*OGL_GLOW1,	"missile glows");
	M_DrawCheckbox (232, 90 + 8*OGL_GLOW1, gl_missile_glows.integer);

	M_Print (32 + (11 * 8), 90 + 8*OGL_GLOW2,	"torch glows");
	M_DrawCheckbox (232, 90 + 8*OGL_GLOW2, gl_glows.integer);

	M_Print (32 + (11 * 8), 90 + 8*OGL_GLOW3,	"other glows");
	M_DrawCheckbox (232, 90 + 8*OGL_GLOW3, gl_other_glows.integer);

	M_Print (32 + (6 * 8), 90 + 8*OGL_LIGHTMAPFMT,	"Lightmap Format:");
	for (i = 0; i < (int)MAX_LMFORMATS; i++)
	{
		if (!q_strcasecmp(gl_lightmapfmt.string, lm_formats[i].name))
			break;
	}
	lm_format = i;
	M_Print (232, 90 + 8*OGL_LIGHTMAPFMT, lm_formats[lm_format].desc);

	M_Print (32 + (6 * 8), 90 + 8*OGL_COLOREDLIGHT,	"Colored lights :");
	M_Print (32 + (9 * 8), 90 + 8*OGL_COLOREDSTATIC, "static lights");
	M_Print (32 + (8 * 8), 90 + 8*OGL_COLOREDDYNAMIC, "dynamic lights");
	M_Print (32 + (10 * 8), 90 + 8*OGL_COLOREDEXTRA, "extra lights");
	// bound the gl_coloredlight value
	if (gl_coloredlight.integer < 0)
		Cvar_Set ("gl_coloredlight", "0");
	switch (gl_coloredlight.integer)
	{
	case 0:
		M_Print (232, 90 + 8*OGL_COLOREDSTATIC, "none (white)");
		break;
	case 1:
		M_Print (232, 90 + 8*OGL_COLOREDSTATIC, "colored");
		break;
	default:
		M_Print (232, 90 + 8*OGL_COLOREDSTATIC, "blend");
		break;
	}
	M_DrawCheckbox (232, 90 + 8*OGL_COLOREDDYNAMIC, gl_colored_dynamic_lights.integer);
	M_DrawCheckbox (232, 90 + 8*OGL_COLOREDEXTRA, gl_extra_dynamic_lights.integer);

	M_Print (32 + (5 * 8), 90 + 8*OGL_TEXFILTER,	"Texture filtering");
	M_Print (232, 90 + 8*OGL_TEXFILTER, gl_texmodes[gl_filter_idx].name);

	M_Print (32 + (5 * 8), 90 + 8*OGL_ANISOTROPY,	"Anisotropy level:");
	M_Print (232, 90 + 8*OGL_ANISOTROPY, (gl_max_anisotropy < 2) ? "N/A" :
				Cvar_VariableString("gl_texture_anisotropy"));

	M_Print (32 + (15 * 8), 90 + 8*OGL_SHADOWS,	"Shadows");
	M_DrawCheckbox (232, 90 + 8*OGL_SHADOWS, r_shadows.integer);

	// cursor
	M_DrawCharacter (216, 90 + opengl_cursor*8, 12+((int)(realtime*4)&1));

	if (opengl_cursor == OGL_LIGHTMAPFMT && gl_lightmap_format != lm_formats[lm_format].glenum)
	{
		int	x = (320-25*8)/2;
		for (i = 0; i < (int)MAX_LMFORMATS-1; i++)
		{
			if (gl_lightmap_format == lm_formats[i].glenum)
				break;
		}
		M_DrawTextBox (x, 94 + 8*(OGL_LIGHTMAPFMT), 25, 5);
		M_Print (x+16, 98 + 8*(OGL_LIGHTMAPFMT+1), "currently running with:");
		M_Print (x+16, 98 + 8*(OGL_LIGHTMAPFMT+2), lm_formats[i].desc);
		M_Print (x+16, 98 + 8*(OGL_LIGHTMAPFMT+3), "your selection will take");
		M_Print (x+16, 98 + 8*(OGL_LIGHTMAPFMT+4), "effect upon level reload");
	}
	else if (opengl_cursor == OGL_COLOREDSTATIC)
	{
		int	x = (320-25*8)/2;

		if (gl_lightmap_format != GL_RGBA)
		{
			if (gl_coloredlight.integer)
			{
				M_DrawTextBox (x, 94 + 8*(OGL_COLOREDSTATIC), 25, 3);
				M_Print (x+16, 98 + 8*(OGL_COLOREDSTATIC+1), "  rgba lightmap format  ");
				M_Print (x+16, 98 + 8*(OGL_COLOREDSTATIC+2), "      is required");
			}
		}
		else if (gl_coloredlight.integer != gl_coloredstatic)
		{
			M_DrawTextBox (x, 94 + 8*(OGL_COLOREDSTATIC), 25, 3);
			M_Print (x+16, 98 + 8*(OGL_COLOREDSTATIC+1), "your selection will take");
			M_Print (x+16, 98 + 8*(OGL_COLOREDSTATIC+2), "effect upon level reload");
		}
	}
}


static void M_OpenGL_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
		M_Menu_Options_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
		if (--opengl_cursor == OGL_COLOREDLIGHT)
			--opengl_cursor;
		if (opengl_cursor < 0)
			opengl_cursor = OGL_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("raven/menu1.wav");
		if (++opengl_cursor == OGL_COLOREDLIGHT)
			++opengl_cursor;
		if (opengl_cursor >= OGL_ITEMS)
			opengl_cursor = 0;
		break;

	case K_ENTER:
	case K_LEFTARROW:
	case K_RIGHTARROW:
		m_entersound = true;
		switch (opengl_cursor)
		{
		case OGL_MULTITEX:	// multitexturing
			Cvar_Set ("gl_multitexture", gl_multitexture.integer ? "0" : "1");
			break;

		case OGL_NPOT:		// texture_non_power_of_two
			Cvar_Set ("gl_texture_NPOT", gl_texture_NPOT.integer ? "0" : "1");
			break;

		case OGL_PURGETEX:	// purge gl textures on map change
			Cvar_Set ("gl_purge_maptex", gl_purge_maptex.integer ? "0" : "1");
			break;

		case OGL_GLOW1:	// glow effects, missiles
			Cvar_Set ("gl_missile_glows", gl_missile_glows.integer ? "0" : "1");
			break;

		case OGL_GLOW2:	// glow effects, torches
			Cvar_Set ("gl_glows", gl_glows.integer ? "0" : "1");
			break;

		case OGL_GLOW3:	// glow effects, other: mana, etc.
			Cvar_Set ("gl_other_glows", gl_other_glows.integer ? "0" : "1");
			break;

		case OGL_LIGHTMAPFMT:	// lightmap format
			switch (k)
			{
			case K_RIGHTARROW:
				if (++lm_format >= (int)MAX_LMFORMATS - 1)
					lm_format = MAX_LMFORMATS - 2;
				Cvar_Set ("gl_lightmapfmt", lm_formats[lm_format].name);
				break;
			case K_LEFTARROW:
				if (--lm_format < 0)
					lm_format = 0;
				Cvar_Set ("gl_lightmapfmt", lm_formats[lm_format].name);
				break;
			default:
				break;
			}
			break;

		case OGL_COLOREDSTATIC:	// static colored lights
			switch (k)
			{
			case K_RIGHTARROW:
				Cvar_Set ("gl_coloredlight", (gl_coloredlight.integer >= 1) ? "2" : "1");
				break;
			case K_LEFTARROW:
				Cvar_Set ("gl_coloredlight", (gl_coloredlight.integer <= 1) ? "0" : "1");
				break;
			default:
				break;
			}
			break;

		case OGL_COLOREDDYNAMIC:	// dynamic colored lights
			Cvar_Set ("gl_colored_dynamic_lights", gl_colored_dynamic_lights.integer ? "0" : "1");
			break;

		case OGL_COLOREDEXTRA:	// extra dynamic colored lights
			Cvar_Set ("gl_extra_dynamic_lights", gl_extra_dynamic_lights.integer ? "0" : "1");
			break;

		case OGL_TEXFILTER:	// texture filter
			tex_mode = gl_filter_idx;
			switch (k)
			{
			case K_LEFTARROW:
				if (--tex_mode < 0)
					tex_mode = 0;
				break;
			case K_RIGHTARROW:
				if (++tex_mode >= NUM_GL_FILTERS)
					tex_mode = NUM_GL_FILTERS-1;
				break;
			default:
				return;
			}
			Cvar_Set ("gl_texturemode", gl_texmodes[tex_mode].name);
			break;

		case OGL_ANISOTROPY:	// anisotropic filter level
			if (gl_max_anisotropy < 2)
				return;
			tex_mode = Cvar_VariableValue("gl_texture_anisotropy");
			switch (k)
			{
			case K_LEFTARROW:
				if (--tex_mode < 1)
					tex_mode = 1;
				break;
			case K_RIGHTARROW:
				if (++tex_mode > gl_max_anisotropy)
					tex_mode = gl_max_anisotropy;
				break;
			default:
				return;
			}
			Cvar_SetValue ("gl_texture_anisotropy", tex_mode);
			break;

		case OGL_SHADOWS:	// shadows
			Cvar_Set ("r_shadows", r_shadows.integer ? "0" : "1");
			break;

		default:
			break;
		}

	default:
		break;
	}
}

#endif


//=============================================================================
/* KEYS MENU */

static const char *bindnames[][2] =
{
	{"+attack",		"attack"},
	{"impulse 10",		"next weapon"},
	{"impulse 12",		"prev.weapon"},
	{"+jump",		"jump / swim up"},
	{"+forward",		"walk forward"},
	{"+back",		"backpedal"},
	{"+left",		"turn left"},
	{"+right",		"turn right"},
	{"+speed",		"run"},
	{"+moveleft",		"step left"},
	{"+moveright",		"step right"},
	{"+strafe",		"sidestep"},
	{"+crouch",		"crouch"},
	{"+lookup",		"look up"},
	{"+lookdown",		"look down"},
	{"centerview",		"center view"},
	{"+moveup",		"swim up"},
	{"+movedown",		"swim down"},
	{"impulse 13",		"use object"},
	{"invuse",		"use inv item"},
	{"invdrop",		"drop inv item"},
	{"+showinfo",		"full inventory"},
	{"+showdm",		"info / frags"},
	{"toggle_dm",		"toggle frags"},
	{"+shownames",		"player names"},
	{"invleft",		"inv move left"},
	{"invright",		"inv move right"},
	{"impulse 100",		"inv:torch"},
	{"impulse 101",		"inv:qrtz flask"},
	{"impulse 102",		"inv:mystic urn"},
	{"impulse 103",		"inv:krater"},
	{"impulse 104",		"inv:chaos devc"},
	{"impulse 105",		"inv:tome power"},
	{"impulse 106",		"inv:summon stn"},
	{"impulse 107",		"inv:invisiblty"},
	{"impulse 108",		"inv:glyph"},
	{"impulse 109",		"inv:boots"},
	{"impulse 110",		"inv:repulsion"},
	{"impulse 111",		"inv:bo peep"},
	{"impulse 112",		"inv:flight"},
	{"impulse 113",		"inv:force cube"},
	{"impulse 114",		"inv:icon defn"}
};

#define	NUMCOMMANDS	(sizeof(bindnames)/sizeof(bindnames[0]))

#define KEYS_SIZE 14

static int		keys_cursor;
static int		keys_top = 0;

static void M_Menu_Keys_f (void)
{
	Key_SetDest (key_menu);
	m_state = m_keys;
	m_entersound = true;
}


static void M_FindKeysForCommand (const char *command, int *twokeys)
{
	int		count;
	int		j;
	int		l,l2;
	char	*b;

	twokeys[0] = twokeys[1] = -1;
	l = strlen(command);
	count = 0;

	for (j = 0; j < 256; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!strncmp (b, command, l))
		{
			l2 = strlen(b);
			if (l == l2)
			{
				twokeys[count] = j;
				count++;
				if (count == 2)
					break;
			}
		}
	}
}

static void M_UnbindCommand (const char *command)
{
	int		j;
	int		l;
	char	*b;

	l = strlen(command);

	for (j = 0; j < 256; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!strncmp (b, command, l) )
			Key_SetBinding (j, NULL);
	}
}


static void M_Keys_Draw (void)
{
	int		i, x, y;
	int		keys[2];
	const char	*name;
//	qpic_t	*p;

	ScrollTitle("gfx/menu/title6.lmp");

//	M_DrawTextBox (6,56, 35,16);

//	p = Draw_CachePic("gfx/menu/hback.lmp");
//	M_DrawTransPicCropped(8, 62, p);

	if (keys_top)
		M_DrawCharacter (6, 80, 128);
	if (keys_top + KEYS_SIZE < (int)NUMCOMMANDS)
		M_DrawCharacter (6, 80 + ((KEYS_SIZE-1)*8), 129);

// search for known bindings
	for (i = 0; i < KEYS_SIZE; i++)
	{
		y = 80 + 8*i;

		M_Print (16, y, bindnames[i+keys_top][1]);

		M_FindKeysForCommand (bindnames[i+keys_top][0], keys);

		if (keys[0] == -1)
		{
			M_Print (140, y, "???");
		}
		else
		{
			name = Key_KeynumToString (keys[0]);
			M_Print (140, y, name);
			x = strlen(name) * 8;
			if (keys[1] != -1)
			{
				M_Print (140 + x + 8, y, "or");
				M_Print (140 + x + 32, y, Key_KeynumToString (keys[1]));
			}
		}
	}

	if (Key_GetDest() & key_bindbit)
	{
		M_Print (12, 64, "Press a key or button for this action");
		M_DrawCharacter (130, 80 + (keys_cursor-keys_top)*8, '=');
	}
	else
	{
		M_Print (18, 64, "Enter to change, backspace to clear");
		M_DrawCharacter (130, 80 + (keys_cursor-keys_top)*8, 12+((int)(realtime*4)&1));
	}
}


static void M_Keys_Key (int k)
{
	int		keys[2];

	switch (k)
	{
	case K_ESCAPE:
		M_Menu_Options_f ();
		break;

	case K_LEFTARROW:
	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
		keys_cursor--;
		if (keys_cursor < 0)
			keys_cursor = NUMCOMMANDS-1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("raven/menu1.wav");
		keys_cursor++;
		if (keys_cursor >= (int)NUMCOMMANDS)
			keys_cursor = 0;
		break;

	case K_ENTER:		// go into bind mode
		M_FindKeysForCommand (bindnames[keys_cursor][0], keys);
		S_LocalSound ("raven/menu2.wav");
		if (keys[1] != -1)
			M_UnbindCommand (bindnames[keys_cursor][0]);
		Key_SetDest (key_menubind);
		break;

	case K_BACKSPACE:	// delete bindings
	case K_DEL:		// delete bindings
		S_LocalSound ("raven/menu2.wav");
		M_UnbindCommand (bindnames[keys_cursor][0]);
		break;
	}

	if (keys_cursor < keys_top)
		keys_top = keys_cursor;
	else if (keys_cursor >= keys_top+KEYS_SIZE)
		keys_top = keys_cursor - KEYS_SIZE + 1;
}

//=============================================================================
/* VIDEO MENU */

static void M_Menu_Video_f (void)
{
	Key_SetDest (key_menu);
	m_state = m_video;
	m_entersound = true;
}


static void M_Video_Draw (void)
{
	(*vid_menudrawfn) ();
}


static void M_Video_Key (int key)
{
	(*vid_menukeyfn) (key);
}

//=============================================================================
/* HELP MENU */

static int		help_page;

#define	NUM_HELP_PAGES		5
#define	NUM_SG_HELP_PAGES	10	/* Siege has more help */

static void M_Menu_Help_f (void)
{
	Key_SetDest (key_menu);
	m_state = m_help;
	m_entersound = true;
	help_page = 0;
}


#if FULLSCREEN_INTERMISSIONS
#	ifdef GLQUAKE
#		define	Load_HelpPic_FN(X,Y,Z)	Draw_CachePicNoTrans((X))
#		define	Draw_HelpPic_FN(X,Y,Z)	Draw_IntermissionPic((Z))
#	else
#		define	Load_HelpPic_FN(X,Y,Z)	Draw_CachePicResize((X),(Y),(Z))
#		define	Draw_HelpPic_FN(X,Y,Z)	Draw_Pic(0,0,(Z))
#	endif
#else
#	ifdef GLQUAKE
#		define	Load_HelpPic_FN(X,Y,Z)	Draw_CachePic((X))
#		define	Draw_HelpPic_FN(X,Y,Z)	Draw_Pic((X),(Y),(Z))
#	else
#		define	Load_HelpPic_FN(X,Y,Z)	Draw_CachePic((X))
#		define	Draw_HelpPic_FN(X,Y,Z)	Draw_Pic((X),(Y),(Z))
#	endif
#endif

static void M_Help_Draw (void)
{
	if (cl_siege)
		Draw_HelpPic_FN ((vid.width - 320)>>1, 0, Load_HelpPic_FN(va("gfx/menu/sghelp%02i.lmp", help_page+1), vid.width, vid.height));
	else
		Draw_HelpPic_FN ((vid.width - 320)>>1, 0, Load_HelpPic_FN(va("gfx/menu/help%02i.lmp", help_page+1), vid.width, vid.height));
}


static void M_Help_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Main_f ();
		break;

	case K_UPARROW:
	case K_RIGHTARROW:
		m_entersound = true;
		if (cl_siege)
		{
			if (++help_page >= NUM_SG_HELP_PAGES)
				help_page = 0;
		}
		else if (++help_page >= NUM_HELP_PAGES)
			help_page = 0;
		break;

	case K_DOWNARROW:
	case K_LEFTARROW:
		m_entersound = true;
		if (--help_page < 0)
		{
			if (cl_siege)
				help_page = NUM_SG_HELP_PAGES-1;
			else
				help_page = NUM_HELP_PAGES-1;
		}
		break;
	}
}

//=============================================================================
/* QUIT MENU */

//static int		msgNumber;
static enum m_state_e	m_quit_prevstate;
static qboolean		wasInMenus;

#if 0
static const char *quitMessage [] = 
{
/* .........1.........2.... */
  "   Look! Behind you!    ",
  "  There's a big nasty   ",
  "   thing - shoot it!    ",
  "                        ",

  "  You can't go now, I   ",
  "   was just getting     ",
  "    warmed up.          ",
  "                        ",

  "    One more game.      ",
  "      C'mon...          ",
  "   Who's gonna know?    ",
  "                        ",

  "   What's the matter?   ",
  "   Palms too sweaty to  ",
  "     keep playing?      ",
  "                        ",

  "  Watch your local store",
  "      for Hexen 2       ",
  "    plush toys and      ",
  "    greeting cards!     ",

  "  Hexen 2...            ",
  "                        ",
  "    Too much is never   ",
  "        enough.         ",

  "  Sure go ahead and     ",
  "  leave.  But I know    ",
  "  you'll be back.       ",
  "                        ",

  "                        ",
  "  Insert cute phrase    ",
  "        here            ",
  "                        "
};
#endif

static float LinePos;
static int LineTimes;
static int MaxLines;
static const char **LineText;
static qboolean LineTxt2;
static qboolean SoundPlayed;


#define	MAX_LINES	170

static const char *CreditText[MAX_LINES] =
{
   "HexenWorld",
   "",
   "Lead Programmer: Rick Johnson",
   "",
   "Programming:",
   "   Nathan Albury",
   "   Ron Midthun",
   "   Steve Sengele",
   "   Mike Gummelt",
   "   James Monroe",
   "",
   "Deathmatch Levels:",
   "   Kenn Hoekstra",
   "   Mike Renner",
   "   Jeremy Statz",
   "   Jon Zuk",
   "",
   "Special thanks to:",
   "   Dave Kirsch",
   "   William Mull",
   "   Jack Mathews",
   "",
   "",
   "Hexen2",
   "",
   "Project Director: Brian Raffel",
   "",
   "Lead Programmer: Rick Johnson",
   "",
   "Programming:",
   "   Ben Gokey",
   "   Bob Love",
   "   Mike Gummelt",
   "",
   "Additional Programming:",
   "   Josh Weier",
   "",
   "Lead Design: Eric Biessman",
   "",
   "Design:",
   "   Brian Raffel",
   "   Brian Frank",
   "   Tom Odell",
   "",
   "Art Director: Brian Pelletier",
   "",
   "Art:",
   "   Shane Gurno",
   "   Jim Sumwalt",
   "   Mark Morgan",
   "   Kim Lathrop",
   "   Ted Halsted",
   "   Rebecca Rettenmund",
   "   Les Dorscheid",
   "",
   "Animation:",
   "   Chaos (Mike Werckle)",
   "   Brian Shubat",
   "",
   "Cinematics:",
   "   Jeff Dewitt",
   "   Jeffrey P. Lampo",
   "",
   "Music:",
   "   Kevin Schilder",
   "",
   "Sound:",
   "   Kevin Schilder",
   "   Chia Chin Lee",
   "",
   "",
   "Activision",
   "",
   "Producer:",
   "   Steve Stringer",
   "",
   "Localization Producer:",
   "   Sandi Isaacs",
   "",
   "Marketing Product Manager:",
   "   Henk Hartong",
   "",
   "European Marketing",
   "Product Director:",
   "   Janine Johnson",
   "",
   "Marketing Associate:",
   "   Kevin Kraff",
   "",
   "Senior Quality",
   "Assurance Lead:",
   "   Tim Vanlaw",
   "",
   "Quality Assurance Lead:",
   "   John Tam",
   "",
   "Quality Assurance Team:",
   "   Steve Rosenthal, Mike Spann,",
   "   Steve Elwell, Kelly Wand,",
   "   Kip Stolberg, Igor Krinitskiy,",
   "   Ian Stevens, Marilena Wahmann,",
   "   David Baker, Winnie Lee",
   "",
   "Documentation:",
   "   Mike Rivera, Sylvia Orzel,",
   "   Belinda Vansickle",
   "",
   "Chronicle of Deeds written by:",
   "   Joe Grant Bell",
   "",
   "Localization:",
   "   Nathalie Dove, Lucy Morgan,",
   "   Alex Wylde, Nicky Kerth",
   "",
   "Installer by:",
   "   Steve Stringer, Adam Goldberg,",
   "   Tanya Martino, Eric Schmidt,",
   "   Ronnie Lane",
   "",
   "Art Assistance by:",
   "   Carey Chico and Franz Boehm",
   "",
   "BizDev Babe:",
   "   Jamie Bafus",
   "",
   "And...",
   "",
   "Deal Guru:",
   "   Mitch Lasky",
   "",
   "",
   "Thanks to Id software:",
   "   John Carmack",
   "   Adrian Carmack",
   "   Kevin Cloud",
   "   Barrett 'Bear'  Alexander",
   "   American McGee",
   "",
   "",
   "Published by Id Software, Inc.",
   "Distributed by Activision, Inc.",
   "",
   "The Id Software Technology used",
   "under license in Hexen II (tm)",
   "(c) 1996, 1997 Id Software, Inc.",
   "All Rights Reserved.",
   "",
   "Hexen(r) is a registered trademark",
   "of Raven Software Corp.",
   "Hexen II (tm) and the Raven logo",
   "are trademarks of Raven Software",
   "Corp.  The Id Software name and",
   "id logo are trademarks of",
   "Id Software, Inc.  Activision(r)",
   "is a registered trademark of",
   "Activision, Inc. All other",
   "trademarks are the property of",
   "their respective owners.",
   "",
   "",
   "",
   "Send bug descriptions to:",
   "   h2bugs@mail.ravensoft.com",
   "",
   "Special thanks to Gary McTaggart",
   "at 3dfx for his help with",
   "the gl version!",
   "",
   "No snails were harmed in the",
#ifdef DEMOBUILD
   "making of this demo!"
#else
   "making of this game!"
#endif
};

#define	MAX_LINES2	185

static const char *Credit2Text[MAX_LINES2] =
{
   "HexenWorld",
   "",
   "Superior Groucher:",
   "   Rick 'Grrr' Johnson",
   "",
   "Bug Creators:",
   "   Nathan 'Glory Code' Albury",
   "   Ron 'Stealth' Midthun",
   "   Steve 'Tie Dye' Sengele",
   "   Mike 'Foos' Gummelt",
   "   James 'Happy' Monroe",
   "",
   "Sloppy Joe Makers:",
   "   Kenn 'I'm a broken man'",
   "      Hoekstra",
   "   Mike 'Outback' Renner",
   "   Jeremy 'Under-rated' Statz",
   "   Jon Zuk",
   "",
   "Avoid the Noid:",
   "   Dave 'Zoid' Kirsch",
   "   William 'Phoeb' Mull",
   "   Jack 'Morbid' Mathews",
   "",
   "",
   "Hexen2",
   "",
   "Map Master: ",
   "   'Caffeine Buzz' Raffel",
   "",
   "Code Warrior:",
   "   Rick 'Superfly' Johnson",
   "",
   "Grunt Boys:",
   "   'Judah' Ben Gokey",
   "   Bob 'Whipped' Love",
   "   Mike '9-Pointer' Gummelt",
   "",
   "Additional Grunting:",
   "   Josh 'Intern' Weier",
   "",
   "Whippin' Boy:",
   "   Eric 'Baby' Biessman",
   "",
   "Crazy Levelers:",
   "   'Big Daddy' Brian Raffel",
   "   Brian 'Red' Frank",
   "   Tom 'Texture Alignment' Odell",
   "",
   "Art Lord:",
   "   Brian 'Mr. Calm' Pelletier",
   "",
   "Pixel Monkies:",
   "   Shane 'Duh' Gurno",
   "   'Slim' Jim Sumwalt",
   "   Mark 'Dad Gummit' Morgan",
   "   Kim 'Toy Master' Lathrop",
   "   'Drop Dead' Ted Halsted",
   "   Rebecca 'Zombie' Rettenmund",
   "   Les 'is not more' Dorscheid",
   "",
   "Salad Shooters:",
   "   Mike 'Chaos' Werckle",
   "   Brian 'Mutton Chops' Shubat",
   "",
   "Last Minute Addition:",
   "   Jeff 'Bud Bundy' Dewitt",
   "   Jeffrey 'Misspalld' Lampo",
   "",
   "Random Notes:",
   "   Kevin 'I Already Paid' Schilder",
   "",
   "Grunts, Groans, and Moans:",
   "   Kevin 'I Already Paid' Schilder",
   "   Chia 'Pet' Chin Lee",
   "",
   "",
   "Activision",
   "",
   "Producer:",
   "   Steve 'Ferris' Stringer",
   "",
   "Localization Producer:",
   "   Sandi 'Boneduster' Isaacs",
   "",
   "Marketing Product Manager:",
   "   Henk 'A-10' Hartong",
   "",
   "European Marketing",
   "Product Director:",
   "   Janine Johnson",
   "",
   "Marketing Associate:",
   "   Kevin 'Savage' Kraff",
   "",
   "Senior Quality",
   "Assurance Lead:",
   "   Tim 'Outlaw' Vanlaw",
   "",
   "Quality Assurance Lead:",
   "   John 'Armadillo' Tam",
   "",
   "Quality Assurance Team:",
   "   Steve 'Rhinochoadicus'",
   "      Rosenthal,",
   "   Mike 'Dragonhawk' Spann,",
   "   Steve 'Zendog' Elwell,",
   "   Kelly 'Li'l Bastard' Wand,",
   "   Kip 'Angus' Stolberg,",
   "   Igor 'Russo' Krinitskiy,",
   "   Ian 'Cracker' Stevens,",
   "   Marilena 'Raveness-X' Wahmann,",
   "   David 'Spicegirl' Baker,",
   "   Winnie 'Mew' Lee",
   "",
   "Documentation:",
   "   Mike Rivera, Sylvia Orzel,",
   "   Belinda Vansickle",
   "",
   "Chronicle of Deeds written by:",
   "   Joe Grant Bell",
   "",
   "Localization:",
   "   Nathalie Dove, Lucy Morgan,",
   "   Alex Wylde, Nicky Kerth",
   "",
   "Installer by:",
   "   Steve 'Bahh' Stringer,",
   "   Adam Goldberg, Tanya Martino,",
   "   Eric Schmidt, Ronnie Lane",
   "",
   "Art Assistance by:",
   "   Carey 'Damien' Chico and",
   "   Franz Boehm",
   "",
   "BizDev Babe:",
   "   Jamie Bafus",
   "",
   "And...",
   "",
   "Deal Guru:",
   "   Mitch 'I'll buy that' Lasky",
   "",
   "",
   "Thanks to Id software:",
   "   John Carmack",
   "   Adrian Carmack",
   "   Kevin Cloud",
   "   Barrett 'Bear'  Alexander",
   "   American McGee",
   "",
   "",
   "Published by Id Software, Inc.",
   "Distributed by Activision, Inc.",
   "",
   "The Id Software Technology used",
   "under license in Hexen II (tm)",
   "(c) 1996, 1997 Id Software, Inc.",
   "All Rights Reserved.",
   "",
   "Hexen(r) is a registered trademark",
   "of Raven Software Corp.",
   "Hexen II (tm) and the Raven logo",
   "are trademarks of Raven Software",
   "Corp.  The Id Software name and",
   "id logo are trademarks of",
   "Id Software, Inc.  Activision(r)",
   "is a registered trademark of",
   "Activision, Inc. All other",
   "trademarks are the property of",
   "their respective owners.",
   "",
   "",
   "",
   "Send bug descriptions to:",
   "   h2bugs@mail.ravensoft.com",
   "",
   "Special thanks to Bob for",
   "remembering 'P' is for Polymorph",
   "",
   "",
   "See the next movie in the long",
   "awaited sequel, starring",
   "Bobby Love in,",
   "   Out of Traction, Back in Action!"
};

#define QUIT_SIZE 16	/* was 18. stole 2 for the two uHexen2 lines. */

void M_Menu_Quit_f (void)
{
	if (m_state == m_quit)
		return;
	wasInMenus = !!(Key_GetDest () & key_menu);
	Key_SetDest (key_menu);
	m_quit_prevstate = m_state;
	m_state = m_quit;
	m_entersound = true;
//	msgNumber = rand()&7;

	LinePos = 0;
	LineTimes = 0;
	LineText = CreditText;
	MaxLines = MAX_LINES;
	LineTxt2 = false;
	SoundPlayed = false;
}


static void M_Quit_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
	case 'n':
	case 'N':
		if (wasInMenus)
		{
			m_state = m_quit_prevstate;
			m_entersound = true;
		}
		else
		{
			Key_SetDest (key_game);
			m_state = m_none;
		}
		break;

	case 'Y':
	case 'y':
		Key_SetDest (key_console);
		CL_Disconnect ();
		Sys_Quit ();
		break;

	default:
		break;
	}
}

static void M_Quit_Draw (void)
{
	int i, x, y, place, topy;
	qpic_t	*p;

	if (wasInMenus)
	{
		m_state = m_quit_prevstate;
		m_recursiveDraw = true;
		M_Draw ();
		m_state = m_quit;
	}

	LinePos += host_frametime*1.75;
	if (LinePos > MaxLines + QUIT_SIZE + 2)
	{
		LinePos = 0;
		SoundPlayed = false;
		LineTimes++;
		if (LineTimes >= 2)
		{
			MaxLines = MAX_LINES2;
			LineText = Credit2Text;
			LineTxt2 = true;
		}
	}

	y = 12;
	M_DrawTextBox (0, 0, 38, 23);

// the increment to the x offset is for properly centering the line
	M_Print      (16 + (6 * 8), y,		"Hexen2World version " STRINGIFY(ENGINE_VERSION));
	M_Print      (16 + (9 * 8), y + 8,	"by Raven Software");
	M_PrintWhite (16 + (7 * 8), y + 16,	"Hammer of Thyrion " HOT_VERSION_STR);
	M_PrintWhite (16 +(13 * 8), y + 24,	"Source Port");
	y += 40;

	if (LinePos > 55 && !SoundPlayed && LineTxt2)
	{
		S_LocalSound ("rj/steve.wav");
		SoundPlayed = true;
	}
	topy = y;
	place = LinePos;
	y -= (LinePos - (int)LinePos) * 8;
	for (i = 0; i < QUIT_SIZE; i++, y += 8)
	{
		if (i + place - QUIT_SIZE >= MaxLines)
			break;
		if (i + place < QUIT_SIZE)
			continue;

		if (LineText[i + place - QUIT_SIZE][0] == ' ')
			M_PrintWhite(24, y, LineText[i + place - QUIT_SIZE]);
		else
			M_Print(24, y, LineText[i + place - QUIT_SIZE]);
	}

	p = Draw_CachePic ("gfx/box_mm2.lmp");
	x = 24;
	y = topy - 8;
	for (i = 4; i < 36; i++, x += 8)
	{
		M_DrawTransPic (x, y, p);
	}

	p = Draw_CachePic ("gfx/box_mm2.lmp");
	x = 24;
	y = topy + (QUIT_SIZE * 8) - 8;
	for (i = 4; i < 36; i++, x += 8)
	{
		M_DrawTransPic (x, y, p);
	}

	y += 8;
	M_PrintWhite (16 + (10 * 8), y,  "Press y to exit");

/*	y = 12;
	M_DrawTextBox (0, 0, 38, 23);
	M_PrintWhite (16, y,  "        Hexen II version 0.0        ");	y += 8;
	M_PrintWhite (16, y,  "         by Raven Software          ");	y += 16;
	M_PrintWhite (16, y,  "Programming        Art              ");	y += 8;
	M_Print (16, y,       " Ben Gokey          Shane Gurno     ");	y += 8;
	M_Print (16, y,       " Rick Johnson       Mike Werckle    ");	y += 8;
	M_Print (16, y,       " Bob Love           Mark Morgan     ");	y += 8;
	M_Print (16, y,       " Mike Gummelt       Brian Pelletier ");	y += 8;
	M_Print (16, y,       "                    Kim Lathrop     ");	y += 8;
	M_PrintWhite (16, y,  "Design                              ");	
	M_Print (16, y,       "                    Les Dorscheid   ");	y += 8;
	M_Print (16, y,       " Brian Raffel       Jim Sumwalt     ");	y += 8;
	M_Print (16, y,       " Eric Biessman      Brian Shubat    ");	y += 16;
	M_PrintWhite (16, y,  "Sound Effects      Intern           ");	y += 8;
	M_Print (16, y,       " Kevin Schilder     Josh Weier      ");	y += 16;
	M_PrintWhite (16, y,  "          Press y to exit           ");	y += 8;
*/
}

//=============================================================================
/* MULTIPLAYER MENU */

static int	m_multiplayer_cursor;
#define	MULTIPLAYER_ITEMS	2

static void M_Menu_MultiPlayer_f (void)
{
	Key_SetDest (key_menu);
	m_state = m_multiplayer;
	m_entersound = true;

	message = NULL;
}


static void M_MultiPlayer_Draw (void)
{
	int		f;

	ScrollTitle("gfx/menu/title4.lmp");

	M_DrawBigString (72,60+(0*20),"JOIN A GAME");
	M_DrawBigString (72,60+(1*20),"SETUP");

	f = (int)(realtime * 10)%8;
	M_DrawTransPic (43, 54 + m_multiplayer_cursor * 20,Draw_CachePic( va("gfx/menu/menudot%i.lmp", f+1 ) ) );

	if (message)
	{
		M_PrintWhite ((320/2) - ((27*8)/2), 168, message);
		M_PrintWhite ((320/2) - ((27*8)/2), 176, message2);
		if (realtime - 5 > message_time)
			message = NULL;
	}
}


static void M_MultiPlayer_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Main_f ();
		break;

	case K_DOWNARROW:
		S_LocalSound ("raven/menu1.wav");
		if (++m_multiplayer_cursor >= MULTIPLAYER_ITEMS)
			m_multiplayer_cursor = 0;
		break;

	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
		if (--m_multiplayer_cursor < 0)
			m_multiplayer_cursor = MULTIPLAYER_ITEMS - 1;
		break;

	case K_ENTER:
		m_entersound = true;
		switch (m_multiplayer_cursor)
		{
		case 0:
			M_Menu_Connect_f ();
			break;

		case 1:
			M_Menu_Setup_f ();
			break;
		}
	}
}

//=============================================================================
/* CONNECT MENU */

#define MAX_HOST_NAMES 10
#define MAX_HOST_SIZE 80
static char save_names[MAX_HOST_NAMES][MAX_HOST_SIZE];

static cvar_t	hostname1 = {"host1", "", CVAR_ARCHIVE};
static cvar_t	hostname2 = {"host2", "", CVAR_ARCHIVE};
static cvar_t	hostname3 = {"host3", "", CVAR_ARCHIVE};
static cvar_t	hostname4 = {"host4", "", CVAR_ARCHIVE};
static cvar_t	hostname5 = {"host5", "", CVAR_ARCHIVE};
static cvar_t	hostname6 = {"host6", "", CVAR_ARCHIVE};
static cvar_t	hostname7 = {"host7", "", CVAR_ARCHIVE};
static cvar_t	hostname8 = {"host8", "", CVAR_ARCHIVE};
static cvar_t	hostname9 = {"host9", "", CVAR_ARCHIVE};
static cvar_t	hostname10 = {"host10", "", CVAR_ARCHIVE};

static int connect_cursor = 0;
#define MAX_CONNECT_CMDS 11

static const int	connect_cursor_table[MAX_CONNECT_CMDS] =
{
	72 + 0 * 8,
	72 + 1 * 8,
	72 + 2 * 8,
	72 + 3 * 8,
	72 + 4 * 8,
	72 + 5 * 8,
	72 + 6 * 8,
	72 + 7 * 8,
	72 + 8 * 8,
	72 + 9 * 8,

	72 +11 * 8,
};


static void M_Menu_Connect_f (void)
{
	Key_SetDest (key_menu);
	m_state = m_mconnect;
	m_entersound = true;

	message = NULL;

	q_strlcpy(save_names[0], hostname1.string, sizeof(save_names[0]));
	q_strlcpy(save_names[1], hostname2.string, sizeof(save_names[0]));
	q_strlcpy(save_names[2], hostname3.string, sizeof(save_names[0]));
	q_strlcpy(save_names[3], hostname4.string, sizeof(save_names[0]));
	q_strlcpy(save_names[4], hostname5.string, sizeof(save_names[0]));
	q_strlcpy(save_names[5], hostname6.string, sizeof(save_names[0]));
	q_strlcpy(save_names[6], hostname7.string, sizeof(save_names[0]));
	q_strlcpy(save_names[7], hostname8.string, sizeof(save_names[0]));
	q_strlcpy(save_names[8], hostname9.string, sizeof(save_names[0]));
	q_strlcpy(save_names[9], hostname10.string, sizeof(save_names[0]));
}

static void M_Connect_Draw (void)
{
	int		length;
	int		i,y;
	char	temp[MAX_HOST_SIZE];

	ScrollTitle("gfx/menu/title4.lmp");

	if (connect_cursor < MAX_HOST_NAMES)
	{
		M_DrawTextBox (16, 48, 34, 1);

		q_strlcpy(temp, save_names[connect_cursor], sizeof(save_names[0]));
		length = strlen(temp);
		if (length > 33)
		{
			i = length-33;
		}
		else
		{
			i = 0;
		}
		M_Print (24, 56, &temp[i]);
		M_DrawCharacter (24 + 8*(length-i), 56, 10+((int)(realtime*4)&1));
	}

	y = 72;
	for (i = 0; i < MAX_HOST_NAMES; i++, y += 8)
	{
		q_snprintf(temp, sizeof(temp), "%d.", i+1);
		if (i == connect_cursor)
		{
			M_Print(24,y,temp);
		}
		else
		{
			M_PrintWhite(24,y,temp);
		}

		q_strlcpy(temp, save_names[i], sizeof(save_names[0]));
		temp[30] = 0;
		if (i == connect_cursor)
		{
			M_Print(56, y, temp);
		}
		else
		{
			M_PrintWhite(56, y, temp);
		}
	}

	M_Print (24, y+8, "Save Changes");

	M_DrawCharacter (8, connect_cursor_table[connect_cursor], 12+((int)(realtime*4)&1));
}

static void M_Connect_Key (int k)
{
	int			l;

	switch (k)
	{
	case K_ESCAPE:
		M_Menu_MultiPlayer_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
		connect_cursor--;
		if (connect_cursor < 0)
			connect_cursor = MAX_CONNECT_CMDS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("raven/menu1.wav");
		connect_cursor++;
		if (connect_cursor >= MAX_CONNECT_CMDS)
			connect_cursor = 0;
		break;

	case K_ENTER:
		Cvar_Set ("host1",save_names[0]);
		Cvar_Set ("host2",save_names[1]);
		Cvar_Set ("host3",save_names[2]);
		Cvar_Set ("host4",save_names[3]);
		Cvar_Set ("host5",save_names[4]);
		Cvar_Set ("host6",save_names[5]);
		Cvar_Set ("host7",save_names[6]);
		Cvar_Set ("host8",save_names[7]);
		Cvar_Set ("host9",save_names[8]);
		Cvar_Set ("host10",save_names[9]);

		if (connect_cursor < MAX_HOST_NAMES)
		{
			Key_SetDest (key_game);
			m_state = m_none;
			Cbuf_AddText ( va ("connect %s\n", save_names[connect_cursor]) );
		}
		else
		{
			m_entersound = true;
			M_Menu_MultiPlayer_f ();
		}
		break;

	case K_BACKSPACE:
		if (connect_cursor < MAX_HOST_NAMES)
		{
			l = strlen(save_names[connect_cursor]);
			if (l)
			{
				save_names[connect_cursor][l-1] = 0;
			}
		}
		break;

	default:
		if (k < 32 || k > 127)
			break;
		if (connect_cursor < MAX_HOST_NAMES)
		{
			l = strlen(save_names[connect_cursor]);
			if (l < MAX_HOST_SIZE-1)
			{
				save_names[connect_cursor][l+1] = 0;
				save_names[connect_cursor][l] = k;
			}
		}
	}
}


//=============================================================================
/* SETUP MENU */

static int		setup_cursor = 6;
static const int	setup_cursor_table[] = {40, 56, 72, 88, 112, 136, 164};

static char	setup_myname[16];
static int		setup_oldtop;
static int		setup_oldbottom;
static int		setup_top;
static int		setup_bottom;
static qboolean		is_siege;

#define	NUM_SETUP_CMDS	7

extern cvar_t	name;
extern cvar_t	topcolor;
extern cvar_t	bottomcolor;

static void M_Menu_Setup_f (void)
{
	Key_SetDest (key_menu);
	m_state = m_setup;
	m_entersound = true;
	q_strlcpy(setup_myname, name.string, sizeof(setup_myname));
	setup_top = setup_oldtop = topcolor.integer;
	setup_bottom = setup_oldbottom = bottomcolor.integer;
	is_siege = (q_strcasecmp(fs_gamedir_nopath, "siege") == 0);

#if ENABLE_OLD_DEMO
	if (gameflags & GAME_OLD_DEMO)
	{
		// FIXME: doesn't handle random class (setup_class == 0) feature.
		if (playerclass.integer != CLASS_PALADIN && playerclass.integer != CLASS_THEIF)
			Cvar_SetValue ("playerclass", CLASS_PALADIN);
	}
#endif	/* OLD_DEMO */
	if (playerclass.integer == CLASS_DEMON)
	{
		if (!(gameflags & GAME_PORTALS))
			Cvar_SetValue ("playerclass", CLASS_PALADIN);
	}
	if (playerclass.integer == CLASS_DWARF)
	{
		if (!is_siege)
			Cvar_SetValue ("playerclass", CLASS_PALADIN);
	}

	setup_class = playerclass.integer;

	if (setup_class < 0 || setup_class > MAX_PLAYER_CLASS)
		setup_class = CLASS_PALADIN;
	which_class = setup_class;
}


static void M_DrawTransPicTranslate (int x, int y, qpic_t *pic, int p_class)
{
	Draw_TransPicTranslate (x + ((vid.width - 320)>>1), y, pic, translationTable, p_class);
}

static void M_Setup_Draw (void)
{
	qpic_t			*p;
	int				i;
	static qboolean wait;

	ScrollTitle("gfx/menu/title4.lmp");

	M_Print (64, 56, "Your name");
	M_DrawTextBox (160, 48, 16, 1);
	M_PrintWhite (168, 56, setup_myname);

	M_Print (64, 72, "Spectator: ");
	if (spectator.integer)
	{
		M_PrintWhite (64 + 12*8, 72, "YES");
	}
	else
	{
		M_PrintWhite (64 + 12*8, 72, "NO");
	}

	M_Print (64, 88, "Current Class: ");

	switch (setup_class)
	{
	case 0:
		M_PrintWhite (88, 96, "Random");
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		M_PrintWhite (88, 96, ClassNames[setup_class-1]);
		break;
	}

	M_Print (64, 112, "First color patch");
	M_Print (64, 136, "Second color patch");

	M_DrawTextBox (64, 164-8, 14, 1);
	M_Print (72, 164, "Accept Changes");

	if (setup_class == 0)
	{
		i = (int)(realtime * 10)%8;

		if ((i == 0 && !wait) || which_class == 0)
		{
			if (!(gameflags & GAME_PORTALS))
			{//not succubus
				if (!is_siege)
					which_class = (rand() % CLASS_THEIF) + 1;
				else
				{
					which_class = (rand() % CLASS_DEMON) + 1;
					if (which_class == CLASS_DEMON)
						which_class = CLASS_DWARF;
				}
			}
			else
			{
				if (!is_siege)
					which_class = (rand() % CLASS_DEMON) + 1;
				else
					which_class = (rand() % CLASS_DWARF) + 1;
			}
			wait = true;
		}
		else if (i)
		{
			wait = false;
		}
	}
	else
	{
		which_class = setup_class;
	}
	p = Draw_CachePic (va("gfx/menu/netp%i.lmp",which_class));
	M_BuildTranslationTable(setup_top, setup_bottom);

	/* garymct */
	M_DrawTransPicTranslate (220, 72, p, which_class);

	M_DrawCharacter (56, setup_cursor_table [setup_cursor], 12+((int)(realtime*4)&1));

	if (setup_cursor == 1)
		M_DrawCharacter (168 + 8*strlen(setup_myname), setup_cursor_table [setup_cursor], 10+((int)(realtime*4)&1));
}


static void M_Setup_Key (int k)
{
	int			l;

	switch (k)
	{
	case K_ESCAPE:
		M_Menu_MultiPlayer_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
		setup_cursor--;
		if (setup_cursor < 1)
			setup_cursor = NUM_SETUP_CMDS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("raven/menu1.wav");
		setup_cursor++;
		if (setup_cursor >= NUM_SETUP_CMDS)
			setup_cursor = 1;
		break;

	case K_LEFTARROW:
		if (setup_cursor < 2)
			return;
		S_LocalSound ("raven/menu3.wav");
		if (setup_cursor == 2)
		{
			Cvar_Set("spectator", spectator.integer ? "0" : "1");
			cl.spectator = spectator.integer;
		}
		else if (setup_cursor == 3)
		{
#if ENABLE_OLD_DEMO
			if (gameflags & GAME_OLD_DEMO)
			{
			// FIXME: doesn't handle random class (setup_class == 0) feature.
				setup_class = (setup_class == CLASS_PALADIN) ? CLASS_THEIF : CLASS_PALADIN;
				break;
			}
#endif	/* OLD_DEMO */
			setup_class--;
			if (setup_class < 0)
				setup_class = MAX_PLAYER_CLASS;
			if (!is_siege && setup_class == CLASS_DWARF)
				setup_class--;
			if (!(gameflags & GAME_PORTALS) && setup_class == CLASS_DEMON)
				setup_class--;
		}
		else if (setup_cursor == 4)
			setup_top = setup_top - 1;
		else if (setup_cursor == 5)
			setup_bottom = setup_bottom - 1;
		break;
	case K_RIGHTARROW:
		if (setup_cursor < 2)
			return;
forward:
		S_LocalSound ("raven/menu3.wav");
		if (setup_cursor == 2)
		{
			Cvar_Set("spectator", spectator.integer ? "0" : "1");
			cl.spectator = spectator.integer;
		}
		else if (setup_cursor == 3)
		{
#if ENABLE_OLD_DEMO
			if (gameflags & GAME_OLD_DEMO)
			{
			// FIXME: doesn't handle random class (setup_class == 0) feature.
				setup_class = (setup_class == CLASS_PALADIN) ? CLASS_THEIF : CLASS_PALADIN;
				break;
			}
#endif	/* OLD_DEMO */
			setup_class++;
			if (!(gameflags & GAME_PORTALS) && setup_class == CLASS_DEMON)
				setup_class++;
			if (!is_siege && setup_class == CLASS_DWARF)
				setup_class++;
			if (setup_class > MAX_PLAYER_CLASS)
				setup_class = 0;
		}
		else if (setup_cursor == 4)
			setup_top = setup_top + 1;
		else if (setup_cursor == 5)
			setup_bottom = setup_bottom + 1;
		break;

	case K_ENTER:
		if (setup_cursor == 0 || setup_cursor == 1)
			return;

		if (setup_cursor == 2 || setup_cursor == 3 || setup_cursor == 4 || setup_cursor == 5)
			goto forward;

		if (strcmp(name.string, setup_myname) != 0)
			Cbuf_AddText ( va ("name \"%s\"\n", setup_myname) );
		if (setup_top != setup_oldtop || setup_bottom != setup_oldbottom)
			Cbuf_AddText( va ("color %i %i\n", setup_top, setup_bottom) );
		Cbuf_AddText ( va ("playerclass %d\n", setup_class) );
		m_entersound = true;
		M_Menu_MultiPlayer_f ();
		break;

	case K_BACKSPACE:
		if (setup_cursor == 1)
		{
			if (strlen(setup_myname))
				setup_myname[strlen(setup_myname)-1] = 0;
		}
		break;

	default:
		if (k < 32 || k > 127)
			break;
		if (setup_cursor == 1)
		{
			l = strlen(setup_myname);
			if (l < 15)
			{
				setup_myname[l+1] = 0;
				setup_myname[l] = k;
			}
		}
	}

	if (setup_top > 10)
		setup_top = 0;
	else if (setup_top < 0)
		setup_top = 10;
	if (setup_bottom > 10)
		setup_bottom = 0;
	else if (setup_bottom < 0)
		setup_bottom = 10;
}


//=============================================================================
/* Menu Subsystem */

void M_Init (void)
{
	char		*ptr;

	ptr = (char *) FS_LoadTempFile (BIGCHAR_WIDTH_FILE, NULL);
	if (ptr == NULL)
		M_BuildBigCharWidth();
	else
	{
		if (fs_filesize == sizeof(BigCharWidth))
			memcpy (BigCharWidth, ptr, sizeof(BigCharWidth));
		else
		{
			Con_Printf ("Unexpected file size (%lu) for %s\n",
					(unsigned long)fs_filesize, BIGCHAR_WIDTH_FILE);
			M_BuildBigCharWidth();
		}
	}

	Cmd_AddCommand ("togglemenu", M_ToggleMenu_f);

	Cmd_AddCommand ("menu_main", M_Menu_Main_f);
	Cmd_AddCommand ("menu_options", M_Menu_Options_f);
	Cmd_AddCommand ("menu_keys", M_Menu_Keys_f);
	Cmd_AddCommand ("menu_video", M_Menu_Video_f);
	Cmd_AddCommand ("help", M_Menu_Help_f);
	Cmd_AddCommand ("menu_quit", M_Menu_Quit_f);
	Cmd_AddCommand ("menu_connect" , M_Menu_Connect_f);

	Cvar_RegisterVariable (&hostname1);
	Cvar_RegisterVariable (&hostname2);
	Cvar_RegisterVariable (&hostname3);
	Cvar_RegisterVariable (&hostname4);
	Cvar_RegisterVariable (&hostname5);
	Cvar_RegisterVariable (&hostname6);
	Cvar_RegisterVariable (&hostname7);
	Cvar_RegisterVariable (&hostname8);
	Cvar_RegisterVariable (&hostname9);
	Cvar_RegisterVariable (&hostname10);

	memset (old_bgmtype, 0, sizeof(old_bgmtype));
}


void M_Draw (void)
{
	if (m_state == m_none || !(Key_GetDest() & key_menu))
		return;

	if (!m_recursiveDraw)
	{
		scr_copyeverything = 1;

		if (scr_con_current)
		{
			Draw_ConsoleBackground (vid.height);
			VID_UnlockBuffer ();
			S_ExtraUpdate ();
			VID_LockBuffer ();
		}
		else
			Draw_FadeScreen ();

		scr_fullupdate = 0;
	}
	else
	{
		m_recursiveDraw = false;
	}

	switch (m_state)
	{
	case m_none:
		break;

	case m_main:
		M_Main_Draw ();
		break;

	case m_multiplayer:
		M_MultiPlayer_Draw ();
		break;

	case m_setup:
		M_Setup_Draw ();
		break;

	case m_options:
		M_Options_Draw ();
		break;

#ifdef GLQUAKE
	case m_opengl:
		M_OpenGL_Draw ();
		break;
#endif

	case m_keys:
		M_Keys_Draw ();
		break;

	case m_video:
		M_Video_Draw ();
		break;

	case m_help:
		M_Help_Draw ();
		break;

	case m_quit:
		M_Quit_Draw ();
		break;

	case m_mconnect:
		M_Connect_Draw ();
		break;
	}

	if (m_entersound)
	{
		S_LocalSound ("raven/menu2.wav");
		m_entersound = false;
	}

	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();
}


void M_Keybind (int key)
{
	char	cmd[80];
	S_LocalSound ("raven/menu1.wav");
	if (key != K_ESCAPE && key != '`')
	{
		q_snprintf (cmd, sizeof(cmd), "bind \"%s\" \"%s\"\n",
			    Key_KeynumToString (key), bindnames[keys_cursor][0]);
		Cbuf_InsertText (cmd);
	}

	Key_SetDest (key_menu);
}

void M_Keydown (int key)
{
	switch (m_state)
	{
	case m_none:
		return;

	case m_main:
		M_Main_Key (key);
		return;

	case m_multiplayer:
		M_MultiPlayer_Key (key);
		return;

	case m_setup:
		M_Setup_Key (key);
		return;

	case m_options:
		M_Options_Key (key);
		return;

#ifdef GLQUAKE
	case m_opengl:
		M_OpenGL_Key (key);
		break;
#endif

	case m_keys:
		M_Keys_Key (key);
		return;

	case m_video:
		M_Video_Key (key);
		return;

	case m_help:
		M_Help_Key (key);
		return;

	case m_quit:
		M_Quit_Key (key);
		return;

	case m_mconnect:
		M_Connect_Key (key);
		break;
	}
}


static void BGM_RestartMusic (void)
{
	// called after exitting the menus and changing the music type
	// this is pretty crude, but doen't seem to break anything S.A

	if (q_strcasecmp(bgmtype.string,"midi") == 0)
	{
		CDAudio_Stop();
		BGM_PlayMIDIorMusic(cl.midi_name);
	}
	else if (q_strcasecmp(bgmtype.string,"cd") == 0)
	{
		BGM_Stop();
		CDAudio_Play ((byte)cl.cdtrack, true);
	}
	else
	{
		CDAudio_Stop();
		BGM_Stop();
	}
}

