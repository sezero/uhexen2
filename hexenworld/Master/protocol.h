/*
	protocol.h
	communications protocols

	$Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Master/protocol.h,v 1.2 2007-03-14 21:04:15 sezero Exp $
*/

#ifndef __HWM_PROTOCOL_H
#define __HWM_PROTOCOL_H

#define	PORT_MASTER	26900
#define	PORT_SERVER	26950

//=========================================

// out of band message id bytes

// M = master, S = server, C = client, A = any
// the second character will always be \n if the message isn't a single
// byte long (?? not true anymore?)

#define	S2C_CHALLENGE		'c'
#define	S2C_CONNECTION		'j'
#define	A2A_PING		'k'	// respond with an A2A_ACK
#define	A2A_ACK			'l'	// general acknowledgement without info
#define	A2A_NACK		'm'	// [+ comment] general failure
#define A2A_ECHO		'e'	// for echoing
#define	A2C_PRINT		'n'	// print a message on client

#define	S2M_HEARTBEAT		'a'	// + serverinfo + userlist + fraglist
#define	A2C_CLIENT_COMMAND	'B'	// + command line
#define	S2M_SHUTDOWN		'C'
#define	M2C_MASTER_REPLY	'd'	// + \n + hw server port list
//KS:
#define A2M_LIST		'o'
#define M2A_SENDLIST		'p'

#endif	/* __HWM_PROTOCOL_H */

