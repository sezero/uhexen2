/*
 * hwmaster.c
 * main master server program
 *
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1999,2004  contributors of the QuakeForge project
 * Copyright (C) 2005-2012  O.Sezer <sezero@users.sourceforge.net>
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
#include "compiler.h"
#include "arch_def.h"
#include "net_sys.h"
#include "defs.h"

#if DO_USERDIRS
char	filters_file[MAX_OSPATH];
#else
#define	filters_file	FILTERS_FILE
#endif


/*
==============================================================================

NET FILTER

==============================================================================
*/

typedef struct filter_s
{
	netadr_t from;
	netadr_t to;
	struct filter_s *next;
	struct filter_s *previous;
} filter_t;

static filter_t	*filter_list = NULL;


static void FL_Remove (filter_t *filter)
{
	if (filter->previous)
		filter->previous->next = filter->next;
	if (filter->next)
		filter->next->previous = filter->previous;

	filter->next = NULL;
	filter->previous = NULL;

	if (filter_list == filter)
		filter_list = NULL;
}

static void FL_Clear (void)
{
	filter_t *filter;

	for (filter = filter_list ; filter ; )
	{
		if (filter)
		{
			filter_t *next = filter->next;
			FL_Remove(filter);
			free(filter);
			filter = next;
		}
	}

	filter_list = NULL;
}

static filter_t *FL_New (const netadr_t *adr1, const netadr_t *adr2)
{
	filter_t *filter;

	filter = (filter_t *)malloc(sizeof(filter_t));
	memcpy(&filter->from, adr1, sizeof(netadr_t));
	memcpy(&filter->to, adr2, sizeof(netadr_t));

	return filter;
}

static void FL_Add (filter_t *filter)
{
	filter->next = filter_list;
	filter->previous = NULL;
	if (filter_list)
		filter_list->previous = filter;
	filter_list = filter;
}

static filter_t *FL_Find (const netadr_t *adr)
{
	filter_t *filter;

	for (filter = filter_list ; filter ; filter = filter->next)
	{
		if (NET_CompareBaseAdr(&filter->from, adr))
			return filter;
	}

	return NULL;
}


static int argv_index_add;

static void FL_FilterAdd (void)
{
	filter_t	*filter;
	netadr_t	to, from;

	if (Cmd_Argc() < 4 + argv_index_add)
	{
		printf("Invalid command parameters. Usage:\nfilter add x.x.x.x:port x.x.x.x:port\n\n");
		return;
	}

	NET_StringToAdr(Cmd_Argv(2+argv_index_add), &from);
	NET_StringToAdr(Cmd_Argv(3+argv_index_add), &to);

	if (to.port == 0)
		from.port = htons(PORT_SERVER);

	if (from.port == 0)
		from.port = htons(PORT_SERVER);

	if (!(filter = FL_Find(&from)))
	{
		printf("Added filter %s\t\t%s\n", Cmd_Argv(2+argv_index_add), Cmd_Argv(3+argv_index_add));

		filter = FL_New(&from, &to);
		FL_Add(filter);
	}
	else
	{
		printf("%s already defined\n\n", Cmd_Argv(2+argv_index_add));
	}
}

static void FL_FilterRemove (void)
{
	filter_t	*filter;
	netadr_t	from;

	if (Cmd_Argc() < 3 + argv_index_add)
	{
		printf("Invalid command parameters. Usage:\nfilter remove x.x.x.x:port\n\n");
		return;
	}

	NET_StringToAdr(Cmd_Argv(2+argv_index_add), &from);

	if ( (filter = FL_Find(&from)) )
	{
		printf("Removed %s\n\n", Cmd_Argv(2+argv_index_add));

		FL_Remove(filter);
		free(filter);
	}
	else
	{
		printf("Cannot find %s\n\n", Cmd_Argv(2+argv_index_add));
	}
}

static void FL_FilterList (void)
{
	filter_t	*filter;

	for (filter = filter_list ; filter ; filter = filter->next)
	{
		printf("%s", NET_AdrToString(&filter->from));
		printf("\t\t%s\n", NET_AdrToString(&filter->to));
	}

	if (filter_list == NULL)
		printf("No filter\n");

	printf("\n");
}

static void FL_FilterClear (void)
{
	printf("Removed all filters\n\n");
	FL_Clear();
}


static void FL_Filter_f (void)
{
	argv_index_add = 0;

	if (!strcmp(Cmd_Argv(1), "add"))
	{
		FL_FilterAdd();
	}
	else if (!strcmp(Cmd_Argv(1), "remove"))
	{
		FL_FilterRemove();
	}
	else if (!strcmp(Cmd_Argv(1), "clear"))
	{
		FL_FilterClear();
	}
	else if (Cmd_Argc() == 3)
	{
		argv_index_add = -1;
		FL_FilterAdd();
	}
	else if (Cmd_Argc() == 2)
	{
		argv_index_add = -1;
		FL_FilterRemove();
	}
	else
	{
		FL_FilterList();
	}
}

