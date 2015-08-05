/*
 * msg_io.c -- Message IO functions
 * Handles byte ordering and avoids alignment errors
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

#include "q_stdinc.h"
#include "h2config.h"
#include "compiler.h"
#include "q_endian.h"
#include "sys.h"
#include "sizebuf.h"
#include "msg_io.h"
#if defined(H2W)
#include "protocol.h"
#endif	/* H2W */


extern	sizebuf_t	net_message;


//
// writing functions
//

void MSG_WriteChar (sizebuf_t *sb, int c)
{
	byte	*buf;

#ifdef PARANOID
	if (c < -128 || c > 127)
		Sys_Error ("%s: range error", __thisfunc__);
#endif

	buf = (byte *) SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteByte (sizebuf_t *sb, int c)
{
	byte	*buf;

#ifdef PARANOID
	if (c < 0 || c > 255)
		Sys_Error ("%s: range error", __thisfunc__);
#endif

	buf = (byte *) SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteShort (sizebuf_t *sb, int c)
{
	byte	*buf;

#ifdef PARANOID
	if (c < ((short)0x8000) || c > (short)0x7fff)
		Sys_Error ("%s: range error", __thisfunc__);
#endif

	buf = (byte *) SZ_GetSpace (sb, 2);
	buf[0] = c&0xff;
	buf[1] = c>>8;
}

void MSG_WriteLong (sizebuf_t *sb, int c)
{
	byte	*buf;

	buf = (byte *) SZ_GetSpace (sb, 4);
	buf[0] = c&0xff;
	buf[1] = (c>>8)&0xff;
	buf[2] = (c>>16)&0xff;
	buf[3] = c>>24;
}

void MSG_WriteFloat (sizebuf_t *sb, float f)
{
	union
	{
		float	f;
		int	l;
	} dat;

	dat.f = f;
	dat.l = LittleLong (dat.l);

	SZ_Write (sb, &dat.l, 4);
}

void MSG_WriteString (sizebuf_t *sb, const char *s)
{
	if (!s)
		SZ_Write (sb, "", 1);
	else
		SZ_Write (sb, s, (int)strlen(s) + 1);
}

void MSG_WriteCoord (sizebuf_t *sb, float f)
{
//	MSG_WriteShort (sb, (int)(f*8));
	if (f >= 0)
		MSG_WriteShort (sb, (int)(f * 8.0f + 0.5f));
	else
		MSG_WriteShort (sb, (int)(f * 8.0f - 0.5f));
}

void MSG_WriteAngle (sizebuf_t *sb, float f)
{
//	MSG_WriteByte (sb, (int)(f*256/360) & 255);
//	LordHavoc: round to nearest value, rather than rounding toward zero
	if (f >= 0)
		MSG_WriteByte (sb, (int)(f*(256.0f/360.0f) + 0.5f) & 255);
	else
		MSG_WriteByte (sb, (int)(f*(256.0f/360.0f) - 0.5f) & 255);
}

#if defined(H2W)
void MSG_WriteAngle16 (sizebuf_t *sb, float f)
{
//	MSG_WriteShort (sb, (int)(f*65536/360) & 65535);
	if (f >= 0)
		MSG_WriteShort (sb, (int)(f*(65536.0f/360.0f) + 0.5f) & 65535);
	else
		MSG_WriteShort (sb, (int)(f*(65536.0f/360.0f) - 0.5f) & 65535);
}

void MSG_WriteUsercmd (sizebuf_t *buf, const usercmd_t *cmd, qboolean long_msg)
{
	int		bits;

//
// send the movement message
//
	bits = 0;
	if (cmd->angles[0])
		bits |= CM_ANGLE1;
	if (cmd->angles[2])
		bits |= CM_ANGLE3;
	if (cmd->forwardmove)
		bits |= CM_FORWARD;
	if (cmd->sidemove)
		bits |= CM_SIDE;
	if (cmd->upmove)
		bits |= CM_UP;
	if (cmd->buttons)
		bits |= CM_BUTTONS;
	if (cmd->impulse)
		bits |= CM_IMPULSE;
	if (cmd->msec)
		bits |= CM_MSEC;

	MSG_WriteByte (buf, bits);
	if (long_msg)
	{
		MSG_WriteByte (buf, cmd->light_level);
	}

	if (bits & CM_ANGLE1)
		MSG_WriteAngle16 (buf, cmd->angles[0]);
	MSG_WriteAngle16 (buf, cmd->angles[1]);
	if (bits & CM_ANGLE3)
		MSG_WriteAngle16 (buf, cmd->angles[2]);

	if (bits & CM_FORWARD)
		MSG_WriteChar (buf, (int)(cmd->forwardmove*0.25f));
	if (bits & CM_SIDE)
	  	MSG_WriteChar (buf, (int)(cmd->sidemove*0.25f));
	if (bits & CM_UP)
		MSG_WriteChar (buf, (int)(cmd->upmove*0.25f));

	if (bits & CM_BUTTONS)
	  	MSG_WriteByte (buf, cmd->buttons);
	if (bits & CM_IMPULSE)
		MSG_WriteByte (buf, cmd->impulse);
	if (bits & CM_MSEC)
		MSG_WriteByte (buf, cmd->msec);
}
#endif	/* H2W */


//
// reading functions
//
int		msg_readcount;
qboolean	msg_badread;

void MSG_BeginReading (void)
{
	msg_readcount = 0;
	msg_badread = false;
}

// returns -1 and sets msg_badread if no more characters are available
int MSG_ReadChar (void)
{
	int	c;

	if (msg_readcount+1 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	c = (signed char)net_message.data[msg_readcount];
	msg_readcount++;

	return c;
}

int MSG_ReadByte (void)
{
	int	c;

	if (msg_readcount+1 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	c = (unsigned char)net_message.data[msg_readcount];
	msg_readcount++;

	return c;
}

int MSG_ReadShort (void)
{
	int	c;

	if (msg_readcount+2 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	c = (short)(net_message.data[msg_readcount]
			+ (net_message.data[msg_readcount+1]<<8));

	msg_readcount += 2;

	return c;
}

int MSG_ReadLong (void)
{
	int	c;

	if (msg_readcount+4 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	c = net_message.data[msg_readcount]
			+ (net_message.data[msg_readcount+1]<<8)
			+ (net_message.data[msg_readcount+2]<<16)
			+ (net_message.data[msg_readcount+3]<<24);

	msg_readcount += 4;

	return c;
}

float MSG_ReadFloat (void)
{
	union
	{
		byte	b[4];
		float	f;
		int	l;
	} dat;

	dat.b[0] = net_message.data[msg_readcount];
	dat.b[1] = net_message.data[msg_readcount+1];
	dat.b[2] = net_message.data[msg_readcount+2];
	dat.b[3] = net_message.data[msg_readcount+3];
	msg_readcount += 4;

	dat.l = LittleLong (dat.l);

	return dat.f;
}

const char *MSG_ReadString (void)
{
	static char	string[2048];
	int		c;
	size_t		l;

	l = 0;
	do
	{
		c = MSG_ReadByte ();
		if (c == -1 || c == 0)
			break;
		string[l] = c;
		l++;
	} while (l < sizeof(string) - 1);

	string[l] = 0;

	return string;
}

#if defined(H2W)
const char *MSG_ReadStringLine (void)
{
	static char	string[2048];
	int		c;
	size_t		l;

	l = 0;
	do
	{
		c = MSG_ReadByte ();
		if (c == -1 || c == 0 || c == '\n')
			break;
		string[l] = c;
		l++;
	} while (l < sizeof(string) - 1);

	string[l] = 0;

	return string;
}
#endif	/* H2W */

float MSG_ReadCoord (void)
{
	return MSG_ReadShort() * (1.0f/8.0f);
}

float MSG_ReadAngle (void)
{
	return MSG_ReadChar() * (360.0f/256.0f);
}

#if defined(H2W)
float MSG_ReadAngle16 (void)
{
	return MSG_ReadShort() * (360.0f/65536.0f);
}

void MSG_ReadUsercmd (usercmd_t *move, qboolean long_msg)
{
	int bits;

	memset (move, 0, sizeof(*move));

	bits = MSG_ReadByte ();
	if (long_msg)
	{
		move->light_level = MSG_ReadByte();
	}
	else
	{
		move->light_level = 0;
	}

// read current angles
	if (bits & CM_ANGLE1)
		move->angles[0] = MSG_ReadAngle16 ();
	else
		move->angles[0] = 0;
	move->angles[1] = MSG_ReadAngle16 ();
	if (bits & CM_ANGLE3)
		move->angles[2] = MSG_ReadAngle16 ();
	else
		move->angles[2] = 0;

// read movement
	if (bits & CM_FORWARD)
		move->forwardmove = MSG_ReadChar () * 4;
	if (bits & CM_SIDE)
		move->sidemove = MSG_ReadChar () * 4;
	if (bits & CM_UP)
		move->upmove = MSG_ReadChar () * 4;

// read buttons
	if (bits & CM_BUTTONS)
		move->buttons = MSG_ReadByte ();
	else
		move->buttons = 0;

	if (bits & CM_IMPULSE)
		move->impulse = MSG_ReadByte ();
	else
		move->impulse = 0;

// read time to run command
	if (bits & CM_MSEC)
		move->msec = MSG_ReadByte ();
	else
		move->msec = 0;
}
#endif	/* H2W */

