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
static void M_Menu_SinglePlayer_f (void);
static void M_Menu_Load_f (void);
static void M_Menu_Save_f (void);
static void M_Menu_MultiPlayer_f (void);
static void M_Menu_Setup_f (void);
static void M_Menu_Net_f (void);
void M_Menu_Options_f (void);
static void M_Menu_Keys_f (void);
static void M_Menu_Video_f (void);
static void M_Menu_Help_f (void);
void M_Menu_Quit_f (void);
static void M_Menu_LanConfig_f (void);
static void M_Menu_GameOptions_f (void);
static void M_Menu_Search_f (void);
static void M_Menu_ServerList_f (void);

static void M_Main_Draw (void);
static void M_SinglePlayer_Draw (void);
static void M_Load_Draw (void);
static void M_Save_Draw (void);
static void M_MultiPlayer_Draw (void);
static void M_Setup_Draw (void);
static void M_Net_Draw (void);
static void M_Options_Draw (void);
static void M_Keys_Draw (void);
static void M_Video_Draw (void);
static void M_Help_Draw (void);
static void M_Quit_Draw (void);
static void M_LanConfig_Draw (void);
static void M_GameOptions_Draw (void);
static void M_Search_Draw (void);
static void M_ServerList_Draw (void);

static void M_Main_Key (int key);
static void M_SinglePlayer_Key (int key);
static void M_Load_Key (int key);
static void M_Save_Key (int key);
static void M_MultiPlayer_Key (int key);
static void M_Setup_Key (int key);
static void M_Net_Key (int key);
static void M_Options_Key (int key);
static void M_Keys_Key (int key);
static void M_Video_Key (int key);
static void M_Help_Key (int key);
static void M_Quit_Key (int key);
static void M_LanConfig_Key (int key);
static void M_GameOptions_Key (int key);
static void M_Search_Key (int key);
static void M_ServerList_Key (int key);

#if defined(NET_USE_SERIAL)
static void M_Menu_SerialConfig_f (void);
static void M_Menu_ModemConfig_f (void);
static void M_SerialConfig_Draw (void);
static void M_ModemConfig_Draw (void);
static void M_SerialConfig_Key (int key);
static void M_ModemConfig_Key (int key);
#endif	/* NET_USE_SERIAL */

static qboolean	m_entersound;		// play after drawing a frame, so caching
					// won't disrupt the sound
static qboolean	m_recursiveDraw;

enum m_state_e	m_return_state;
qboolean	m_return_onerror;
char		m_return_reason [32];

qboolean	menu_disabled_mouse = false;

static float	TitlePercent = 0;
static float	TitleTargetPercent = 1;
static float	LogoPercent = 0;
static float	LogoTargetPercent = 1;

static int	setup_class;

static const char	*msave_message, *msave_message2;
static double	message_time;


static void M_ConfigureNetSubsystem(void);

#define StartingGame	(m_multiplayer_cursor == 1)
#define JoiningGame		(m_multiplayer_cursor == 0)

#if !defined(NET_USE_SERIAL)
#define	_nums_serial		0
#define	_ser_draw_offset	8		/* incr. the Y offset this much pixels */
#else
#define	_item_net_ser		0		/* order of serial menu entry */
#define	_item_net_dc		1		/* order of direct connect menu entry */
#define	_nums_serial		2
#define	_ser_draw_offset	0
#define	SerialConfig		(m_net_cursor == _item_net_ser)
#define	DirectConfig		(m_net_cursor == _item_net_dc)
#endif

#define	_item_net_ipx		(0 + _nums_serial)	/* order of IPX menu entry */
#define	_item_net_tcp		(1 + _nums_serial)	/* order of TCP menu entry */

#define	IPXConfig		(m_net_cursor == _item_net_ipx)
#define	TCPIPConfig		(m_net_cursor == _item_net_tcp)


static void M_Menu_Class_f (void);

const char *ClassNames[MAX_PLAYER_CLASS] =
{
	"Paladin",
	"Crusader",
	"Necromancer",
	"Assassin",
	"Demoness"
};

static const char *ClassNamesU[MAX_PLAYER_CLASS] =
{
	"PALADIN",
	"CRUSADER",
	"NECROMANCER",
	"ASSASSIN",
	"DEMONESS"
};

#define	NUM_DIFFLEVELS		4

static const char *DiffNames[MAX_PLAYER_CLASS][NUM_DIFFLEVELS] =
{
	{	// Paladin
		"APPRENTICE",
		"SQUIRE",
		"ADEPT",
		"LORD"
	},

	{	// Crusader
		"GALLANT",
		"HOLY AVENGER",
		"DIVINE HERO",
		"LEGEND"
	},

	{	// Necromancer
		"SORCERER",
		"DARK SERVANT",
		"WARLOCK",
		"LICH KING"
	},

	{	// Assassin
		"ROGUE",
		"CUTTHROAT",
		"EXECUTIONER",
		"WIDOW MAKER"
	},
	{	// Demoness
		"LARVA",
		"SPAWN",
		"FIEND",
		"SHE BITCH"
	}
};


//=============================================================================
/* Support Routines */

/*
================
M_DrawCharacter

Draws one solid graphics character, centered, on line
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

	colorA = playerTranslation + 256 + color_offsets[(int)setup_class-1];
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
	qpic_t	*p, *tm, *bm;
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
	tm = Draw_CachePic ("gfx/box_tm.lmp");
	bm = Draw_CachePic ("gfx/box_bm.lmp");
	while (width > 0)
	{
		cy = y;
		M_DrawTransPic (cx, cy, tm);
		p = Draw_CachePic ("gfx/box_mm.lmp");
		for (n = 0; n < lines; n++)
		{
			cy += 8;
			if (n == 1)
				p = Draw_CachePic ("gfx/box_mm2.lmp");
			M_DrawTransPic (cx, cy, p);
		}
		M_DrawTransPic (cx, cy+8, bm);
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
	if (dest == key_console)
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
			delta = 0.004;
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
			delta = 0.02;
		TitlePercent -= delta;
		if (TitlePercent <= TitleTargetPercent)
		{
			TitlePercent = TitleTargetPercent;
			CanSwitch = true;
		}
	}

	if (LogoPercent < LogoTargetPercent)
	{
		/*
		delta = ((LogoTargetPercent-LogoPercent)/1.1)*host_frametime;
		if (delta < 0.0015)
			delta = 0.0015;
		*/
		delta = ((LogoTargetPercent-LogoPercent)/.15)*host_frametime;
		if (delta < 0.02)
			delta = 0.02;
		LogoPercent += delta;
		if (LogoPercent > LogoTargetPercent)
			LogoPercent = LogoTargetPercent;
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
#define	MAIN_ITEMS	5

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
	M_DrawBigString (72, 60 + (0 * 20), "SINGLE PLAYER");
	M_DrawBigString (72, 60 + (1 * 20), "MULTIPLAYER");
	M_DrawBigString (72, 60 + (2 * 20), "OPTIONS");
	M_DrawBigString (72, 60 + (3 * 20), "HELP");
	M_DrawBigString (72, 60 + (4 * 20), "QUIT");

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
		if (cls.demonum != -1 && !cls.demoplayback && cls.state != ca_connected)
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
			M_Menu_SinglePlayer_f ();
			break;

		case 1:
			M_Menu_MultiPlayer_f ();
			break;

		case 2:
			M_Menu_Options_f ();
			break;

		case 3:
			M_Menu_Help_f ();
			break;

		case 4:
			M_Menu_Quit_f ();
			break;
		}
	}
}


//=============================================================================
/* DIFFICULTY MENU */

static void M_Menu_Difficulty_f (void)
{
	Key_SetDest (key_menu);
	m_state = m_difficulty;
}

static int	m_diff_cursor;
static int	m_enter_portals = 0;
#define	DIFF_ITEMS	NUM_DIFFLEVELS

static void M_Difficulty_Draw (void)
{
	int	f, i;

	ScrollTitle("gfx/menu/title5.lmp");

	setup_class = cl_playerclass.integer;

	if (setup_class < 1 || setup_class > MAX_PLAYER_CLASS)
		setup_class = MAX_PLAYER_CLASS;
	if (setup_class > MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES && !(gameflags & GAME_PORTALS))
		setup_class = MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES;
	setup_class--;

	for (i = 0; i < NUM_DIFFLEVELS; ++i)
		M_DrawBigString (72, 60 + (i * 20), DiffNames[setup_class][i]);

	f = (int)(realtime * 10)%8;
	M_DrawTransPic (43, 54 + m_diff_cursor * 20, Draw_CachePic(va("gfx/menu/menudot%i.lmp", f+1)) );
}

static void M_NewMissionPackGame (void)
{
/* running a new single player mission pack game through
 * the menu system starts intermission screen #12, first.
 * when the user hits a key, Key_Event () gets us out of
 * the intermission by running the keep1 map.
 */
	Key_SetDest (key_game);
	cls.demonum = m_save_demonum;
	CL_SetupIntermission (12);
/* make sure the mouse is active, so that pressing a mouse
 * button can be captured by Key_Event (see above.) */
	menu_disabled_mouse = false;
	IN_ActivateMouse ();
}

static void M_Difficulty_Key (int key)
{
	switch (key)
	{
	case K_LEFTARROW:
	case K_RIGHTARROW:
		break;
	case K_ESCAPE:
		M_Menu_Class_f ();
		break;
	case K_DOWNARROW:
		S_LocalSound ("raven/menu1.wav");
		if (++m_diff_cursor >= DIFF_ITEMS)
			m_diff_cursor = 0;
		break;
	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
		if (--m_diff_cursor < 0)
			m_diff_cursor = DIFF_ITEMS - 1;
		break;
	case K_ENTER:
		Cvar_SetValue ("skill", m_diff_cursor);
		m_entersound = true;
		m_state = m_none;
		if (m_enter_portals)
		{
			M_NewMissionPackGame ();
			return;
		}
		Cbuf_AddText ("wait\n"); /* make m_none to really work */
		Cbuf_AddText ("map demo1\n");
		break;
	default:
		Key_SetDest (key_game);
		m_state = m_none;
		break;
	}
}


//=============================================================================
/* CLASS CHOICE MENU */

static int	class_flag;

static void M_Menu_Class_f (void)
{
	class_flag = 0;
	Key_SetDest (key_menu);
	m_state = m_class;
}

static void M_Menu_Class2_f (void)
{
	Key_SetDest (key_menu);
	m_state = m_class;
	class_flag = 1;
}

static int	m_class_cursor;
#define	CLASS_ITEMS	MAX_PLAYER_CLASS

static void M_Class_Draw (void)
{
	int	i, f = MAX_PLAYER_CLASS;

	if (! (gameflags & GAME_PORTALS))
		f = MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES;
#if DISALLOW_DEMONESS_IN_OLD_GAME
	else if (!m_enter_portals)
		f = MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES;
#endif

	if (m_class_cursor >= f)
		m_class_cursor = 0;

	ScrollTitle("gfx/menu/title2.lmp");
	for (i = 0; i < f; ++i)
		M_DrawBigString (72, 60 + (i * 20), ClassNamesU[i]);

	f = (int)(realtime * 10)%8;
	M_DrawTransPic (43, 54 + m_class_cursor * 20, Draw_CachePic(va("gfx/menu/menudot%i.lmp", f+1)) );

	M_DrawPic (251, 54 + 21, Draw_CachePic (va("gfx/cport%d.lmp", m_class_cursor + 1)));
	M_DrawTransPic (242, 54, Draw_CachePic ("gfx/menu/frame.lmp"));
}

