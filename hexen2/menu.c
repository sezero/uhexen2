/*
	menu.c

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/menu.c,v 1.61 2006-03-23 18:44:27 sezero Exp $
*/

#include "quakedef.h"
#include "quakeinc.h"

extern	modestate_t	modestate;
extern	cvar_t	crosshair;
extern	float introTime;

void (*vid_menudrawfn)(void);
void (*vid_menukeyfn)(int key);

enum
{
	m_none = 0,
	m_main,
	m_singleplayer,
	m_load,
	m_save,
	m_multiplayer,
	m_setup,
	m_net,
	m_options,
#ifdef GLQUAKE
	m_opengl,
#endif
	m_video,
	m_keys,
	m_help,
	m_quit,
	m_lanconfig,
	m_gameoptions,
	m_search,
	m_slist,
	m_class,
	m_difficulty,
	m_mload,
	m_msave
} m_state;

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

static qboolean	m_entersound;		// play after drawing a frame, so caching
					// won't disrupt the sound
static qboolean	m_recursiveDraw;

int			m_return_state;
qboolean	m_return_onerror;
char		m_return_reason [32];

qboolean	mousestate_sa = false;	// true if we're in menus and mouse is to be disabled

static float TitlePercent = 0;
static float TitleTargetPercent = 1;
static float LogoPercent = 0;
static float LogoTargetPercent = 1;

static int	setup_class;

static char *message,*message2;
static double message_time;

#define StartingGame	(m_multiplayer_cursor == 1)
#define JoiningGame		(m_multiplayer_cursor == 0)
#define	IPXConfig		(m_net_cursor == 0)
#define	TCPIPConfig		(m_net_cursor == 1)
#define NUM_DIFFLEVELS	4

static void M_ConfigureNetSubsystem(void);
static void M_Menu_Class_f (void);

char *ClassNames[MAX_PLAYER_CLASS] = 
{
	"Paladin",
	"Crusader",
	"Necromancer",
	"Assassin",
#ifdef H2MP
	"Demoness"
#endif
};

static char *ClassNamesU[MAX_PLAYER_CLASS] = 
{
	"PALADIN",
	"CRUSADER",
	"NECROMANCER",
	"ASSASSIN",
#ifdef H2MP
	"DEMONESS"
#endif
};

static char *DiffNames[MAX_PLAYER_CLASS][4] =
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
#ifdef H2MP
	{	// Demoness
		"LARVA",
		"SPAWN",
		"FIEND",
		"SHE BITCH"
	}
#endif
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

void M_Print (int cx, int cy, char *str)
{
	while (*str)
	{
		M_DrawCharacter (cx, cy, ((unsigned char)(*str))+256);
		str++;
		cx += 8;
	}
}

/*
================
M_DrawCharacter2

Draws one solid graphics character, centered H and V
================
*/
static void M_DrawCharacter2 (int cx, int line, int num)
{
	Draw_Character ( cx + ((vid.width - 320)>>1), line + ((vid.height - 200)>>1), num);
}

void M_Print2 (int cx, int cy, char *str)
{
	while (*str)
	{
		M_DrawCharacter2 (cx, cy, ((unsigned char)(*str))+256);
		str++;
		cx += 8;
	}
}

void M_PrintWhite (int cx, int cy, char *str)
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

static void M_DrawTransPic2 (int x, int y, qpic_t *pic)
{
	Draw_TransPic (x + ((vid.width - 320)>>1), y + ((vid.height - 200)>>1), pic);
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
extern const int color_offsets[MAX_PLAYER_CLASS];

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
	for(j=0;j<256;j++,colorA++,colorB++,sourceA++,sourceB++)
	{
		if (top >= 0 && (*colorA != 255))
			dest[j] = source[*sourceA];
		if (bottom >= 0 && (*colorB != 255))
			dest[j] = source[*sourceB];
	}
}

void M_DrawTextBox (int x, int y, int width, int lines)
{
	qpic_t	*p,*tm,*bm;
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


void M_DrawTextBox2 (int x, int y, int width, int lines, qboolean bottom)
{
	qpic_t	*p,*tm,*bm;
	int		cx, cy;
	int		n;

	// draw left side
	cx = x;
	cy = y;
	p = Draw_CachePic ("gfx/box_tl.lmp");
	if(bottom)
		M_DrawTransPic (cx, cy, p);
	else
		M_DrawTransPic2 (cx, cy, p);
	p = Draw_CachePic ("gfx/box_ml.lmp");
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		if(bottom)
			M_DrawTransPic (cx, cy, p);
		else
			M_DrawTransPic2 (cx, cy, p);
	}
	p = Draw_CachePic ("gfx/box_bl.lmp");
	if(bottom)
		M_DrawTransPic (cx, cy+8, p);
	else
		M_DrawTransPic2 (cx, cy+8, p);

	// draw middle
	cx += 8;
	tm = Draw_CachePic ("gfx/box_tm.lmp");
	bm = Draw_CachePic ("gfx/box_bm.lmp");
	while (width > 0)
	{
		cy = y;

		if(bottom)
			M_DrawTransPic (cx, cy, tm);
		else
			M_DrawTransPic2 (cx, cy, tm);
		p = Draw_CachePic ("gfx/box_mm.lmp");
		for (n = 0; n < lines; n++)
		{
			cy += 8;
			if (n == 1)
				p = Draw_CachePic ("gfx/box_mm2.lmp");
			if(bottom)
				M_DrawTransPic (cx, cy, p);
			else
				M_DrawTransPic2 (cx, cy, p);
		}
		if(bottom)
			M_DrawTransPic (cx, cy+8, bm);
		else
			M_DrawTransPic2 (cx, cy+8, bm);
		width -= 2;
		cx += 16;
	}

	// draw right side
	cy = y;
	p = Draw_CachePic ("gfx/box_tr.lmp");
	if(bottom)
		M_DrawTransPic (cx, cy, p);
	else
		M_DrawTransPic2 (cx, cy, p);
	p = Draw_CachePic ("gfx/box_mr.lmp");
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		if(bottom)
			M_DrawTransPic (cx, cy, p);
		else
			M_DrawTransPic2 (cx, cy, p);
	}
	p = Draw_CachePic ("gfx/box_br.lmp");
	if(bottom)
		M_DrawTransPic (cx, cy+8, p);
	else
		M_DrawTransPic2 (cx, cy+8, p);
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
	m_entersound = true;

	if (key_dest == key_menu)
	{
		if (m_state != m_main)
		{
			LogoTargetPercent = TitleTargetPercent = 1;
			LogoPercent = TitlePercent = 0;
			M_Menu_Main_f ();
			return;
		}
		key_dest = key_game;
		m_state = m_none;
		return;
	}
	if (key_dest == key_console)
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

char BigCharWidth[27][27];
//static char unused_filler;  // cuz the COM_LoadStackFile puts a 0 at the end of the data

//#define BUILD_BIG_CHAR 1

