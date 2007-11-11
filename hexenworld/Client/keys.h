/*
	keys.h
	key definitions and keyboard public functions

	$Id: keys.h,v 1.13 2007-11-11 13:17:44 sezero Exp $
*/

#ifndef __HX2_KEYS_H
#define __HX2_KEYS_H

//
// these are the key numbers that should be passed to Key_Event
//
#define	K_TAB			9
#define	K_ENTER			13
#define	K_ESCAPE		27
#define	K_SPACE			32

// normal keys should be passed as lowercased ascii

#define	K_BACKSPACE		127
#define	K_UPARROW		128
#define	K_DOWNARROW		129
#define	K_LEFTARROW		130
#define	K_RIGHTARROW		131

#define	K_ALT			132
#define	K_CTRL			133
#define	K_SHIFT			134
#define	K_F1			135
#define	K_F2			136
#define	K_F3			137
#define	K_F4			138
#define	K_F5			139
#define	K_F6			140
#define	K_F7			141
#define	K_F8			142
#define	K_F9			143
#define	K_F10			144
#define	K_F11			145
#define	K_F12			146
#define	K_INS			147
#define	K_DEL			148
#define	K_PGDN			149
#define	K_PGUP			150
#define	K_HOME			151
#define	K_END			152

#define K_PAUSE			255

//
// mouse buttons generate virtual keys
//
#define	K_MOUSE1		200
#define	K_MOUSE2		201	/* right mouse button			*/
#define	K_MOUSE3		202	/* middle mouse button			*/
#define	K_MWHEELUP		203	/* wheel-up as a virtual button		*/
#define	K_MWHEELDOWN		204	/* wheel-down as a virtual button	*/
#define	K_MOUSE4		205	/* thumb buttons			*/
#define	K_MOUSE5		206	/* thumb buttons			*/

//
// joystick buttons
//
#define	K_JOY1			207
#define	K_JOY2			208
#define	K_JOY3			209
#define	K_JOY4			210

//
// aux keys are for multi-buttoned joysticks to generate so they can use
// the normal binding process
//
#define	K_AUX1			211
#define	K_AUX2			212
#define	K_AUX3			213
#define	K_AUX4			214
#define	K_AUX5			215
#define	K_AUX6			216
#define	K_AUX7			217
#define	K_AUX8			218
#define	K_AUX9			219
#define	K_AUX10			220
#define	K_AUX11			221
#define	K_AUX12			222
#define	K_AUX13			223
#define	K_AUX14			224
#define	K_AUX15			225
#define	K_AUX16			226
#define	K_AUX17			227
#define	K_AUX18			228
#define	K_AUX19			229
#define	K_AUX20			230
#define	K_AUX21			231
#define	K_AUX22			232
#define	K_AUX23			233
#define	K_AUX24			234
#define	K_AUX25			235
#define	K_AUX26			236
#define	K_AUX27			237
#define	K_AUX28			238
#define	K_AUX29			239
#define	K_AUX30			240
#define	K_AUX31			241
#define	K_AUX32			242


#define	MAXCMDLINE	256

typedef enum {key_game, key_console, key_message, key_menu} keydest_t;

extern keydest_t	key_dest;
extern char *keybindings[256];
extern	int		key_count;			// incremented every key event
extern	int		key_lastpress;

extern char chat_buffer[];
extern	int chat_bufferlen;
extern	qboolean	chat_team;

void Key_Event (int key, qboolean down);
void Key_Init (void);
void Key_WriteBindings (FILE *f);
void Key_ClearStates (void);

void Key_SetBinding (int keynum, const char *binding);
const char *Key_KeynumToString (int keynum);

#endif	/* __HX2_KEYS_H */

