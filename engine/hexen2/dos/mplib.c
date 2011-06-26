/*
	mplib.c
	support library for dosquake MPATH network driver.
	from quake1 source with minor adaptations for uhexen2.

	$Id$

	Copyright (C) 1996-1997  Id Software, Inc.

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		59 Temple Place - Suite 330
		Boston, MA  02111-1307, USA

*/

#include <dpmi.h>

/*#include "types.h"*/
typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;

/*#include "mgenord.h"*/
#define	MGENVXD_REGISTER_ORD		1
#define	MGENVXD_GETMEM_ORD		2
#define	MGENVXD_DEREGISTER_ORD		3
#define	MGENVXD_WAKEUP_ORD		4
#define	MGENVXD_MAKEDQS_ORD		5

/* Virtual 8086 API Ordinals */
#define	V86API_GETSELECTOR16_ORD	(1)
#define	V86API_GETSELECTOR32_ORD	(2)
#define	V86API_GETFLAT32_ORD		(3)
#define	V86API_MOVERP_ORD		(6)
#define	V86API_MOVEPR_ORD		(7)
#define	V86API_POST_ORD			(8)
#define	V86API_INIT_ORD			(9)
#define	V86API_UNINIT_ORD		(10)
#define	V86API_INSERTKEY_ORD		(11)
#define	V86API_REMOVEHOTKEY_ORD		(12)
#define	V86API_INSTALLHOTKEY_ORD	(13)
#define	V86API_HOOKINT48_ORD		(14)
#define	V86API_WAKEUPDLL_ORD		(15)

#define	DPMIAPI_GETFLAT32_ORD		(1)
#define	DPMIAPI_POST_WINDOWS_ORD	(2)
/* these are DPMI functions.  Make sure they don't clash with the
 * other MGENVXD_XXXX functions above, or the DPMI functions! */
#define	MGENVXD_GETQUEUECTR_ORD		6
#define	MGENVXD_MOVENODE_ORD		7
#define	MGENVXD_GETNODE_ORD		8
#define	MGENVXD_FLUSHNODE_ORD		9
#define	MGENVXD_MCOUNT_ORD		10
#define	MGENVXD_MASTERNODE_ORD		11
#define	MGENVXD_SANITYCHECK_ORD		12
#define	MGENVXD_WAKEUPDLL_ORD		13
#define	MGENVXD_WAIT_ORD		14

#define	HWND_OFFSET			(0)
#define	UMSG_OFFSET			(1)
#define	SIZEREQUEST_OFFSET		(2)
#define	HVXD_OFFSET			(3)
#define	DATUM_OFFSET			(4)
#define	SLOT_OFFSET			(5)
#define	SIZEGIVEN_OFFSET		(6)
#define	SELECTOR32_OFFSET		(7)
#define	SELECTOR16_OFFSET		(8)

/*#include "magic.h"*/
#define	MGENVXD_DEVICE_ID		0x18AA

/*#include "rtq.h"*/
typedef struct rtq_node
{
	struct rtq_node	*self;	/* Ring zero address of this node */
	struct rtq_node	*left;	/* Ring zero address of preceding node */
	struct rtq_node	*right;	/* Ring zero address of succeding node */

	BYTE	*rtqDatum;	/* Ring 3 Datum of Buffer (start of preface) */
	BYTE	*rtqInsert;	/* Ring 3 insertion position */
	WORD	rtqLen;		/* Length of buffer, excluding preface */
	WORD	rtqUpCtr;	/* Up Counter of bytes used so far */
	WORD	rtqQCtr;	/* number of nodes attached */
	WORD	padding;	/* DWORD alignment */
} RTQ_NODE;

typedef struct rtq_param_movenode
{
	WORD	rtqFromDQ;
	WORD	rtqToDQ;
} RTQ_PARAM_MOVENODE;

#if 0
RTQ_NODE	*rtq_fetch(RTQ_NODE*, RTQ_NODE*);	/* To, From */
#endif

