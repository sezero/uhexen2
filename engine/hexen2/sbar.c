/*
 * sbar.c -- Hexen II status bar
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
#include "r_shared.h"

// MACROS ------------------------------------------------------------------

#define	STAT_MINUS			10	/* num frame for '-' stats digit */

#define BAR_TOP_HEIGHT			46.0
#define BAR_BOTTOM_HEIGHT		98.0
#define BAR_TOTAL_HEIGHT		(BAR_TOP_HEIGHT+BAR_BOTTOM_HEIGHT)
#define BAR_BUMP_HEIGHT			23.0
#define INVENTORY_DISPLAY_TIME		4

#define RING_FLIGHT			1
#define RING_WATER			2
#define RING_REGENERATION		4
#define RING_TURNING			8

/* Max number of inventory icons to display at once */
#define	INV_MAX_ICON			6

/* artifact counts are relative to the cnt_torch in entvars_t struct: */
#define Inv_GetCount(artifact_num)	(int)(&cl.v.cnt_torch)[(artifact_num)]

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void Sbar_DeathmatchOverlay(void);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void Sbar_PuzzlePieceOverlay(void);
static void Sbar_SmallDeathmatchOverlay(void);

static void Sbar_DrawPic(int x, int y, qpic_t *pic);
static void Sbar_DrawTransPic(int x, int y, qpic_t *pic);
static int Sbar_itoa(int num, char *buf);
static void Sbar_DrawNum(int x, int y, int number, int digits);
static void Sbar_SortFrags(void);
#if 0	/* all uses are commented out */
static void Sbar_DrawCharacter(int x, int y, int num);
static void Sbar_DrawSmallCharacter(int x, int y, int num);
static void Sbar_DrawString(int x, int y, const char *str);
#endif
static void Sbar_DrawSmallString(int x, int y, const char *str);
static void DrawBarArtifactNumber(int x, int y, int number);

static void DrawFullScreenInfo(void);
static void DrawLowerBar(void);
static void DrawActiveRings(void);
static void DrawActiveArtifacts(void);
static int CalcAC(void);
static void DrawBarArtifactIcon(int x, int y, int artifact);

static qboolean SetChainPosition(float health, float maxHealth, qboolean immediate);

static void ShowInfoDown_f(void);
static void ShowInfoUp_f(void);
static void ShowDMDown_f(void);
static void ShowDMUp_f(void);
static void ToggleDM_f(void);
static void InvLeft_f(void);
static void InvRight_f(void);
static void InvUse_f(void);
static void InvOff_f(void);

static void DrawArtifactInventory(void);

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int	sb_lines;	// scan lines to draw

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int sb_updates; // if >= vid.numpages, no update needed

static float BarHeight;
static float BarTargetHeight;
static cvar_t BarSpeed = {"barspeed", "5", CVAR_NONE};
static cvar_t DMMode = {"dm_mode", "1", CVAR_ARCHIVE};
static cvar_t sbtrans = {"sbtrans", "0", CVAR_ARCHIVE};
static cvar_t dmtrans = {"dmtrans", "0", CVAR_ARCHIVE};

static qpic_t *sb_nums[11];
static qpic_t *sb_colon, *sb_slash;

static int fragsort[MAX_CLIENTS];

static int scoreboardlines;

static float ChainPosition = 0;

static qboolean sb_ShowInfo;
static qboolean sb_ShowDM;

static qboolean inv_flg;	// true - show inventory interface

static float InventoryHideTime;

extern const char *ClassNames[MAX_PLAYER_CLASS];	//from menu.c

static const int AmuletAC[MAX_PLAYER_CLASS] =
{
	8,		// Paladin
	4,		// Crusader
	2,		// Necromancer
	6,		// Assassin
	6		// Demoness
};

static const int BracerAC[MAX_PLAYER_CLASS] =
{
	6,		// Paladin
	8,		// Crusader
	4,		// Necromancer
	2,		// Assassin
	2		// Demoness
};

static const int BreastplateAC[MAX_PLAYER_CLASS] =
{
	2,		// Paladin
	6,		// Crusader
	8,		// Necromancer
	4,		// Assassin
	4		// Demoness
};