static void M_BuildBigCharWidth (void)
{
#ifdef BUILD_BIG_CHAR
	qpic_t	*p;
	int ypos,xpos;
	byte			*source;
	int biggestX,adjustment;
	char After[20], Before[20];
	int numA,numB;
	FILE *FH;
	char temp[MAX_OSPATH];

	p = Draw_CachePic ("gfx/menu/bigfont.lmp");

	for(numA = 0; numA < 27; numA++)
	{
		memset(After,20,sizeof(After));
		source = p->data + ((numA % 8) * 20) + (numA / 8 * p->width * 20);
		biggestX = 0;

		for(ypos=0;ypos < 19;ypos++)
		{
			for(xpos=0;xpos<19;xpos++,source++)
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

		for(numB = 0; numB < 27; numB++)
		{
			memset(Before,0,sizeof(Before));
			source = p->data + ((numB % 8) * 20) + (numB / 8 * p->width * 20);
			adjustment = 0;

			for(ypos=0;ypos < 19;ypos++)
			{
				for(xpos=0;xpos<19;xpos++,source++)
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

			while(1)
			{
				for(ypos=0;ypos<19;ypos++)
				{
					if (After[ypos] - Before[ypos] >= 15)
						break;
					Before[ypos]--;
				}
				if (ypos < 19)
					break;
				adjustment--;
			}
			BigCharWidth[numA][numB] = adjustment+biggestX;
		}
	}

	sprintf(temp,"%s\\gfx\\menu\\fontsize.lmp",com_gamedir);
	FH = fopen(temp,"wb");
	fwrite(BigCharWidth,1,sizeof(BigCharWidth),FH);
	fclose(FH);
#else
	COM_LoadStackFile ("gfx/menu/fontsize.lmp",BigCharWidth,sizeof(BigCharWidth)+1);
#endif
}

/*
================
Draw_Character

Draws one 8*8 graphics character with 0 being transparent.
It can be clipped to the top of the screen to allow the console to be
smoothly scrolled off.
================
*/

#ifndef	GLQUAKE

static int M_DrawBigCharacter (int x, int y, int num, int numNext)
{
	qpic_t	*p;
	int	ypos,xpos;
	byte	*dest;
	byte	*source;
	int	add;

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

	p = Draw_CachePic ("gfx/menu/bigfont.lmp");
	source = p->data + ((num % 8) * 20) + (num / 8 * p->width * 20);

	for(ypos=0;ypos < 19;ypos++)
	{
		dest = vid.buffer + (y+ypos) * vid.rowbytes + x;
		for(xpos=0;xpos<19;xpos++,dest++,source++)
		{
			if (*source)
			{
				*dest = *source;
			}
		}
		source += (p->width - 19);
	}

	if (numNext < 0 || numNext >= 27)
		return 0;

	add = 0;
	if (num == (int)'C'-65 && numNext == (int)'P'-65)
		add = 3;

	return BigCharWidth[num][numNext] + add;
}

#endif

static void M_DrawBigString(int x, int y, char *string)
{
	int c,length;

	x += ((vid.width - 320)>>1);

	length = strlen(string);
	for(c=0;c<length;c++)
	{
		x += M_DrawBigCharacter(x,y,string[c],string[c+1]);
	}
}


void ScrollTitle (char *name)
{
	float delta;
	qpic_t	*p;
	static char *LastName = "";
	int finaly;
	static qboolean CanSwitch = true;

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

	if (Q_strcasecmp(LastName,name) != 0 && TitleTargetPercent != 0)
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

static void ReInitMusic(void);
static char	old_bgmtype[20];	// S.A


void M_Menu_Main_f (void)
{
	// Deactivate the mouse when the menus are drawn - S.A.
	mousestate_sa = true;
	if (modestate == MS_WINDOWED)
		IN_DeactivateMouse ();

	if (key_dest != key_menu)
	{
		m_save_demonum = cls.demonum;
		cls.demonum = -1;
	}
	key_dest = key_menu;
	m_state = m_main;
	m_entersound = true;
}


static void M_Main_Draw (void)
{
	int	f;

	ScrollTitle("gfx/menu/title0.lmp");
//	M_DrawTransPic (72, 32, Draw_CachePic ("gfx/mainmenu.lmp") );
	M_DrawBigString (72,60+(0*20),"SINGLE PLAYER");
	M_DrawBigString (72,60+(1*20),"MULTIPLAYER");
	M_DrawBigString (72,60+(2*20),"OPTIONS");
	M_DrawBigString (72,60+(3*20),"HELP");
	M_DrawBigString (72,60+(4*20),"QUIT");

	f = (int)(host_time * 10)%8;
	M_DrawTransPic (43, 54 + m_main_cursor * 20,Draw_CachePic( va("gfx/menu/menudot%i.lmp", f+1 ) ) );
}


static void M_Main_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		// leaving the main menu, reactivate mouse - S.A.
		mousestate_sa = false;
		IN_ActivateMouse ();
		// and check we haven't changed the music type
		if (strlen(old_bgmtype)!=0 && strcmp(old_bgmtype,bgmtype.string)!=0)
			ReInitMusic ();
		strcpy (old_bgmtype, "");

		key_dest = key_game;
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

char	*plaquemessage = NULL;   // Pointer to current plaque message
char    *errormessage = NULL;


//=============================================================================
/* DIFFICULTY MENU */

static void M_Menu_Difficulty_f (void)
{
	key_dest = key_menu;
	m_state = m_difficulty;
}

static int	m_diff_cursor;
static int	m_enter_portals = 0;
#define	DIFF_ITEMS	NUM_DIFFLEVELS

static void M_Difficulty_Draw (void)
{
	int	f, i;

	ScrollTitle("gfx/menu/title5.lmp");

	setup_class = cl_playerclass.value;

	if (setup_class < 1 || setup_class > MAX_PLAYER_CLASS)
		setup_class = MAX_PLAYER_CLASS;
	setup_class--;

	for(i = 0; i < NUM_DIFFLEVELS; ++i)
		M_DrawBigString (72,60+(i*20),DiffNames[setup_class][i]);

	f = (int)(host_time * 10)%8;
	M_DrawTransPic (43, 54 + m_diff_cursor * 20,Draw_CachePic( va("gfx/menu/menudot%i.lmp", f+1 ) ) );
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
		if (m_enter_portals)
		{
			introTime = 0.0;
			cl.intermission = 12;
			cl.completed_time = cl.time;
			key_dest = key_game;
			m_state = m_none;
			cls.demonum = m_save_demonum;
			//Cbuf_AddText ("map keep1\n");
			return;
		}
		//Cbuf_AddText ("map demo1\n");
		m_state = m_none;
		Cbuf_AddText ("wait; map demo1\n");
		break;
	default:
		key_dest = key_game;
		m_state = m_none;
		break;
	}
}


//=============================================================================
/* CLASS CHOICE MENU */

static int class_flag;

static void M_Menu_Class_f (void)
{
	class_flag=0;
	key_dest = key_menu;
	m_state = m_class;
}

static void M_Menu_Class2_f (void)
{
	key_dest = key_menu;
	m_state = m_class;
	class_flag=1;
}

// change the define below to 0 if you want to allow the
// demoness class  in old mission through the menu system
#define DISALLOW_DEMONESS_IN_OLD_GAME	1

#ifndef H2MP
// do not touch these
#undef DISALLOW_DEMONESS_IN_OLD_GAME
#define DISALLOW_DEMONESS_IN_OLD_GAME	0
#endif

static int	m_class_cursor;
#define	CLASS_ITEMS	MAX_PLAYER_CLASS

static void M_Class_Draw (void)
{
	int	f, i;

	ScrollTitle("gfx/menu/title2.lmp");
#if DISALLOW_DEMONESS_IN_OLD_GAME
	if (!m_enter_portals)
	{
		for(i = 0; i < MAX_PLAYER_CLASS -1; ++i)
			M_DrawBigString (72,60+(i*20),ClassNamesU[i]);
	}
	else
#endif
		for(i = 0; i < MAX_PLAYER_CLASS; ++i)
			M_DrawBigString (72,60+(i*20),ClassNamesU[i]);

#if DISALLOW_DEMONESS_IN_OLD_GAME
	if (!m_enter_portals)
	{
		if (m_class_cursor >= CLASS_ITEMS -1)
			m_class_cursor = 0;
	}
#endif

	f = (int)(host_time * 10)%8;
	M_DrawTransPic (43, 54 + m_class_cursor * 20,Draw_CachePic( va("gfx/menu/menudot%i.lmp", f+1 ) ) );

	M_DrawPic (251,54 + 21, Draw_CachePic (va("gfx/cport%d.lmp", m_class_cursor + 1)));
	M_DrawTransPic (242,54, Draw_CachePic ("gfx/menu/frame.lmp"));
}

static void M_Class_Key (int key)
{
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
#if DISALLOW_DEMONESS_IN_OLD_GAME
		if (!m_enter_portals)
		{
			if (++m_class_cursor >= CLASS_ITEMS -1)
				m_class_cursor = 0;
		}
		else
#endif
			if (++m_class_cursor >= CLASS_ITEMS)
				m_class_cursor = 0;

//		if ((!registered.value && !oem.value) && m_class_cursor >= 1 && m_class_cursor <= 2)
//			m_class_cursor = CLASS_ITEMS - 1;

		break;

	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
#if DISALLOW_DEMONESS_IN_OLD_GAME
		if (!m_enter_portals)
		{
			if (--m_class_cursor < 0)
				m_class_cursor = CLASS_ITEMS - 2;
		}
		else
#endif
			if (--m_class_cursor < 0)
				m_class_cursor = CLASS_ITEMS - 1;

//		if ((!registered.value && !oem.value) && m_class_cursor >= 1 && m_class_cursor <= 2)
//			m_class_cursor = 0;

		break;

	case K_ENTER:
//		sv_player->v.playerclass=m_class_cursor+1;
		Cbuf_AddText ( va ("playerclass %d\n", m_class_cursor+1) );
		m_entersound = true;
		if (!class_flag)
		{
			M_Menu_Difficulty_f();
		}
		else
		{
			key_dest = key_game;
			m_state = m_none;
		}
		break;
	default:
		key_dest = key_game;
		m_state = m_none;
		break;
	}
}


//=============================================================================
/* SINGLE PLAYER MENU */

static int	m_singleplayer_cursor;
#ifdef H2MP
#define	SINGLEPLAYER_ITEMS	5
#else
#define SINGLEPLAYER_ITEMS	3
#endif


static void M_Menu_SinglePlayer_f (void)
{
	key_dest = key_menu;
	m_state = m_singleplayer;
	m_entersound = true;
	Cvar_SetValue ("timelimit", 0);		//put this here to help play single after dm
}

static void M_SinglePlayer_Draw (void)
{
	int	f;

	ScrollTitle("gfx/menu/title1.lmp");

	M_DrawBigString (72,60+(0*20),"NEW MISSION");
	M_DrawBigString (72,60+(1*20),"LOAD");
	M_DrawBigString (72,60+(2*20),"SAVE");
#ifdef H2MP
	M_DrawBigString (72,60+(3*20),"OLD MISSION");
	M_DrawBigString (72,60+(4*20),"VIEW INTRO");
#endif

	f = (int)(host_time * 10)%8;
	M_DrawTransPic (43, 54 + m_singleplayer_cursor * 20,Draw_CachePic( va("gfx/menu/menudot%i.lmp", f+1 ) ) );
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
		if (++m_singleplayer_cursor >= SINGLEPLAYER_ITEMS)
			m_singleplayer_cursor = 0;
		break;
	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
		if (--m_singleplayer_cursor < 0)
			m_singleplayer_cursor = SINGLEPLAYER_ITEMS - 1;
		break;
	case K_ENTER:
		m_entersound = true;
		m_enter_portals = 0;
		switch (m_singleplayer_cursor)
		{
		case 0:
#ifdef H2MP
			m_enter_portals = 1;
#endif
		case 3:
			if (sv.active)
				if (!SCR_ModalMessage("Are you sure you want to\nstart a new game?\n"))
					break;
			key_dest = key_game;
			if (sv.active)
				Cbuf_AddText ("disconnect\n");
			CL_RemoveGIPFiles(NULL);
			Cbuf_AddText ("maxplayers 1\n");
			M_Menu_Class_f ();
			break;

		case 1:
			M_Menu_Load_f ();
			break;

		case 2:
			M_Menu_Save_f ();
			break;
#ifdef H2MP
		case 4:
			key_dest = key_game;
			Cbuf_AddText("playdemo t9\n");
			break;
#endif
		}
	}
}

//=============================================================================
/* LOAD/SAVE MENU */

static int		load_cursor;		// 0 < load_cursor < MAX_SAVEGAMES

#define	MAX_SAVEGAMES		12
static char	m_filenames[MAX_SAVEGAMES][SAVEGAME_COMMENT_LENGTH+1];
static int		loadable[MAX_SAVEGAMES];

static void M_ScanSaves (void)
{
	int		i, j;
	char	name[MAX_OSPATH];
	FILE	*f;
	int		version;

	for (i=0 ; i<MAX_SAVEGAMES ; i++)
	{
		strcpy (m_filenames[i], "--- UNUSED SLOT ---");
		loadable[i] = false;
		sprintf (name, "%s/s%i/info.dat", com_savedir, i);
		f = fopen (name, "r");
		if (!f)
			continue;
		fscanf (f, "%i\n", &version);
		fscanf (f, "%79s\n", name);
		strncpy (m_filenames[i], name, sizeof(m_filenames[i])-1);

	// change _ back to space
		for (j=0 ; j<SAVEGAME_COMMENT_LENGTH ; j++)
			if (m_filenames[i][j] == '_')
				m_filenames[i][j] = ' ';
		loadable[i] = true;
		fclose (f);
	}
}

static void M_Menu_Load_f (void)
{
	m_entersound = true;
	m_state = m_load;
	key_dest = key_menu;
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
	key_dest = key_menu;
	M_ScanSaves ();
}


static void M_Load_Draw (void)
{
	int		i;

	ScrollTitle("gfx/menu/load.lmp");

	for (i=0 ; i< MAX_SAVEGAMES; i++)
		M_Print (16, 60 + 8*i, m_filenames[i]);

// line cursor
	M_DrawCharacter (8, 60 + load_cursor*8, 12+((int)(realtime*4)&1));
}