static void M_Class_Key (int key)
{
	int		f = MAX_PLAYER_CLASS;

	if (! (gameflags & GAME_PORTALS))
		f = MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES;
#if DISALLOW_DEMONESS_IN_OLD_GAME
	else if (!m_enter_portals)
		f = MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES;
#endif

	switch (key)
	{
	case K_LEFTARROW:
	case K_RIGHTARROW:
		break;
	case K_ESCAPE:
		M_Menu_SinglePlayer_f ();
		break;
	case K_DOWNARROW:
		S_LocalSound ("raven/menu1.wav");
#if ENABLE_OLD_DEMO
		if (gameflags & GAME_OLD_DEMO)
			m_class_cursor = (m_class_cursor == CLASS_PALADIN-1) ? CLASS_THEIF-1 : CLASS_PALADIN-1;
		else
#endif	/* OLD_DEMO */
		if (++m_class_cursor >= f)
			m_class_cursor = 0;
		break;

	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
#if ENABLE_OLD_DEMO
		if (gameflags & GAME_OLD_DEMO)
			m_class_cursor = (m_class_cursor == CLASS_PALADIN-1) ? CLASS_THEIF-1 : CLASS_PALADIN-1;
		else
#endif	/* OLD_DEMO */
		if (--m_class_cursor < 0)
			m_class_cursor = f - 1;
		break;

	case K_ENTER:
		Cbuf_AddText ( va ("playerclass %d\n", m_class_cursor+1) );
		m_entersound = true;
		if (!class_flag)
		{
			M_Menu_Difficulty_f();
		}
		else
		{
			Key_SetDest (key_game);
			m_state = m_none;
		}
		break;
	default:
		Key_SetDest (key_game);
		m_state = m_none;
		break;
	}
}


//=============================================================================
/* SINGLE PLAYER MENU */

#define SINGLEPLAYER_ITEMS	3
#define	SP_PORTALS_ITEMS	2

static int	m_singleplayer_cursor;

static void M_Menu_SinglePlayer_f (void)
{
	Key_SetDest (key_menu);
	m_state = m_singleplayer;
	m_entersound = true;
	Cvar_Set ("timelimit", "0");		//put this here to help play single after dm
}

static void M_SinglePlayer_Draw (void)
{
	int	f;

	ScrollTitle("gfx/menu/title1.lmp");

	if (gameflags & GAME_PORTALS)
		M_DrawBigString (72, 60 + (0 * 20), "NEW MISSION");
	else
		M_DrawBigString (72, 60 + (0 * 20), "NEW GAME");

	M_DrawBigString (72, 60 + (1 * 20), "LOAD");
	M_DrawBigString (72, 60 + (2 * 20), "SAVE");

	if (gameflags & GAME_PORTALS)
	{
		M_DrawBigString (72, 60 + (3 * 20), "OLD MISSION");
		M_DrawBigString (72, 60 + (4 * 20), "VIEW INTRO");
	}

	f = (int)(realtime * 10)%8;
	M_DrawTransPic (43, 54 + m_singleplayer_cursor * 20, Draw_CachePic(va("gfx/menu/menudot%i.lmp", f+1)) );
}


static void M_SinglePlayer_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Main_f ();
		break;
	case K_DOWNARROW:
		S_LocalSound ("raven/menu1.wav");
		m_singleplayer_cursor++;
		if (gameflags & GAME_PORTALS)
		{
			if (m_singleplayer_cursor >= SINGLEPLAYER_ITEMS + SP_PORTALS_ITEMS)
				m_singleplayer_cursor = 0;
		}
		else
		{
			if (m_singleplayer_cursor >= SINGLEPLAYER_ITEMS)
				m_singleplayer_cursor = 0;
		}
		break;
	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
		if (--m_singleplayer_cursor < 0)
		{
			if (gameflags & GAME_PORTALS)
				m_singleplayer_cursor = SINGLEPLAYER_ITEMS + SP_PORTALS_ITEMS - 1;
			else
				m_singleplayer_cursor = SINGLEPLAYER_ITEMS - 1;
		}
		break;
	case K_ENTER:
		m_entersound = true;
		m_enter_portals = 0;
		switch (m_singleplayer_cursor)
		{
		case 0:
			if (gameflags & GAME_PORTALS)
				m_enter_portals = 1;
		case 3:
			if (sv.active)
				if (!SCR_ModalMessage("Are you sure you want to\nstart a new game?\n"))
					break;
			Key_SetDest (key_game);
			if (sv.active)
				Cbuf_AddText ("disconnect\n");
			Host_RemoveGIPFiles(NULL);
			Cbuf_AddText ("maxplayers 1\n");
			Cbuf_AddText ("coop 0\n");
			Cbuf_AddText ("deathmatch 0\n");
			M_Menu_Class_f ();
			break;

		case 1:
			M_Menu_Load_f ();
			break;

		case 2:
			M_Menu_Save_f ();
			break;
		case 4:
			if (gameflags & GAME_PORTALS)
			{
				Key_SetDest (key_game);
				Cbuf_AddText("playdemo t9\n");
			}
			break;
		}
	}
}

//=============================================================================
/* LOAD/SAVE MENU */

static int		load_cursor;		// 0 < load_cursor < MAX_SAVEGAMES

static char	m_filenames[MAX_SAVEGAMES][SAVEGAME_COMMENT_LENGTH+1];
static char	savefile[MAX_OSPATH];
static int		loadable[MAX_SAVEGAMES];

static void M_ScanSaves (void)
{
	int	i, j, version;
	char	name[MAX_OSPATH];
	FILE	*f;

	for (i = 0; i < MAX_SAVEGAMES; i++)
	{
		q_strlcpy (m_filenames[i], "--- UNUSED SLOT ---", SAVEGAME_COMMENT_LENGTH+1);
		loadable[i] = false;
		FS_MakePath_VABUF (FS_USERDIR, NULL, name, sizeof(name), "s%i/info.dat", i);
		f = fopen (name, "r");
		if (!f)
			continue;
		fscanf (f, "%i\n", &version);
		if (version != SAVEGAME_VERSION)
		{
			fclose (f);
			continue;
		}
		fscanf (f, "%79s\n", name);
		q_strlcpy (m_filenames[i], name, SAVEGAME_COMMENT_LENGTH+1);

	// change _ back to space
		for (j = 0; j < SAVEGAME_COMMENT_LENGTH; j++)
		{
			if (m_filenames[i][j] == '_')
				m_filenames[i][j] = ' ';
		}
		loadable[i] = true;
		fclose (f);
	}
}

static void M_Menu_Load_f (void)
{
	m_entersound = true;
	m_state = m_load;
	Key_SetDest (key_menu);
	M_ScanSaves ();
}


static void M_Menu_Save_f (void)
{
	if (!sv.active)
		return;
	if (cl.intermission)
		return;
	if (svs.maxclients != 1)
		return;
	m_entersound = true;
	m_state = m_save;
	Key_SetDest (key_menu);
	M_ScanSaves ();
}


static void M_Load_Draw (void)
{
	int		i;

	ScrollTitle("gfx/menu/load.lmp");

	for (i = 0; i < MAX_SAVEGAMES; i++)
		M_Print (16, 60 + 8*i, m_filenames[i]);

// line cursor
	M_DrawCharacter (8, 60 + load_cursor*8, 12+((int)(realtime*4)&1));
}


static void M_Save_Draw (void)
{
	int		i;

	ScrollTitle("gfx/menu/save.lmp");

	for (i = 0; i < MAX_SAVEGAMES; i++)
		M_Print (16, 60 + 8*i, m_filenames[i]);

// line cursor
	M_DrawCharacter (8, 60 + load_cursor*8, 12+((int)(realtime*4)&1));
}


static void M_Load_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
		M_Menu_SinglePlayer_f ();
		break;

	case K_DEL:
		S_LocalSound ("raven/menu2.wav");
		if (!loadable[load_cursor])
			return;
		if (!SCR_ModalMessage("Are you sure you want to\ndelete this saved game?\n"))
			return;
		FS_MakePath_VABUF (FS_USERDIR, NULL, savefile, sizeof(savefile), "s%i", load_cursor);
		Host_DeleteSave (savefile);
		M_ScanSaves ();
		break;

	case K_ENTER:
		S_LocalSound ("raven/menu2.wav");
		if (!loadable[load_cursor])
			return;
		m_state = m_none;
		Key_SetDest (key_game);

	// Host_Loadgame_f can't bring up the loading plaque because too much
	// stack space has been used, so do it now
		SCR_BeginLoadingPlaque ();

	// issue the load command
		Cbuf_AddText (va ("load s%i\n", load_cursor) );
		return;

	case K_UPARROW:
	case K_LEFTARROW:
		S_LocalSound ("raven/menu1.wav");
		load_cursor--;
		if (load_cursor < 0)
			load_cursor = MAX_SAVEGAMES-1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("raven/menu1.wav");
		load_cursor++;
		if (load_cursor >= MAX_SAVEGAMES)
			load_cursor = 0;
		break;
	}
}


static void M_Save_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
		M_Menu_SinglePlayer_f ();
		break;

	case K_DEL:
		S_LocalSound ("raven/menu2.wav");
		if (!loadable[load_cursor])
			return;
		if (!SCR_ModalMessage("Are you sure you want to\ndelete this saved game?\n"))
			return;
		FS_MakePath_VABUF (FS_USERDIR, NULL, savefile, sizeof(savefile), "s%i", load_cursor);
		Host_DeleteSave (savefile);
		M_ScanSaves ();
		break;

	case K_ENTER:
		m_state = m_none;
		Key_SetDest (key_game);
		Cbuf_AddText (va("save s%i\n", load_cursor));
		menu_disabled_mouse = false;
		IN_ActivateMouse ();
		return;

	case K_UPARROW:
	case K_LEFTARROW:
		S_LocalSound ("raven/menu1.wav");
		load_cursor--;
		if (load_cursor < 0)
			load_cursor = MAX_SAVEGAMES-1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("raven/menu1.wav");
		load_cursor++;
		if (load_cursor >= MAX_SAVEGAMES)
			load_cursor = 0;
		break;
	}
}


//=============================================================================
/* MULTIPLAYER LOAD/SAVE MENU */

static void M_ScanMSaves (void)
{
	int	i, j, version;
	char	name[MAX_OSPATH];
	FILE	*f;

	for (i = 0; i < MAX_SAVEGAMES; i++)
	{
		q_strlcpy (m_filenames[i], "--- UNUSED SLOT ---", SAVEGAME_COMMENT_LENGTH+1);
		loadable[i] = false;
		FS_MakePath_VABUF (FS_USERDIR, NULL, name, sizeof(name), "ms%i/info.dat", i);
		f = fopen (name, "r");
		if (!f)
			continue;
		fscanf (f, "%i\n", &version);
		if (version != SAVEGAME_VERSION)
		{
			fclose (f);
			continue;
		}
		fscanf (f, "%79s\n", name);
		q_strlcpy (m_filenames[i], name, SAVEGAME_COMMENT_LENGTH+1);

	// change _ back to space
		for (j = 0; j < SAVEGAME_COMMENT_LENGTH; j++)
		{
			if (m_filenames[i][j] == '_')
				m_filenames[i][j] = ' ';
		}
		loadable[i] = true;
		fclose (f);
	}
}

static void M_Menu_MLoad_f (void)
{
	m_entersound = true;
	m_state = m_mload;
	Key_SetDest (key_menu);
	M_ScanMSaves ();
}


static void M_Menu_MSave_f (void)
{
	if (!sv.active || cl.intermission || svs.maxclients == 1)
	{
		msave_message = "Only a network server";
		msave_message2 = "can save a multiplayer game";
		message_time = realtime;
		return;
	}
	m_entersound = true;
	m_state = m_msave;
	Key_SetDest (key_menu);
	M_ScanMSaves ();
}