static const int HelmetAC[MAX_PLAYER_CLASS] =
{
	4,		// Paladin
	2,		// Crusader
	6,		// Necromancer
	8,		// Assassin
	8		// Demoness
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
// Sbar_Init
//
//==========================================================================

void Sbar_Init(void)
{
	int i;

	for (i = 0; i < 10; i++)
	{
		sb_nums[i] = Draw_PicFromWad(va("num_%i",i));
	}
	sb_nums[10] = Draw_PicFromWad("num_minus");
	sb_colon = Draw_PicFromWad("num_colon");
	sb_slash = Draw_PicFromWad("num_slash");

	if (draw_reinit)
		return;

	Cmd_AddCommand("+showinfo", ShowInfoDown_f);
	Cmd_AddCommand("-showinfo", ShowInfoUp_f);
	Cmd_AddCommand("+showdm", ShowDMDown_f);
	Cmd_AddCommand("-showdm", ShowDMUp_f);
	Cmd_AddCommand("invleft", InvLeft_f);
	Cmd_AddCommand("invright", InvRight_f);
	Cmd_AddCommand("invuse", InvUse_f);
	Cmd_AddCommand("invoff", InvOff_f);
	Cmd_AddCommand("toggle_dm", ToggleDM_f);

	Cvar_RegisterVariable(&DMMode);
	Cvar_RegisterVariable(&sbtrans);
	Cvar_RegisterVariable(&dmtrans);
	Cvar_RegisterVariable(&BarSpeed);

	BarHeight = BarTargetHeight = BAR_TOP_HEIGHT;
}

//==========================================================================
//
// Sbar_Draw
//
//==========================================================================

void Sbar_Draw(void)
{
	float	delta;
	char	tempStr[80];
	int	mana, maxMana;

	if (intro_playing)		// the mission pack intro is active
	{
		scr_fullupdate = 0;
		scr_copyeverything = 1;
		return;
	}

	if (scr_con_current == vid.height)	// console is full screen
		return;

	DrawArtifactInventory();

	DrawActiveRings();
	DrawActiveArtifacts();

	trans_level = 0;

	if (sb_ShowDM)
	{
		if (cl.gametype == GAME_DEATHMATCH)
			Sbar_DeathmatchOverlay();
		else
			Sbar_PuzzlePieceOverlay();
	}
	else if (cl.gametype == GAME_DEATHMATCH && DMMode.integer)
	{
		Sbar_SmallDeathmatchOverlay();
	}

	trans_level = sbtrans.integer;
	if (trans_level < 0 || trans_level > 2)
		trans_level = 0;

	if (BarHeight < BarTargetHeight)
	{
		delta = ((BarTargetHeight-BarHeight)*BarSpeed.value) * host_frametime;
		if (delta < 0.5)
			delta = 0.5;
		BarHeight += delta;
		if (BarHeight > BarTargetHeight)
			BarHeight = BarTargetHeight;
		if (scr_viewsize.integer < 100)
			scr_fullupdate = 0;
	}
	else if (BarHeight > BarTargetHeight)
	{
		delta = ((BarHeight-BarTargetHeight)*BarSpeed.value) * host_frametime;
		if (delta < 0.5)
			delta = 0.5;
		BarHeight -= delta;
		if (BarHeight < BarTargetHeight)
			BarHeight = BarTargetHeight;
		if (scr_viewsize.integer < 100)
			scr_fullupdate = 0;
	}

	Sbar_DrawTransPic(0, -23, Draw_CachePic("gfx/topbumpl.lmp"));
	Sbar_DrawTransPic(138, -8, Draw_CachePic("gfx/topbumpm.lmp"));
	Sbar_DrawTransPic(269, -23, Draw_CachePic("gfx/topbumpr.lmp"));

	if (BarHeight > BAR_TOP_HEIGHT || m_state != m_none || scr_viewsize.integer > 100)
		sb_updates = 0;

#ifndef GLQUAKE
	if (sb_updates >= vid.numpages)
		return;
#endif

//	if (BarHeight == BarTargetHeight)
//		return;

	scr_copyeverything = 1;
	sb_updates++;

	if (BarHeight < 0 && scr_viewsize.integer <= 120)
	{
		SetChainPosition(cl.v.health, cl.v.max_health, true);
		DrawFullScreenInfo();
		return;
	}

	Sbar_DrawPic(0, 0, Draw_CachePic("gfx/topbar1.lmp"));
	Sbar_DrawPic(160, 0, Draw_CachePic("gfx/topbar2.lmp"));
	//Sbar_DrawTransPic(0, -23, Draw_CachePic("gfx/topbumpl.lmp"));
	//Sbar_DrawTransPic(138, -8, Draw_CachePic("gfx/topbumpm.lmp"));
	//Sbar_DrawTransPic(269, -23, Draw_CachePic("gfx/topbumpr.lmp"));

	maxMana = (int)cl.v.max_mana;
	// Blue mana
	mana = (int)cl.v.bluemana;
	if (mana < 0)
		mana = 0;
	else if (mana > maxMana)
		mana = maxMana;
	sprintf(tempStr, "%03d", mana);
	Sbar_DrawSmallString(201, 22, tempStr);
	if (mana)
	{
		Sbar_DrawPic(190, 26-(int)((mana*18.0)/(float)maxMana+0.5), Draw_CachePic("gfx/bmana.lmp"));
		Sbar_DrawPic(190, 27, Draw_CachePic("gfx/bmanacov.lmp"));
	}

	// Green mana
	mana = (int)cl.v.greenmana;
	if (mana < 0)
		mana = 0;
	else if (mana > maxMana)
		mana = maxMana;
	sprintf(tempStr, "%03d", mana);
	Sbar_DrawSmallString(243, 22, tempStr);
	if (mana)
	{
		Sbar_DrawPic(232, 26-(int)((mana*18.0)/(float)maxMana+0.5), Draw_CachePic("gfx/gmana.lmp"));
		Sbar_DrawPic(232, 27, Draw_CachePic("gfx/gmanacov.lmp"));
	}

	// HP
	if (cl.v.health < -99)
		Sbar_DrawNum(58, 14, -99, 3);
	else
		Sbar_DrawNum(58, 14, cl.v.health, 3);
	if (SetChainPosition(cl.v.health, cl.v.max_health, false))
		sb_updates = 0;
	Sbar_DrawTransPic(45+((int)ChainPosition&7), 38, Draw_CachePic("gfx/hpchain.lmp"));
	Sbar_DrawTransPic(45+(int)ChainPosition, 36, Draw_CachePic("gfx/hpgem.lmp"));
	Sbar_DrawPic(43, 36, Draw_CachePic("gfx/chnlcov.lmp"));
	Sbar_DrawPic(267, 36, Draw_CachePic("gfx/chnrcov.lmp"));

	// AC
	Sbar_DrawNum(105, 14, CalcAC(), 2);

	if (BarHeight > BAR_TOP_HEIGHT)
	{
		DrawLowerBar();
	}

	// Current inventory item
	if (cl.inv_selected >= 0)
	{
		DrawBarArtifactIcon(144, 3, cl.inv_order[cl.inv_selected]);
	//	Sbar_DrawTransPic(144, 3, Draw_CachePic(va("gfx/arti%02d.lmp", cl.inv_order[cl.inv_selected])));
	}
}

//==========================================================================
//
// SetChainPosition
//
//==========================================================================

static qboolean SetChainPosition(float health, float maxHealth, qboolean immediate)
{
	float delta;
	float chainTargetPosition;

	if (health < 0)
		health = 0;
	else if (health > maxHealth)
		health = maxHealth;
	chainTargetPosition = (health*195)/maxHealth;
	if (fabs(ChainPosition-chainTargetPosition) < 0.1)
		return false;
	if (immediate)
		ChainPosition = chainTargetPosition;
	if (ChainPosition < chainTargetPosition)
	{
		delta = ((chainTargetPosition-ChainPosition)*5)*host_frametime;
		if (delta < 0.5)
			delta = 0.5;
		ChainPosition += delta;
		if (ChainPosition > chainTargetPosition)
			ChainPosition = chainTargetPosition;
	}
	else if (ChainPosition > chainTargetPosition)
	{
		delta = ((ChainPosition-chainTargetPosition)*5)*host_frametime;
		if (delta < 0.5)
			delta = 0.5;
		ChainPosition -= delta;
		if (ChainPosition < chainTargetPosition)
			ChainPosition = chainTargetPosition;
	}
	return true;
}

//==========================================================================
//
// DrawFullScreenInfo
//
//==========================================================================

static void DrawFullScreenInfo(void)
{
	int		y;
	int	mana, maxMana;
	char	tempStr[80];

	y = BarHeight-37;
	Sbar_DrawPic(3, y, Draw_CachePic("gfx/bmmana.lmp"));
	Sbar_DrawPic(3, y+18, Draw_CachePic("gfx/gmmana.lmp"));

	maxMana = (int)cl.v.max_mana;
	// Blue mana
	mana = (int)cl.v.bluemana;
	if (mana < 0)
		mana = 0;
	else if (mana > maxMana)
		mana = maxMana;
	sprintf(tempStr, "%03d", mana);
	Sbar_DrawSmallString(10, y+6, tempStr);

	// Green mana
	mana = (int)cl.v.greenmana;
	if (mana < 0)
		mana = 0;
	else if (mana > maxMana)
		mana = maxMana;
	sprintf(tempStr, "%03d", mana);
	Sbar_DrawSmallString(10, y+18+6, tempStr);

	// HP
	Sbar_DrawNum(38, y+18, cl.v.health, 3);

	// Current inventory item
	if (cl.inv_selected >= 0)
	{
		DrawBarArtifactIcon(288, y+7, cl.inv_order[cl.inv_selected]);
	}
}

//==========================================================================
//
// DrawLowerBar
//
//==========================================================================

static void DrawLowerBar(void)
{
	int	i;
//	int	minutes, seconds, tens, units;
	char	tempStr[80];
	int	playerClass;
	int	piece;
	int	ringhealth;

	//playerClass = cl.v.playerclass;
	playerClass = cl_playerclass.integer;
	if (playerClass < 1 || playerClass > MAX_PLAYER_CLASS)
		playerClass = 1;	// Default to paladin
	if (!(gameflags & GAME_PORTALS) && playerClass > MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES)
		playerClass = 1;	// Default to paladin

	// Backdrop
	Sbar_DrawPic(0, 46, Draw_CachePic("gfx/btmbar1.lmp"));
	Sbar_DrawPic(160, 46, Draw_CachePic("gfx/btmbar2.lmp"));

	/*
	// Game time
	minutes = cl.time / 60;
	seconds = cl.time - 60*minutes;
	tens = seconds / 10;
	units = seconds - 10*tens;
	sprintf(tempStr, "Time :%3i:%i%i", minutes, tens, units);
	Sbar_DrawSmallString(116, 114, tempStr);

	// Map name
	Sbar_DrawSmallString(10, 114, cl.levelname);
	*/

	// Stats
	Sbar_DrawSmallString(11, 48, ClassNames[playerClass-1]);

	Sbar_DrawSmallString(11, 58, "int");
	sprintf(tempStr, "%02d", (int)cl.v.intelligence);
	Sbar_DrawSmallString(33, 58, tempStr);

	Sbar_DrawSmallString(11, 64, "wis");
	sprintf(tempStr, "%02d", (int)cl.v.wisdom);
	Sbar_DrawSmallString(33, 64, tempStr);

	Sbar_DrawSmallString(11, 70, "dex");
	sprintf(tempStr, "%02d", (int)cl.v.dexterity);
	Sbar_DrawSmallString(33, 70, tempStr);

	Sbar_DrawSmallString(58, 58, "str");
	sprintf(tempStr, "%02d", (int)cl.v.strength);
	Sbar_DrawSmallString(80, 58, tempStr);

	Sbar_DrawSmallString(58, 64, "lvl");
	sprintf(tempStr, "%02d", (int)cl.v.level);
	Sbar_DrawSmallString(80, 64, tempStr);

	Sbar_DrawSmallString(58, 70, "exp");
	sprintf(tempStr, "%06d", (int)cl.v.experience);
	Sbar_DrawSmallString(80, 70, tempStr);

	// Abilities
	Sbar_DrawSmallString(11, 79, "abilities");
	i = ABILITIES_STR_INDEX + (playerClass - 1)*2;
	if (i + 1 < host_string_count)
	{
		if (((int)cl.v.flags) & FL_SPECIAL_ABILITY1)
		{
			Sbar_DrawSmallString(8, 89, Host_GetString(i));
		}
		if (((int)cl.v.flags) & FL_SPECIAL_ABILITY2)
		{
			Sbar_DrawSmallString(8, 96, Host_GetString(i + 1));
		}
	}

	// Portrait
	sprintf(tempStr, "gfx/cport%d.lmp", playerClass);
	Sbar_DrawPic(134, 50, Draw_CachePic(tempStr));

	// Armor
	if (cl.v.armor_helmet > 0)
	{
		Sbar_DrawPic(164, 115, Draw_CachePic("gfx/armor1.lmp"));
		sprintf(tempStr, "+%d", (int)cl.v.armor_helmet);
		Sbar_DrawSmallString(168, 136, tempStr);
	}
	if (cl.v.armor_amulet > 0)
	{
		Sbar_DrawPic(205, 115, Draw_CachePic("gfx/armor2.lmp"));
		sprintf(tempStr, "+%d", (int)cl.v.armor_amulet);
		Sbar_DrawSmallString(208, 136, tempStr);
	}
	if (cl.v.armor_breastplate > 0)
	{
		Sbar_DrawPic(246, 115, Draw_CachePic("gfx/armor3.lmp"));
		sprintf(tempStr, "+%d", (int)cl.v.armor_breastplate);
		Sbar_DrawSmallString(249, 136, tempStr);
	}
	if (cl.v.armor_bracer > 0)
	{
		Sbar_DrawPic(285, 115, Draw_CachePic("gfx/armor4.lmp"));
		sprintf(tempStr, "+%d", (int)cl.v.armor_bracer);
		Sbar_DrawSmallString(288, 136, tempStr);
	}

	// Rings
	if (cl.v.ring_flight > 0)
	{
		Sbar_DrawTransPic(6, 119, Draw_CachePic("gfx/ring_f.lmp"));

		ringhealth = (int)cl.v.ring_flight;
		if (ringhealth > 100)
			ringhealth = 100;
		Sbar_DrawPic(35 - (int)(26 * (ringhealth/(float)100)), 142, Draw_CachePic("gfx/ringhlth.lmp"));
		Sbar_DrawPic(35, 142, Draw_CachePic("gfx/rhlthcvr.lmp"));
	}

	if (cl.v.ring_water > 0)
	{
		Sbar_DrawTransPic(44, 119, Draw_CachePic("gfx/ring_w.lmp"));
		ringhealth = (int)cl.v.ring_water;
		if (ringhealth > 100)
			ringhealth = 100;
		Sbar_DrawPic(73 - (int)(26 * (ringhealth/(float)100)), 142, Draw_CachePic("gfx/ringhlth.lmp"));
		Sbar_DrawPic(73, 142, Draw_CachePic("gfx/rhlthcvr.lmp"));
	}

	if (cl.v.ring_turning > 0)
	{
		Sbar_DrawTransPic(81, 119, Draw_CachePic("gfx/ring_t.lmp"));
		ringhealth = (int)cl.v.ring_turning;
		if (ringhealth > 100)
			ringhealth = 100;
		Sbar_DrawPic(110 - (int)(26 * (ringhealth/(float)100)), 142, Draw_CachePic("gfx/ringhlth.lmp"));
		Sbar_DrawPic(110, 142, Draw_CachePic("gfx/rhlthcvr.lmp"));
	}

	if (cl.v.ring_regeneration > 0)
	{
		Sbar_DrawTransPic(119, 119, Draw_CachePic("gfx/ring_r.lmp"));
		ringhealth = (int)cl.v.ring_regeneration;
		if (ringhealth > 100)
			ringhealth = 100;
		Sbar_DrawPic(148 -(int)(26 * (ringhealth/(float)100)), 142, Draw_CachePic("gfx/ringhlth.lmp"));
		Sbar_DrawPic(148, 142, Draw_CachePic("gfx/rhlthcv2.lmp"));
	}

	// Puzzle pieces
	piece = 0;
	for (i = 0; i < 8; i++)
	{
		if (cl.puzzle_pieces[i][0] == 0)
			continue;
		Sbar_DrawPic(194+(piece%4)*31, piece < 4 ? 51 : 82, Draw_CachePic(va("gfx/puzzle/%s.lmp", cl.puzzle_pieces[i])));
		piece++;
	}
}

//==========================================================================
//
// CalcAC
//
//==========================================================================

static int CalcAC(void)
{
	int	a;
	int	playerClass;

	//playerClass = cl.v.playerclass;
	playerClass = cl_playerclass.integer -1;
	if (playerClass < 0 || playerClass >= MAX_PLAYER_CLASS)
		playerClass = 1;	// Default to paladin
	if (!(gameflags & GAME_PORTALS) && playerClass > MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES)
		playerClass = 1;	// Default to paladin

	a = 0;
	if (cl.v.armor_amulet > 0)
	{
		a += AmuletAC[playerClass];
		a += cl.v.armor_amulet/5;
	}
	if (cl.v.armor_bracer > 0)
	{
		a += BracerAC[playerClass];
		a += cl.v.armor_bracer/5;
	}
	if (cl.v.armor_breastplate > 0)
	{
		a += BreastplateAC[playerClass];
		a += cl.v.armor_breastplate/5;
	}
	if (cl.v.armor_helmet > 0)
	{
		a += HelmetAC[playerClass];
		a += cl.v.armor_helmet/5;
	}
	return a;
}

//==========================================================================
//
// Sbar_Changed
//
//==========================================================================

void Sbar_Changed(void)
{
	sb_updates = 0;	// Update next frame
}

//==========================================================================
//
// Sbar_itoa
//
//==========================================================================

static int Sbar_itoa(int num, char *buf)
{
	char	*str;
	int		pow10;
	int		dig;

	str = buf;
	if (num < 0)
	{
		*str++ = '-';
		num = -num;
	}

	for (pow10 = 10; num >= pow10; pow10 *= 10)
		;

	do
	{
		pow10 /= 10;
		dig = num / pow10;
		*str++ = '0' + dig;
		num -= dig * pow10;
	} while (pow10 != 1);

	*str = 0;

	return str-buf;
}

//==========================================================================
//
// Sbar_DrawNum
//
//==========================================================================

static void Sbar_DrawNum(int x, int y, int number, int digits)
{
	char	str[12];
	char	*ptr;
	int		l, frame;

	l = Sbar_itoa(number, str);
	ptr = str;
	if (l > digits)
	{
		ptr += (l-digits);
	}
	if (l < digits)
	{
		x += ((digits-l)*13)/2;
	}

	while (*ptr)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';
		Sbar_DrawTransPic(x, y, sb_nums[frame]);
		x += 13;
		ptr++;
	}
}