static void M_Save_Draw (void)
{
	int		i;

	ScrollTitle("gfx/menu/save.lmp");

	for (i=0 ; i<MAX_SAVEGAMES ; i++)
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

	case K_ENTER:
		S_LocalSound ("raven/menu2.wav");
		if (!loadable[load_cursor])
			return;
		m_state = m_none;
		key_dest = key_game;

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

	case K_ENTER:
		m_state = m_none;
		key_dest = key_game;
		Cbuf_AddText (va("save s%i\n", load_cursor));
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
	int		i, j;
	char	name[MAX_OSPATH];
	FILE	*f;
	int		version;

	for (i=0 ; i<MAX_SAVEGAMES ; i++)
	{
		strcpy (m_filenames[i], "--- UNUSED SLOT ---");
		loadable[i] = false;
		sprintf (name, "%s/ms%i/info.dat", com_savedir, i);
		f = fopen (name, "r");
		if (!f)
			continue;
		fscanf (f, "%i\n", &version);
		fscanf (f, "%79s\n", name);
		strncpy (m_filenames[i], name, sizeof(m_filenames[i])-1);

	// change _ back to space
		for (j=0 ; j<SAVEGAME_COMMENT_LENGTH ; j++)
			if (m_filenames[i][j] == '_')
				m_filenames[i][j] = ' ';
		loadable[i] = true;
		fclose (f);
	}
}

static void M_Menu_MLoad_f (void)
{
	m_entersound = true;
	m_state = m_mload;
	key_dest = key_menu;
	M_ScanMSaves ();
}


static void M_Menu_MSave_f (void)
{
	if (!sv.active || cl.intermission || svs.maxclients == 1)
	{
		message = "Only a network server";
		message2 = "can save a multiplayer game";
		message_time = host_time;
		return;
	}
	m_entersound = true;
	m_state = m_msave;
	key_dest = key_menu;
	M_ScanMSaves ();
}


static void M_MLoad_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
		M_Menu_MultiPlayer_f ();
		break;

	case K_ENTER:
		S_LocalSound ("raven/menu2.wav");
		if (!loadable[load_cursor])
			return;
		m_state = m_none;
		key_dest = key_game;

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

	case K_ENTER:
		m_state = m_none;
		key_dest = key_game;
		Cbuf_AddText (va("save ms%i\n", load_cursor));
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
	key_dest = key_menu;
	m_state = m_multiplayer;
	m_entersound = true;

	message = NULL;
}


static void M_MultiPlayer_Draw (void)
{
	int	f;

	ScrollTitle("gfx/menu/title4.lmp");
//	M_DrawTransPic (72, 32, Draw_CachePic ("gfx/mp_menu.lmp") );

	M_DrawBigString (72,60+(0*20),"JOIN A GAME");
	M_DrawBigString (72,60+(1*20),"NEW GAME");
	M_DrawBigString (72,60+(2*20),"SETUP");
	M_DrawBigString (72,60+(3*20),"LOAD");
	M_DrawBigString (72,60+(4*20),"SAVE");

	f = (int)(host_time * 10)%8;
	M_DrawTransPic (43, 54 + m_multiplayer_cursor * 20,Draw_CachePic( va("gfx/menu/menudot%i.lmp", f+1 ) ) );

	if (message)
	{
		M_PrintWhite ((320/2) - ((27*8)/2), 168, message);
		M_PrintWhite ((320/2) - ((27*8)/2), 176, message2);
		if (host_time - 5 > message_time)
			message = NULL;
	}

	if (ipxAvailable || tcpipAvailable)
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
			if (ipxAvailable || tcpipAvailable)
				M_Menu_Net_f ();
			break;

		case 1:
			if (ipxAvailable || tcpipAvailable)
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
static int		setup_cursor_table[] = {40, 56, 80, 104, 128, 156};

static char	setup_hostname[16];
static char	setup_myname[16];
static int		setup_oldtop;
static int		setup_oldbottom;
static int		setup_top;
static int		setup_bottom;

#define	NUM_SETUP_CMDS	6

static void M_Menu_Setup_f (void)
{
	key_dest = key_menu;
	m_state = m_setup;
	m_entersound = true;
	strcpy(setup_myname, cl_name.string);
	strcpy(setup_hostname, hostname.string);
	setup_top = setup_oldtop = (((int)cl_color.value) >> 4) & 15;
	setup_bottom = setup_oldbottom = ((int)cl_color.value) & 15;
	setup_class = cl_playerclass.value;
	if (setup_class < 1 || setup_class > MAX_PLAYER_CLASS)
		setup_class = MAX_PLAYER_CLASS;
}


#if 0
static void M_DrawTransPicTranslate (int x, int y, qpic_t *pic)
{
	Draw_TransPicTranslate (x + ((vid.width - 320)>>1), y, pic, translationTable);
}
#endif

#ifdef GLQUAKE
#define M_DrawTransPicTranslate(x,y,pic,p_class,top,bottom) Draw_TransPicTranslate(x + ((vid.width - 320)>>1), y, pic, translationTable, p_class, top, bottom)
#else
#define M_DrawTransPicTranslate(x,y,pic,p_class,top,bottom) Draw_TransPicTranslate(x + ((vid.width - 320)>>1), y, pic, translationTable)
#endif

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
	M_DrawTransPicTranslate (220, 72, p, setup_class, setup_top, setup_bottom);

	M_DrawCharacter (56, setup_cursor_table [setup_cursor], 12+((int)(realtime*4)&1));

	if (setup_cursor == 0)
		M_DrawCharacter (168 + 8*strlen(setup_hostname), setup_cursor_table [setup_cursor], 10+((int)(realtime*4)&1));

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
			setup_class--;
			if (setup_class < 1)
				setup_class = MAX_PLAYER_CLASS;

//			if ((!registered.value && !oem.value) && setup_class >= 2 && setup_class < MAX_PLAYER_CLASS)
//				setup_class = 5;
		}
		if (setup_cursor == 3)
			setup_top = setup_top - 1;
		if (setup_cursor == 4)
			setup_bottom = setup_bottom - 1;
		break;
	case K_RIGHTARROW:
		if (setup_cursor < 2)
			return;
forward:
		S_LocalSound ("raven/menu3.wav");
		if (setup_cursor == 2)
		{
			setup_class++;
			if (setup_class > MAX_PLAYER_CLASS)
				setup_class = 1;

//			if ((!registered.value && !oem.value) && setup_class >= 2 && setup_class < MAX_PLAYER_CLASS)
//				setup_class = MAX_PLAYER_CLASS;
		}
		if (setup_cursor == 3)
			setup_top = setup_top + 1;
		if (setup_cursor == 4)
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
	if (setup_top < 0)
		setup_top = 10;
	if (setup_bottom > 10)
		setup_bottom = 0;
	if (setup_bottom < 0)
		setup_bottom = 10;
}

//=============================================================================
/* NET MENU */

static int	m_net_cursor = 0;
static int	m_net_items;

char *net_helpMessage [] = 
{
/* .........1.........2.... */
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
	key_dest = key_menu;
	m_state = m_net;
	m_entersound = true;
	m_net_items = 2;

	if (m_net_cursor >= m_net_items)
		m_net_cursor = 0;
	m_net_cursor--;
	M_Net_Key (K_DOWNARROW);
}


static void M_Net_Draw (void)
{
	int	f;

	ScrollTitle("gfx/menu/title4.lmp");

	M_DrawBigString (72,70+(0*20),"IPX");
	M_DrawBigString (72,70+(1*20),"TCP/IP");

	f = (320-26*8)/2;
	M_DrawTextBox (f, 134, 24, 4);
	f += 8;
	M_Print (f, 142, net_helpMessage[m_net_cursor*4+0]);
	M_Print (f, 150, net_helpMessage[m_net_cursor*4+1]);
	M_Print (f, 158, net_helpMessage[m_net_cursor*4+2]);
	M_Print (f, 166, net_helpMessage[m_net_cursor*4+3]);

	f = (int)(host_time * 10)%8;
	M_DrawTransPic (43, 64 + m_net_cursor * 20,Draw_CachePic( va("gfx/menu/menudot%i.lmp", f+1 ) ) );
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
		if (++m_net_cursor >= m_net_items)
			m_net_cursor = 0;
		break;

	case K_UPARROW:
//		S_LocalSound ("raven/menu1.wav");
		if (--m_net_cursor < 0)
			m_net_cursor = m_net_items - 1;
		break;

	case K_ENTER:
		m_entersound = true;
		M_Menu_LanConfig_f ();
		break;
	}

	if (m_net_cursor == 0 && !ipxAvailable)
		goto again;
	if (m_net_cursor == 1 && !tcpipAvailable)
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
	OPT_SCRSIZE,	//3
	OPT_GAMMA,	//4
	OPT_MOUSESPEED,	//5
	OPT_MUSICTYPE,	//6
	OPT_MUSICVOL,	//7
	OPT_SNDVOL,	//8
	OPT_ALWAYRUN,	//9
	OPT_INVMOUSE,	//10
	OPT_ALWAYSMLOOK,//11
	OPT_USEMOUSE,	//12
	OPT_CROSSHAIR,	//13
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
	key_dest = key_menu;
	m_state = m_options;
	m_entersound = true;

	// get the current music type
	if (!strlen(old_bgmtype))
		strncpy(old_bgmtype,bgmtype.string,20);
#if 0	// change to 1 if dont want to disable mouse in fullscreen
	if ((options_cursor == OPT_USEMOUSE) && (modestate != MS_WINDOWED))
		options_cursor = 0;
#endif
}


