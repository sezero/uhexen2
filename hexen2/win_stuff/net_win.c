#include "quakedef.h"

#include "net_loop.h"
#include "net_dgrm.h"
#include "net_ser.h"

net_driver_t net_drivers[MAX_NET_DRIVERS] =
{
	{
	"Loopback",
	false,
	Loop_Init,
	Loop_Listen,
	Loop_SearchForHosts,
	Loop_Connect,
	Loop_CheckNewConnections,
	Loop_GetMessage,
	Loop_SendMessage,
	Loop_SendUnreliableMessage,
	Loop_CanSendMessage,
	Loop_CanSendUnreliableMessage,
	Loop_Close,
	Loop_Shutdown
	}
	,
	{
	"Datagram",
	false,
	Datagram_Init,
	Datagram_Listen,
	Datagram_SearchForHosts,
	Datagram_Connect,
	Datagram_CheckNewConnections,
	Datagram_GetMessage,
	Datagram_SendMessage,
	Datagram_SendUnreliableMessage,
	Datagram_CanSendMessage,
	Datagram_CanSendUnreliableMessage,
	Datagram_Close,
	Datagram_Shutdown
	}
};

int net_numdrivers = 2;


#include "net_wins.h"
#include "net_wipx.h"

net_landriver_t	net_landrivers[MAX_NET_DRIVERS] =
{
	{
	"TCPIP",
	false,
	0,
	WINS_Init,
	WINS_Shutdown,
	WINS_Listen,
	WINS_OpenSocket,
	WINS_CloseSocket,
	WINS_Connect,
	WINS_CheckNewConnections,
	WINS_Read,
	WINS_Write,
	WINS_Broadcast,
	WINS_AddrToString,
	WINS_StringToAddr,
	WINS_GetSocketAddr,
	WINS_GetNameFromAddr,
	WINS_GetAddrFromName,
	WINS_AddrCompare,
	WINS_GetSocketPort,
	WINS_SetSocketPort
	},
	{
	"IPX",
	false,
	0,
	WIPX_Init,
	WIPX_Shutdown,
	WIPX_Listen,
	WIPX_OpenSocket,
	WIPX_CloseSocket,
	WIPX_Connect,
	WIPX_CheckNewConnections,
	WIPX_Read,
	WIPX_Write,
	WIPX_Broadcast,
	WIPX_AddrToString,
	WIPX_StringToAddr,
	WIPX_GetSocketAddr,
	WIPX_GetNameFromAddr,
	WIPX_GetAddrFromName,
	WIPX_AddrCompare,
	WIPX_GetSocketPort,
	WIPX_SetSocketPort
	}

};

int net_numlandrivers = 2;