//==========================================================================
//
// Sbar_SortFrags
//
//==========================================================================

static void Sbar_SortFrags(void)
{
	int	i, j, k;

	// sort by frags
	scoreboardlines = 0;
	for (i = 0; i < cl.maxclients; i++)
	{
		if (cl.scores[i].name[0])
		{
			fragsort[scoreboardlines] = i;
			scoreboardlines++;
		}
	}

	for (i = 0; i < scoreboardlines; i++)
	{
		for (j = 0; j < scoreboardlines-1-i; j++)
		{
			if (cl.scores[fragsort[j]].frags < cl.scores[fragsort[j+1]].frags)
			{
				k = fragsort[j];
				fragsort[j] = fragsort[j+1];
				fragsort[j+1] = k;
			}
		}
	}
}

static void FindColor (int slot, int *color1, int *color2)
{
	int		j;
	int		top, bottom, done;
	byte	*sourceA, *sourceB, *colorA, *colorB;

	if (slot > cl.maxclients)
		Sys_Error ("%s: slot > cl.maxclients", __thisfunc__);

	if (!cl.scores[slot].playerclass)
	{
		*color1 = *color2 = 0;
		return;
	}

	top = (cl.scores[slot].colors & 0xf0) >> 4;
	bottom = (cl.scores[slot].colors & 15);

	if (top > 10)
		top = 0;
	if (bottom > 10)
		bottom = 0;

	top -= 1;
	bottom -= 1;

	colorA = playerTranslation + 256 + color_offsets[(int)cl.scores[slot].playerclass-1];
	colorB = colorA + 256;
	sourceA = colorB + 256 + (top * 256);
	sourceB = colorB + 256 + (bottom * 256);
	done = 0;
	for (j = 0; j < 256; j++, colorA++, colorB++, sourceA++, sourceB++)
	{
		if ((*colorA != 255) && !(done & 1))
		{
			if (top >= 0)
				*color1 = *sourceA;
			else
				*color1 = *colorA;
		//	done |= 1;
		}
		if ((*colorB != 255) && !(done & 2))
		{
			if (bottom >= 0)
				*color2 = *sourceB;
			else
				*color2 = *colorB;
		//	done |= 2;
		}
	}
}

