/*
 * net_chan.c -- net channel
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

#define	PACKET_HEADER	8

/*

packet header
-------------
31	sequence
1	does this message contain a reliable payload
31	acknowledge sequence
1	acknowledge receipt of even/odd message

The remote connection never knows if it missed a reliable message, the
local side detects that it has been dropped by seeing a sequence acknowledge
higher thatn the last reliable sequence, but without the correct evon/odd
bit for the reliable set.

If the sender notices that a reliable message has been dropped, it will be
retransmitted.  It will not be retransmitted again until a message after
the retransmit has been acknowledged and the reliable still failed to get there.

if the sequence number is -1, the packet should be handled without a netcon

The reliable message can be added to at any time by doing
MSG_Write* (&netchan->message, <data>).

If the message buffer is overflowed, either by a single message, or by
multiple frames worth piling up while the last reliable transmit goes
unacknowledged, the netchan signals a fatal error.

Reliable messages are always placed first in a packet, then the unreliable
message is included if there is sufficient room.

To the receiver, there is no distinction between the reliable and unreliable
parts of the message, they are just processed out as a single larger message.

Illogical packet sequence numbers cause the packet to be dropped, but do
not kill the connection.  This, combined with the tight window of valid
reliable acknowledgement numbers provides protection against malicious
address spoofing.

*/

int		net_drop;
static	cvar_t	showpackets = {"showpackets", "0", CVAR_NONE};
static	cvar_t	showdrop = {"showdrop", "0", CVAR_NONE};

#ifdef SERVERONLY
#define NOT_DEMOPLAYBACK	true
#else
#define NOT_DEMOPLAYBACK	(!cls.demoplayback)
#endif

/*
===============
Netchan_Init

===============
*/
void Netchan_Init (void)
{
	Cvar_RegisterVariable (&showpackets);
	Cvar_RegisterVariable (&showdrop);
}

/*
===============
Netchan_OutOfBand

Sends an out-of-band datagram
================
*/
void Netchan_OutOfBand (const netadr_t *adr, int length, byte *data)
{
	sizebuf_t	senddata;
	byte		send_buf[MAX_MSGLEN + PACKET_HEADER];

// write the packet header
	SZ_Init (&senddata, send_buf, sizeof(send_buf));

	MSG_WriteLong (&senddata, -1);	// -1 sequence means out of band
	SZ_Write (&senddata, data, length);

// send the datagram
	if (NOT_DEMOPLAYBACK)	// zoid, no input in demo playback mode
		NET_SendPacket (senddata.cursize, senddata.data, adr);
}

/*
===============
Netchan_OutOfBandPrint

Sends a text message in an out-of-band datagram
================
*/
void Netchan_OutOfBandPrint (const netadr_t *adr, const char *format, ...)
{
	va_list		argptr;
	static char	string[8192];

	va_start (argptr, format);
	q_vsnprintf (string, sizeof (string), format, argptr);
	va_end (argptr);

	Netchan_OutOfBand (adr, strlen(string), (byte *)string);
}


/*
==============
Netchan_Setup

called to open a channel to a remote system
==============
*/
void Netchan_Setup (netchan_t *chan, const netadr_t *adr)
{
	memset (chan, 0, sizeof(*chan));

	chan->remote_address = *adr;
	chan->last_received = realtime;

	SZ_Init (&chan->message, chan->message_buf, sizeof(chan->message_buf));
	chan->message.allowoverflow = true;

	chan->rate = 1.0/2500;
}


/*
===============
Netchan_CanPacket

Returns true if the bandwidth choke isn't active
================
*/
#define	MAX_BACKUP	200
qboolean Netchan_CanPacket (const netchan_t *chan)
{
	if (chan->cleartime < realtime + MAX_BACKUP*chan->rate)
		return true;
	return false;
}


/*
===============
Netchan_CanReliable

Returns true if the bandwidth choke isn't 
================
*/
qboolean Netchan_CanReliable (const netchan_t *chan)
{
	if (chan->reliable_length)
		return false;			// waiting for ack
	return Netchan_CanPacket (chan);
}


