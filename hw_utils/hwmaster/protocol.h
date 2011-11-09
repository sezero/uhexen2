/*
	protocol.h
	communications protocols

	$Id$
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

#define	A2S_ECHO		'g'	// echo back a message

#define	S2C_CHALLENGE		'c'

#define	S2C_CONNECTION		'j'
#define	A2A_PING		'k'	// respond with an A2A_ACK
#define	A2A_ACK			'l'	// general acknowledgement without info
#define	A2A_NACK		'm'	// [+ comment] general failure
#define	A2C_PRINT		'n'	// print a message on client

#define	S2M_HEARTBEAT		'a'	// + serverinfo + userlist + fraglist
#define	A2C_CLIENT_COMMAND	'B'	// + command line
#define	S2M_SHUTDOWN		'C'

#define	M2C_SERVERLST		'd'	// + \n + hw server port list

#endif	/* __HWM_PROTOCOL_H */