static void M_MLoad_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
		M_Menu_MultiPlayer_f ();
		break;

	case K_DEL:
		S_LocalSound ("raven/menu2.wav");
		if (!loadable[load_cursor])
			return;
		if (!SCR_ModalMessage("Are you sure you want to\ndelete this saved game?\n"))
			return;
		FS_MakePath_VABUF (FS_USERDIR, NULL, savefile, sizeof(savefile), "ms%i", load_cursor);
		Host_DeleteSave (savefile);
		M_ScanMSaves ();
		break;

	case K_ENTER:
		S_LocalSound ("raven/menu2.wav");
		if (!loadable[load_cursor])
			return;
		m_state = m_none;
		Key_SetDest (key_game);

		if (sv.active)
			Cbuf_AddText ("disconnect\n");
		Cbuf_AddText ("listen 1\n");	// so host_netport will be re-examined

	// Host_Loadgame_f can't bring up the loading plaque because too much
	// stack space has been used, so do it now
		SCR_BeginLoadingPlaque ();

	// issue the load command
		Cbuf_AddText (va ("load ms%i\n", load_cursor) );
		return;

	case K_UPARROW:
	case K_LEFTARROW:
		S_LocalSound ("raven/menu1.wav");
		load_cursor--;
		if (load_cursor < 0)
			load_cursor = MAX_SAVEGAMES-1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("raven/menu1.wav");
		load_cursor++;
		if (load_cursor >= MAX_SAVEGAMES)
			load_cursor = 0;
		break;
	}
}


static void M_MSave_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
		M_Menu_MultiPlayer_f ();
		break;

	case K_DEL:
		S_LocalSound ("raven/menu2.wav");
		if (!loadable[load_cursor])
			return;
		if (!SCR_ModalMessage("Are you sure you want to\ndelete this saved game?\n"))
			return;
		FS_MakePath_VABUF (FS_USERDIR, NULL, savefile, sizeof(savefile), "ms%i", load_cursor);
		Host_DeleteSave (savefile);
		M_ScanMSaves ();
		break;

	case K_ENTER:
		m_state = m_none;
		Key_SetDest (key_game);
		Cbuf_AddText (va("save ms%i\n", load_cursor));
		menu_disabled_mouse = false;
		IN_ActivateMouse ();
		return;

	case K_UPARROW:
	case K_LEFTARROW:
		S_LocalSound ("raven/menu1.wav");
		load_cursor--;
		if (load_cursor < 0)
			load_cursor = MAX_SAVEGAMES-1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("raven/menu1.wav");
		load_cursor++;
		if (load_cursor >= MAX_SAVEGAMES)
			load_cursor = 0;
		break;
	}
}

//=============================================================================
/* MULTIPLAYER MENU */

static int	m_multiplayer_cursor;
#define	MULTIPLAYER_ITEMS	5

static void M_Menu_MultiPlayer_f (void)
{
	Key_SetDest (key_menu);
	m_state = m_multiplayer;
	m_entersound = true;

	msave_message = NULL;
}


static void M_MultiPlayer_Draw (void)
{
	int	f;

	ScrollTitle("gfx/menu/title4.lmp");
//	M_DrawTransPic (72, 32, Draw_CachePic ("gfx/mp_menu.lmp") );

	M_DrawBigString (72, 60 + (0 * 20), "JOIN A GAME");
	M_DrawBigString (72, 60 + (1 * 20), "NEW GAME");
	M_DrawBigString (72, 60 + (2 * 20), "SETUP");
	M_DrawBigString (72, 60 + (3 * 20), "LOAD");
	M_DrawBigString (72, 60 + (4 * 20), "SAVE");

	f = (int)(realtime * 10)%8;
	M_DrawTransPic (43, 54 + m_multiplayer_cursor * 20,Draw_CachePic( va("gfx/menu/menudot%i.lmp", f+1 ) ) );

	if (msave_message)
	{
		M_PrintWhite ((320/2) - ((27*8)/2), 168, msave_message);
		M_PrintWhite ((320/2) - ((27*8)/2), 176, msave_message2);
		if (realtime - 5 > message_time)
			msave_message = NULL;
	}

	if (serialAvailable || ipxAvailable || tcpipAvailable)
		return;
	M_PrintWhite ((320/2) - ((27*8)/2), 160, "No Communications Available");
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
			if (serialAvailable || ipxAvailable || tcpipAvailable)
				M_Menu_Net_f ();
			break;

		case 1:
			if (serialAvailable || ipxAvailable || tcpipAvailable)
				M_Menu_Net_f ();
			break;

		case 2:
			M_Menu_Setup_f ();
			break;

		case 3:
			M_Menu_MLoad_f ();
			break;

		case 4:
			M_Menu_MSave_f ();
			break;
		}
	}
}

//=============================================================================
/* SETUP MENU */

static int		setup_cursor = 5;
static const int	setup_cursor_table[] = {40, 56, 80, 104, 128, 156};

static char	setup_hostname[16];
static char	setup_myname[16];
static int		setup_oldtop;
static int		setup_oldbottom;
static int		setup_top;
static int		setup_bottom;

#define	NUM_SETUP_CMDS	6

static void M_Menu_Setup_f (void)
{
	Key_SetDest (key_menu);
	m_state = m_setup;
	m_entersound = true;
	q_strlcpy(setup_myname, cl_name.string, sizeof(setup_myname));
	q_strlcpy(setup_hostname, hostname.string, sizeof(setup_hostname));
	setup_top = setup_oldtop = (cl_color.integer >> 4) & 15;
	setup_bottom = setup_oldbottom = cl_color.integer & 15;
	setup_class = cl_playerclass.integer;
	if (setup_class < 1 || setup_class > MAX_PLAYER_CLASS)
		setup_class = MAX_PLAYER_CLASS;
#if ENABLE_OLD_DEMO
	if (gameflags & GAME_OLD_DEMO)
	{
		if (setup_class != CLASS_PALADIN && setup_class != CLASS_THEIF)
			setup_class = CLASS_PALADIN;
	}
	else
#endif	/* OLD_DEMO */
	if (!(gameflags & GAME_PORTALS))
	{
		if (setup_class > MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES)
			setup_class = MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES;
	}
}


static void M_DrawTransPicTranslate (int x, int y, qpic_t *pic, int p_class)
{
	Draw_TransPicTranslate (x + ((vid.width - 320)>>1), y, pic, translationTable, p_class);
}

static void M_Setup_Draw (void)
{
	qpic_t	*p;

	ScrollTitle("gfx/menu/title4.lmp");

	M_Print (64, 40, "Hostname");
	M_DrawTextBox (160, 32, 16, 1);
	M_Print (168, 40, setup_hostname);

	M_Print (64, 56, "Your name");
	M_DrawTextBox (160, 48, 16, 1);
	M_Print (168, 56, setup_myname);

	M_Print (64, 80, "Current Class: ");
	M_Print (88, 88, ClassNames[setup_class-1]);

	M_Print (64, 104, "First color patch");
	M_Print (64, 128, "Second color patch");

	M_DrawTextBox (64, 156-8, 14, 1);
	M_Print (72, 156, "Accept Changes");

	p = Draw_CachePic (va("gfx/menu/netp%i.lmp",setup_class));
	M_BuildTranslationTable(setup_top, setup_bottom);

	/* garymct */
	M_DrawTransPicTranslate (220, 72, p, setup_class);

	M_DrawCharacter (56, setup_cursor_table [setup_cursor], 12+((int)(realtime*4)&1));

	if (setup_cursor == 0)
		M_DrawCharacter (168 + 8*strlen(setup_hostname), setup_cursor_table [setup_cursor], 10+((int)(realtime*4)&1));

	if (setup_cursor == 1)
		M_DrawCharacter (168 + 8*strlen(setup_myname), setup_cursor_table [setup_cursor], 10+((int)(realtime*4)&1));
}


