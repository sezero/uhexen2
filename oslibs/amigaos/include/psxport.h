#ifndef	PSXPORT_H
#define	PSXPORT_H

/*
**	$VER: psxport.h 2.1 (05.05.2000)
**
**	psxport.device definitions
**
**	(C) Copyright 1999-2000 Joseph Fenton. All rights reserved.
*/

#ifndef DEVICES_INPUTEVENT_H
#include "devices/inputevent.h"
#endif

/* extension command for psxport.device */

#define	GPD_COMMUNICATE	14
#define	GPD_SETPORTRATE	15

#define PXUB_BUSYWAIT	0
#define PXUB_ALWAYSOPEN	1

#define PXUF_BUSYWAIT	1
#define	PXUF_ALWAYSOPEN	2

/* all controllers */

#define	PSX_CLASS(e)	e.ie_Class
#define	PSX_SUBCLASS(e)	e.ie_SubClass
#define	PSX_BUTTONS(e)	e.ie_Code

/* class/subclass */

#define	PSX_CLASS_MOUSE		0x12
#define	PSX_CLASS_WHEEL		0x23
#define	PSX_CLASS_JOYPAD	0x41
#define	PSX_CLASS_ANALOG_MODE2	0x53
#define PSX_CLASS_LIGHTGUN	0x63
#define	PSX_CLASS_ANALOG	0x73
#define	PSX_CLASS_MULTITAP	0x80	/* will never be received */
#define	PSX_CLASS_ANALOG2	0xF3	/* only Mad Catz Dual Shock returns this */

#define	PSX_SUBCLASS_PSX	0x5A	/* all PSX controllers and cards return this */

/* note: all buttons are active low */

/* joypad */

#define	PSX_LEFT	0x8000
#define	PSX_DOWN	0x4000
#define	PSX_RIGHT	0x2000
#define	PSX_UP		0x1000
#define	PSX_START	0x0800
#define	PSX_R3		0x0400		/* R3 and L3 only if PSX_CLASS_ANALOG */
#define	PSX_L3		0x0200		/* or PSX_CLASS_ANALOG2 */
#define	PSX_SELECT	0x0100
#define	PSX_SQUARE	0x0080
#define	PSX_CROSS	0x0040
#define	PSX_CIRCLE	0x0020
#define	PSX_TRIANGLE	0x0010
#define	PSX_R1		0x0008
#define	PSX_L1		0x0004
#define	PSX_R2		0x0002
#define	PSX_L2		0x0001

#define	PSX_RIGHTX(e)	(e.ie_X>>8)&0xFF
#define	PSX_RIGHTY(e)	(e.ie_X)&0xFF

#define	PSX_LEFTX(e)	(e.ie_Y>>8)&0xFF
#define	PSX_LEFTY(e)	(e.ie_Y)&0xFF

/* mouse */

#define	PSX_LMB		PSX_R1
#define	PSX_RMB		PSX_L1

#define	PSX_MOUSEDX(e)	e.ie_X
#define	PSX_MOUSEDY(e)	e.ie_Y

/* wheel */

#define	PSX_A		PSX_CIRCLE
#define	PSX_B		PSX_TRIANGLE
#define	PSX_R		PSX_R1

#define	PSX_WHEEL(e)	(e.ie_X>>8)&0xFF
#define	PSX_I(e)	(e.ie_X)&0xFF
#define	PSX_II(e)	(e.ie_Y>>8)&0xFF
#define	PSX_L(e)	(e.ie_Y)&0xFF

/* Light Gun */

#define PSX_GUN_A	PSX_START
#define PSX_GUN_B	PSX_CROSS
#define PSX_GUN_TRIGGER	PSX_CIRCLE

#define PSX_GUN_X(e)	e.ie_X
#define PSX_GUN_Y(e)	e.ie_Y

/* SetPortRate data */

struct	PSXSetPortRate {
	WORD	SelDelay;
	WORD	CmdDelay;
	WORD	ClkDelay;
	WORD	DatDelay;
	};

#endif