/*
===============
Netchan_Transmit

tries to send an unreliable message to a connection, and handles the
transmition / retransmition of the reliable messages.

A 0 length will still generate a packet and deal with the reliable messages.
================
*/
void Netchan_Transmit (netchan_t *chan, int length, byte *data)
{
	sizebuf_t	senddata;
	byte		send_buf[MAX_MSGLEN + PACKET_HEADER];
	qboolean	send_reliable;
	unsigned int	w1, w2;
	int		i;

// check for message overflow
	if (chan->message.overflowed)
	{
		chan->fatal_error = true;
		Con_Printf ("%s:Outgoing message overflow\n", NET_AdrToString (&chan->remote_address));
		return;
	}

// if the remote side dropped the last reliable message, resend it
	send_reliable = false;

	if (chan->incoming_acknowledged > chan->last_reliable_sequence &&
	    chan->incoming_reliable_acknowledged != chan->reliable_sequence)
		send_reliable = true;

// if the reliable transmit buffer is empty, copy the current message out
	if (!chan->reliable_length && chan->message.cursize)
	{
		memcpy (chan->reliable_buf, chan->message_buf, chan->message.cursize);
		chan->reliable_length = chan->message.cursize;
		chan->message.cursize = 0;
		chan->reliable_sequence ^= 1;
		send_reliable = true;
	}

// write the packet header
	SZ_Init (&senddata, send_buf, sizeof(send_buf));

	w1 = chan->outgoing_sequence | (send_reliable<<31);
	w2 = chan->incoming_sequence | (chan->incoming_reliable_sequence<<31);

	chan->outgoing_sequence++;

	MSG_WriteLong (&senddata, w1);
	MSG_WriteLong (&senddata, w2);

// copy the reliable message to the packet first
	if (send_reliable)
	{
		SZ_Write (&senddata, chan->reliable_buf, chan->reliable_length);
		chan->last_reliable_sequence = chan->outgoing_sequence;
	}

// add the unreliable part if space is available
	if (senddata.maxsize - senddata.cursize >= length)
		SZ_Write (&senddata, data, length);

// send the datagram
	i = chan->outgoing_sequence & (MAX_LATENT-1);
	chan->outgoing_size[i] = senddata.cursize;
	chan->outgoing_time[i] = realtime;

	if (NOT_DEMOPLAYBACK)	// zoid, no input in demo playback mode
		NET_SendPacket (senddata.cursize, senddata.data, &chan->remote_address);

	if (chan->cleartime < realtime)
		chan->cleartime = realtime + senddata.cursize*chan->rate;
	else
		chan->cleartime += senddata.cursize*chan->rate;

	if (showpackets.integer)
	{
		Con_Printf ("--> s=%i(%i) a=%i(%i) %i\n",
				chan->outgoing_sequence,
				send_reliable,
				chan->incoming_sequence,
				chan->incoming_reliable_sequence,
				senddata.cursize);
	}
}


/*
=================
Netchan_Process

called when the current net_message is from remote_address
modifies net_message so that it points to the packet payload
=================
*/
qboolean Netchan_Process (netchan_t *chan)
{
	unsigned int	sequence, sequence_ack;
	int	reliable_ack, reliable_message;

	if (NOT_DEMOPLAYBACK && !NET_CompareAdr (&net_from, &chan->remote_address))
	{
		return false;
	}

// get sequence numbers
	MSG_BeginReading ();
	sequence = MSG_ReadLong ();
	sequence_ack = MSG_ReadLong ();

	reliable_message = (int)(sequence >> 31);
	reliable_ack = (int)(sequence_ack >> 31);

	sequence &= ~(1<<31);
	sequence_ack &= ~(1<<31);

	if (showpackets.integer)
	{
		Con_Printf ("<-- s=%u(%i) a=%u(%i) %i\n",
				sequence,
				reliable_message,
				sequence_ack,
				reliable_ack,
				net_message.cursize);
	}

// get a rate estimation
#if 0
	if (chan->outgoing_sequence - sequence_ack < MAX_LATENT)
	{
		int	i;
		double	time, rate;

		i = sequence_ack & (MAX_LATENT - 1);
		time = realtime - chan->outgoing_time[i];
		time -= 0.1;	// subtract 100 ms
		if (time <= 0)
		{	// gotta be a digital link for <100 ms ping
			if (chan->rate > 1.0/5000)
				chan->rate = 1.0/5000;
		}
		else
		{
			if (chan->outgoing_size[i] < 512)
			{	// only deal with small messages
				rate = chan->outgoing_size[i]/time;
				if (rate > 5000)
					rate = 5000;
				rate = 1.0/rate;
				if (chan->rate > rate)
					chan->rate = rate;
			}
		}
	}
#endif

//
// discard stale or duplicated packets
//
	if (sequence <= (unsigned int) chan->incoming_sequence)
	{
		if (showdrop.integer)
		{
			Con_Printf ("%s:Out of order packet %u at %i\n",
					NET_AdrToString (&chan->remote_address),
					sequence,
					chan->incoming_sequence);
		}
		return false;
	}

//
// dropped packets don't keep the message from being used
//
	net_drop = sequence - (chan->incoming_sequence+1);
	if (net_drop > 0)
	{
		chan->drop_count += 1;

		if (showdrop.integer)
		{
			Con_Printf ("%s:Dropped %u packets at %u\n",
					NET_AdrToString (&chan->remote_address),
					sequence - (unsigned int)(chan->incoming_sequence + 1),
					sequence);
		}
	}

//
// if the current outgoing reliable message has been acknowledged
// clear the buffer to make way for the next
//
	if (reliable_ack == chan->reliable_sequence)
		chan->reliable_length = 0;	// it has been received

//
// if this message contains a reliable message, bump incoming_reliable_sequence
//
	chan->incoming_sequence = sequence;
	chan->incoming_acknowledged = sequence_ack;
	chan->incoming_reliable_acknowledged = reliable_ack;
	if (reliable_message)
		chan->incoming_reliable_sequence ^= 1;

//
// the message can now be read from the current message pointer
// update statistics counters
//
	chan->frame_latency = chan->frame_latency*OLD_AVG + (chan->outgoing_sequence-sequence_ack)*(1.0-OLD_AVG);
	chan->frame_rate = chan->frame_rate*OLD_AVG + (realtime-chan->last_received)*(1.0-OLD_AVG);
	chan->good_count += 1;

	chan->last_received = realtime;

	return true;
}