static void M_Setup_Key (int k)
{
	int		l;

	switch (k)
	{
	case K_ESCAPE:
		M_Menu_MultiPlayer_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
		setup_cursor--;
		if (setup_cursor < 0)
			setup_cursor = NUM_SETUP_CMDS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("raven/menu1.wav");
		setup_cursor++;
		if (setup_cursor >= NUM_SETUP_CMDS)
			setup_cursor = 0;
		break;

	case K_LEFTARROW:
		if (setup_cursor < 2)
			return;
		S_LocalSound ("raven/menu3.wav");
		if (setup_cursor == 2)
		{
#if ENABLE_OLD_DEMO
			if (gameflags & GAME_OLD_DEMO)
			{
				setup_class = (setup_class == CLASS_PALADIN) ? CLASS_THEIF : CLASS_PALADIN;
				break;
			}
#endif	/* OLD_DEMO */
			setup_class--;
			if (setup_class < 1)
				setup_class = MAX_PLAYER_CLASS;
			if (setup_class > MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES && !(gameflags & GAME_PORTALS))
				setup_class = MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES;
		}
		else if (setup_cursor == 3)
			setup_top = setup_top - 1;
		else if (setup_cursor == 4)
			setup_bottom = setup_bottom - 1;
		break;
	case K_RIGHTARROW:
		if (setup_cursor < 2)
			return;
forward:
		S_LocalSound ("raven/menu3.wav");
		if (setup_cursor == 2)
		{
#if ENABLE_OLD_DEMO
			if (gameflags & GAME_OLD_DEMO)
			{
				setup_class = (setup_class == CLASS_PALADIN) ? CLASS_THEIF : CLASS_PALADIN;
				break;
			}
#endif	/* OLD_DEMO */
			setup_class++;
			if (setup_class > MAX_PLAYER_CLASS)
				setup_class = 1;
			if (setup_class > MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES && !(gameflags & GAME_PORTALS))
				setup_class = 1;
		}
		else if (setup_cursor == 3)
			setup_top = setup_top + 1;
		else if (setup_cursor == 4)
			setup_bottom = setup_bottom + 1;
		break;

	case K_ENTER:
		if (setup_cursor == 0 || setup_cursor == 1)
			return;

		if (setup_cursor == 2 || setup_cursor == 3 || setup_cursor == 4)
			goto forward;

		if (strcmp(cl_name.string, setup_myname) != 0)
			Cbuf_AddText ( va ("name \"%s\"\n", setup_myname) );
		if (strcmp(hostname.string, setup_hostname) != 0)
			Cvar_Set("hostname", setup_hostname);
		if (setup_top != setup_oldtop || setup_bottom != setup_oldbottom)
			Cbuf_AddText( va ("color %i %i\n", setup_top, setup_bottom) );
		Cbuf_AddText ( va ("playerclass %d\n", setup_class) );
		m_entersound = true;
		M_Menu_MultiPlayer_f ();
		break;

	case K_BACKSPACE:
		if (setup_cursor == 0)
		{
			if (strlen(setup_hostname))
				setup_hostname[strlen(setup_hostname)-1] = 0;
		}

		if (setup_cursor == 1)
		{
			if (strlen(setup_myname))
				setup_myname[strlen(setup_myname)-1] = 0;
		}
		break;

	default:
		if (k < 32 || k > 127)
			break;
		if (setup_cursor == 0)
		{
			l = strlen(setup_hostname);
			if (l < 15)
			{
				setup_hostname[l+1] = 0;
				setup_hostname[l] = k;
			}
		}
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
/* NET MENU */

#define NET_ITEMS	(2 + _nums_serial)

static int	m_net_cursor = 0;

static const char *net_helpMessage[] =
{
/* .........1.........2.... */
#if defined(NET_USE_SERIAL)
  "                        ",
  " Two computers connected",
  "   through two modems.  ",
  "                        ",

  "                        ",
  " Two computers connected",
  " by a null-modem cable. ",
  "                        ",
#endif	/* NET_USE_SERIAL */

  " Novell network LANs    ",
  " or Windows 95 DOS-box. ",
  "                        ",
  "(LAN=Local Area Network)",

  " Commonly used to play  ",
  " over the Internet, but ",
  " also used on a Local   ",
  " Area Network.          "
};

static void M_Menu_Net_f (void)
{
	Key_SetDest (key_menu);
	m_state = m_net;
	m_entersound = true;

	if (m_net_cursor >= NET_ITEMS)
		m_net_cursor = 0;
	m_net_cursor--;
	M_Net_Key (K_DOWNARROW);
}


static void M_Net_Draw (void)
{
	int	f;

	ScrollTitle("gfx/menu/title4.lmp");

#if defined(NET_USE_SERIAL)
	M_DrawBigString (72, (64 + _ser_draw_offset) + (_item_net_ser * 20), "MODEM");
	M_DrawBigString (72, (64 + _ser_draw_offset) + (_item_net_dc  * 20), "DIRECT CONNECT");
#endif
	M_DrawBigString (72, (64 + _ser_draw_offset) + (_item_net_ipx * 20), "IPX");
	M_DrawBigString (72, (64 + _ser_draw_offset) + (_item_net_tcp * 20), "TCP/IP");

	f = (320 - 26*8) / 2;
	M_DrawTextBox (f, 142, 24, 4);
	f += 8;
	M_Print (f, (142 + 1*8), net_helpMessage[m_net_cursor*4 + 0]);
	M_Print (f, (142 + 2*8), net_helpMessage[m_net_cursor*4 + 1]);
	M_Print (f, (142 + 3*8), net_helpMessage[m_net_cursor*4 + 2]);
	M_Print (f, (142 + 4*8), net_helpMessage[m_net_cursor*4 + 3]);

	f = (int)(realtime * 10)%8;
	M_DrawTransPic (43, (56 + _ser_draw_offset) + m_net_cursor * 20, Draw_CachePic(va("gfx/menu/menudot%i.lmp", f+1)) );
}

static void M_Net_Key (int k)
{
again:
	switch (k)
	{
	case K_ESCAPE:
		M_Menu_MultiPlayer_f ();
		break;

	case K_DOWNARROW:
// Tries to re-draw the menu here, and m_net_cursor could be set to -1
//		S_LocalSound ("raven/menu1.wav");
		if (++m_net_cursor >= NET_ITEMS)
			m_net_cursor = 0;
		break;

	case K_UPARROW:
//		S_LocalSound ("raven/menu1.wav");
		if (--m_net_cursor < 0)
			m_net_cursor = NET_ITEMS - 1;
		break;

	case K_ENTER:
		m_entersound = true;
		switch (m_net_cursor)
		{
#if defined(NET_USE_SERIAL)
		case _item_net_ser:
			M_Menu_SerialConfig_f ();
			break;
		case _item_net_dc :
			M_Menu_SerialConfig_f ();
			break;
#endif	/* NET_USE_SERIAL*/
		case _item_net_ipx:
			M_Menu_LanConfig_f ();
			break;
		case _item_net_tcp:
			M_Menu_LanConfig_f ();
			break;
		default:
		// multiprotocol
			break;
		}
		break;
	}

#if defined(NET_USE_SERIAL)
	if (SerialConfig && !serialAvailable)
		goto again;
	if (DirectConfig && !serialAvailable)
		goto again;
#endif	/* NET_USE_SERIAL*/
	if (IPXConfig && !ipxAvailable)
		goto again;
	if (TCPIPConfig && !tcpipAvailable)
		goto again;

	switch (k)
	{
		case K_DOWNARROW:
		case K_UPARROW:
			S_LocalSound ("raven/menu1.wav");
			break;
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
	OPT_CHASE_ACTIVE,
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
#if 0	/* change to 1 if dont want to disable mouse in fullscreen */
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

	case OPT_CHASE_ACTIVE:	// chase_active
		Cvar_Set ("chase_active", chase_active.integer ? "0" : "1");
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

	M_Print (16 + (12 * 8), 60 + 8*OPT_CHASE_ACTIVE,	"Chase Mode");
	M_DrawCheckbox (220, 60 + 8*OPT_CHASE_ACTIVE, chase_active.integer);

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
			m_state = m_none;
			Con_ToggleConsole_f ();
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
#if 0	/* change to 1 if dont want to disable mouse in fullscreen */
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
	{"impulse 13",		"lift object"},
	{"invuse",		"use inv item"},
	{"impulse 44",		"drop inv item"},
	{"+showinfo",		"full inventory"},
	{"+showdm",		"info / frags"},
	{"toggle_dm",		"toggle frags"},
	{"+infoplaque",		"objectives"},	/* command to display the mission pack's objectives */
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

	ScrollTitle("gfx/menu/title6.lmp");

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
		if (++help_page >= NUM_HELP_PAGES)
			help_page = 0;
		break;

	case K_DOWNARROW:
	case K_LEFTARROW:
		m_entersound = true;
		if (--help_page < 0)
			help_page = NUM_HELP_PAGES-1;
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


#define	MAX_LINES	145

static const char *CreditText[MAX_LINES] =
{
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

#define	MAX_LINES2	158

static const char *Credit2Text[MAX_LINES2] =
{
   "Map Master: ",
   "   'Caffeine Buzz' Raffel",
   "",
   "Code Warrior:",
   "   Rick 'Superfly' Johnson",
   "",
   "Grunt Boys:",
   "   'Judah' Ben Gokey",
   "   Bob 'Back In Action' Love",
   "   Mike 'Jethro' Gummelt",
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
   "Pixel Pushers:",
   "   Shane 'Duh' Gurno",
   "   'Slim' Jim Sumwalt",
   "   Mark 'Dad Gummit' Morgan",
   "   Kim 'Toy Master' Lathrop",
   "   'Drop Dead' Ted Halsted",
   "   Rebecca 'Zombie' Rettenmund",
   "   Les 'Be Friends' Dorscheid",
   "",
   "Salad Shooters:",
   "   Mike 'Chaos' Werckle",
   "   Brian 'Mutton Chops' Shubat",
   "",
   "Last Minute Addition:",
   "   Jeff 'Spanky' Dewitt",
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
   "Special thanks to Bob for",
   "remembering 'P' is for Polymorph",
   "",
   "",
   "See the next movie in the long",
   "awaited sequel, starring",
   "Bobby Love in,",
   "   Out of Traction, Back in Action!"
};

#define	MAX_LINES_MP	138

static const char *CreditTextMP[MAX_LINES_MP] =
{
   "Project Director: James Monroe",
   "Creative Director: Brian Raffel",
   "Project Coordinator: Kevin Schilder",
   "",
   "Lead Programmer: James Monroe",
   "",
   "Programming:",
   "   Mike Gummelt",
   "   Josh Weier",
   "",
   "Additional Programming:",
   "   Josh Heitzman",
   "   Nathan Albury",
   "   Rick Johnson",
   "",
   "Assembly Consultant:",
   "   Mr. John Scott",
   "",
   "Lead Design: Jon Zuk",
   "",
   "Design:",
   "   Tom Odell",
   "   Jeremy Statz",
   "   Mike Renner",
   "   Eric Biessman",
   "   Kenn Hoekstra",
   "   Matt Pinkston",
   "   Bobby Duncanson",
   "   Brian Raffel",
   "",
   "Art Director: Les Dorscheid",
   "",
   "Art:",
   "   Kim Lathrop",
   "   Gina Garren",
   "   Joe Koberstein",
   "   Kevin Long",
   "   Jeff Butler",
   "   Scott Rice",
   "   John Payne",
   "   Steve Raffel",
   "",
   "Animation:",
   "   Eric Turman",
   "   Chaos (Mike Werckle)",
   "",
   "Music:",
   "   Kevin Schilder",
   "",
   "Sound:",
   "   Chia Chin Lee",
   "",
   "Activision",
   "",
   "Producer:",
   "   Steve Stringer",
   "",
   "Marketing Product Manager:",
   "   Henk Hartong",
   "",
   "Marketing Associate:",
   "   Kevin Kraff",
   "",
   "Senior Quality",
   "Assurance Lead:",
   "   Tim Vanlaw",
   "",
   "Quality Assurance Lead:",
   "   Doug Jacobs",
   "",
   "Quality Assurance Team:",
   "   Steve Rosenthal, Steve Elwell,",
   "   Chad Bordwell, David Baker,",
   "   Aaron Casillas, Damien Fischer,",
   "   Winnie Lee, Igor Krinitskiy,",
   "   Samantha Lee, John Park",
   "   Ian Stevens, Chris Toft",
   "",
   "Production Testers:",
   "   Steve Rosenthal and",
   "   Chad Bordwell",
   "",
   "Additional QA and Support:",
   "    Tony Villalobos",
   "    Jason Sullivan",
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
   "Our Big Toe:",
   "   Mitch Lasky",
   "",
   "",
   "Special Thanks to:",
   "  Id software",
   "  The original Hexen2 crew",
   "   We couldn't have done it",
   "   without you guys!",
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
   "",
   "No yaks were harmed in the",
   "making of this game!"
};

#define	MAX_LINES2_MP	150

static const char *Credit2TextMP[MAX_LINES2_MP] =
{
   "PowerTrip: James (emorog) Monroe",
   "Cartoons: Brian Raffel",
   "         (use more puzzles)",
   "Doc Keeper: Kevin Schilder",
   "",
   "Whip cracker: James Monroe",
   "",
   "Whipees:",
   "   Mike (i didn't break it) Gummelt",
   "   Josh (extern) Weier",
   "",
   "We don't deserve whipping:",
   "   Josh (I'm not on this project)",
   "         Heitzman",
   "   Nathan (deer hunter) Albury",
   "   Rick (model crusher) Johnson",
   "",
   "Bit Packer:",
   "   Mr. John (Slaine) Scott",
   "",
   "Lead Slacker: Jon (devil boy) Zuk",
   "",
   "Other Slackers:",
   "   Tom (can i have an office) Odell",
   "   Jeremy (nt crashed again) Statz",
   "   Mike (i should be doing my ",
   "         homework) Renner",
   "   Eric (the nose) Biessman",
   "   Kenn (.plan) Hoekstra",
   "   Matt (big elbow) Pinkston",
   "   Bobby (needs haircut) Duncanson",
   "   Brian (they're in my town) Raffel",
   "",
   "Use the mouse: Les Dorscheid",
   "",
   "What's a mouse?:",
   "   Kim (where's my desk) Lathrop",
   "   Gina (i can do your laundry)",
   "        Garren",
   "   Joe (broken axle) Koberstein",
   "   Kevin (titanic) Long",
   "   Jeff (norbert) Butler",
   "   Scott (what's the DEL key for?)",
   "          Rice",
   "   John (Shpluuurt!) Payne",
   "   Steve (crash) Raffel",
   "",
   "Boners:",
   "   Eric (terminator) Turman",
   "   Chaos Device",
   "",
   "Drum beater:",
   "   Kevin Schilder",
   "",
   "Whistle blower:",
   "   Chia Chin (bruce) Lee",
   "",
   "",
   "Activision",
   "",
   "Producer:",
   "   Steve 'Ferris' Stringer",
   "",
   "Marketing Product Manager:",
   "   Henk 'GODMODE' Hartong",
   "",
   "Marketing Associate:",
   "   Kevin 'Kraffinator' Kraff",
   "",
   "Senior Quality",
   "Assurance Lead:",
   "   Tim 'Outlaw' Vanlaw",
   "",
   "Quality Assurance Lead:",
   "   Doug Jacobs",
   "",
   "Shadow Finders:",
   "   Steve Rosenthal, Steve Elwell,",
   "   Chad Bordwell,",
   "   David 'Spice Girl' Baker,",
   "   Error Casillas, Damien Fischer,",
   "   Winnie Lee,"
   "   Ygor Krynytyskyy,",
   "   Samantha (Crusher) Lee, John Park",
   "   Ian Stevens, Chris Toft",
   "",
   "Production Testers:",
   "   Steve 'Damn It's Cold!'",
   "       Rosenthal and",
   "   Chad 'What Hotel Receipt?'",
   "        Bordwell",
   "",
   "Additional QA and Support:",
   "    Tony Villalobos",
   "    Jason Sullivan",
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
   "Our Big Toe:",
   "   Mitch Lasky",
   "",
   "",
   "Special Thanks to:",
   "  Id software",
   "  Anyone who ever worked for Raven,",
   "  (except for Alex)",
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
   "Special Thanks To:",
   "   E.H.S., The Osmonds,",
   "   B.B.V.D., Daisy The Lovin' Lamb,",
   "  'You Killed' Kenny,",
   "   and Baby Biessman.",
   ""
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
	if (gameflags & GAME_PORTALS)
	{
		LineText = CreditTextMP;
		MaxLines = MAX_LINES_MP;
	}
	else
	{
		LineText = CreditText;
		MaxLines = MAX_LINES;
	}
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
		Host_Quit_f ();
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
			if (gameflags & GAME_PORTALS)
			{
				MaxLines = MAX_LINES2_MP;
				LineText = Credit2TextMP;
				CDAudio_Play (12, false);
			}
			else
			{
				MaxLines = MAX_LINES2;
				LineText = Credit2Text;
			}
			LineTxt2 = true;
		}
	}

	y = 12;
	M_DrawTextBox (0, 0, 38, 23);

// the increment to the x offset is for properly centering the line
	M_Print      (16 + (8 * 8), y,		"Hexen II version " STRINGIFY(ENGINE_VERSION));
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
	place = floor(LinePos);
	y -= floor((LinePos - place) * 8);
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
	for (i = 4; i < 38; i++, x += 8)
	{
		M_DrawPic(x, y, p);	// background at top for smooth scroll out
		M_DrawPic(x, y + (QUIT_SIZE*8), p);	// draw at bottom for smooth scroll in
	}

	y += (QUIT_SIZE * 8) + 8;
	M_PrintWhite (16 + (10 * 8), y,  "Press y to exit");
}

//=============================================================================

/* SERIAL CONFIG MENU */

#if defined(NET_USE_SERIAL)

static int	serialConfig_cursor;
static const int	serialConfig_cursor_table[] = { 80, 96, 112, 128, 144, 164 };	// { 48, 64, 80, 96, 112, 132 }
#define	NUM_SERIALCONFIG_CMDS		6

static const int	ISA_uarts[4]	= { 0x3f8, 0x2f8, 0x3e8, 0x2e8};
static const int	ISA_IRQs[4]	= { 4, 3, 4, 3 };
static const int	serialConfig_baudrate[6] = { 9600, 14400, 19200, 28800, 38400, 57600 };

static int	serialConfig_comport;
static int	serialConfig_irq ;
static int	serialConfig_baud;
static char	serialConfig_phone[16];

static void M_Menu_SerialConfig_f (void)
{
	int		n;
	int		port;
	int		baudrate;
	qboolean	useModem;

	Key_SetDest (key_menu);
	m_state = m_serialconfig;
	m_entersound = true;
	if (JoiningGame && SerialConfig)
		serialConfig_cursor = 4;
	else
		serialConfig_cursor = 5;

	(*GetComPortConfig) (0, &port, &serialConfig_irq, &baudrate, &useModem);

	// map uart's port to COMx
	for (n = 0; n < 4; n++)
	{
		if (ISA_uarts[n] == port)
			break;
	}
	if (n == 4)
	{
		n = 0;
		serialConfig_irq = 4;
	}
	serialConfig_comport = n + 1;

	// map baudrate to index
	for (n = 0; n < 6; n++)
	{
		if (serialConfig_baudrate[n] == baudrate)
			break;
	}
	if (n == 6)
		n = 5;
	serialConfig_baud = n;

	m_return_onerror = false;
	m_return_reason[0] = 0;
}

static void M_SerialConfig_Draw (void)
{
	qpic_t	*p;
	int		basex;
	const char	*startJoin;
	const char	*directModem;

	p = Draw_CachePic ("gfx/menu/title4.lmp");
	basex = (320 - p->width) / 2;
	ScrollTitle("gfx/menu/title4.lmp");

	if (StartingGame)
		startJoin = "New Game";
	else
		startJoin = "Join Game";
	if (SerialConfig)
		directModem = "Modem";
	else
		directModem = "Direct Connect";
	M_Print (basex, serialConfig_cursor_table[0]-16, va ("%s - %s", startJoin, directModem));
	basex += 8;

	M_Print (basex, serialConfig_cursor_table[0], "Port");
	M_DrawTextBox (168, serialConfig_cursor_table[0]-8, 4, 1);
	M_Print (176, serialConfig_cursor_table[0], va("COM%u", serialConfig_comport));

	M_Print (basex, serialConfig_cursor_table[1], "IRQ");
	M_DrawTextBox (168, serialConfig_cursor_table[1]-8, 1, 1);
	M_Print (176, serialConfig_cursor_table[1], va("%u", serialConfig_irq));

	M_Print (basex, serialConfig_cursor_table[2], "Baud");
	M_DrawTextBox (168, serialConfig_cursor_table[2]-8, 5, 1);
	M_Print (176, serialConfig_cursor_table[2], va("%u", serialConfig_baudrate[serialConfig_baud]));

	if (SerialConfig)
	{
		M_Print (basex, serialConfig_cursor_table[3], "Modem Setup...");
		if (JoiningGame)
		{
			M_Print (basex, serialConfig_cursor_table[4], "Phone number");
			M_DrawTextBox (168, serialConfig_cursor_table[4]-8, 16, 1);
			M_Print (176, serialConfig_cursor_table[4], serialConfig_phone);
		}
	}

	if (JoiningGame)
	{
		M_DrawTextBox (basex, serialConfig_cursor_table[5]-8, 7, 1);
		M_Print (basex+8, serialConfig_cursor_table[5], "Connect");
	}
	else
	{
		M_DrawTextBox (basex, serialConfig_cursor_table[5]-8, 2, 1);
		M_Print (basex+8, serialConfig_cursor_table[5], "OK");
	}

	M_DrawCharacter (basex-8, serialConfig_cursor_table [serialConfig_cursor], 12+((int)(realtime*4)&1));

	if (serialConfig_cursor == 4)
		M_DrawCharacter (176 + 8*strlen(serialConfig_phone), serialConfig_cursor_table [serialConfig_cursor], 10+((int)(realtime*4)&1));

	if (*m_return_reason)
		M_PrintWhite (basex, 148, m_return_reason);
}

static void M_SerialConfig_Key (int key)
{
	int		l;

	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Net_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
		serialConfig_cursor--;
		if (serialConfig_cursor < 0)
			serialConfig_cursor = NUM_SERIALCONFIG_CMDS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("raven/menu1.wav");
		serialConfig_cursor++;
		if (serialConfig_cursor >= NUM_SERIALCONFIG_CMDS)
			serialConfig_cursor = 0;
		break;

	case K_LEFTARROW:
		if (serialConfig_cursor > 2)
			break;
		S_LocalSound ("raven/menu3.wav");

		if (serialConfig_cursor == 0)
		{
			serialConfig_comport--;
			if (serialConfig_comport == 0)
				serialConfig_comport = 4;
			serialConfig_irq = ISA_IRQs[serialConfig_comport-1];
		}
		else if (serialConfig_cursor == 1)
		{
			serialConfig_irq--;
			if (serialConfig_irq == 6)
				serialConfig_irq = 5;
			if (serialConfig_irq == 1)
				serialConfig_irq = 7;
		}
		else if (serialConfig_cursor == 2)
		{
			serialConfig_baud--;
			if (serialConfig_baud < 0)
				serialConfig_baud = 5;
		}

		break;

	case K_RIGHTARROW:
		if (serialConfig_cursor > 2)
			break;
forward:
		S_LocalSound ("raven/menu3.wav");

		if (serialConfig_cursor == 0)
		{
			serialConfig_comport++;
			if (serialConfig_comport > 4)
				serialConfig_comport = 1;
			serialConfig_irq = ISA_IRQs[serialConfig_comport-1];
		}
		else if (serialConfig_cursor == 1)
		{
			serialConfig_irq++;
			if (serialConfig_irq == 6)
				serialConfig_irq = 7;
			if (serialConfig_irq == 8)
				serialConfig_irq = 2;
		}
		else if (serialConfig_cursor == 2)
		{
			serialConfig_baud++;
			if (serialConfig_baud > 5)
				serialConfig_baud = 0;
		}

		break;

	case K_ENTER:
		if (serialConfig_cursor < 3)
			goto forward;

		m_entersound = true;

		if (serialConfig_cursor == 3)
		{
			(*SetComPortConfig) (0, ISA_uarts[serialConfig_comport-1], serialConfig_irq, serialConfig_baudrate[serialConfig_baud], SerialConfig);

			M_Menu_ModemConfig_f ();
			break;
		}

		if (serialConfig_cursor == 4)
		{
			serialConfig_cursor = 5;
			break;
		}

		// serialConfig_cursor == 5 (OK/CONNECT)
		(*SetComPortConfig) (0, ISA_uarts[serialConfig_comport-1], serialConfig_irq, serialConfig_baudrate[serialConfig_baud], SerialConfig);

		M_ConfigureNetSubsystem ();

		if (StartingGame)
		{
			M_Menu_GameOptions_f ();
			break;
		}

		m_return_state = m_state;
		m_return_onerror = true;
		Key_SetDest (key_game);
		m_state = m_none;

		if (SerialConfig)
			Cbuf_AddText (va ("connect \"%s\"\n", serialConfig_phone));
		else
			Cbuf_AddText ("connect\n");
		break;

	case K_BACKSPACE:
		if (serialConfig_cursor == 4)
		{
			if (strlen(serialConfig_phone))
				serialConfig_phone[strlen(serialConfig_phone)-1] = 0;
		}
		break;

	default:
		if (key < 32 || key > 127)
			break;
		if (serialConfig_cursor == 4)
		{
			l = strlen(serialConfig_phone);
			if (l < 15)
			{
				serialConfig_phone[l+1] = 0;
				serialConfig_phone[l] = key;
			}
		}
	}

	if (DirectConfig && (serialConfig_cursor == 3 || serialConfig_cursor == 4))
	{
		if (key == K_UPARROW)
			serialConfig_cursor = 2;
		else
			serialConfig_cursor = 5;
	}
	if (SerialConfig && StartingGame && serialConfig_cursor == 4)
	{
		if (key == K_UPARROW)
			serialConfig_cursor = 3;
		else
			serialConfig_cursor = 5;
	}
}

//=============================================================================
/* MODEM CONFIG MENU */

static int	modemConfig_cursor;
static const int	modemConfig_cursor_table[] = { 64, 78, 108, 138, 172 };	// { 40, 56, 88, 120, 156 }
#define NUM_MODEMCONFIG_CMDS		5

static char	modemConfig_dialing;
static char	modemConfig_clear[16];
static char	modemConfig_init[32];
static char	modemConfig_hangup[16];

static void M_Menu_ModemConfig_f (void)
{
	Key_SetDest (key_menu);
	m_state = m_modemconfig;
	m_entersound = true;
	(*GetModemConfig) (0, &modemConfig_dialing, modemConfig_clear, modemConfig_init, modemConfig_hangup);
}

static void M_ModemConfig_Draw (void)
{
	qpic_t	*p;
	int		basex;

	p = Draw_CachePic ("gfx/menu/title4.lmp");
	/* our p->width == 185: if we don't do the -8 here, drawing of
	 * the init string textbox with 30 chars width shall fail with
	 * a 'bad coordinates' message in Draw_TransPic() at 320x200.  */
	basex = (320 - p->width) / 2 - 8;
	ScrollTitle("gfx/menu/title4.lmp");

	if (modemConfig_dialing == 'P')
		M_Print (basex, modemConfig_cursor_table[0], "Pulse Dialing");
	else
		M_Print (basex, modemConfig_cursor_table[0], "Touch Tone Dialing");

	M_Print (basex, modemConfig_cursor_table[1], "Clear");
	M_DrawTextBox (basex, modemConfig_cursor_table[1]+4, 16, 1);
	M_Print (basex+8, modemConfig_cursor_table[1]+12, modemConfig_clear);
	if (modemConfig_cursor == 1)
		M_DrawCharacter (basex+8 + 8*strlen(modemConfig_clear), modemConfig_cursor_table[1]+12, 10+((int)(realtime*4)&1));

	M_Print (basex, modemConfig_cursor_table[2], "Init");
	M_DrawTextBox (basex, modemConfig_cursor_table[2]+4, 30, 1);
	M_Print (basex+8, modemConfig_cursor_table[2]+12, modemConfig_init);
	if (modemConfig_cursor == 2)
		M_DrawCharacter (basex+8 + 8*strlen(modemConfig_init), modemConfig_cursor_table[2]+12, 10+((int)(realtime*4)&1));

	M_Print (basex, modemConfig_cursor_table[3], "Hangup");
	M_DrawTextBox (basex, modemConfig_cursor_table[3]+4, 16, 1);
	M_Print (basex+8, modemConfig_cursor_table[3]+12, modemConfig_hangup);
	if (modemConfig_cursor == 3)
		M_DrawCharacter (basex+8 + 8*strlen(modemConfig_hangup), modemConfig_cursor_table[3]+12, 10+((int)(realtime*4)&1));

	M_DrawTextBox (basex, modemConfig_cursor_table[4]-8, 2, 1);
	M_Print (basex+8, modemConfig_cursor_table[4], "OK");

	M_DrawCharacter (basex-8, modemConfig_cursor_table [modemConfig_cursor], 12+((int)(realtime*4)&1));
}

static void M_ModemConfig_Key (int key)
{
	int		l;

	switch (key)
	{
	case K_ESCAPE:
		M_Menu_SerialConfig_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
		modemConfig_cursor--;
		if (modemConfig_cursor < 0)
			modemConfig_cursor = NUM_MODEMCONFIG_CMDS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("raven/menu1.wav");
		modemConfig_cursor++;
		if (modemConfig_cursor >= NUM_MODEMCONFIG_CMDS)
			modemConfig_cursor = 0;
		break;

	case K_LEFTARROW:
	case K_RIGHTARROW:
		if (modemConfig_cursor == 0)
		{
			if (modemConfig_dialing == 'P')
				modemConfig_dialing = 'T';
			else
				modemConfig_dialing = 'P';
			S_LocalSound ("raven/menu1.wav");
		}
		break;

	case K_ENTER:
		if (modemConfig_cursor == 0)
		{
			if (modemConfig_dialing == 'P')
				modemConfig_dialing = 'T';
			else
				modemConfig_dialing = 'P';
			m_entersound = true;
		}
		else if (modemConfig_cursor == 4)
		{
			(*SetModemConfig) (0, va ("%c", modemConfig_dialing), modemConfig_clear, modemConfig_init, modemConfig_hangup);
			m_entersound = true;
			M_Menu_SerialConfig_f ();
		}
		break;

	case K_BACKSPACE:
		if (modemConfig_cursor == 1)
		{
			l = strlen(modemConfig_clear);
			if (l)
				modemConfig_clear[l-1] = 0;
		}
		else if (modemConfig_cursor == 2)
		{
			l = strlen(modemConfig_init);
			if (l)
				modemConfig_init[l-1] = 0;
		}
		else if (modemConfig_cursor == 3)
		{
			l = strlen(modemConfig_hangup);
			if (l)
				modemConfig_hangup[l-1] = 0;
		}
		break;

	default:
		if (key < 32 || key > 127)
			break;

		if (modemConfig_cursor == 1)
		{
			l = strlen(modemConfig_clear);
			if (l < 15)
			{
				modemConfig_clear[l+1] = 0;
				modemConfig_clear[l] = key;
			}
		}
		else if (modemConfig_cursor == 2)
		{
			l = strlen(modemConfig_init);
			if (l < 29)
			{
				modemConfig_init[l+1] = 0;
				modemConfig_init[l] = key;
			}
		}
		else if (modemConfig_cursor == 3)
		{
			l = strlen(modemConfig_hangup);
			if (l < 15)
			{
				modemConfig_hangup[l+1] = 0;
				modemConfig_hangup[l] = key;
			}
		}
	}
}
#endif	/* NET_USE_SERIAL */

//=============================================================================

/* LAN CONFIG MENU */

static int	lanConfig_cursor = -1;
static const int	lanConfig_cursor_table[] = {100, 120, 140, 172};
#define NUM_LANCONFIG_CMDS	4

static int	lanConfig_port;
static char	lanConfig_portname[6];
static char	lanConfig_joinname[30];

static void M_Menu_LanConfig_f (void)
{
	Key_SetDest (key_menu);
	m_state = m_lanconfig;
	m_entersound = true;
	if (lanConfig_cursor == -1)
	{
		if (JoiningGame && TCPIPConfig)
			lanConfig_cursor = 2;
		else
			lanConfig_cursor = 1;
	}
	if (StartingGame && lanConfig_cursor >= 2)
		lanConfig_cursor = 1;
	lanConfig_port = DEFAULTnet_hostport;
	q_snprintf(lanConfig_portname, sizeof(lanConfig_portname), "%d", lanConfig_port);

	m_return_onerror = false;
	m_return_reason[0] = 0;

	setup_class = cl_playerclass.integer;
	if (setup_class < 1 || setup_class > MAX_PLAYER_CLASS)
		setup_class = MAX_PLAYER_CLASS;
#if ENABLE_OLD_DEMO
	if (gameflags & GAME_OLD_DEMO)
	{
		if (setup_class != CLASS_PALADIN && setup_class != CLASS_THEIF)
			setup_class = CLASS_PALADIN;
	}
	else
#endif	/* OLD_DEMO */
	if (!(gameflags & GAME_PORTALS))
	{
		if (setup_class > MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES)
			setup_class = MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES;
	}
	setup_class--;
}


static void M_LanConfig_Draw (void)
{
	int	basex;
	const char	*startJoin;
	const char	*protocol;

	ScrollTitle("gfx/menu/title4.lmp");
	basex = 48;

	if (StartingGame)
		startJoin = "New Game";
	else
		startJoin = "Join Game";
	if (IPXConfig)
		protocol = "IPX";
	else
		protocol = "TCP/IP";
	M_Print (basex, 60, va ("%s - %s", startJoin, protocol));
	basex += 8;

	M_Print (basex, 80, "Address:");
	if (IPXConfig)
		M_Print (basex+9*8, 80, my_ipx_address);
	else
		M_Print (basex+9*8, 80, my_tcpip_address);

	M_Print (basex, lanConfig_cursor_table[0], "Port");
	M_DrawTextBox (basex+8*8, lanConfig_cursor_table[0]-8, 6, 1);
	M_Print (basex+9*8, lanConfig_cursor_table[0], lanConfig_portname);

	if (JoiningGame)
	{
		M_Print (basex, lanConfig_cursor_table[1], "Class:");
		M_Print (basex+8*7, lanConfig_cursor_table[1], ClassNames[setup_class]);

		M_Print (basex, lanConfig_cursor_table[2], "Search for local games...");
		M_Print (basex, 156, "Join game at:");
		M_DrawTextBox (basex, lanConfig_cursor_table[3]-8, 30, 1);
		M_Print (basex+8, lanConfig_cursor_table[3], lanConfig_joinname);
	}
	else
	{
		M_DrawTextBox (basex, lanConfig_cursor_table[1]-8, 2, 1);
		M_Print (basex+8, lanConfig_cursor_table[1], "OK");
	}

	M_DrawCharacter (basex-8, lanConfig_cursor_table [lanConfig_cursor], 12+((int)(realtime*4)&1));

	if (lanConfig_cursor == 0)
		M_DrawCharacter (basex+9*8 + 8*strlen(lanConfig_portname), lanConfig_cursor_table [0], 10+((int)(realtime*4)&1));

	if (lanConfig_cursor == 3)
		M_DrawCharacter (basex+8 + 8*strlen(lanConfig_joinname), lanConfig_cursor_table [3], 10+((int)(realtime*4)&1));

	if (*m_return_reason)
		M_PrintWhite (basex, 192, m_return_reason);
}


static void M_LanConfig_Key (int key)
{
	int		l;

	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Net_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
		lanConfig_cursor--;

		if (JoiningGame)
		{
			if (lanConfig_cursor < 0)
				lanConfig_cursor = NUM_LANCONFIG_CMDS-1;
		}
		else
		{
			if (lanConfig_cursor < 0)
				lanConfig_cursor = NUM_LANCONFIG_CMDS-2;
		}
		break;

	case K_DOWNARROW:
		S_LocalSound ("raven/menu1.wav");
		lanConfig_cursor++;
		if (lanConfig_cursor >= NUM_LANCONFIG_CMDS)
			lanConfig_cursor = 0;
		break;

	case K_ENTER:
		if ((JoiningGame && lanConfig_cursor <= 1) ||
		    (!JoiningGame && lanConfig_cursor == 0))
			break;

		m_entersound = true;
		if (JoiningGame)
			Cbuf_AddText ( va ("playerclass %d\n", setup_class+1) );

		M_ConfigureNetSubsystem ();

		if ((JoiningGame && lanConfig_cursor == 2) ||
		    (!JoiningGame && lanConfig_cursor == 1))
		{
			if (StartingGame)
			{
				M_Menu_GameOptions_f ();
				break;
			}
			M_Menu_Search_f();
			break;
		}

		if (lanConfig_cursor == 3)
		{
			m_return_state = m_state;
			m_return_onerror = true;
			Key_SetDest (key_game);
			m_state = m_none;
			Cbuf_AddText ( va ("connect \"%s\"\n", lanConfig_joinname) );
			break;
		}

		break;

	case K_BACKSPACE:
		if (lanConfig_cursor == 0)
		{
			l = strlen(lanConfig_portname);
			if (l)
				lanConfig_portname[l-1] = 0;
		}
		else if (lanConfig_cursor == 3)
		{
			l = strlen(lanConfig_joinname);
			if (l)
				lanConfig_joinname[l-1] = 0;
		}
		break;

	case K_LEFTARROW:
		if (lanConfig_cursor != 1 || !JoiningGame)
			break;

		S_LocalSound ("raven/menu3.wav");
#if ENABLE_OLD_DEMO
		if (gameflags & GAME_OLD_DEMO)
		{
			setup_class = (setup_class == CLASS_PALADIN-1) ? CLASS_THEIF-1 : CLASS_PALADIN-1;
			break;
		}
#endif	/* OLD_DEMO */
		setup_class--;
		if (setup_class < 0)
			setup_class = MAX_PLAYER_CLASS -1;
		if (setup_class > MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES - 1 && !(gameflags & GAME_PORTALS))
			setup_class = MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES -1;
		break;

	case K_RIGHTARROW:
		if (lanConfig_cursor != 1 || !JoiningGame)
			break;

		S_LocalSound ("raven/menu3.wav");
#if ENABLE_OLD_DEMO
		if (gameflags & GAME_OLD_DEMO)
		{
			setup_class = (setup_class == CLASS_PALADIN-1) ? CLASS_THEIF-1 : CLASS_PALADIN-1;
			break;
		}
#endif	/* OLD_DEMO */
		setup_class++;
		if (setup_class > MAX_PLAYER_CLASS - 1)
			setup_class = 0;
		if (setup_class > MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES - 1 && !(gameflags & GAME_PORTALS))
			setup_class = 0;
		break;

	default:
		if (key < 32 || key > 127)
			break;

		if (lanConfig_cursor == 3)
		{
			l = strlen(lanConfig_joinname);
			if (l < 29)
			{
				lanConfig_joinname[l+1] = 0;
				lanConfig_joinname[l] = key;
			}
		}

		if (key < '0' || key > '9')
			break;
		if (lanConfig_cursor == 0)
		{
			l = strlen(lanConfig_portname);
			if (l < 5)
			{
				lanConfig_portname[l+1] = 0;
				lanConfig_portname[l] = key;
			}
		}
	}

	if (StartingGame && lanConfig_cursor == 2)
	{
		if (key == K_UPARROW)
			lanConfig_cursor = 1;
		else
			lanConfig_cursor = 0;
	}
	l =  atoi(lanConfig_portname);
	if (l > 65535)
		l = lanConfig_port;
	else
		lanConfig_port = l;
	q_snprintf(lanConfig_portname, sizeof(lanConfig_portname), "%d", lanConfig_port);
}

//=============================================================================
/* GAME OPTIONS MENU */

static const struct
{
	const char	*name;
	const char	*description;
} levels[] =
{
	{"demo1", "Blackmarsh"},			// 0
	{"demo2", "Barbican"},				// 1

	{"ravdm1", "Deathmatch 1"},			// 2

	{"demo1","Blackmarsh"},				// 3
	{"demo2","Barbican"},				// 4
	{"demo3","The Mill"},				// 5
	{"village1","King's Court"},			// 6
	{"village2","Inner Courtyard"},			// 7
	{"village3","Stables"},				// 8
	{"village4","Palace Entrance"},			// 9
	{"village5","The Forgotten Chapel"},		// 10
	{"rider1a","Famine's Domain"},			// 11

	{"meso2","Plaza of the Sun"},			// 12
	{"meso1","The Palace of Columns"},		// 13
	{"meso3","Square of the Stream"},		// 14
	{"meso4","Tomb of the High Priest"},		// 15
	{"meso5","Obelisk of the Moon"},		// 16
	{"meso6","Court of 1000 Warriors"},		// 17
	{"meso8","Bridge of Stars"},			// 18
	{"meso9","Well of Souls"},			// 19

	{"egypt1","Temple of Horus"},			// 20
	{"egypt2","Ancient Temple of Nefertum"},	// 21
	{"egypt3","Temple of Nefertum"},		// 22
	{"egypt4","Palace of the Pharaoh"},		// 23
	{"egypt5","Pyramid of Anubis"},			// 24
	{"egypt6","Temple of Light"},			// 25
	{"egypt7","Shrine of Naos"},			// 26
	{"rider2c","Pestilence's Lair"},		// 27

	{"romeric1","The Hall of Heroes"},		// 28
	{"romeric2","Gardens of Athena"},		// 29
	{"romeric3","Forum of Zeus"},			// 30
	{"romeric4","Baths of Demetrius"},		// 31
	{"romeric5","Temple of Mars"},			// 32
	{"romeric6","Coliseum of War"},			// 33
	{"romeric7","Reflecting Pool"},			// 34

	{"cath","Cathedral"},				// 35
	{"tower","Tower of the Dark Mage"},		// 36
	{"castle4","The Underhalls"},			// 37
	{"castle5","Eidolon's Ordeal"},			// 38
	{"eidolon","Eidolon's Lair"},			// 39

	{"ravdm1","Atrium of Immolation"},		// 40
	{"ravdm2","Total Carnage"},			// 41
	{"ravdm3","Reckless Abandon"},			// 42
	{"ravdm4","Temple of RA"},			// 43
	{"ravdm5","Tom Foolery"},			// 44

	{"ravdm1", "Deathmatch 1"},			// 45

//OEM
	{"demo1","Blackmarsh"},				// 46
	{"demo2","Barbican"},				// 47
	{"demo3","The Mill"},				// 48
	{"village1","King's Court"},			// 49
	{"village2","Inner Courtyard"},			// 50
	{"village3","Stables"},				// 51
	{"village4","Palace Entrance"},			// 52
	{"village5","The Forgotten Chapel"},		// 53
	{"rider1a","Famine's Domain"},			// 54

//Mission Pack
	{"keep1",	"Eidolon's Lair"},		// 55
	{"keep2",	"Village of Turnabel"},		// 56
	{"keep3",	"Duke's Keep"},			// 57
	{"keep4",	"The Catacombs"},		// 58
	{"keep5",	"Hall of the Dead"},		// 59

	{"tibet1",	"Tulku"},			// 60
	{"tibet2",	"Ice Caverns"},			// 61
	{"tibet3",	"The False Temple"},		// 62
	{"tibet4",	"Courtyards of Tsok"},		// 63
	{"tibet5",	"Temple of Kalachakra"},	// 64
	{"tibet6",	"Temple of Bardo"},		// 65
	{"tibet7",	"Temple of Phurbu"},		// 66
	{"tibet8",	"Palace of Emperor Egg Chen"},	// 67
	{"tibet9",	"Palace Inner Chambers"},	// 68
	{"tibet10",	"The Inner Sanctum of Praevus"},// 69
};

static const struct
{
	const char	*description;
	int		firstLevel;
	int		levels;
} episodes[] =
{
	// Demo
	{"Demo", 0, 2},
	{"Demo Deathmatch", 2, 1},

	// Registered
	{"Village", 3, 9},
	{"Meso", 12, 8},
	{"Egypt", 20, 8},
	{"Romeric", 28, 7},
	{"Cathedral", 35, 5},

	{"MISSION PACK", 55, 15},

	{"Deathmatch", 40, 5},

	// OEM
	{"Village", 46, 9},
	{"Deathmatch", 45, 1},
};

#define OEM_START 9
#define REG_START 2
#define MP_START 7
#define DM_START 8

static int	startepisode;
static int	startlevel;
static int	maxplayers;
//static qboolean m_serverInfoMessage = false;
//static double	m_serverInfoMessageTime;

static const int	gameoptions_cursor_table[] = {40, 56, 64, 72, 80, 88, 96, 104, 112, 128, 136};
#define	NUM_GAMEOPTIONS	11
static int	gameoptions_cursor;

static void M_Menu_GameOptions_f (void)
{
	Key_SetDest (key_menu);
	m_state = m_gameoptions;
	m_entersound = true;
	if (maxplayers == 0)
		maxplayers = svs.maxclients;
	if (maxplayers < 2)
		maxplayers = svs.maxclientslimit;

	setup_class = cl_playerclass.integer;
	if (setup_class < 1 || setup_class > MAX_PLAYER_CLASS)
		setup_class = MAX_PLAYER_CLASS;
#if ENABLE_OLD_DEMO
	if (gameflags & GAME_OLD_DEMO)
	{
		if (setup_class != CLASS_PALADIN && setup_class != CLASS_THEIF)
			setup_class = CLASS_PALADIN;
	}
	else
#endif	/* OLD_DEMO */
	if (!(gameflags & GAME_PORTALS))
	{
		if (setup_class > MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES)
			setup_class = MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES;
	}
	setup_class--;

	if (oem.integer)
	{
		if (startepisode < OEM_START || startepisode > OEM_START+1)
			startepisode = OEM_START;
		if (coop.integer)
			startepisode = OEM_START;
	}
	else if (registered.integer)
	{
		if (startepisode < REG_START || startepisode >= OEM_START)
			startepisode = REG_START;
		else if (startepisode == MP_START && !(gameflags & GAME_PORTALS))
			startepisode = REG_START;
		if (coop.integer && startepisode == DM_START)
			startepisode = REG_START;
	}
	else	// demo
	{
		if (startepisode < 0 || startepisode > 1)
			startepisode = 0;
		if (coop.integer)
			startepisode = 0;
	}

	if (coop.integer)
	{
		startlevel = 0;
		if (gameoptions_cursor >= NUM_GAMEOPTIONS-1)
			gameoptions_cursor = 0;
	}
}

static void M_GameOptions_Draw (void)
{
	ScrollTitle("gfx/menu/title4.lmp");

	M_DrawTextBox (152+8, 60, 10, 1);
	M_Print (160+8, 68, "begin game");

//	we use 17 character option titles. the second increment
//	to the x offset is: (17 - strlen(option_title)) * 8
	M_Print (0+8 + 6*8, 84, "Max players");
	M_Print (160+8, 84, va("%i", maxplayers) );

	M_Print (0+8 + 8*8, 92, "Game Type");
	if (coop.integer)
		M_Print (160+8, 92, "Cooperative");
	else
		M_Print (160+8, 92, "Deathmatch");

	M_Print (0+8 + 9*8, 100, "Teamplay");
	{
		const char	*msg;

		switch (teamplay.integer)
		{
			case 1:
				msg = "No Friendly Fire";
				break;
			case 2:
				msg = "Friendly Fire";
				break;
			default:
				msg = "Off";
				break;
		}
		M_Print (160+8, 100, msg);
	}

	M_Print (0+8 + 12*8, 108, "Class");
	M_Print (160+8, 108, ClassNames[setup_class]);

	M_Print (0+8 + 7*8, 116, "Difficulty");

	M_Print (160+8, 116, DiffNames[setup_class][skill.integer]);

	M_Print (0+8 + 7*8, 124, "Frag Limit");
	if (fraglimit.integer == 0)
		M_Print (160+8, 124, "none");
	else
		M_Print (160+8, 124, va("%i frags", fraglimit.integer));

	M_Print (0+8 + 7*8, 132, "Time Limit");
	if (timelimit.integer == 0)
		M_Print (160+8, 132, "none");
	else
		M_Print (160+8, 132, va("%i minutes", timelimit.integer));

	M_Print (0+8 + 5*8, 140, "Random Class");
	if (randomclass.integer)
		M_Print (160+8, 140, "on");
	else
		M_Print (160+8, 140, "off");

	M_Print (0+8 + 10*8, 156, "Episode");
	M_Print (160+8, 156, episodes[startepisode].description);

	M_Print (0+8 + 12*8, 164, "Level");
	M_Print (160+8, 164, levels[episodes[startepisode].firstLevel + startlevel].name);
	M_Print (96, 180, levels[episodes[startepisode].firstLevel + startlevel].description);

// line cursor
	M_DrawCharacter (172-16, gameoptions_cursor_table[gameoptions_cursor]+28, 12+((int)(realtime*4)&1));

/*	rjr
	if (m_serverInfoMessage)
	{
		if ((realtime - m_serverInfoMessageTime) < 5.0)
		{
			x = (320-26*8)/2;
			M_DrawTextBox (x, 138, 24, 4);
			x += 8;
			M_Print (x, 146, "  More than 4 players   ");
			M_Print (x, 154, " requires using command ");
			M_Print (x, 162, "line parameters; please ");
			M_Print (x, 170, "   see techinfo.txt.    ");
		}
		else
		{
			m_serverInfoMessage = false;
		}
	}*/
}


static void M_NetStart_Change (int dir)
{
	int	val;
	switch (gameoptions_cursor)
	{
	case 1:
		maxplayers += dir;
		if (maxplayers > svs.maxclientslimit)
		{
			maxplayers = svs.maxclientslimit;
		//	m_serverInfoMessage = true;
		//	m_serverInfoMessageTime = realtime;
		}
		if (maxplayers < 2)
			maxplayers = 2;
		break;

	case 2:
		if (coop.integer)
		{
			Cvar_Set ("coop", "0");
			break;
		}
		Cvar_Set ("coop", "1");
		startlevel = 0;
		if (startepisode == 1)
			startepisode = 0;
		else if (startepisode == DM_START)
			startepisode = REG_START;
		else if (startepisode == OEM_START+1)
			startepisode = OEM_START;
		break;

	case 3:
		val = teamplay.integer + dir;
		if (val > 2)
			val = 0;
		else if (val < 0)
			val = 2;
		Cvar_SetValue ("teamplay", val);
		break;

	case 4:
#if ENABLE_OLD_DEMO
		if (gameflags & GAME_OLD_DEMO)
		{
			setup_class = (setup_class == CLASS_PALADIN-1) ? CLASS_THEIF-1 : CLASS_PALADIN-1;
			break;
		}
#endif	/* OLD_DEMO */
		setup_class += dir;
		if (setup_class < 0)
			setup_class = MAX_PLAYER_CLASS - 1;
		else if(setup_class > MAX_PLAYER_CLASS - 1)
			setup_class = 0;
		if (setup_class > MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES - 1 && !(gameflags & GAME_PORTALS))
			setup_class = (dir > 0)? 0 : MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES - 1;
		break;

	case 5:
		val = skill.integer + dir;
		if (val > 3)
			val = 0;
		else if (val < 0)
			val = 3;
		Cvar_SetValue ("skill", val);
		break;

	case 6:
		val = fraglimit.integer + dir*10;
		if (val > 100)
			val = 0;
		else if (val < 0)
			val = 100;
		Cvar_SetValue ("fraglimit", val);
		break;

	case 7:
		val = timelimit.integer + dir*5;
		if (val > 60)
			val = 0;
		else if (val < 0)
			val = 60;
		Cvar_SetValue ("timelimit", val);
		break;

	case 8:
		Cvar_Set ("randomclass", randomclass.integer ? "0" : "1");
		break;

	case 9:
		if (registered.integer)
		{
			startepisode += dir;
			startlevel = 0;
			if (startepisode > DM_START)
				startepisode = REG_START;
			else
			{
				if (startepisode == MP_START && !(gameflags & GAME_PORTALS))
					startepisode += dir;
				if (coop.integer && startepisode == DM_START)
					startepisode = (dir > 0) ? REG_START : ((gameflags & GAME_PORTALS) ? MP_START : MP_START-1);
				if (startepisode < REG_START)
					startepisode = (coop.integer) ? ((gameflags & GAME_PORTALS) ? MP_START : MP_START-1) : DM_START;
			}
		}
		else if (oem.integer)
		{
			if (!coop.integer)
			{
				startepisode = (startepisode != OEM_START) ? OEM_START : OEM_START+1;
				startlevel = 0;
			}
		}
		else	// demo version
		{
			if (!coop.integer)
			{
				startepisode = (startepisode != 0) ? 0 : 1;
				startlevel = 0;
			}
		}
		break;

	case 10:
		if (coop.integer)
		{
			startlevel = 0;
			break;
		}
		startlevel += dir;

		if (startlevel < 0)
			startlevel = episodes[startepisode].levels - 1;
		else if (startlevel >= episodes[startepisode].levels)
			startlevel = 0;
		break;
	}
}

static void M_GameOptions_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Net_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
		gameoptions_cursor--;
		if (gameoptions_cursor < 0)
		{
			gameoptions_cursor = NUM_GAMEOPTIONS-1;
			if (coop.integer)
				gameoptions_cursor--;
		}
		break;

	case K_DOWNARROW:
		S_LocalSound ("raven/menu1.wav");
		gameoptions_cursor++;
		if (coop.integer)
		{
			if (gameoptions_cursor >= NUM_GAMEOPTIONS-1)
				gameoptions_cursor = 0;
		}
		else
		{
			if (gameoptions_cursor >= NUM_GAMEOPTIONS)
				gameoptions_cursor = 0;
		}
		break;

	case K_LEFTARROW:
		if (gameoptions_cursor == 0)
			break;
		S_LocalSound ("raven/menu3.wav");
		M_NetStart_Change (-1);
		break;

	case K_RIGHTARROW:
		if (gameoptions_cursor == 0)
			break;
		S_LocalSound ("raven/menu3.wav");
		M_NetStart_Change (1);
		break;

	case K_ENTER:
		S_LocalSound ("raven/menu2.wav");
		if (gameoptions_cursor == 0)
		{
			if (sv.active)
				Cbuf_AddText ("disconnect\n");
			Cbuf_AddText ( va ("playerclass %d\n", setup_class+1) );
			Cbuf_AddText ("listen 0\n");	// so host_netport will be re-examined
			Cbuf_AddText ( va ("maxplayers %d\n", maxplayers) );
			SCR_BeginLoadingPlaque ();

			Cbuf_AddText ( va ("map %s\n", levels[episodes[startepisode].firstLevel + startlevel].name) );

			return;
		}

		M_NetStart_Change (1);
		break;
	}
}