//==========================================================================
//
// Sbar_DeathmatchOverlay
//
//==========================================================================

void Sbar_DeathmatchOverlay(void)
{
	qpic_t		*pic;
	int		i, k, l;
	int		top, bottom;
	int		x, y, f;
	char		num[12];
	scoreboard_t	*s;

	scr_copyeverything = 1;
	if (scr_viewsize.integer < 100)
		scr_fullupdate = 0;

	pic = Draw_CachePic ("gfx/menu/title8.lmp");
	M_DrawTransPic ((320-pic->width)/2, 0, pic);

	// scores
	Sbar_SortFrags ();

	// draw the text
	l = scoreboardlines;

	x = 80 + ((vid.width - 320)>>1);
	y = 62;
	top = bottom = 0;

	for (i = 0; i < l; i++)
	{
		if (y+10 >= vid.height)
			break;

		k = fragsort[i];
		s = &cl.scores[k];
		if (!s->name[0])
			continue;

		// draw background
		FindColor (k, &top, &bottom);

		Draw_Fill (x, y, 40, 4, top);
		Draw_Fill (x, y+4, 40, 4, bottom);

		// draw number
		f = s->frags;
		sprintf (num, "%3i", f);

		if (k == sv_kingofhill)
			Draw_Character (x-12 , y-1, 130);
		Draw_Character (x+8 , y-1, num[0]);
		Draw_Character (x+16 , y-1, num[1]);
		Draw_Character (x+24 , y-1, num[2]);

		if (k == cl.viewentity - 1)
			Draw_Character (x - 8, y-1, 12);

		// draw name
		Draw_String (x+64, y, s->name);

		y += 10;
	}
}

