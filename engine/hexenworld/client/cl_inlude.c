/*
 * cl_interlude.c: Setup the Hexen II intermission screen flags. Only
 * the intermission index is sent by the server, therefore the rest of
 * the stuff is unfortunately hardcoded here in the engine.
 *
 * Copyright (C) 2012 O.Sezer <sezero@users.sourceforge.net>
 *
 * $Id$
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
 *
 */

#include "quakedef.h"

void CL_SetupIntermission (int num)
{
	if (oem.integer && num == 1)
		cl.intermission = 9;
	else	cl.intermission = num;

	switch (cl.intermission)
	{
	case 1: /* defeated famine: episode 1 (village) to 2 (mazaera) */
		cl.completed_time = cl.time;
		cl.message_index = 1 + 394;
		cl.intermission_flags = 0;
		cl.intermission_pic = "gfx/meso.lmp";
		cl.lasting_time = 0;
		cl.intermission_next = 0;
		break;
	case 2: /* defeated death: episode 2 (mazaera) to 3 (egypt) */
		cl.completed_time = cl.time;
		cl.message_index = 2 + 394;
		cl.intermission_flags = 0;
		cl.intermission_pic = "gfx/egypt.lmp";
		cl.lasting_time = 0;
		cl.intermission_next = 0;
		break;
	case 3: /* defeated pestilence: episode 3 (egypt) to 4 (roman) */
		cl.completed_time = cl.time;
		cl.message_index = 3 + 394;
		cl.intermission_flags = 0;
		cl.intermission_pic = "gfx/roman.lmp";
		cl.lasting_time = 0;
		cl.intermission_next = 0;
		break;
	case 4: /* defeated war: episode 4 (roman) to last (castle) */
		cl.completed_time = cl.time;
		cl.message_index = 4 + 394;
		cl.intermission_flags = 0;
		cl.intermission_pic = "gfx/castle.lmp";
		cl.lasting_time = 0;
		cl.intermission_next = 0;
		break;
	case 5: /* finale for the demo version */
		cl.completed_time = cl.time;
		/* DEMO_MSG_INDEX is 408 for H2, 410 for H2MP strings.txt.
		 * in uHexen2, the demo version isn't allowed in combination
		 * with the mission pack, so we are good with 408. */
		cl.message_index = 408;
		cl.intermission_flags = 0;
		cl.intermission_pic = "gfx/castle.lmp";
		cl.lasting_time = 0;
		cl.intermission_next = 0;
		break;
	case 6: /* defeated eidolon: finale, part 1/3 */
		cl.completed_time = cl.time;
		cl.message_index = 6 + 386;
		cl.intermission_flags = INTERMISSION_PRINT_DELAY|INTERMISSION_PRINT_WHITE|INTERMISSION_PRINT_TOP;
		cl.intermission_pic = "gfx/end-1.lmp";
		cl.lasting_time = 15;
		cl.intermission_next = 7;
		break;
	case 7: /* defeated eidolon: finale, part 2/3 */
		cl.completed_time = cl.time;
		cl.message_index = 7 + 386;
		cl.intermission_flags = INTERMISSION_PRINT_DELAY|INTERMISSION_PRINT_WHITE|INTERMISSION_PRINT_TOP;
		cl.intermission_pic = "gfx/end-2.lmp";
		cl.lasting_time = 15;
		cl.intermission_next = 8;
		break;
	case 8: /* defeated eidolon: finale, part 2/3 */
		cl.completed_time = cl.time;
		cl.message_index = 8 + 386;
		cl.intermission_flags = INTERMISSION_PRINT_WHITE|INTERMISSION_PRINT_DELAY|INTERMISSION_PRINT_TOPMOST;
		cl.intermission_pic = "gfx/end-3.lmp";
		cl.lasting_time = 0;
		cl.intermission_next = 0;
		break;
	case 9: /* finale for the bundle (oem) version */
		cl.completed_time = cl.time;
		cl.message_index = 391;
		cl.intermission_flags = INTERMISSION_PRINT_WHITE;
		cl.intermission_pic = "gfx/castle.lmp";
		cl.lasting_time = 0;
		cl.intermission_next = 0;
		break;

	case 10: /* Siege: Defender win - wipe out or time limit */
		cl.completed_time = cl.time;
		cl.message_index = -1;
		cl.intermission_flags = INTERMISSION_NO_MESSAGE;
		cl.intermission_pic = "gfx/defwin.lmp";
		cl.lasting_time = 0;
		cl.intermission_next = 0;
		break;
	case 11: /* Siege: Attacker win - caught crown */
		cl.completed_time = cl.time;
		cl.message_index = -1;
		cl.intermission_flags = INTERMISSION_NO_MESSAGE;
		cl.intermission_pic = "gfx/attwin.lmp";
		cl.lasting_time = 0;
		cl.intermission_next = 0;
		break;
	case 12: /* Siege: Attacker win 2 - wiped out. */
		cl.completed_time = cl.time;
		cl.message_index = -1;
		cl.intermission_flags = INTERMISSION_NO_MESSAGE;
		cl.intermission_pic = "gfx/attwin2.lmp";
		cl.lasting_time = 0;
		cl.intermission_next = 0;
		break;

	default: /* unexpected: */
		cl.completed_time = cl.time;
		cl.message_index = Q_MAXINT;
		cl.intermission_flags = 0;
		cl.intermission_pic = NULL;
		cl.lasting_time = 0;
		cl.intermission_next = 0;
	//	Host_Error("%s: Bad intermission number %d", __thisfunc__, cl.intermission);
		Con_Printf("%s: Bad intermission number %d\n", __thisfunc__, cl.intermission);
		break;
	}
}