//=============================================================================
/* SEARCH MENU */

static qboolean	searchComplete = false;
static double	searchCompleteTime;

static void M_Menu_Search_f (void)
{
	Key_SetDest (key_menu);
	m_state = m_search;
	m_entersound = false;
	slistSilent = true;
	slistLocal = false;
	searchComplete = false;
	NET_Slist_f();
}


static void M_Search_Draw (void)
{
	int x;

	ScrollTitle("gfx/menu/title4.lmp");

	x = (320/2) - ((12*8)/2) + 4;
	M_DrawTextBox (x-8, 60, 12, 1);
	M_Print (x, 68, "Searching...");

	if (slistInProgress)
	{
		NET_Poll();
		return;
	}

	if (! searchComplete)
	{
		searchComplete = true;
		searchCompleteTime = realtime;
	}

	if (hostCacheCount)
	{
		M_Menu_ServerList_f ();
		return;
	}

	M_PrintWhite ((320/2) - ((22*8)/2), 92, "No Hexen II servers found");
	if ((realtime - searchCompleteTime) < 3.0)
		return;

	M_Menu_LanConfig_f ();
}


static void M_Search_Key (int key)
{
}

//=============================================================================
/* SLIST MENU */

static int		slist_cursor;
static qboolean		slist_sorted;