static void SV_WriteFilterList (void)
{
	FILE	*filters;
	filter_t	*filter;

	if ( (filters = fopen(filters_file,"wt")) )
	{
		if (filter_list == NULL)
		{
			fclose(filters);
			return;
		}

		for (filter = filter_list ; filter ; filter = filter->next)
		{
			fprintf(filters, "%s", NET_AdrToString(&filter->from));
			fprintf(filters, " %s\n", NET_AdrToString(&filter->to));
		}
		fclose(filters);
	}
}

static void SV_Filter (void)
{
	filter_t	*filter;
	netadr_t	filter_adr;
	int		hold_port;

	hold_port = net_from.port;

	NET_StringToAdr("127.0.0.1:26950", &filter_adr);

	if (NET_CompareBaseAdr(&net_from, &filter_adr))
	{
		NET_StringToAdr("0.0.0.0:26950", &filter_adr);
		if (!NET_CompareBaseAdr(&net_local_adr, &filter_adr))
		{
			memcpy(&net_from, &net_local_adr, sizeof(netadr_t));
			net_from.port = hold_port;
		}
		return;
	}

	/* if no compare with filter list */
	if ((filter = FL_Find(&net_from)) != NULL)
	{
		memcpy(&net_from, &filter->to, sizeof(netadr_t));
		net_from.port = hold_port;
	}
}


/*
==============================================================================

SERVER LISTS HANDLING

==============================================================================
*/

typedef struct server_s
{
	netadr_t	ip;
	struct	server_s *next;
	struct	server_s *previous;
	double	timeout;
} server_t;

server_t *sv_list = NULL;


static void SVL_Remove (server_t *sv)
{
	if (sv_list == sv)
		sv_list = sv->next;

	if (sv->previous)
		sv->previous->next = sv->next;
	if (sv->next)
		sv->next->previous = sv->previous;

	sv->next = NULL;
	sv->previous = NULL;
}

static void SVL_Clear (void)
{
	server_t *sv;

	for (sv = sv_list ; sv ; )
	{
		if (sv)
		{
			server_t *next = sv->next;
			SVL_Remove(sv);
			free(sv);
			sv = next;
		}
	}

	sv_list = NULL;
	printf("Cleared the server list\n\n");
}

static server_t *SVL_New (const netadr_t *adr)
{
	server_t *sv;

	sv = (server_t *)malloc(sizeof(server_t));
	memset(sv, 0, sizeof(server_t));
	memcpy(&sv->ip, adr, sizeof(netadr_t));

	return sv;
}

static void SVL_Add (server_t *sv)
{
	sv->next = sv_list;
	sv->previous = NULL;
	if (sv_list)
		sv_list->previous = sv;
	sv_list = sv;
}

static server_t *SVL_Find (const netadr_t *adr)
{
	server_t *sv;

	for (sv = sv_list ; sv ; sv = sv->next)
	{
		if (NET_CompareAdr(&sv->ip, adr))
			return sv;
	}

	return NULL;
}

static void SVL_ServerList_f (void)
{
	server_t *sv;

	for (sv = sv_list ; sv ; sv = sv->next)
		printf("\t%s\n", NET_AdrToString(&sv->ip));
}


/*
==============================================================================

MASTER SERVER PACKET HANDLING

==============================================================================
*/

static void SV_AnalysePacket (void)
{
	byte	buf[16];
	byte	*p, *data;
	int	i, size, rsize;

	printf ("%s >> unknown packet:\n", NET_AdrToString(&net_from));

	data = net_message.data;
	size = net_message.cursize;

	for (p = data; (rsize = q_min(size - (int)(p - data), 16)); p += rsize)
	{
		printf ("%04X:", (unsigned) (p - data));
		memcpy (buf, p, rsize);
		for (i = 0; i < rsize; i++)
		{
			printf (" %02X", buf[i]);
			if (buf[i] < ' ' || buf [i] > '~')
				buf[i] = '.';
		}
		printf ("%*.*s\n", 1 + (16 - rsize) * 3 + rsize, rsize, buf);
	}
}