//==========================================================================
//
// Sbar_PuzzlePieceOverlay
//
//==========================================================================

static void FindPuzzlePieceName(const char *which, char *name)
{
	const char *str = CL_FindPuzzleString (which);
	if (!str) str = "Unknown";
	q_strlcpy (name, str, 40);	/* 40 == array size in caller */
}

static void Sbar_PuzzlePieceOverlay(void)
{
	int		i, y, piece;
	char		Name[40];

	scr_copyeverything = 1;
	if (scr_viewsize.integer < 100)
		scr_fullupdate = 0;

	piece = 0;
	y = 40;
	for (i = 0; i < 8; i++)
	{
		if (cl.puzzle_pieces[i][0] == 0)
			continue;

		if (piece == 4)
			y = 40;

		FindPuzzlePieceName(cl.puzzle_pieces[i], Name);

		if (piece < 4)
		{
			M_DrawPic(10, y, Draw_CachePic(va("gfx/puzzle/%s.lmp", cl.puzzle_pieces[i])));
			M_PrintWhite (45, y, Name);
		}
		else
		{
			M_DrawPic(310-32, y, Draw_CachePic(va("gfx/puzzle/%s.lmp", cl.puzzle_pieces[i])));
			M_PrintWhite (310-32-3-(strlen(Name)*8), 18+y, Name);
		}

		y += 32;
		piece++;
	}
}

