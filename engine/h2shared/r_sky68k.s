**
** Quake for AMIGA
** r_sky.c assembler implementations by Frank Wille <frank@phoenix.owl.de>
**

		XREF    _r_skymade
		XREF    _bottommask
		XREF    _bottomsky
		XREF    _newsky
		XREF    _skytime
		XREF    _skyspeed


		XDEF    _R_MakeSky

SKYSHIFT                =       7
SKYSIZE                 =       (1 << SKYSHIFT)
SKYMASK                 =       (SKYSIZE - 1)


******************************************************************************
*
*       void R_MakeSky (void)
*
*       This function contains unaligned accesses to longwords (supported
*       by 68K)
*
******************************************************************************

		cnop    0,4
_R_MakeSky

*        xshift = skytime*skyspeed;
*        yshift = skytime*skyspeed;
*
*        if ((xshift == xlast) && (yshift == ylast))
*                return;
*
*        xlast = xshift;
*        ylast = yshift;
*
*        pnewsky = (unsigned *)&newsky[0];

		movem.l d2-d5/a2,-(sp)
		lea     _bottommask,a1
		lea     _bottomsky,a2
		fmove.s _skytime,fp0            ;xshift = skytime*skyspeed
		fmul.s  _skyspeed,fp0
		fmove.l fp0,d0
		cmp.l   _xlast,d0               ;if (xshift==xlast)&&(yshift==ylast)
		bne.b   .cont
		cmp.l   _ylast,d0
		beq.b   .exit                   ;return
.cont
		move.l  d0,_xlast               ;_xlast = xshift
		move.l  d0,_ylast               ;_ylast = yshift
		lea     _newsky,a0              ;pnewsky = (unsigned *)&newsky[0]

*        for (y=0 ; y<SKYSIZE ; y++)
*        {
*                baseofs = ((y+yshift) & SKYMASK) * 131;
*
*                for (x=0 ; x<SKYSIZE ; x += 4)
*                {
*                        ofs = baseofs + ((x+xshift) & SKYMASK);
*
*                // PORT: unaligned dword access to bottommask and bottomsky
*
*                        *pnewsky = (*(pnewsky + (128 / sizeof (unsigned))) &
*                                                *(unsigned *)&bottommask[ofs]) |
*                                                *(unsigned *)&bottomsky[ofs];
*                        pnewsky++;
*                }
*
*                pnewsky += 128 / sizeof (unsigned);
*        }
*
*        r_skymade = 1;


		moveq   #0,d1
.loop
		move.l  d1,d2
		add.l   d0,d2                   ;d2 = y+yshift
		and.l   #SKYMASK,d2             ;d2 = (y+yshift)&SKYMASK
		muls    #131,d2                 ;baseofs = d2 * 131
		moveq   #0,d3
.loop2
		move.l  d3,d4
		add.l   d0,d4                   ;d3 = x+xshift
		and.l   #SKYMASK,d4             ;d3 = (x+xshift)&SKYMASK
		add.l   d2,d4                   ;ofs = baseofs + d3
		move.l  0(a1,d4.l),d5           ;*(unsigned *)&bottommask[ofs]
		and.l   128(a0),d5              ;& (*(pnewsky + (128/sizeof(unsigned)))
		or.l    0(a2,d4.l),d5           ;| *(unsigned *)&bottomsky[ofs]
		move.l  d5,(a0)+                ;*pnewsky = d5; pnewsky++
		addq.l  #4,d3                   ;x += 4
		cmp.l   #SKYSIZE,d3
		blt.b   .loop2
		lea     128(a0),a0              ;pnewsky += 128/sizeof(unsigned)
		addq.l  #1,d1                   ;y++
		cmp.l   #SKYSIZE,d1
		blt.b   .loop
		move.b  #1,_r_skymade           ;r_skymade = 1
.exit
		movem.l (sp)+,d2-d5/a2
		rts

_xlast          dc.l    -1
_ylast          dc.l    -1