static void M_Menu_ServerList_f (void)
{
	Key_SetDest (key_menu);
	m_state = m_slist;
	m_entersound = true;
	slist_cursor = 0;
	m_return_onerror = false;
	m_return_reason[0] = 0;
	slist_sorted = false;
}


static void M_ServerList_Draw (void)
{
	int	n;

	if (!slist_sorted)
	{
		slist_sorted = true;
		NET_SlistSort ();
	}

	ScrollTitle("gfx/menu/title4.lmp");
	for (n = 0; n < hostCacheCount; n++)
		M_Print (16, 60 + 8*n, NET_SlistPrintServer (n));

	M_DrawCharacter (0, 60 + slist_cursor*8, 12+((int)(realtime*4)&1));

	if (*m_return_reason)
		M_PrintWhite (16, 176, m_return_reason);
}


static void M_ServerList_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
		M_Menu_LanConfig_f ();
		break;

	case K_SPACE:
		M_Menu_Search_f ();
		break;

	case K_UPARROW:
	case K_LEFTARROW:
		S_LocalSound ("raven/menu1.wav");
		slist_cursor--;
		if (slist_cursor < 0)
			slist_cursor = hostCacheCount - 1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("raven/menu1.wav");
		slist_cursor++;
		if (slist_cursor >= hostCacheCount)
			slist_cursor = 0;
		break;

	case K_ENTER:
		S_LocalSound ("raven/menu2.wav");
		m_return_state = m_state;
		m_return_onerror = true;
		slist_sorted = false;
		Key_SetDest (key_game);
		m_state = m_none;
		Cbuf_AddText ( va ("connect \"%s\"\n", NET_SlistPrintServerName(slist_cursor)) );
		break;

	default:
		break;
	}
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
	Cmd_AddCommand ("menu_singleplayer", M_Menu_SinglePlayer_f);
	Cmd_AddCommand ("menu_load", M_Menu_Load_f);
	Cmd_AddCommand ("menu_save", M_Menu_Save_f);
	Cmd_AddCommand ("menu_multiplayer", M_Menu_MultiPlayer_f);
	Cmd_AddCommand ("menu_setup", M_Menu_Setup_f);
	Cmd_AddCommand ("menu_options", M_Menu_Options_f);
	Cmd_AddCommand ("menu_keys", M_Menu_Keys_f);
	Cmd_AddCommand ("menu_video", M_Menu_Video_f);
	Cmd_AddCommand ("help", M_Menu_Help_f);
	Cmd_AddCommand ("menu_quit", M_Menu_Quit_f);
	Cmd_AddCommand ("menu_class", M_Menu_Class2_f);

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

	case m_singleplayer:
		M_SinglePlayer_Draw ();
		break;

	case m_difficulty:
		M_Difficulty_Draw ();
		break;

	case m_class:
		M_Class_Draw ();
		break;

	case m_load:
	case m_mload:
		M_Load_Draw ();
		break;

	case m_save:
	case m_msave:
		M_Save_Draw ();
		break;

	case m_multiplayer:
		M_MultiPlayer_Draw ();
		break;

	case m_setup:
		M_Setup_Draw ();
		break;

	case m_net:
		M_Net_Draw ();
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