//==========================================================================
//
// Sbar_SmallDeathmatchOverlay
//
//==========================================================================

static void Sbar_SmallDeathmatchOverlay(void)
{
	int		i, k, l;
	int		top, bottom;
	int		x, y, f;
	char		num[12];
	scoreboard_t	*s;

	if (!DMMode.integer)
		return;
	if (DMMode.integer > 2)
		DMMode.integer = 2;

	if (DMMode.integer == 2 && BarHeight != BAR_TOP_HEIGHT)
		return;

	trans_level = dmtrans.integer;
	if (trans_level < 0 || trans_level > 2)
		trans_level = 0;

	scr_copyeverything = 1;
	if (scr_viewsize.integer < 100)
		scr_fullupdate = 0;

	// scores
	Sbar_SortFrags ();

	// draw the text
	l = scoreboardlines;

	if (DMMode.integer == 1)
	{
		if (l > 8)
			l = 8;
		y = 46;
	}
	//else if (DMMode.integer == 2)
	else
	{
		if (l > 4)
			l = 4;
		y = vid.height - BAR_TOP_HEIGHT;
	}

	x = 10;
	top = bottom = 0;

	for (i = 0; i < l; i++)
	{
		k = fragsort[i];
		s = &cl.scores[k];
		if (!s->name[0])
			continue;

		// draw background
		FindColor (k, &top, &bottom);

		Draw_Fill (x, y, 28, 4, top);
		Draw_Fill (x, y+4, 28, 4, bottom);

		// draw number
		f = s->frags;
		sprintf (num, "%3i", f);

		if (k != cl.viewentity - 1)
		{
			Draw_Character (x+2 , y-1, num[0]);
			Draw_Character (x+10 , y-1, num[1]);
			Draw_Character (x+18 , y-1, num[2]);
			if (k == sv_kingofhill)
				Draw_Character (x+30 , y-1, 130);
		}
		else
		{
			Draw_Character (x+2 , y-1, num[0] + 256);
			Draw_Character (x+10 , y-1, num[1] + 256);
			Draw_Character (x+18 , y-1, num[2] + 256);
			if (k == sv_kingofhill)
				Draw_Character ( x+30 , y-1, 130);
		}

		y += 10;
	}

	trans_level = 0;
}


//==========================================================================
//
// DrawActiveRings
//
//==========================================================================
static int art_col;
static int ring_row;

static void DrawActiveRings(void)
{
	int		flag;
	int		frame;
	char	tempStr[24];

	if (scr_con_current == vid.height)
		return;		// console is full screen

	ring_row = 1;

	flag = (int)cl.v.rings_active;

	if (flag & RING_TURNING)
	{
		frame = 1 + ((int)(cl.time * 16) & 15);
		sprintf(tempStr, "gfx/rngtrn%d.lmp", frame);
		Draw_TransPic(vid.width - 50, ring_row, Draw_CachePic(tempStr));
		ring_row += 33;
	}

	/*
	if (flag & RING_REGENERATION)
	{
		frame = 1 + ((int)(cl.time * 16) & 15);
		sprintf(tempStr, "gfx/rngreg%d.lmp", frame);
		Draw_TransPic(vid.width - 50, ring_row, Draw_CachePic(tempStr));
		ring_row += 33;
	}
	*/

	if (flag & RING_WATER)
	{
		frame = 1 + ((int)(cl.time * 16) & 15);
		sprintf(tempStr, "gfx/rngwtr%d.lmp", frame);
		Draw_TransPic(vid.width - 50, ring_row, Draw_CachePic(tempStr));
		ring_row += 33;
	}

	if (flag & RING_FLIGHT)
	{
		frame = 1 + ((int)(cl.time * 16) & 15);
		sprintf(tempStr, "gfx/rngfly%d.lmp", frame);
		Draw_TransPic(vid.width - 50, ring_row, Draw_CachePic(tempStr));
		ring_row += 33;
	}
}

//==========================================================================
//
// DrawActiveArtifacts
//
//==========================================================================
static void DrawActiveArtifacts(void)
{
	int		flag;
	static int	oldflags = 0;
	int		frame;
	char	tempStr[24];

	if (scr_con_current == vid.height)
		return;

	art_col = 50;

	if (ring_row != 1)
		art_col += 50;

	flag = (int)cl.v.artifact_active;
	if (flag & ART_TOMEOFPOWER)
	{
		frame = 1 + ((int)(cl.time * 16) & 15);
		sprintf(tempStr, "gfx/pwrbook%d.lmp", frame);
		Draw_TransPic(vid.width-art_col, 1, Draw_CachePic(tempStr));
		art_col += 50;
		scr_topupdate = 0;
	}
	else if (oldflags & ART_TOMEOFPOWER)
	{
		scr_topupdate = 0;
	}

	if (flag & ART_HASTE)
	{
		frame = 1 + ((int)(cl.time * 16) & 15);
		sprintf(tempStr, "gfx/durhst%d.lmp", frame);
		Draw_TransPic(vid.width-art_col, 1, Draw_CachePic(tempStr));
		art_col += 50;
		scr_topupdate = 0;
	}
	else if (oldflags & ART_HASTE)
	{
		scr_topupdate = 0;
	}

	if (flag & ART_INVINCIBILITY)
	{
		frame = 1 + ((int)(cl.time * 16) & 15);
		sprintf(tempStr, "gfx/durshd%d.lmp", frame);
		Draw_TransPic(vid.width-art_col, 1, Draw_CachePic(tempStr));
		art_col += 50;
		scr_topupdate = 0;
	}
	else if (oldflags & ART_INVINCIBILITY)
	{
		scr_topupdate = 0;
	}

	if (scr_viewsize.integer >= 100)
		scr_topupdate = vid.numpages;

	oldflags = flag;
}