static void M_AdjustSliders (int dir)
{
	S_LocalSound ("raven/menu3.wav");

	switch (options_cursor)
	{
	case OPT_SCRSIZE:	// screen size
		scr_viewsize.value += dir * 10;
		Cvar_SetValue ("viewsize", scr_viewsize.value);
		break;
	case OPT_GAMMA:	// gamma
		v_gamma.value -= dir * 0.05;
		if (v_gamma.value < 0.5)
			v_gamma.value = 0.5;
		if (v_gamma.value > 1)
			v_gamma.value = 1;
		Cvar_SetValue ("gamma", v_gamma.value);
		break;
	case OPT_MOUSESPEED:	// mouse speed
		sensitivity.value += dir * 0.5;
		if (sensitivity.value < 1)
			sensitivity.value = 1;
		if (sensitivity.value > 11)
			sensitivity.value = 11;
		Cvar_SetValue ("sensitivity", sensitivity.value);
		break;
	case OPT_MUSICTYPE: // bgm type
		if (Q_strcasecmp(bgmtype.string,"midi") == 0)
		{
			if (dir < 0)
				Cvar_Set("bgmtype","none");
			else
				Cvar_Set("bgmtype","cd");
		}
		else if (Q_strcasecmp(bgmtype.string,"cd") == 0)
		{
			if (dir < 0)
				Cvar_Set("bgmtype","midi");
			else
				Cvar_Set("bgmtype","none");
		}
		else
		{
			if (dir < 0)
				Cvar_Set("bgmtype","cd");
			else
				Cvar_Set("bgmtype","midi");
		}
		break;
	case OPT_MUSICVOL:	// music volume
		bgmvolume.value += dir * 0.1;

		if (bgmvolume.value < 0)
			bgmvolume.value = 0;
		if (bgmvolume.value > 1)
			bgmvolume.value = 1;
		Cvar_SetValue ("bgmvolume", bgmvolume.value);
		break;
	case OPT_SNDVOL:	// sfx volume
		sfxvolume.value += dir * 0.1;
		if (sfxvolume.value < 0)
			sfxvolume.value = 0;
		if (sfxvolume.value > 1)
			sfxvolume.value = 1;
		Cvar_SetValue ("volume", sfxvolume.value);
		break;

	case OPT_ALWAYRUN:	// always run
		if (cl_forwardspeed.value > 200)
		{
			Cvar_SetValue ("cl_forwardspeed", 200);
			Cvar_SetValue ("cl_backspeed", 200);
		}
		else
		{
			Cvar_SetValue ("cl_forwardspeed", 400);
			Cvar_SetValue ("cl_backspeed", 400);
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
		Cvar_SetValue ("crosshair", !crosshair.value);
		break;

	case OPT_CHASE_ACTIVE:	// chase_active
		Cvar_SetValue ("chase_active", !chase_active.value);
		break;

	case OPT_USEMOUSE:	// _enable_mouse
		Cvar_SetValue ("_enable_mouse", !_enable_mouse.value);
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
	if (range > 1)
		range = 1;
	M_DrawCharacter (x-8, y, 256);
	for (i=0 ; i<SLIDER_RANGE ; i++)
		M_DrawCharacter (x + i*8, y, 257);
	M_DrawCharacter (x+i*8, y, 258);
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
	float	r;

	mousestate_sa = false;
	IN_ActivateMouse ();	// we entered the customization menu

	ScrollTitle("gfx/menu/title3.lmp");

	M_Print (16, 60+(0*8),	"    Customize controls");
	M_Print (16, 60+(1*8),	"         Go to console");
	M_Print (16, 60+(2*8),	"     Reset to defaults");

	M_Print (16, 60+(3*8),	"           Screen size");
	r = (scr_viewsize.value - 30) / (120 - 30);
	M_DrawSlider (220, 60+(3*8), r);

	M_Print (16, 60+(4*8),	"            Brightness");
	r = (1.0 - v_gamma.value) / 0.5;
	M_DrawSlider (220, 60+(4*8), r);

	M_Print (16, 60+(5*8),	"           Mouse Speed");
	r = (sensitivity.value - 1)/10;
	M_DrawSlider (220, 60+(5*8), r);

	M_Print (16, 60+(6*8),	"            Music Type");
	if (Q_strcasecmp(bgmtype.string,"midi") == 0)
		M_Print (220, 60+(6*8), "MIDI");
	else if (Q_strcasecmp(bgmtype.string,"cd") == 0)
		M_Print (220, 60+(6*8), "CD");
	else
		M_Print (220, 60+(6*8), "None");

	M_Print (16, 60+(7*8),	"          Music Volume");
	r = bgmvolume.value;
	M_DrawSlider (220, 60+(7*8), r);

	M_Print (16, 60+(8*8),	"          Sound Volume");
	r = sfxvolume.value;
	M_DrawSlider (220, 60+(8*8), r);

	M_Print (16, 60+(9*8),			"            Always Run");
	M_DrawCheckbox (220, 60+(9*8), cl_forwardspeed.value > 200);

	M_Print (16, 60+(OPT_INVMOUSE*8),	"          Invert Mouse");
	M_DrawCheckbox (220, 60+(OPT_INVMOUSE*8), m_pitch.value < 0);

	M_Print (16,60+(OPT_ALWAYSMLOOK*8),	"            Mouse Look");
	M_DrawCheckbox (220, 60+(OPT_ALWAYSMLOOK*8), in_mlook.state & 1);

	M_Print (16, 60+(OPT_USEMOUSE*8),	"             Use Mouse");
	M_DrawCheckbox (220, 60+(OPT_USEMOUSE*8), _enable_mouse.value);

	M_Print (16, 60+(OPT_CROSSHAIR*8),	"        Show Crosshair");
	M_DrawCheckbox (220, 60+(OPT_CROSSHAIR*8), crosshair.value);

#ifdef GLQUAKE
	M_Print (16, 60+(OPT_OPENGL*8),		"       OpenGL Features");
#endif

	M_Print (16, 60+(OPT_CHASE_ACTIVE*8),	"            Chase Mode");
	M_DrawCheckbox (220, 60+(OPT_CHASE_ACTIVE*8), chase_active.value);

	if (vid_menudrawfn)
		M_Print (16, 60+(OPT_VIDEO*8),	"           Video Modes");

	// cursor
	// doesn't get drawn properly with XFree4.3/MGA200 S.A.
	M_DrawCharacter (200, 60 + options_cursor*8, 12+((int)(realtime*4)&1));
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
	OGL_PURGETEX,
	OGL_GLOW1,
	OGL_GLOW2,
	OGL_GLOW3,
	OGL_COLOREDLIGHT,
	OGL_COLOREDSTATIC,
	OGL_COLOREDDYNAMIC,
	OGL_COLOREDEXTRA,
	OGL_TEXFILTER,
	OGL_SHADOWS,
	OGL_STENCIL,
	OGL_ITEMS
};

typedef struct
{
	char *name;
	int	minimize, maximize;
} glmode_t;

// this must match modes[] in gl_draw.c
#define MAX_GL_FILTERS	6
extern glmode_t modes[];
static int	tex_mode;
static int	opengl_cursor;

static void M_Menu_OpenGL_f (void)
{
	key_dest = key_menu;
	m_state = m_opengl;
	m_entersound = true;
}


static void M_OpenGL_Draw (void)
{
	int		i;

	ScrollTitle("gfx/menu/title3.lmp");
	M_PrintWhite (96, 72, "OpenGL Features:");

	M_Print (32, 90 + 8*OGL_MULTITEX,	"        Multitexturing");
	if (gl_mtexable)
		M_DrawCheckbox (232, 90 + 8*OGL_MULTITEX, gl_multitexture.value);
	else
		M_Print (232, 90 + 8*OGL_MULTITEX, "Not found");

	M_Print (32, 90 + 8*OGL_PURGETEX,	"    Purge map textures");
	M_DrawCheckbox (232, 90 + 8*OGL_PURGETEX, gl_purge_maptex.value);

	M_Print (32, 90 + 8*OGL_GLOW1,		"          Glow effects");
	M_DrawCheckbox (232, 90 + 8*OGL_GLOW1, gl_glows.value);
	M_Print (32, 90 + 8*OGL_GLOW2,		"         missile glows");
	M_DrawCheckbox (232, 90 + 8*OGL_GLOW2, gl_missile_glows.value);
	M_Print (32, 90 + 8*OGL_GLOW3,		"           other glows");
	M_DrawCheckbox (232, 90 + 8*OGL_GLOW3, gl_other_glows.value);

	M_Print (32, 90 + 8*OGL_COLOREDLIGHT,	"      Colored lights :");
	M_Print (32, 90 + 8*OGL_COLOREDSTATIC,	"         static lights");
	M_Print (32, 90 + 8*OGL_COLOREDDYNAMIC,	"        dynamic lights");
	M_Print (32, 90 + 8*OGL_COLOREDEXTRA,	"          extra lights");
	M_Print (232, 90 + 8*OGL_COLOREDLIGHT, "(requires level reload)");
	if (gl_lightmap_format == GL_RGBA)
	{
		switch ((int)gl_coloredlight.value)
		{
		case 0:
			M_Print (232, 90 + 8*OGL_COLOREDSTATIC, "none (white)");
			break;
		case 1:
			M_Print (232, 90 + 8*OGL_COLOREDSTATIC, "colored");
			break;
		case 2:
			M_Print (232, 90 + 8*OGL_COLOREDSTATIC, "blend");
			break;
		}
	//	M_DrawCheckbox (232, 90 + 8*OGL_COLOREDDYNAMIC, (int)gl_colored_dynamic_lights.value);
	//	M_DrawCheckbox (232, 90 + 8*OGL_COLOREDEXTRA, (int)gl_extra_dynamic_lights.value);
	}
	else
	{
		M_Print (232, 90 + 8*OGL_COLOREDSTATIC, "hot available");
	//	M_Print (232, 90 + 8*OGL_COLOREDDYNAMIC, "hot available");
	//	M_Print (232, 90 + 8*OGL_COLOREDEXTRA, "hot available");
	}
	M_DrawCheckbox (232, 90 + 8*OGL_COLOREDDYNAMIC, (int)gl_colored_dynamic_lights.value);
	M_DrawCheckbox (232, 90 + 8*OGL_COLOREDEXTRA, (int)gl_extra_dynamic_lights.value);

	M_Print (32, 90 + 8*OGL_TEXFILTER,	"     Texture filtering");
	for (i = 0; i < MAX_GL_FILTERS; i++)
	{
		if (modes[i].minimize == gl_filter_min)
		{
			tex_mode = i;
			M_Print (232, 90 + 8*OGL_TEXFILTER, modes[i].name);
			break;
		}
	}

	M_Print (32, 90 + 8*OGL_SHADOWS,	"               Shadows");
	M_DrawCheckbox (232, 90 + 8*OGL_SHADOWS, r_shadows.value);
	M_Print (32, 90 + 8*OGL_STENCIL,	"        Stencil buffer");
	if (have_stencil)
		M_DrawCheckbox (232, 90 + 8*OGL_STENCIL, gl_stencilshadow.value);
	else
		M_Print (232, 90 + 8*OGL_STENCIL, "Not found");

	// cursor
	M_DrawCharacter (216, 90 + opengl_cursor*8, 12+((int)(realtime*4)&1));
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
		opengl_cursor--;
		if (opengl_cursor == OGL_COLOREDLIGHT)
			opengl_cursor--;
		if (opengl_cursor < 0)
			opengl_cursor = OGL_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("raven/menu1.wav");
		opengl_cursor++;
		if (opengl_cursor == OGL_COLOREDLIGHT)
			opengl_cursor++;
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
			Cvar_SetValue ("gl_multitexture", !gl_multitexture.value);
			break;

		case OGL_PURGETEX:	// purge gl textures on map change
			Cvar_SetValue ("gl_purge_maptex", !gl_purge_maptex.value);
			break;

		case OGL_GLOW1:	// glow effects, main: torches
			Cvar_SetValue ("gl_glows", !gl_glows.value);
			break;

		case OGL_GLOW2:	// glow effects, missiles
			Cvar_SetValue ("gl_missile_glows", !gl_missile_glows.value);
			break;

		case OGL_GLOW3:	// glow effects, other: mana, etc.
			Cvar_SetValue ("gl_other_glows", !gl_other_glows.value);
			break;

		case OGL_COLOREDSTATIC:	// static colored lights
			if (gl_lightmap_format != GL_RGBA)
				break;
			switch (k)
			{
			case K_RIGHTARROW:
				if ((int)gl_coloredlight.value >= 1)
					Cvar_SetValue ("gl_coloredlight", 2);
				else
					Cvar_SetValue ("gl_coloredlight", 1);
				break;
			case K_LEFTARROW:
				if ((int)gl_coloredlight.value <= 1)
					Cvar_SetValue ("gl_coloredlight", 0);
				else
					Cvar_SetValue ("gl_coloredlight", 1);
				break;
			default:
				break;
			}
			break;

		case OGL_COLOREDDYNAMIC:	// dynamic colored lights
		//	if (gl_lightmap_format != GL_RGBA)
		//		break;
			Cvar_SetValue ("gl_colored_dynamic_lights", !gl_colored_dynamic_lights.value);
			break;

		case OGL_COLOREDEXTRA:	// extra dynamic colored lights
		//	if (gl_lightmap_format != GL_RGBA)
		//		break;
			Cvar_SetValue ("gl_extra_dynamic_lights", !gl_extra_dynamic_lights.value);
			break;

		case OGL_TEXFILTER:	// texture filter
			switch (k)
			{
			case K_LEFTARROW:
				tex_mode--;
				if (tex_mode < 0)
					tex_mode = 0;
				break;
			case K_RIGHTARROW:
				tex_mode++;
				if (tex_mode >= MAX_GL_FILTERS)
					tex_mode = MAX_GL_FILTERS-1;
				break;
			default:
				return;
			}
			if (modes[tex_mode].minimize != gl_filter_min)
			{
				Cbuf_AddText(va("gl_texturemode %s\n",modes[tex_mode].name));
				Cbuf_Execute();
			}
			break;

		case OGL_SHADOWS:	// shadows
			Cvar_SetValue ("r_shadows", !r_shadows.value);
			break;

		case OGL_STENCIL:	// stencil buffered shadows
			if (have_stencil)
				Cvar_SetValue ("gl_stencilshadow", !gl_stencilshadow.value);
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

static char *bindnames[][2] =
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
// command to display the mission pack's objectives
	{"+infoplaque",		"objectives"},
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
static int		bind_grab;
static int		keys_top = 0;

static void M_Menu_Keys_f (void)
{
	key_dest = key_menu;
	m_state = m_keys;
	m_entersound = true;
}


static void M_FindKeysForCommand (char *command, int *twokeys)
{
	int		count;
	int		j;
	int		l,l2;
	char	*b;

	twokeys[0] = twokeys[1] = -1;
	l = strlen(command);
	count = 0;

	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!strncmp (b, command, l))
		{
			l2= strlen(b);
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

static void M_UnbindCommand (char *command)
{
	int		j;
	int		l;
	char	*b;

	l = strlen(command);

	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!strncmp (b, command, l) )
			Key_SetBinding (j, "");
	}
}


static void M_Keys_Draw (void)
{
	int		i, l;
	int		keys[2];
	char		*name;
	int		x, y;
//	qpic_t	*p;

	ScrollTitle("gfx/menu/title6.lmp");

//	M_DrawTextBox (6,56, 35,16);

//	p = Draw_CachePic("gfx/menu/hback.lmp");
//	M_DrawTransPicCropped(8, 62, p);

	if (bind_grab)
		M_Print (12, 64, "Press a key or button for this action");
	else
		M_Print (18, 64, "Enter to change, backspace to clear");

	if (keys_top)
		M_DrawCharacter (6, 80, 128);
	if (keys_top + KEYS_SIZE < NUMCOMMANDS)
		M_DrawCharacter (6, 80 + ((KEYS_SIZE-1)*8), 129);

// search for known bindings
	for (i=0 ; i<KEYS_SIZE ; i++)
	{
		y = 80 + 8*i;

		M_Print (16, y, bindnames[i+keys_top][1]);

		l = strlen (bindnames[i+keys_top][0]);

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

	if (bind_grab)
		M_DrawCharacter (130, 80 + (keys_cursor-keys_top)*8, '=');
	else
		M_DrawCharacter (130, 80 + (keys_cursor-keys_top)*8, 12+((int)(realtime*4)&1));
}


static void M_Keys_Key (int k)
{
	char	cmd[80];
	int		keys[2];

	if (bind_grab)
	{	// defining a key
		S_LocalSound ("raven/menu1.wav");
		if (k == K_ESCAPE)
		{
			bind_grab = false;
		}
		else if (k != '`')
		{
			sprintf (cmd, "bind \"%s\" \"%s\"\n", Key_KeynumToString (k), bindnames[keys_cursor][0]);
			Cbuf_InsertText (cmd);
		}

		bind_grab = false;
		return;
	}

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
		if (keys_cursor >= NUMCOMMANDS)
			keys_cursor = 0;
		break;

	case K_ENTER:		// go into bind mode
		M_FindKeysForCommand (bindnames[keys_cursor][0], keys);
		S_LocalSound ("raven/menu2.wav");
		if (keys[1] != -1)
			M_UnbindCommand (bindnames[keys_cursor][0]);
		bind_grab = true;
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
	key_dest = key_menu;
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

#define	NUM_HELP_PAGES	5

static void M_Menu_Help_f (void)
{
	key_dest = key_menu;
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
static int		m_quit_prevstate;
static qboolean		wasInMenus;

#if 0
static char *quitMessage [] = 
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
static char **LineText;
static qboolean SoundPlayed;


#define MAX_LINES 138

static char *CreditText[MAX_LINES] =
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

#define MAX_LINES2 150

static char *Credit2Text[MAX_LINES2] =
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
   "",
};

#define QUIT_SIZE 16	// was 18. reduced two lines for two HoT strings

void M_Menu_Quit_f (void)
{
	if (m_state == m_quit)
		return;
	wasInMenus = (key_dest == key_menu);
	key_dest = key_menu;
	m_quit_prevstate = m_state;
	m_state = m_quit;
	m_entersound = true;
//	msgNumber = rand()&7;

	LinePos = 0;
	LineTimes = 0;
	LineText = CreditText;
	MaxLines = MAX_LINES;
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
			key_dest = key_game;
			m_state = m_none;
		}
		break;

	case 'Y':
	case 'y':
		key_dest = key_console;
		Host_Quit_f ();
		break;

	default:
		break;
	}
}

static void M_Quit_Draw (void)
{
	int i,x,y,place,topy;
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
			CDAudio_Play (12, false);
		}
	}

	y = 12;
	M_DrawTextBox (0, 0, 38, 23);
	M_Print      (16, y,    "        Hexen II version " STRINGIFY(ENGINE_VERSION));
	M_Print      (16, y+8,  "         by Raven Software");
#if HOT_VERSION_BETA
	M_PrintWhite (16, y+16, "     Hammer of Thyrion " HOT_VERSION_STR "-" HOT_VERSION_BETA_STR);
#else
	M_PrintWhite (16, y+16, "       Hammer of Thyrion " HOT_VERSION_STR);
#endif
	M_PrintWhite (16, y+24, "             Source Port");
	y += 40;

	if (LinePos > 55 && !SoundPlayed && LineText == Credit2Text)
	{
		S_LocalSound ("rj/steve.wav");
		SoundPlayed = true;
	}
	topy = y;
	place = floor(LinePos);
	y -= floor((LinePos - place) * 8);
	for(i=0;i<QUIT_SIZE;i++,y+=8)
	{
		if (i+place-QUIT_SIZE >= MaxLines)
			break;
		if (i+place < QUIT_SIZE)
			continue;

		if (LineText[i+place-QUIT_SIZE][0] == ' ')
			M_PrintWhite(24,y,LineText[i+place-QUIT_SIZE]);
		else
			M_Print(24,y,LineText[i+place-QUIT_SIZE]);
	}

	p = Draw_CachePic ("gfx/box_mm2.lmp");
	x = 24;
	y = topy-8;
	for(i=4;i<38;i++,x+=8)
	{
		M_DrawPic(x, y, p);	//background at top for smooth scroll out
		M_DrawPic(x, y+(QUIT_SIZE*8), p);	//draw at bottom for smooth scroll in
	}

	y += (QUIT_SIZE*8)+8;
	M_PrintWhite (16, y,  "          Press y to exit           ");
}