#ifdef __cplusplus
extern "C" {
#endif
int _int86 (int vector, __dpmi_regs *iregs, __dpmi_regs *oregs);
#ifdef __cplusplus
}
#endif
#undef	int386
#define	int386		_int86

#define	CHUNNEL_INT	0x48

static void PostWindowsMessage (void)
{
	__dpmi_regs	regs;

	regs.d.eax = DPMIAPI_POST_WINDOWS_ORD << 16 | MGENVXD_DEVICE_ID;
	regs.d.ebx = 0;
	regs.d.ecx = 0;
	int386(CHUNNEL_INT, &regs, &regs);
}

static int MGenGetQueueCtr (int qNo)
{
	__dpmi_regs	regs;

	regs.d.eax = MGENVXD_GETQUEUECTR_ORD << 16 | MGENVXD_DEVICE_ID;
	regs.d.ebx = qNo;
	int386(CHUNNEL_INT, &regs, &regs);

	return regs.d.eax;
}

static RTQ_NODE *MGenMoveTo (int qFrom, int qTo)
{
	__dpmi_regs	regs;

	regs.d.eax = MGENVXD_MOVENODE_ORD << 16 | MGENVXD_DEVICE_ID;
	regs.d.ebx = qFrom;
	regs.d.ecx = qTo;
	int386(CHUNNEL_INT, &regs, &regs);

	return (RTQ_NODE *) regs.d.eax;
}

static RTQ_NODE *MGenGetNode (int q)
{
	__dpmi_regs	regs;

	regs.d.eax = MGENVXD_GETNODE_ORD << 16 | MGENVXD_DEVICE_ID;
	regs.d.ebx = q;
	int386(CHUNNEL_INT, &regs, &regs);

	return (RTQ_NODE *) regs.d.eax;
}

#if 0	/* unused */
static RTQ_NODE *MGenGetMasterNode (unsigned *size)
{
	__dpmi_regs	regs;

	regs.d.eax = MGENVXD_MASTERNODE_ORD << 16 | MGENVXD_DEVICE_ID;
	int386(CHUNNEL_INT, &regs, &regs);
	*size = regs.d.ecx;

	return (RTQ_NODE *) regs.d.eax;
}

static RTQ_NODE *MGenFlushNodes (int qFrom, int qTo)
{
	__dpmi_regs	regs;

	regs.d.eax = MGENVXD_FLUSHNODE_ORD << 16 | MGENVXD_DEVICE_ID;
	regs.d.ebx = qFrom;
	regs.d.ecx = qTo;
	int386(CHUNNEL_INT, &regs, &regs);

	return (RTQ_NODE *) regs.d.eax;
}

static int MGenMCount (unsigned lowerOrderBits, unsigned upperOrderBits)
{
	__dpmi_regs	regs;

	regs.d.eax = MGENVXD_MCOUNT_ORD << 16 | MGENVXD_DEVICE_ID;
	regs.d.ebx = lowerOrderBits;
	regs.d.ecx = upperOrderBits;
	int386(CHUNNEL_INT, &regs, &regs);

	return regs.d.eax;
}

static int MGenSanityCheck (void)
{
	__dpmi_regs	regs;

	regs.d.eax = MGENVXD_SANITYCHECK_ORD << 16 | MGENVXD_DEVICE_ID;
	int386(CHUNNEL_INT, &regs, &regs);

	return regs.d.eax;
}

static int MGenWait (void)
{
	__dpmi_regs	regs;

	regs.d.eax = MGENVXD_WAIT_ORD << 16 | MGENVXD_DEVICE_ID;
	int386(CHUNNEL_INT, &regs, &regs);
	return regs.d.eax;
}

static void MGenWakeupDll (void)
{
	__dpmi_regs	regs;

	regs.d.eax = MGENVXD_WAKEUPDLL_ORD << 16 | MGENVXD_DEVICE_ID;
	int386(CHUNNEL_INT, &regs, &regs);
}
#endif	/* end of unused */