static void Mst_SendList (void)
{
	byte		buf[MAX_DATAGRAM];
	sizebuf_t	msg;
	server_t	*sv;

	SZ_Init (&msg, buf, sizeof(buf));
	msg.allowoverflow = true;

	MSG_WriteByte(&msg,255);
	MSG_WriteByte(&msg,255);
	MSG_WriteByte(&msg,255);
	MSG_WriteByte(&msg,255);
	MSG_WriteByte(&msg,255);
	MSG_WriteByte(&msg,M2C_SERVERLST);
	MSG_WriteByte(&msg,'\n');

	for (sv = sv_list ; sv ; sv = sv->next)
	{
		MSG_WriteByte(&msg,sv->ip.ip[0]);
		MSG_WriteByte(&msg,sv->ip.ip[1]);
		MSG_WriteByte(&msg,sv->ip.ip[2]);
		MSG_WriteByte(&msg,sv->ip.ip[3]);
		MSG_WriteShort(&msg,sv->ip.port);
	}

	NET_SendPacket(msg.cursize,msg.data,&net_from);
}

static void Mst_Packet (void)
{
	char		msg;
	server_t	*sv;

	msg = net_message.data[1];

	switch (msg)
	{
	case A2A_PING:
		SV_Filter();
		printf("%s >> A2A_PING\n", NET_AdrToString(&net_from));
		if (!(sv = SVL_Find(&net_from)))
		{
			sv = SVL_New(&net_from);
			SVL_Add(sv);
		}
		sv->timeout = Sys_DoubleTime();
		break;

	case S2M_HEARTBEAT:
		SV_Filter();
		printf("%s >> S2M_HEARTBEAT\n", NET_AdrToString(&net_from));
		if (!(sv = SVL_Find(&net_from)))
		{
			sv = SVL_New(&net_from);
			SVL_Add(sv);
		}
		sv->timeout = Sys_DoubleTime();
		break;

	case S2M_SHUTDOWN:
		SV_Filter();
		printf("%s >> S2M_SHUTDOWN\n", NET_AdrToString(&net_from));
		if ( (sv = SVL_Find(&net_from)) )
		{
			SVL_Remove(sv);
			free(sv);
		}
		break;

	case S2C_CHALLENGE:
		printf("%s >> ", NET_AdrToString(&net_from));
		printf("Gamespy server list request\n");
		Mst_SendList();
		break;

	default:
		{
			byte		*p;
			p = net_message.data;

			if (p[0] == 0 && p[1] == 'y')
			{
				printf("%s >> ", NET_AdrToString(&net_from));
				printf("Pingtool server list request\n");
				Mst_SendList();
			}
			else
			{
				SV_AnalysePacket();
			}
		}
	}
}

static void SV_ReadPackets (void)
{
	while ( NET_GetPacket() )
	{
		Mst_Packet();
	}
}


/*
==============================================================================

SERVER FRAME

==============================================================================
*/

static void SV_GetConsoleCommands (void)
{
	char	*cmd;

	while (1)
	{
		cmd = Sys_ConsoleInput ();
		if (!cmd)
			break;
		Cbuf_AddText (cmd);
	}
}

#define SV_TIMEOUT 450

static void SV_TimeOut(void)
{
	double t = Sys_DoubleTime();
	server_t *sv;
	server_t *next;

	if (sv_list == NULL)
		return;

	/* remove servers that haven't sent a heartbeat for some time */
	for (sv = sv_list ; sv ; )
	{
		if (sv->timeout + SV_TIMEOUT < t)
		{
			next = sv->next;
			printf("%s timed out\n",NET_AdrToString(&sv->ip));
			SVL_Remove(sv);
			free(sv);
			sv = next;
		}
		else
		{
			sv = sv->next;
		}
	}
}

void SV_Frame (void)
{
	SV_GetConsoleCommands ();

	Cbuf_Execute ();

	SV_TimeOut();

	NET_CheckReadTimeout (0, 10000);	/* FIXME: check return code ? */

	SV_ReadPackets();
}


/*
==============================================================================

INIT and SHUTDOWN

==============================================================================
*/

static void SV_Shutdown (void)
{
	NET_Shutdown ();
	SV_WriteFilterList();
}

static void SV_Quit_f (void)
{
	printf ("Shutting down.\n");

	SV_Shutdown ();
	Sys_Quit();
}

void SV_InitNet (void)
{
	int	port;
	int	p;
	FILE	*filters;
	char	str[64];

	port = PORT_MASTER;

	p = COM_CheckParm ("-port");
	if (p && p < com_argc-1)
	{
		port = atoi(com_argv[p+1]);
		printf ("Port: %i\n", port);
	}
	NET_Init (port);

	/* Add filters */
	if ( (filters = fopen(filters_file,"rt")) )
	{
		while (fgets(str, sizeof(str), filters))
		{
			Cbuf_AddText("filter add ");
			Cbuf_AddText(str);
			Cbuf_AddText("\n");
		}

		fclose(filters);
	}

	Cmd_AddCommand("clear", SVL_Clear);
	Cmd_AddCommand("list", SVL_ServerList_f);
	Cmd_AddCommand("filter", FL_Filter_f);
	Cmd_AddCommand("quit", SV_Quit_f);
}