//=============================================================================
/* LAN CONFIG MENU */

static int		lanConfig_cursor = -1;
static int		lanConfig_cursor_table [] = {100, 120, 140, 172};
#define NUM_LANCONFIG_CMDS	4

static int	lanConfig_port;
static char	lanConfig_portname[6];
static char	lanConfig_joinname[30];

static void M_Menu_LanConfig_f (void)
{
	key_dest = key_menu;
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
	sprintf(lanConfig_portname, "%u", lanConfig_port);

	m_return_onerror = false;
	m_return_reason[0] = 0;

	setup_class = cl_playerclass.value;
	if (setup_class < 1 || setup_class > MAX_PLAYER_CLASS)
		setup_class = MAX_PLAYER_CLASS;
	setup_class--;
}


static void M_LanConfig_Draw (void)
{
	int	basex;
	char	*startJoin;
	char	*protocol;

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
			key_dest = key_game;
			m_state = m_none;
			Cbuf_AddText ( va ("connect \"%s\"\n", lanConfig_joinname) );
			break;
		}

		break;

	case K_BACKSPACE:
		if (lanConfig_cursor == 0)
		{
			if (strlen(lanConfig_portname))
				lanConfig_portname[strlen(lanConfig_portname)-1] = 0;
		}

		if (lanConfig_cursor == 3)
		{
			if (strlen(lanConfig_joinname))
				lanConfig_joinname[strlen(lanConfig_joinname)-1] = 0;
		}
		break;

	case K_LEFTARROW:
		if (lanConfig_cursor != 1 || !JoiningGame)
			break;

		S_LocalSound ("raven/menu3.wav");
		setup_class--;
		if (setup_class < 0)
			setup_class = MAX_PLAYER_CLASS -1;
		break;

	case K_RIGHTARROW:
		if (lanConfig_cursor != 1 || !JoiningGame)
			break;

		S_LocalSound ("raven/menu3.wav");
		setup_class++;
		if (setup_class > MAX_PLAYER_CLASS - 1)
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
	sprintf(lanConfig_portname, "%u", lanConfig_port);
}

//=============================================================================
/* GAME OPTIONS MENU */

typedef struct
{
	char	*name;
	char	*description;
} level_t;

static level_t	levels[] =
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

#ifdef H2MP
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
#endif
};

typedef struct
{
	char	*description;
	int		firstLevel;
	int		levels;
} episode_t;

static episode_t	episodes[] =
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
#ifdef H2MP
	{"MISSION PACK", 55, 15},
#endif
	{"Deathmatch", 40, 5},

	// OEM
	{"Village", 46, 9},
	{"Deathmatch", 45, 1},
};

#ifndef H2MP
#define WITH_H2MP 0
#else
#define WITH_H2MP 1
#endif
#define REG_START 2
#define MP_START 7
#define DM_START  (MP_START+WITH_H2MP)
#define OEM_START (DM_START+1)

static int	startepisode;
static int	startlevel;
static int	maxplayers;
static qboolean m_serverInfoMessage = false;
static double	m_serverInfoMessageTime;

static int gameoptions_cursor_table[] = {40, 56, 64, 72, 80, 88, 96, 104, 112, 128, 136};
#define	NUM_GAMEOPTIONS	11
static int	gameoptions_cursor;

