/*
 * msg_io.h -- Message IO functions
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

#ifndef __MSGIO_H
#define __MSGIO_H

void MSG_WriteChar (sizebuf_t *sb, int c);
void MSG_WriteByte (sizebuf_t *sb, int c);
void MSG_WriteShort (sizebuf_t *sb, int c);
void MSG_WriteLong (sizebuf_t *sb, int c);
void MSG_WriteFloat (sizebuf_t *sb, float f);
void MSG_WriteString (sizebuf_t *sb, const char *s);
void MSG_WriteCoord (sizebuf_t *sb, float f);
void MSG_WriteAngle (sizebuf_t *sb, float f);
#if defined(H2W)
struct usercmd_s;
void MSG_WriteAngle16 (sizebuf_t *sb, float f);
void MSG_WriteUsercmd (sizebuf_t *sb, const struct usercmd_s *cmd, qboolean long_msg);
#endif	/* H2W */

void MSG_BeginReading (void);
int MSG_ReadChar (void);
int MSG_ReadByte (void);
int MSG_ReadShort (void);
int MSG_ReadLong (void);
float MSG_ReadFloat (void);
const char *MSG_ReadString (void);
#if defined(H2W)
const char *MSG_ReadStringLine (void);
#endif	/* H2W */

float MSG_ReadCoord (void);
float MSG_ReadAngle (void);
#if defined(H2W)
float MSG_ReadAngle16 (void);
void MSG_ReadUsercmd (struct usercmd_s *cmd, qboolean long_msg);
#endif	/* H2W*/

extern	int		msg_readcount;
extern	qboolean	msg_badread;	/* set if a read goes beyond end of message */

#endif	/* __MSGIO_H */