#if defined(NET_USE_SERIAL)
	case m_serialconfig:
		M_SerialConfig_Draw ();
		break;

	case m_modemconfig:
		M_ModemConfig_Draw ();
		break;
#endif

	case m_lanconfig:
		M_LanConfig_Draw ();
		break;

	case m_gameoptions:
		M_GameOptions_Draw ();
		break;

	case m_search:
		M_Search_Draw ();
		break;

	case m_slist:
		M_ServerList_Draw ();
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

	case m_singleplayer:
		M_SinglePlayer_Key (key);
		return;

	case m_difficulty:
		M_Difficulty_Key (key);
		return;

	case m_class:
		M_Class_Key (key);
		return;

	case m_load:
		M_Load_Key (key);
		return;

	case m_save:
		M_Save_Key (key);
		return;

	case m_mload:
		M_MLoad_Key (key);
		return;

	case m_msave:
		M_MSave_Key (key);
		return;

	case m_multiplayer:
		M_MultiPlayer_Key (key);
		return;

	case m_setup:
		M_Setup_Key (key);
		return;

	case m_net:
		M_Net_Key (key);
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

#if defined(NET_USE_SERIAL)
	case m_serialconfig:
		M_SerialConfig_Key (key);
		return;

	case m_modemconfig:
		M_ModemConfig_Key (key);
		return;
#endif

	case m_lanconfig:
		M_LanConfig_Key (key);
		return;

	case m_gameoptions:
		M_GameOptions_Key (key);
		return;

	case m_search:
		M_Search_Key (key);
		break;

	case m_slist:
		M_ServerList_Key (key);
		return;
	}
}


static void M_ConfigureNetSubsystem(void)
{
// enable/disable net systems to match desired config

	Cbuf_AddText ("stopdemo\n");

#if defined(NET_USE_SERIAL)
	if (SerialConfig || DirectConfig)
	{
		Cbuf_AddText ("com1 enable\n");
	}
#endif
	if (IPXConfig || TCPIPConfig)
		net_hostport = lanConfig_port;
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