static void M_Menu_GameOptions_f (void)
{
	key_dest = key_menu;
	m_state = m_gameoptions;
	m_entersound = true;
	if (maxplayers == 0)
		maxplayers = svs.maxclients;
	if (maxplayers < 2)
		maxplayers = svs.maxclientslimit;

	setup_class = cl_playerclass.value;
	if (setup_class < 1 || setup_class > MAX_PLAYER_CLASS)
		setup_class = MAX_PLAYER_CLASS;
	setup_class--;

	if (oem.value && startepisode < OEM_START)
		startepisode = OEM_START;

	if (registered.value && (startepisode < REG_START || startepisode >= OEM_START))
		startepisode = REG_START;

	if (coop.value)
	{
		startlevel = 0;
		if (startepisode == 1)
			startepisode = 0;
		else if (startepisode == DM_START)
			startepisode = REG_START;
		if (gameoptions_cursor >= NUM_GAMEOPTIONS-1)
			gameoptions_cursor = 0;
	}
}

static void M_GameOptions_Draw (void)
{
	ScrollTitle("gfx/menu/title4.lmp");

	M_DrawTextBox (152+8, 60, 10, 1);
	M_Print (160+8, 68, "begin game");

	M_Print (0+8, 84, "      Max players");
	M_Print (160+8, 84, va("%i", maxplayers) );

	M_Print (0+8, 92, "        Game Type");
	if (coop.value)
		M_Print (160+8, 92, "Cooperative");
	else
		M_Print (160+8, 92, "Deathmatch");

	M_Print (0+8, 100, "        Teamplay");
	{
		char *msg;

		switch((int)teamplay.value)
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

	M_Print (0+8, 108, "            Class");
	M_Print (160+8, 108, ClassNames[setup_class]);

	M_Print (0+8, 116, "       Difficulty");

	M_Print (160+8, 116, DiffNames[setup_class][(int)skill.value]);

	M_Print (0+8, 124, "       Frag Limit");
	if (fraglimit.value == 0)
		M_Print (160+8, 124, "none");
	else
		M_Print (160+8, 124, va("%i frags", (int)fraglimit.value));

	M_Print (0+8, 132, "       Time Limit");
	if (timelimit.value == 0)
		M_Print (160+8, 132, "none");
	else
		M_Print (160+8, 132, va("%i minutes", (int)timelimit.value));

	M_Print (0+8, 140, "     Random Class");
	if (randomclass.value)
		M_Print (160+8, 140, "on");
	else
		M_Print (160+8, 140, "off");

	M_Print (0+8, 156, "         Episode");
	M_Print (160+8, 156, episodes[startepisode].description);

	M_Print (0+8, 164, "           Level");
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
	int count;

	switch (gameoptions_cursor)
	{
	case 1:
		maxplayers += dir;
		if (maxplayers > svs.maxclientslimit)
		{
			maxplayers = svs.maxclientslimit;
			m_serverInfoMessage = true;
			m_serverInfoMessageTime = realtime;
		}
		if (maxplayers < 2)
			maxplayers = 2;
		break;

	case 2:
		Cvar_SetValue ("coop", coop.value ? 0 : 1);
		if (coop.value)
		{
			startlevel = 0;
			if (startepisode == 1)
				startepisode = 0;
			else if (startepisode == DM_START)
				startepisode = REG_START;
		}
		break;

	case 3:
		count = 2;

		Cvar_SetValue ("teamplay", teamplay.value + dir);
		if (teamplay.value > count)
			Cvar_SetValue ("teamplay", 0);
		else if (teamplay.value < 0)
			Cvar_SetValue ("teamplay", count);
		break;

	case 4:
		setup_class += dir;
//		if ((!registered.value && !oem.value) && setup_class == 1)
//			setup_class = MAX_PLAYER_CLASS - 1;
//		if ((!registered.value && !oem.value) && setup_class == 2)
//			setup_class = 0;
		if (setup_class < 0) 
			setup_class = MAX_PLAYER_CLASS - 1;
		if (setup_class > MAX_PLAYER_CLASS - 1)
			setup_class = 0;
		break;

	case 5:
		Cvar_SetValue ("skill", skill.value + dir);
		if (skill.value > 3)
			Cvar_SetValue ("skill", 0);
		if (skill.value < 0)
			Cvar_SetValue ("skill", 3);
		break;

	case 6:
		Cvar_SetValue ("fraglimit", fraglimit.value + dir*10);
		if (fraglimit.value > 100)
			Cvar_SetValue ("fraglimit", 0);
		if (fraglimit.value < 0)
			Cvar_SetValue ("fraglimit", 100);
		break;

	case 7:
		Cvar_SetValue ("timelimit", timelimit.value + dir*5);
		if (timelimit.value > 60)
			Cvar_SetValue ("timelimit", 0);
		if (timelimit.value < 0)
			Cvar_SetValue ("timelimit", 60);
		break;

	case 8:
		if (randomclass.value)
			Cvar_SetValue ("randomclass", 0);
		else
			Cvar_SetValue ("randomclass", 1);
		break;

	case 9:
		startepisode += dir;

		if (registered.value)
		{
			count = DM_START;
			if (!coop.value)
				count++;

			if (startepisode < REG_START)
				startepisode = count - 1;

			if (startepisode >= count)
				startepisode = REG_START;

			startlevel = 0;
		}
		else if (oem.value)
		{
			count = 10;

			if (startepisode < 8)
				startepisode = count - 1;

			if (startepisode >= count)
				startepisode = 8;

			startlevel = 0;
		}
		else
		{
			count = 2;

			if (startepisode < 0)
				startepisode = count - 1;

			if (startepisode >= count)
				startepisode = 0;

			startlevel = 0;
		}
		break;

	case 10:
		if (coop.value)
		{
			startlevel = 0;
			break;
		}
		startlevel += dir;
		count = episodes[startepisode].levels;

		if (startlevel < 0)
			startlevel = count - 1;

		if (startlevel >= count)
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
			if (coop.value)
				gameoptions_cursor--;
		}
		break;

	case K_DOWNARROW:
		S_LocalSound ("raven/menu1.wav");
		gameoptions_cursor++;
		if (coop.value)
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
			Cbuf_AddText ( va ("maxplayers %u\n", maxplayers) );
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
	key_dest = key_menu;
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

	if(slistInProgress)
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
	key_dest = key_menu;
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
	char	string [64], *name;

	if (!slist_sorted)
	{
		if (hostCacheCount > 1)
		{
			int	i,j;
			hostcache_t temp;
			for (i = 0; i < hostCacheCount; i++)
				for (j = i+1; j < hostCacheCount; j++)
					if (strcmp(hostcache[j].name, hostcache[i].name) < 0)
					{
						memcpy(&temp, &hostcache[j], sizeof(hostcache_t));
						memcpy(&hostcache[j], &hostcache[i], sizeof(hostcache_t));
						memcpy(&hostcache[i], &temp, sizeof(hostcache_t));
					}
		}
		slist_sorted = true;
	}

	ScrollTitle("gfx/menu/title4.lmp");
	for (n = 0; n < hostCacheCount; n++)
	{
		if (hostcache[n].driver == 0)
			name = net_drivers[hostcache[n].driver].name;
		else
			name = net_landrivers[hostcache[n].ldriver].name;

		if (hostcache[n].maxusers)
			sprintf(string, "%-11.11s %-8.8s %-10.10s %2u/%2u\n", hostcache[n].name, name, hostcache[n].map, hostcache[n].users, hostcache[n].maxusers);
		else
			sprintf(string, "%-11.11s %-8.8s %-10.10s\n", hostcache[n].name, name, hostcache[n].map);
		M_Print (16, 60 + 8*n, string);
	}
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
		key_dest = key_game;
		m_state = m_none;
		Cbuf_AddText ( va ("connect \"%s\"\n", hostcache[slist_cursor].cname) );
		break;

	default:
		break;
	}
}


//=============================================================================
/* Menu Subsystem */

void M_Init (void)
{
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

	M_BuildBigCharWidth();
}


void M_Draw (void)
{
	if (m_state == m_none || key_dest != key_menu)
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

	if (IPXConfig || TCPIPConfig)
		net_hostport = lanConfig_port;
}