//============================================================================


void Inv_Update(qboolean force)
{
	if (inv_flg || force)
	{
		// Just to be safe
		if (cl.inv_selected >= 0 && cl.inv_count > 0)
			cl.v.inventory = cl.inv_order[cl.inv_selected] + 1;
		else
			cl.v.inventory = 0;

		if (!force)
		{
			if (scr_viewsize.integer < 100)
				scr_fullupdate = 0;
			inv_flg = false;	// Toggle menu off
		}

		// This will cause the server to set the client's edict's inventory value
		MSG_WriteByte (&cls.message, clc_inv_select);
		MSG_WriteByte (&cls.message, cl.v.inventory);
	}
}

//==========================================================================
//
// DrawBarArtifactIcon
//
//==========================================================================

static void DrawBarArtifactIcon(int x, int y, int artifact)
{
	int	count;

	if (artifact < 0 || artifact >= MAX_INVENTORY)
		return;
	Sbar_DrawTransPic(x, y, Draw_CachePic(va("gfx/arti%02d.lmp", artifact)));
	if ((count = Inv_GetCount(artifact)) > 0)
	{
		DrawBarArtifactNumber(x+20, y+21, count);
	}
}

//==========================================================================
//
// DrawArtifactInventory
//
//==========================================================================

static void DrawArtifactInventory(void)
{
	int	i;
	int	x, y;

	if (InventoryHideTime < cl.time)
	{
		Inv_Update(false);
		return;
	}
	if (!inv_flg)
		return;
	if (!cl.inv_count)
	{
		Inv_Update(false);
		return;
	}

	if (BarHeight < 0)
		y = BarHeight-34;
	else
		y = -37;

	// InvLeft_f and InvRight_f scrolls the inventory as needed - S.A.
	for (i = 0, x = 64; i < INV_MAX_ICON; i++, x += 33)
	{
		if (i >= cl.inv_count)
			break;

		if ((cl.inv_startpos + i) % cl.inv_count == cl.inv_selected)
		{ // Highlight icon
			Sbar_DrawTransPic(x+9, y-12, Draw_CachePic("gfx/artisel.lmp"));
		}
		DrawBarArtifactIcon(x, y, cl.inv_order[(cl.inv_startpos + i) % cl.inv_count]);
	}

	/*
	//Inv_DrawArrows (x, y);
	// Left arrow showing there are icons to the left
	if (cl.inv_startpos)
		Draw_Fill ( x , y - 5, 3, 1, 30);
	// Right arrow showing there are icons to the right
	if (cl.inv_startpos + INV_MAX_ICON < cl.inv_count)
		Draw_Fill ( x + 200, y - 5 , 3, 1, 30);
	*/
}

//==========================================================================
//
// ShowDMDown_f
//
//==========================================================================

static void ShowDMDown_f(void)
{
	if (sb_ShowDM)
		return;
	sb_ShowDM = true;
}

//==========================================================================
//
// ShowDMUp_f
//
//==========================================================================

static void ShowDMUp_f(void)
{
	sb_ShowDM = false;
}

//==========================================================================
//
// ShowInfoDown_f
//
//==========================================================================

static void ShowInfoDown_f(void)
{
	if (sb_ShowInfo || cl.intermission || cls.state < ca_connected)
		return;
//	if (cls.demoplayback) return;	// demoplay check not needed -- O.S.
	S_LocalSound("misc/barmovup.wav");
	BarTargetHeight = BAR_TOTAL_HEIGHT;
	sb_ShowInfo = true;
	sb_updates = 0;
}

//==========================================================================
//
// ShowInfoUp_f
//
//==========================================================================

static void ShowInfoUp_f(void)
{
	if (cl.intermission || (scr_viewsize.integer > 110/* && !sbtrans.integer*/))
		BarTargetHeight = 0.0-BAR_BUMP_HEIGHT;
	else	BarTargetHeight = BAR_TOP_HEIGHT;
	S_LocalSound("misc/barmovdn.wav");
	sb_ShowInfo = false;
	sb_updates = 0;
}

//==========================================================================
//
// InvLeft_f
//
//==========================================================================

static void InvLeft_f(void)
{
	if (!cl.inv_count || cl.intermission)
		return;

	if (inv_flg)
	{
		// scroll inventory icons if we're at the left-most already
		if (cl.inv_selected == cl.inv_startpos)
		{
			cl.inv_startpos = (cl.inv_startpos - 1 + cl.inv_count) % cl.inv_count;
			cl.inv_selected = cl.inv_startpos;
		}
		else
		{
			cl.inv_selected = (cl.inv_selected - 1 + cl.inv_count) % cl.inv_count;
		}

		if (scr_viewsize.integer < 100)
			scr_fullupdate = 0;
	}
	else
	{
		inv_flg = true;
	}
	S_LocalSound("misc/invmove.wav");
	InventoryHideTime = cl.time+INVENTORY_DISPLAY_TIME;
}

//==========================================================================
//
// InvRight_f
//
//==========================================================================

static void InvRight_f(void)
{
	int		right_icon;

	if (!cl.inv_count || cl.intermission)
		return;

	if (inv_flg)
	{

		if (cl.inv_count >= INV_MAX_ICON)
			right_icon = (cl.inv_startpos + INV_MAX_ICON - 1) % cl.inv_count;
		else
			right_icon = (cl.inv_startpos + cl.inv_count - 1) % cl.inv_count;
		// scroll inventory icons if we're at the right most already
		if (cl.inv_selected == right_icon)
			cl.inv_startpos = (cl.inv_startpos + 1) % cl.inv_count;

		cl.inv_selected = (cl.inv_selected + 1) % cl.inv_count;

		if (scr_viewsize.integer < 100)
			scr_fullupdate = 0;
	}
	else
	{
		inv_flg = true;
	}
	S_LocalSound("misc/invmove.wav");
	InventoryHideTime = cl.time+INVENTORY_DISPLAY_TIME;
}