static void ReInitMusic (void)
{
	// called after exitting the menus and changing the music type
	// this is pretty crude, but doen't seem to break anything S.A

	if (Q_strcasecmp(bgmtype.string,"midi") == 0)
	{
		CDAudio_Stop();
		MIDI_Play(cl.midi_name);
	}

	if (Q_strcasecmp(bgmtype.string,"cd") == 0)
	{
		MIDI_Stop();
		CDAudio_Play ((byte)cl.cdtrack, true);
	}

	if (Q_strcasecmp(bgmtype.string,"none") == 0)
	{
		CDAudio_Stop();
		MIDI_Stop();
	}
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.60  2006/03/17 14:12:48  sezero
 * put back mission-pack only objectives stuff back into pure h2 builds.
 * it was a total screw-up...
 *
 * Revision 1.59  2006/03/13 22:28:51  sezero
 * removed the expansion pack only feature of objective strings from
 * hexen2-only builds (many new ifdef H2MP stuff). removed the expansion
 * pack only intermission picture and string searches from hexen2-only
 * builds.
 *
 * Revision 1.58  2006/03/13 22:25:22  sezero
 * properly macroized the fullscreen intermissions as a compile time
 * option. editing only one line in screen.h is now enough.
 *
 * Revision 1.57  2006/03/13 22:23:11  sezero
 * fixed a bug where with viewsize (scr_viewsize) being set to 120,
 * the game wouldn't start with a mini status bar unless the user did
 * a size-up/size-down.
 *
 * Revision 1.56  2006/03/10 10:59:38  sezero
 * added colored light opions to the opengl features menu.
 *
 * Revision 1.55  2006/02/24 19:23:16  sezero
 * got rid of the menu panels upon starting a new games through the menu system
 *
 * Revision 1.54  2006/02/24 14:43:55  sezero
 * created a new "opengl features" entry under the options menu and moved opengl
 * options under it. added new opengl menu options for texture filtering, glow
 * effects, multitexturing, stencil buffered shadows and texture purging upon
 * map change.
 *
 * Revision 1.53  2006/02/18 08:51:10  sezero
 * continue making static functions and vars static. whitespace and coding style
 * cleanup. also renamed the variables name and dest to savename and savedest in
 * host_cmd.c to prevent any confusion and pollution.
 *
 * Revision 1.52  2006/01/23 20:22:49  sezero
 * tidied up the version and help display stuff. bumped the HoT version to
 * 1.4.0-pre1. added conditionals to properly display beta version strings.
 *
 * Revision 1.51  2006/01/12 12:34:38  sezero
 * added video modes enumeration via SDL. added on-the-fly video mode changing
 * partially based on the Pa3PyX hexen2 tree. TODO: make the game remember its
 * video settings, clean it up, fix it up...
 *
 * Revision 1.50  2005/12/11 11:53:12  sezero
 * added menu.c arguments to gl version of Draw_TransPicTranslate, and
 * macroized M_DrawTransPicTranslate accordingly. this synchronizes h2
 * and h2w versions of gl_draw.c
 *
 * Revision 1.49  2005/12/04 11:19:18  sezero
 * gamma stuff update
 *
 * Revision 1.48  2005/10/25 20:08:41  sezero
 * coding style and whitespace cleanup.
 *
 * Revision 1.47  2005/10/25 20:04:17  sezero
 * static functions part-1: started making local functions static,
 * killing nested externs, const vars clean-up.
 *
 * Revision 1.46  2005/10/25 17:14:23  sezero
 * added a STRINGIFY macro. unified version macros. simplified version
 * printing. simplified and enhanced version watermark print onto console
 * background. added HoT lines to the quit menu (shameless plug)
 *
 * Revision 1.45  2005/10/13 15:26:10  sezero
 * M_ScanSaves() and M_ScanMSaves() should actually use com_savedir, not
 * com_userdir. By default, they are the same but this is for correctness.
 *
 * Revision 1.44  2005/09/28 06:07:32  sezero
 * renamed ToggleFullScreenSA to VID_ToggleFullscreen which
 * actually is of VID_ class and now is easier to locate
 *
 * Revision 1.43  2005/09/19 19:50:10  sezero
 * fixed those famous spelling errors
 *
 * Revision 1.42  2005/08/30 21:25:14  sezero
 * indentation fix for readablity and cosmetics
 *
 * Revision 1.41  2005/08/18 14:20:28  sezero
 * moved music volume update back into frame update. it has its merits (such as
 * correct bgmvolume on startup) this way..
 *
 * Revision 1.40  2005/08/02 18:06:58  sezero
 * removed lookspring and lookstrafe options and
 * +mlook and +klook key bindings from the menu
 *
 * Revision 1.39  2005/06/07 20:26:02  sezero
 * Draw help messages fullscreen for software version, as well
 *
 * Revision 1.38  2005/06/03 13:25:29  sezero
 * Latest mouse fixes and clean-ups
 *
 * Revision 1.37  2005/05/29 08:53:57  sezero
 * get rid of silly name changes
 *
 * Revision 1.36  2005/05/26 08:39:22  sezero
 * enabled mouse in whole of the options menu group
 *
 * Revision 1.35  2005/05/22 13:50:04  sezero
 * temporary fix for multiplayer episode selection.
 * next time we should get rid of these H2MP defines
 * and decide in runtime (com_portals from H2W?)
 *
 * Revision 1.34  2005/05/21 17:10:58  sezero
 * re-enabled complete disabling/enabling of mousa in fullscreen
 * mode. (only replaced a bunch of if 1's to if 0's)
 *
 * Revision 1.33  2005/05/21 17:04:16  sezero
 * - revived -nomouse that "disables mouse no matter what"
 * - renamed _windowed_mouse to _enable_mouse which is our intention,
 *   that is, dynamically disabling/enabling mouse while in game
 * - old code had many oversights/leftovers that prevented mouse being
 *   really disabled in fullscreen mode. fixed and cleaned-up here
 * - even in windowed mode, when mouse was disabled, mouse buttons and
 *   the wheel got processed. fixed it here.
 * - mouse cursor is never shown while the game is alive, regardless
 *   of mouse being enabled/disabled (I never liked an ugly pointer
 *   around while playing) Its only intention would be to be able to
 *   use the desktop, and for that see, the grab notes below
 * - if mouse is disabled, it is un-grabbed in windowed mode. Note: One
 *   can always use the keyboard shortcut CTRL-G for grabbing-ungrabbing
 *   the mouse regardless of mouse being enabled/disabled.
 * - ToggleFullScreenSA() used to update vid_mode but always forgot
 *   modestate. It now updates modestate as well.
 * - Now that IN_ActivateMouse() and IN_DeactivateMouse() are fixed,
 *   IN_ActivateMouseSA() and IN_DeactivateMouseSA() are redundant and
 *   are removed. BTW, I added a new qboolean mousestate_sa (hi Steve)
 *   which keeps track of whether we intentionally disabled the mouse.
 * - mouse disabling in fullscreen mode (in the absence of -nomouse
 *   arg) is not allowed in this patch, but this is done by a if 1/if 0
 *   conditional compilation. Next patch will change all those if 1 to
 *   if 0, and voila!, we can fully disable/enable mouse in fullscreen.
 * - moved modestate enums/defines to vid.h so that they can be used
 *   by other code properly.
 *
 * Revision 1.32  2005/05/21 16:27:27  sezero
 * removed net_serial which has been dead for ages
 *
 * Revision 1.31  2005/05/20 15:26:33  sezero
 * separated winquake.h into winquake.h and linquake.h
 * changed all occurances of winquake.h to quakeinc.h,
 * which includes the correct header
 *
 * Revision 1.30  2005/05/17 22:56:19  sezero
 * cleanup the "stricmp, strcmpi, strnicmp, Q_strcasecmp, Q_strncasecmp" mess:
 * Q_strXcasecmp will now be used throughout the code which are implementation
 * dependant defines for __GNUC__ (strXcasecmp) and _WIN32 (strXicmp)
 *
 * Revision 1.29  2005/05/17 06:48:54  sezero
 * initial cosmetic cleanup in menu.c
 *
 * Revision 1.28  2005/04/30 10:42:39  sezero
 * Updated gamma stuff: We don't use gamma ramps (in fact we can, we have
 * SDL_SetGammaRamp), therefore eliminated V_CheckGamma & friends from
 * V_UpdatePalette for PLATFORM_UNIX. Also killed unused function WarpPalette.
 *
 * Revision 1.27  2005/04/30 08:39:07  sezero
 * silenced shadowed decleration warnings about volume (now sfxvolume)
 *
 * Revision 1.26  2005/03/08 12:14:16  sezero
 * my silly oversight in reinit_music: fixes bug "midi
 * always restarts upon pressing Esc twice"
 *
 * Revision 1.25  2005/03/06 10:44:41  sezero
 * - move reinit_music to menu.c where it belongs
 * - fix reinit_music so that it works for the F4 key as well
 * - don't mess with music volume on every frame update, it's just silly
 *
 * Revision 1.24  2005/02/20 12:28:47  sezero
 * - old mission option is now always enabled, removed my cmdline thing,
 *   killed the cvar m_oldmission
 * - completely disabled demoness class for the old mission, killed
 *   my cvar m_demoness
 *
 * Revision 1.23  2004/12/29 19:49:40  sezero
 * From Steven (2004-12-29):
 * - Fullscreen/Windowed mode is now switchable. Seems to work good.
 * - Mouse can now be disabled for fullscreen modes.
 *
 * Revision 1.22  2004/12/28 22:33:17  sezero
 * fix the cvar m_oldmission again ;(
 *
 * Revision 1.21  2004/12/28 17:38:03  sezero
 * Add/enable the cmdline options -witholdmission and -noold
 *
 * Revision 1.20  2004/12/21 17:53:09  sezero
 * fix the GL_GLOWS entry once again...
 *
 * Revision 1.19  2004/12/21 16:15:58  sezero
 * - Do the right thing about the not-yet-enabled (not-to-be-enabled)
 *   --witholdmission and --withdemoness cmdline args.
 *
 * Revision 1.18  2004/12/21 16:07:05  sezero
 * - add Previous Weapon entry to the menu
 * - fix the GL_GLOWS entry. Make it affect gl_other_glows, too.
 *
 * Revision 1.17  2004/12/18 14:15:35  sezero
 * Clean-up and kill warnings 10:
 * Remove some already commented-out functions and code fragments.
 * They seem to be of no-future use. Also remove some unused functions.
 *
 * Revision 1.16  2004/12/18 14:08:07  sezero
 * Clean-up and kill warnings 9:
 * Kill many unused vars.
 *
 * Revision 1.15  2004/12/18 13:46:25  sezero
 * Clean-up and kill warnings 2:
 * Kill " suggest explicit braces to avoid ambiguous `else' " warnings
 *
 * Revision 1.14  2004/12/18 13:30:50  sezero
 * Hack to prevent textures going awol and some info-plaques start looking
 * white upon succesive load games. The solution is not beautiful but seems
 * to work for now. Adapted from Pa3PyX sources.
 *
 * Revision 1.13  2004/12/18 13:20:37  sezero
 * make the music automatically restart when changed in the options menu
 *
 * Revision 1.12  2004/12/13 14:52:23  sezero
 * prevent the m_demoness.value spam, do some defines
 *
 * Revision 1.11  2004/12/13 14:50:41  sezero
 * fix an oversight in oldmission menu drawing
 *
 * Revision 1.10  2004/12/12 23:16:44  sezero
 * two minor cvar annoyances
 *
 * Revision 1.9  2004/12/12 18:38:40  sezero
 * fix compilation. not a good day ;(
 *
 * Revision 1.8  2004/12/12 14:57:06  sezero
 * Back-out the Old Mission menu in PoP:
 *  It can be enabled with "m_oldmission 1" console command.
 * Disabled  playing the Old Mission with the Demoness class:
 *  It can be enabled with "m_demoness 1" console command.
 *
 * Revision 1.7  2004/12/12 14:38:18  sezero
 * steven fixed the mouse again ;)
 *
 * Revision 1.6  2004/12/12 14:14:42  sezero
 * style changes to our liking
 *
 * Revision 1.5  2004/12/05 10:52:18  sezero
 * Sync with Steven, 2004-12-04 :
 *  Fix the "Old Mission" menu PoP
 *  Also release the windowed mouse on pause
 *  Heapsize is now 32768 default
 *  The final splash screens now centre the messages properly
 *  Add more mods to the video mods table
 *  Add the docs folder and update it
 *
 * Revision 1.4  2004/11/29 12:17:46  sezero
 * draw fullscreen intermission pics. borrowed from Pa3PyX sources.
 *
 * Revision 1.3  2004/11/28 00:58:08  sezero
 *
 * Commit Steven's changes as of 2004.11.24:
 *
 * * Rewritten Help/Version message(s)
 * * Proper fullscreen mode(s) for OpenGL.
 * * Screen sizes are selectable with "-width" and "-height" options.
 * * Mouse grab in window modes , which is released when menus appear.
 * * Interactive video modes in software game disabled.
 * * Replaced Video Mode menu with a helpful message.
 * * New menu items for GL Glow, Chase mode, Draw Shadows.
 * * Changes to initial cvar_t variables:
 *      r_shadows, gl_other_glows, _windowed_mouse,
 *
 * Revision 1.2  2004/11/28 00:45:18  sezero
 * activate the Old Mission menu option in the mission pack for now.
 *
 * Revision 1.1.1.1  2004/11/28 00:05:05  sezero
 * Initial import of AoT 1.2.0 code
 *
 * Revision 1.5  2002/01/04 20:39:55  phneutre
 * fixed SINGLEPLAYER_ITEMS value (3 instead of 4) when PoP is not installed
 *
 * Revision 1.4  2001/12/13 20:04:43  phneutre
 * search, load and write savegames in ~/.aot/s* instead of data1/s*
 *
 * Revision 1.3  2001/12/13 13:10:04  phneutre
 * enable gamma slider in Options menu
 *
 * Revision 1.2  2001/12/03 21:17:15  theoddone33
 * Remove vain crap
 *
 * 41    3/20/98 2:03p Jmonroe
 * changed default to not allow old missions.
 * 
 * 40    3/19/98 1:21p Jmonroe
 * 
 * 39    3/19/98 12:58a Jmonroe
 * 
 * 38    3/18/98 11:34p Jmonroe
 * fixed gl renderheight in intermission, fixed bottom plaque draw, added
 * credit cd track
 * 
 * 37    3/18/98 1:09p Jmonroe
 * 
 * 36    3/17/98 8:11p Jmonroe
 * 
 * 35    3/17/98 4:22p Jmonroe
 * 
 * 34    3/17/98 4:15p Jmonroe
 * 
 * 33    3/17/98 11:51a Jmonroe
 * 
 * 32    3/16/98 5:33p Jweier
 * 
 * 31    3/16/98 3:52p Jmonroe
 * fixed info_masks for load/save changelevel
 * 
 * 30    3/16/98 12:01a Jweier
 * 
 * 29    3/15/98 10:33p Jweier
 * 
 * 28    3/14/98 5:39p Jmonroe
 * made info_plaque draw safe, fixed bit checking
 * 
 * 27    3/14/98 12:50p Jmonroe
 * 
 * 26    3/13/98 6:25p Jmonroe
 * 
 * 25    3/13/98 12:22p Jweier
 * 
 * 24    3/13/98 12:19a Jmonroe
 * fixed lookspring
 * 
 * 23    3/11/98 7:12p Mgummelt
 * 
 * 22    3/11/98 6:20p Mgummelt
 * 
 * 21    3/10/98 11:38a Jweier
 * 
 * 20    3/04/98 11:42a Jmonroe
 * 
 * 19    3/03/98 1:41p Jmonroe
 * removed old mp stuff
 * 
 * 18    3/02/98 11:04p Jmonroe
 * changed start sound back to byte, added stopsound, put in a hack fix
 * for touchtriggers area getting removed
 * 
 * 17    3/02/98 2:22p Jmonroe
 * ADDED map names in dm start menu 
 * 
 * 16    3/01/98 8:20p Jmonroe
 * removed the slow "quake" version of common functions
 * 
 * 15    2/27/98 11:53p Jweier
 * 
 * 14    2/26/98 3:09p Jmonroe
 * fixed gl for numclasses
 * 
 * 13    2/23/98 9:18p Jmonroe
 * 
 * 12    2/23/98 4:54p Jmonroe
 * added show objectives to customize controls
 * 
 * 11    2/23/98 2:54p Jmonroe
 * added crosshair and mlook to the option menu
 * added start mission pack to the single play menu
 * 
 * 10    2/19/98 3:32p Jweier
 * 
 * 9     2/08/98 6:08p Mgummelt
 * 
 * 8     2/08/98 6:07p Jweier
 * 
 * 7     1/22/98 9:06p Jmonroe
 * final map names  , no descriptions yet
 * 
 * 6     1/21/98 10:29a Plipo
 * 
 * 5     1/18/98 8:06p Jmonroe
 * all of rick's patch code is in now
 * 
 * 4     1/15/98 10:04p Jmonroe
 * added stub mpack menu stuff
 * 
 * 91    10/29/97 5:39p Jheitzman
 * 
 * 90    10/28/97 6:26p Jheitzman
 * 
 * 89    10/28/97 2:58p Jheitzman
 * 
 * 87    10/06/97 6:04p Rjohnson
 * Fix for save games and version update
 * 
 * 86    9/25/97 11:56p Rjohnson
 * Version update
 * 
 * 85    9/25/97 11:47a Rjohnson
 * Update
 * 
 * 84    9/23/97 8:56p Rjohnson
 * Updates
 * 
 * 83    9/15/97 11:15a Rjohnson
 * Updates
 * 
 * 82    9/09/97 10:49a Rjohnson
 * Updates
 * 
 * 81    9/04/97 5:33p Rjohnson
 * Id updates
 * 
 * 80    9/04/97 4:44p Rjohnson
 * Updates
 * 
 * 79    9/02/97 12:25a Rjohnson
 * Font Update
 * 
 * 78    9/01/97 2:08a Rjohnson
 * Updates
 * 
 * 77    8/31/97 9:27p Rjohnson
 * GL Updates
 * 
 * 76    8/31/97 3:45p Rjohnson
 * Map Updates
 * 
 * 75    8/30/97 6:16p Rjohnson
 * Centering text
 * 
 * 74    8/28/97 3:36p Rjohnson
 * Version Update
 * 
 * 73    8/27/97 12:11p Rjohnson
 * Adjustments
 * 
 * 72    8/26/97 8:58p Rjohnson
 * Credit Update
 * 
 * 71    8/26/97 8:51p Rjohnson
 * Credit Changes
 * 
 * 70    8/26/97 8:17a Rjohnson
 * Just a few changes
 * 
 * 69    8/25/97 11:40a Rjohnson
 * Activision People
 * 
 * 68    8/25/97 11:23a Rjohnson
 * name Update
 * 
 * 67    8/24/97 11:45a Rjohnson
 * Gary change
 * 
 * 66    8/24/97 11:06a Rjohnson
 * Fix
 * 
 * 65    8/24/97 10:56a Rjohnson
 * Bob Update
 * 
 * 64    8/24/97 10:52a Rjohnson
 * Updates
 * 
 * 63    8/21/97 10:12p Rjohnson
 * Version Update
 * 
 * 62    8/21/97 2:11p Rjohnson
 * Menu Update
 * 
 * 61    8/21/97 11:45a Rjohnson
 * Fix for menu
 * 
 * 60    8/21/97 11:42a Rjohnson
 * Credit Text
 * 
 * 59    8/21/97 1:09a Rjohnson
 * Credits Update
 * 
 * 58    8/20/97 8:30p Rlove
 * 
 * 57    8/20/97 8:25p Rjohnson
 * Added player class to net menus
 * 
 * 56    8/20/97 5:07p Rjohnson
 * Added a toggle for the quick frag list
 * 
 * 55    8/20/97 4:58p Rjohnson
 * Difficulty name change
 * 
 * 54    8/20/97 3:13p Rlove
 * 
 * 53    8/20/97 2:05p Rjohnson
 * fix for internationalization
 * 
 * 52    8/20/97 11:09a Rjohnson
 * Fix for console font
 * 
 * 51    8/19/97 11:59p Rjohnson
 * Difficulty levels based on class
 * 
 * 50    8/19/97 7:45p Rjohnson
 * Fix for menu
 * 
 * 49    8/18/97 11:44p Rjohnson
 * Fixes for loading
 * 
 * 48    8/18/97 4:47p Rjohnson
 * Difficulty name update
 * 
 * 47    8/18/97 2:16p Rjohnson
 * Fix for gl version
 * 
 * 46    8/18/97 2:06p Rjohnson
 * Difficulty Names
 * 
 * 45    8/17/97 6:27p Rjohnson
 * Added frag and inventory keys to menu
 * 
 * 44    8/17/97 3:28p Rjohnson
 * Fix for color selection
 * 
 * 43    8/16/97 10:52a Rjohnson
 * Level Update
 * 
 * 42    8/15/97 3:10p Rjohnson
 * Precache Update
 * 
 * 41    8/15/97 1:58p Rlove
 * 
 * 40    8/15/97 6:32a Rlove
 * Added a few more keys to keyboard config screen
 * 
 * 39    8/14/97 11:59p Rjohnson
 * Fix
 * 
 * 38    8/14/97 11:45p Rjohnson
 * Connection type info
 * 
 * 37    8/14/97 10:14p Rjohnson
 * Menu Updates
 * 
 * 36    8/14/97 2:37p Rjohnson
 * Fix for save games
 * 
 * 35    8/14/97 12:27p Rlove
 * 
 * 34    8/14/97 9:22a Rlove
 * Added portraits to Player Class Menu
 * 
 * 33    8/12/97 8:17p Rjohnson
 * Change for menu
 * 
 * 32    8/04/97 3:51p Rjohnson
 * Fix for the menus
 * 
 * 31    8/01/97 3:17p Rjohnson
 * Added new menu graphics
 * 
 * 30    7/18/97 4:37p Rjohnson
 * Added deathmatch
 * 
 * 29    7/15/97 5:52p Rjohnson
 * Added help screens
 * 
 * 28    7/15/97 2:21p Rjohnson
 * Added Difficulty graphic
 * 
 * 27    7/14/97 2:52p Rjohnson
 * Demo stuff
 * 
 * 26    7/11/97 5:59p Rjohnson
 * Got menus working for single player
 * 
 * 25    7/11/97 5:21p Rjohnson
 * RJNET Updates
 * 
 * 24    6/17/97 10:28a Bgokey
 * 
 * 23    4/22/97 5:19p Rjohnson
 * More menu updates
 * 
 * 22    4/18/97 6:24p Rjohnson
 * More menu mods
 * 
 * 21    4/18/97 12:24p Rjohnson
 * Added scrolly hexen plaque
 * 
 * 20    4/18/97 12:10p Rjohnson
 * Added josh to the credits
 * 
 * 19    4/18/97 11:25a Rjohnson
 * Changed the quit message
 * 
 * 18    4/17/97 3:42p Rjohnson
 * Modifications for the gl version for menus
 * 
 * 17    4/17/97 12:25p Rjohnson
 * Modifications for the gl version
 * 
 * 16    4/15/97 6:40p Rjohnson
 * Position update of the hexen plaque
 * 
 * 15    4/15/97 5:58p Rjohnson
 * More menu bits
 * 
 * 14    4/14/97 5:02p Rjohnson
 * More menu refinement
 * 
 * 13    4/14/97 12:17p Rjohnson
 * Menus now use new font
 * 
 * 12    4/08/97 11:23a Rjohnson
 * Modified the menu sounds
 * 
 * 11    4/01/97 4:38p Rjohnson
 * Modification to fix a crash in the network options
 * 
 * 10    3/31/97 7:24p Rjohnson
 * Added a playerclass field and made sure the server/clients handle it
 * properly
 * 
 * 9     3/31/97 4:09p Rlove
 * Removing references to Quake
 * 
 * 8     3/14/97 9:21a Rlove
 * Plaques are done 
 * 
 * 7     2/20/97 12:13p Rjohnson
 * Code fixes for id update
 * 
 * 6     2/19/97 12:29p Rjohnson
 * More Id Updates
 * 
 * 5     2/18/97 12:17p Rjohnson
 * Id Updates
 * 
 * 4     1/02/97 1:33p Rlove
 * Class and Difficulty Menus added
 * 
 */