//==========================================================================
//
// InvUse_f
//
//==========================================================================

static void InvUse_f(void)
{
	if (!cl.inv_count || cl.intermission)
		return;
	S_LocalSound("misc/invuse.wav");
	//Inv_Update(false);
	Inv_Update(true);
	inv_flg = false;
	if (scr_viewsize.integer < 100)
		scr_fullupdate = 0;
	in_impulse = 23;
}

//==========================================================================
//
// InvOff_f
//
//==========================================================================

static void InvOff_f(void)
{
	inv_flg = false;
	if (scr_viewsize.integer < 100)
		scr_fullupdate = 0;
}

//==========================================================================
//
// ToggleDM_f
//
//==========================================================================

static void ToggleDM_f(void)
{
	DMMode.integer += 1;
	if (DMMode.integer > 2)
		DMMode.integer = 0;
}

//==========================================================================
//
// SB_InvChanged
//
//==========================================================================

void SB_InvChanged(void)
{
	int		counter, position;
	qboolean	examined[MAX_INVENTORY];
	qboolean	ForceUpdate = false;

	memset (examined, 0, sizeof(examined));

	if (cl.inv_selected >= 0 && Inv_GetCount(cl.inv_order[cl.inv_selected]) == 0)
		ForceUpdate = true;

	// removed items we no longer have from the order
	for (counter = position = 0; counter < cl.inv_count; counter++)
	{
		if (Inv_GetCount(cl.inv_order[counter]) > 0)
		{
			cl.inv_order[position] = cl.inv_order[counter];
			examined[cl.inv_order[position]] = true;

			position++;
		}
	}

	// add in the new items
	for (counter = 0; counter < MAX_INVENTORY; counter++)
	{
		if (!examined[counter])
		{
			if (Inv_GetCount(counter) > 0)
			{
				cl.inv_order[position] = counter;
				position++;
			}
		}
	}

	cl.inv_count = position;
	if (cl.inv_selected >= cl.inv_count)
	{
		cl.inv_selected = cl.inv_count-1;
		ForceUpdate = true;
	}
	if (cl.inv_count && cl.inv_selected < 0)
	{
		cl.inv_selected = 0;
		ForceUpdate = true;
	}
	if (ForceUpdate)
	{
		Inv_Update(true);
	}
	/* make sure that startpos isn't borked */
	if (cl.inv_count <= 1)
		cl.inv_startpos = 0;
	else if (cl.inv_startpos >= cl.inv_count)
		cl.inv_startpos = cl.inv_selected;
	else
	{
		position = cl.inv_selected - cl.inv_startpos;
		if (position < 0)
			position += cl.inv_count;
		if (position >= INV_MAX_ICON/* || position < 0*/)
			cl.inv_startpos = cl.inv_selected;
	}
}

//==========================================================================
//
// SB_InvReset
//
//==========================================================================

void SB_InvReset(void)
{
	cl.inv_count = cl.inv_startpos = 0;
	cl.inv_selected = -1;
	inv_flg = false;
	if (scr_viewsize.integer < 100)
		scr_fullupdate = 0;
}

//==========================================================================
//
// SB_ViewSizeChanged
//
//==========================================================================

void SB_ViewSizeChanged(void)
{
	if (cl.intermission || (scr_viewsize.integer > 110/* && !sbtrans.integer*/))
		BarHeight = BarTargetHeight = 0.0-BAR_BUMP_HEIGHT;
	else	BarHeight = BarTargetHeight = BAR_TOP_HEIGHT;
}

// DRAWING FUNCTIONS *******************************************************

//==========================================================================
//
// Sbar_Draw**Pic
// Relative to the current status bar location.
//
//==========================================================================

static void Sbar_DrawPic(int x, int y, qpic_t *pic)
{
	Draw_PicCropped (x+((vid.width-320)>>1), y+(vid.height-(int)BarHeight), pic);
}

static void Sbar_DrawTransPic(int x, int y, qpic_t *pic)
{
	Draw_TransPicCropped (x+((vid.width-320)>>1), y+(vid.height-(int)BarHeight), pic);
}

#if 0	/* no callers */
static void Sbar_DrawCharacter(int x, int y, int num)
{
	Draw_Character (x+((vid.width-320)>>1)+4, y+vid.height-(int)BarHeight, num);
}

static void Sbar_DrawString(int x, int y, const char *str)
{
	Draw_String (x+((vid.width-320)>>1), y+vid.height-(int)BarHeight, str);
}

static void Sbar_DrawSmallCharacter(int x, int y, int num)
{
	Draw_SmallCharacter (x+((vid.width-320)>>1)+4, y+vid.height-(int)BarHeight, num);
}
#endif

static void Sbar_DrawSmallString(int x, int y, const char *str)
{
	Draw_SmallString (x+((vid.width-320)>>1), y+vid.height-(int)BarHeight, str);
}

static void DrawBarArtifactNumber(int x, int y, int number)
{
	static char artiNumName[18] = "gfx/artinum0.lmp";

	if (number >= 10)
	{
		artiNumName[11] = '0'+(number%100)/10;
		Sbar_DrawTransPic(x -4, y, Draw_CachePic(artiNumName));
	}
	artiNumName[11] = '0'+number%10;
	Sbar_DrawTransPic(x, y, Draw_CachePic(artiNumName));
}

