**
** Quake for AMIGA
** d_polyset.c assembler implementations by Frank Wille <frank@phoenix.owl.de>
** Adapted for Hexen II by Szilard Biro
**

;		INCLUDE	"quakedef68k.i"
R_SEAMFIXUP16	equ	32
R_NUMTRIANGLES	equ	24
R_PFINALVERTS	equ	20
R_PTRIANGLES	equ	16
R_SKINWIDTH	equ	8
R_PSKIN	equ	0
FV_SIZEOF_EXP	equ	5
PSPANP_SIZEOF_EXP	equ	5
PSPANP_SIZEOF	equ	32
PSPANP_COUNT	equ	8
ETAB_PLEV0	equ	8
ETAB_PLEV1	equ	12
ETAB_PLEV2	equ	16
ETAB_NUMRIGHTEDGES	equ	20
ETAB_PREV0	equ	24
ETAB_PREV1	equ	28
ETAB_PREV2	equ	32
ETAB_SIZEOF	equ	36
ETAB_NUMLEFTEDGES	equ	4
PTEMP_QUOTIENT	equ	0
PTEMP_REMAINDER	equ	4
MT_FACESFRONT	equ	0
MT_VERTINDEX	equ	4
MT_SIZEOF	equ	16
REFDEF_VRECTRIGHT	equ	40
REFDEF_VRECTBOTTOM	equ	44

		XREF    _acolormap
		XREF    _d_aspancount
		XREF    _errorterm
		XREF    _erroradjustup
		XREF    _erroradjustdown
		XREF    _d_countextrastep
		XREF    _ubasestep
		XREF    _r_affinetridesc
		XREF    _a_ststepxwhole
		XREF    _a_sstepxfrac
		XREF    _a_tstepxfrac
		XREF    _r_lstepx
		XREF    _r_lstepy
		XREF    _r_sstepx
		XREF    _r_sstepy
		XREF    _r_tstepx
		XREF    _r_tstepy
		XREF    _r_zistepx
		XREF    _r_zistepy
		XREF    _zspantable
		XREF    _skintable
		XREF    _r_p0
		XREF    _r_p1
		XREF    _r_p2
		XREF    _d_xdenom
		XREF    _d_pcolormap
		XREF    _d_scantable
		XREF    _d_viewbuffer
		XREF    _d_pedgespanpackage
		XREF    _d_pdest
		XREF    _d_pz
		XREF    _d_aspancount
		XREF    _d_ptex
		XREF    _d_sfrac
		XREF    _d_tfrac
		XREF    _d_light
		XREF    _d_zi
		XREF    _d_zwidth
		XREF    _d_pzbuffer
		XREF    _a_spans
		XREF    _pedgetable
		XREF    _edgetables
		XREF    _screenwidth
		XREF    _FloorDivMod
		XREF    _mainTransTable
		XREF    _transTable
		;XREF     _D_RasterizeAliasPolySmooth
		XREF    _adivtab
		XREF    _d_polysetdrawspans
		XREF    _r_refdef

		XDEF    _D_PolysetDrawSpans8
		XDEF    _D_PolysetDrawSpans8T
		XDEF    _D_PolysetDrawSpans8T2
		XDEF    _D_PolysetDrawSpans8T3
		XDEF    _D_PolysetDrawSpans8T5
		XDEF    _D_PolysetRecursiveTriangle
		XDEF    _D_PolysetRecursiveTriangleT
		XDEF    _D_PolysetRecursiveTriangleT2
		XDEF    _D_PolysetRecursiveTriangleT3
		XDEF    _D_PolysetRecursiveTriangleT5
		XDEF    _D_PolysetCalcGradients
		XDEF    _D_DrawNonSubdiv
		XDEF    _D_RasterizeAliasPolySmooth
		XDEF	_D_PolysetDrawFinalVerts
		XDEF	_D_PolysetDrawFinalVertsT
		XDEF	_D_PolysetDrawFinalVertsT2
		XDEF	_D_PolysetDrawFinalVertsT3
		XDEF	_D_PolysetDrawFinalVertsT5

ALIAS_ONSEAM            =       $20             ;must match the def. in r_shared.h



******************************************************************************
*
*       void D_PolysetDrawSpans8 (spanpackage_t *pspanpackage)
*
*       standard scan drawing function for alias models
*
******************************************************************************

		cnop    0,4
_D_PolysetDrawSpans8


*****   stackframe

		rsreset
.intregs        rs.l    11
		rs.l    1
.pspan          rs.l    1


****** prologue

		movem.l d2-d7/a2-a6,-(sp)
		move.l  .pspan(sp),a0
		move.l  _d_aspancount,d0
		move.l  _errorterm,d1
		move.l  _erroradjustup,d2
		move.l  _erroradjustdown,d3
		move.l  _d_countextrastep,a1
		move.l  _ubasestep,d5
		move.l  _acolormap,a3
		move.l  _r_zistepx,d4
		move.l  _a_ststepxwhole,a5
		move.l  _a_sstepxfrac,a4
		move.l  _a_tstepxfrac,d6
		swap    d6
		move.l  _r_lstepx,a6
		move.l  _r_affinetridesc+R_SKINWIDTH,d7
		move    d7,d6
.loop

*                lcount = d_aspancount - pspanpackage->count;
*
*                errorterm += erroradjustup;
*                if (errorterm >= 0)
*                {
*                        d_aspancount += d_countextrastep;
*                        errorterm -= erroradjustdown;
*                }
*                else
*                {
*                        d_aspancount += ubasestep;
*                }

		move.l  d0,d7
		sub.l   PSPANP_COUNT(a0),d7     ;lcount = d_aspancount-pspa...
		add.l   d2,d1                   ;errorterm += erroradjustup
		blt.b   .else                   ;if (errorterm >= 0)
		add.l   a1,d0                   ;d_aspancount += d_countextrastep
		sub.l   d3,d1                   ;errorterm -= error adjustdown
		bra.b   .next
.else
		add.l   d5,d0                   ;d_aspancount += ubasestep
.next
		subq.l  #1,d7
		blt.b   .loopend

*                        lpdest = pspanpackage->pdest;
*                        lptex = pspanpackage->ptex;
*                        lpz = pspanpackage->pz;
*                        lsfrac = pspanpackage->sfrac;
*                        ltfrac = pspanpackage->tfrac;
*                        llight = pspanpackage->light;
*                        lzi = pspanpackage->zi;

		movem.l d0-d3/d5/a0/a1,-(sp)
		move.l  (a0)+,d2                ;lpdest = pspanpackage->dest
		move.l  (a0)+,a1                ;lpz = pspanpackage->pz
		addq.l  #4,a0
		move.l  (a0)+,a2                ;lptex = pspanpackage->ptex
		move.l  (a0)+,d3                ;lsfrac = pspanpackage->sfrac
		move.l  (a0)+,d5                ;tsfrac = pspanpackage->tfrac
		swap    d5
		move.l  (a0)+,d1                ;llight = pspanpackage->light
		move.l  (a0)+,d0                ;lzi = pspanpackage->zi
		move.l  d2,a0
		moveq   #0,d2

****** main drawing loop

****** d0 = lzi
****** d1 = llight
****** d3 = lsfrac
****** d4 = r_zistepx
****** d5 = ltfrac
****** d6 = a_tstepxfrac [high] and r_affinetridesc.skinwidth [low]
****** d7 = lcount-1
****** a0 -> lpdest
****** a1 -> lpz
****** a2 -> lptex
****** a3 -> acolormap
****** a4 = a_sstepxfrac
****** a5 = a_ststepxwhole
****** a6 = r_lstepx

*                        do
*                        {
*                                if ((lzi >> 16) >= *lpz)
*                                {
*                                        *lpdest = ((byte *)acolormap)[*lptex + (llight & 0xFF00)];
*                                        *lpz = lzi >> 16;
*                                }
*                                lpdest++;
*                                lzi += r_zistepx;
*                                lpz++;
*                                llight += r_lstepx;
*                                lptex += a_ststepxwhole;
*                                lsfrac += a_sstepxfrac;
*                                lptex += lsfrac >> 16;
*                                lsfrac &= 0xFFFF;
*                                ltfrac += a_tstepxfrac;
*                                if (ltfrac & 0x10000)
*                                {
*                                        lptex += r_affinetridesc.skinwidth;
*                                        ltfrac &= 0xFFFF;
*                                }
*                        } while (--lcount);

.loop2
		swap    d0
		cmp     (a1)+,d0                ;if ((lzi >> 16) >= *lpz)
		blt.b   .cont
		move    d1,d2
		move.b  (a2),d2                 ;d3 = *lptex + (llight & $ff00)
		move.b  0(a3,d2.l),(a0)         ;*lpdest = ((byte *)acolormap[d2]
		move    d0,-2(a1)               ;*lpz = lzi >> 16
.cont
		addq.l  #1,a0                   ;lpdest++
		swap    d0
		add.l   d4,d0                   ;lzi += r_zistepx
		add.l   a6,d1                   ;llight += r_lstepx
		add.l   a5,a2                   ;lptex += a_ststepxwhole
		add.l   a4,d3                   ;lsfrac += a_sstepxfrac
		swap    d3
		add     d3,a2                   ;lptex += lsfrac >> 16
		clr     d3
		swap    d3                      ;lsfrac &= $ffff
		add.l   d6,d5                   ;ltfrac += a_tstepxfrac
		bcc.b   .cont2                  ;if (ltfrac & $10000)
		add     d6,a2                   ;lptex += r_affine...
.cont2
		dbra    d7,.loop2               ;while (--lcount)
		movem.l (sp)+,d0-d3/d5/a0/a1
.loopend

*                pspanpackage++;
*        } while (pspanpackage->count != -999999);

		lea     PSPANP_SIZEOF(a0),a0    ;pspanpackage++
		cmp.l   #-999999,PSPANP_COUNT(a0) ; while (pspanpackage->count...)
		bne.b   .loop
		movem.l (sp)+,d2-d7/a2-a6
		rts







******************************************************************************
*
*       void D_PolysetDrawSpans8T (spanpackage_t *pspanpackage)
*
*       translucent scan drawing function for alias models
*
******************************************************************************

		cnop    0,4
_D_PolysetDrawSpans8T


*****   stackframe

		rsreset
.intregs        rs.l    11
		rs.l    1
.pspan          rs.l    1


****** prologue

		movem.l d2-d7/a2-a6,-(sp)
		move.l  .pspan(sp),a0
		move.l  _d_aspancount,d0
		move.l  _errorterm,d1
		move.l  _erroradjustup,d2
		move.l  _erroradjustdown,d3
		move.l  _d_countextrastep,a1
		move.l  _ubasestep,d5
		move.l  _acolormap,a3
		move.l  _r_zistepx,d4
		move.l  _a_ststepxwhole,a5
		move.l  _a_sstepxfrac,a4
		move.l  _a_tstepxfrac,d6
		swap    d6
		move.l  _r_lstepx,a6
		move.l  _r_affinetridesc+R_SKINWIDTH,d7
		move    d7,d6
.loop

*                lcount = d_aspancount - pspanpackage->count;
*
*                errorterm += erroradjustup;
*                if (errorterm >= 0)
*                {
*                        d_aspancount += d_countextrastep;
*                        errorterm -= erroradjustdown;
*                }
*                else
*                {
*                        d_aspancount += ubasestep;
*                }

		move.l  d0,d7
		sub.l   PSPANP_COUNT(a0),d7     ;lcount = d_aspancount-pspa...
		add.l   d2,d1                   ;errorterm += erroradjustup
		blt.b   .else                   ;if (errorterm >= 0)
		add.l   a1,d0                   ;d_aspancount += d_countextrastep
		sub.l   d3,d1                   ;errorterm -= error adjustdown
		bra.b   .next
.else
		add.l   d5,d0                   ;d_aspancount += ubasestep
.next
		subq.l  #1,d7
		blt.b   .loopend

*                        lpdest = pspanpackage->pdest;
*                        lptex = pspanpackage->ptex;
*                        lpz = pspanpackage->pz;
*                        lsfrac = pspanpackage->sfrac;
*                        ltfrac = pspanpackage->tfrac;
*                        llight = pspanpackage->light;
*                        lzi = pspanpackage->zi;

		movem.l d0-d3/d5/a0/a1,-(sp)
		move.l  (a0)+,d2                ;lpdest = pspanpackage->dest
		move.l  (a0)+,a1                ;lpz = pspanpackage->pz
		addq.l  #4,a0
		move.l  (a0)+,a2                ;lptex = pspanpackage->ptex
		move.l  (a0)+,d3                ;lsfrac = pspanpackage->sfrac
		move.l  (a0)+,d5                ;tsfrac = pspanpackage->tfrac
		swap    d5
		move.l  (a0)+,d1                ;llight = pspanpackage->light
		move.l  (a0)+,d0                ;lzi = pspanpackage->zi
		move.l  d2,a0
		moveq   #0,d2

****** main drawing loop

****** d0 = lzi
****** d1 = llight
****** d3 = lsfrac
****** d4 = r_zistepx
****** d5 = ltfrac
****** d6 = a_tstepxfrac [high] and r_affinetridesc.skinwidth [low]
****** d7 = lcount-1
****** a0 -> lpdest
****** a1 -> lpz
****** a2 -> lptex
****** a3 -> acolormap
****** a4 = a_sstepxfrac
****** a5 = a_ststepxwhole
****** a6 = r_lstepx

*                        do
*                        {
*                                if (*lptex != 0)
*                                {
*                                        if ((lzi >> 16) >= *lpz)
*                                        {
*                                                btemp = ((byte *) acolormap)[*lptex + (llight & 0xFF00)];
*                                                *lpdest = mainTransTable[(btemp<<8) + (*lpdest)];
*                                                *lpz = lzi >> 16;
*                                        }
*                                }
*                                lpdest++;
*                                lzi += r_zistepx;
*                                lpz++;
*                                llight += r_lstepx;
*                                lptex += a_ststepxwhole;
*                                lsfrac += a_sstepxfrac;
*                                lptex += lsfrac >> 16;
*                                lsfrac &= 0xFFFF;
*                                ltfrac += a_tstepxfrac;
*                                if (ltfrac & 0x10000)
*                                {
*                                        lptex += r_affinetridesc.skinwidth;
*                                        ltfrac &= 0xFFFF;
*                                }
*                        } while (--lcount);

.loop2
		move.b  (a2),d2                 ;d2 = *lptex
		tst.b   d2                      ;if (*lptex != 0)
		beq.b   .cont3
		swap    d0
		cmp     (a1),d0                 ;if ((lzi >> 16) >= *lpz)
		;cmp     (a1)+,d0                ;if ((lzi >> 16) >= *lpz)
		blt.b   .cont
		move    d1,d2
		move.b  (a2),d2                 ;d2 = *lptex + (llight & $ff00)
		;move.b  0(a3,d2.l),(a0)         ;*lpdest = ((byte *)acolormap[d2]
		move.b  0(a3,d2.l),d2           ;d2 = ((byte *)acolormap[d2]
		lsl.w   #8,d2
		move.b  (a0),d2                 ;d2 = (btemp<<8) + (*lpdest)
		;move.l  a1,-(sp)
		;move.l  _mainTransTable,a1
		;move.b  0(a1,d2.l),(a0)         ;*lpdest = mainTransTable[d2];
		;move.l  (sp)+,a1
		move.b	([_mainTransTable],d2.l),(a0) ;*lpdest = mainTransTable[d2]
		;move    d0,-2(a1)               ;*lpz = lzi >> 16
		move    d0,(a1)                 ;*lpz = lzi >> 16
.cont
		swap    d0
.cont3
		addq.l  #1,a0                   ;lpdest++
		add.l   d4,d0                   ;lzi += r_zistepx
		addq.l  #2,a1                   ;lpz++;
		add.l   a6,d1                   ;llight += r_lstepx
		add.l   a5,a2                   ;lptex += a_ststepxwhole
		add.l   a4,d3                   ;lsfrac += a_sstepxfrac
		swap    d3
		add     d3,a2                   ;lptex += lsfrac >> 16
		clr     d3
		swap    d3                      ;lsfrac &= $ffff
		add.l   d6,d5                   ;ltfrac += a_tstepxfrac
		bcc.b   .cont2                  ;if (ltfrac & $10000)
		add     d6,a2                   ;lptex += r_affine...
.cont2
		dbra    d7,.loop2               ;while (--lcount)
		movem.l (sp)+,d0-d3/d5/a0/a1
.loopend

*                pspanpackage++;
*        } while (pspanpackage->count != -999999);

		lea     PSPANP_SIZEOF(a0),a0    ;pspanpackage++
		cmp.l   #-999999,PSPANP_COUNT(a0) ; while (pspanpackage->count...)
		bne.b   .loop
		movem.l (sp)+,d2-d7/a2-a6
		rts







******************************************************************************
*
*       void D_PolysetDrawSpans8T2 (spanpackage_t *pspanpackage)
*
*       transparent scan drawing function for alias models
*
******************************************************************************

		cnop    0,4
_D_PolysetDrawSpans8T2


*****   stackframe

		rsreset
.intregs        rs.l    11
		rs.l    1
.pspan          rs.l    1


****** prologue

		movem.l d2-d7/a2-a6,-(sp)
		move.l  .pspan(sp),a0
		move.l  _d_aspancount,d0
		move.l  _errorterm,d1
		move.l  _erroradjustup,d2
		move.l  _erroradjustdown,d3
		move.l  _d_countextrastep,a1
		move.l  _ubasestep,d5
		move.l  _acolormap,a3
		move.l  _r_zistepx,d4
		move.l  _a_ststepxwhole,a5
		move.l  _a_sstepxfrac,a4
		move.l  _a_tstepxfrac,d6
		swap    d6
		move.l  _r_lstepx,a6
		move.l  _r_affinetridesc+R_SKINWIDTH,d7
		move    d7,d6
.loop

*                lcount = d_aspancount - pspanpackage->count;
*
*                errorterm += erroradjustup;
*                if (errorterm >= 0)
*                {
*                        d_aspancount += d_countextrastep;
*                        errorterm -= erroradjustdown;
*                }
*                else
*                {
*                        d_aspancount += ubasestep;
*                }

		move.l  d0,d7
		sub.l   PSPANP_COUNT(a0),d7     ;lcount = d_aspancount-pspa...
		add.l   d2,d1                   ;errorterm += erroradjustup
		blt.b   .else                   ;if (errorterm >= 0)
		add.l   a1,d0                   ;d_aspancount += d_countextrastep
		sub.l   d3,d1                   ;errorterm -= error adjustdown
		bra.b   .next
.else
		add.l   d5,d0                   ;d_aspancount += ubasestep
.next
		subq.l  #1,d7
		blt.b   .loopend

*                        lpdest = pspanpackage->pdest;
*                        lptex = pspanpackage->ptex;
*                        lpz = pspanpackage->pz;
*                        lsfrac = pspanpackage->sfrac;
*                        ltfrac = pspanpackage->tfrac;
*                        llight = pspanpackage->light;
*                        lzi = pspanpackage->zi;

		movem.l d0-d3/d5/a0/a1,-(sp)
		move.l  (a0)+,d2                ;lpdest = pspanpackage->dest
		move.l  (a0)+,a1                ;lpz = pspanpackage->pz
		addq.l  #4,a0
		move.l  (a0)+,a2                ;lptex = pspanpackage->ptex
		move.l  (a0)+,d3                ;lsfrac = pspanpackage->sfrac
		move.l  (a0)+,d5                ;tsfrac = pspanpackage->tfrac
		swap    d5
		move.l  (a0)+,d1                ;llight = pspanpackage->light
		move.l  (a0)+,d0                ;lzi = pspanpackage->zi
		move.l  d2,a0
		moveq   #0,d2

****** main drawing loop

****** d0 = lzi
****** d1 = llight
****** d3 = lsfrac
****** d4 = r_zistepx
****** d5 = ltfrac
****** d6 = a_tstepxfrac [high] and r_affinetridesc.skinwidth [low]
****** d7 = lcount-1
****** a0 -> lpdest
****** a1 -> lpz
****** a2 -> lptex
****** a3 -> acolormap
****** a4 = a_sstepxfrac
****** a5 = a_ststepxwhole
****** a6 = r_lstepx

*                        do
*                        {
*								if (*lptex != 0)
*								{
*									if ((lzi >> 16) >= *lpz)
*									{
*										if (*lptex % 2 == 0)
*										{
*											btemp = ((byte *) acolormap)[*lptex + (llight & 0xFF00)];
*											*lpdest = (byte) btemp;
*											*lpz = lzi >> 16;
*										}
*										else
*										{
*											btemp = ((byte *) acolormap)[*lptex + (llight & 0xFF00)];
*											*lpdest = mainTransTable[(btemp<<8) + (*lpdest)];
*											*lpz = lzi >> 16;
*										}
*									}
*								}
*                                lpdest++;
*                                lzi += r_zistepx;
*                                lpz++;
*                                llight += r_lstepx;
*                                lptex += a_ststepxwhole;
*                                lsfrac += a_sstepxfrac;
*                                lptex += lsfrac >> 16;
*                                lsfrac &= 0xFFFF;
*                                ltfrac += a_tstepxfrac;
*                                if (ltfrac & 0x10000)
*                                {
*                                        lptex += r_affinetridesc.skinwidth;
*                                        ltfrac &= 0xFFFF;
*                                }
*                        } while (--lcount);

.loop2
		move.b  (a2),d2                 ;d2 = *lptex
		tst.b   d2                      ;if (*lptex != 0)
		beq.b   .cont3
		swap    d0
		cmp     (a1),d0                 ;if ((lzi >> 16) >= *lpz)
		blt.b   .cont
		move    d1,d2
		move.b  (a2),d2                 ;d2 = *lptex + (llight & $ff00)
		btst    #0,d2                   ;if (*lptex % 2 == 0)
		bne.b   .transluc
		move.b  0(a3,d2.l),(a0)         ;*lpdest = ((byte *)acolormap[d2]
		bra.b   .writez
.transluc
		move.b  0(a3,d2.l),d2           ;d2 = ((byte *)acolormap[d2]
		lsl.w   #8,d2
		move.b  (a0),d2                 ;d2 = (btemp<<8) + (*lpdest)
		;move.l  a1,-(sp)
		;move.l  _mainTransTable,a1
		;move.b  0(a1,d2.l),(a0)         ;*lpdest = mainTransTable[d2];
		;move.l  (sp)+,a1
		move.b	([_mainTransTable],d2.l),(a0) ;*lpdest = mainTransTable[d2]
.writez
		move    d0,(a1)                 ;*lpz = lzi >> 16
.cont
		swap    d0
.cont3
		addq.l  #1,a0                   ;lpdest++
		add.l   d4,d0                   ;lzi += r_zistepx
		addq.l  #2,a1                   ;lpz++;
		add.l   a6,d1                   ;llight += r_lstepx
		add.l   a5,a2                   ;lptex += a_ststepxwhole
		add.l   a4,d3                   ;lsfrac += a_sstepxfrac
		swap    d3
		add     d3,a2                   ;lptex += lsfrac >> 16
		clr     d3
		swap    d3                      ;lsfrac &= $ffff
		add.l   d6,d5                   ;ltfrac += a_tstepxfrac
		bcc.b   .cont2                  ;if (ltfrac & $10000)
		add     d6,a2                   ;lptex += r_affine...
.cont2
		dbra    d7,.loop2               ;while (--lcount)
		movem.l (sp)+,d0-d3/d5/a0/a1
.loopend

*                pspanpackage++;
*        } while (pspanpackage->count != -999999);

		lea     PSPANP_SIZEOF(a0),a0    ;pspanpackage++
		cmp.l   #-999999,PSPANP_COUNT(a0) ; while (pspanpackage->count...)
		bne.b   .loop
		movem.l (sp)+,d2-d7/a2-a6
		rts







******************************************************************************
*
*       void D_PolysetDrawSpans8T3 (spanpackage_t *pspanpackage)
*
*       holey scan drawing function for alias models
*
******************************************************************************

		cnop    0,4
_D_PolysetDrawSpans8T3


*****   stackframe

		rsreset
.intregs        rs.l    11
		rs.l    1
.pspan          rs.l    1


****** prologue

		movem.l d2-d7/a2-a6,-(sp)
		move.l  .pspan(sp),a0
		move.l  _d_aspancount,d0
		move.l  _errorterm,d1
		move.l  _erroradjustup,d2
		move.l  _erroradjustdown,d3
		move.l  _d_countextrastep,a1
		move.l  _ubasestep,d5
		move.l  _acolormap,a3
		move.l  _r_zistepx,d4
		move.l  _a_ststepxwhole,a5
		move.l  _a_sstepxfrac,a4
		move.l  _a_tstepxfrac,d6
		swap    d6
		move.l  _r_lstepx,a6
		move.l  _r_affinetridesc+R_SKINWIDTH,d7
		move    d7,d6
.loop

*                lcount = d_aspancount - pspanpackage->count;
*
*                errorterm += erroradjustup;
*                if (errorterm >= 0)
*                {
*                        d_aspancount += d_countextrastep;
*                        errorterm -= erroradjustdown;
*                }
*                else
*                {
*                        d_aspancount += ubasestep;
*                }

		move.l  d0,d7
		sub.l   PSPANP_COUNT(a0),d7     ;lcount = d_aspancount-pspa...
		add.l   d2,d1                   ;errorterm += erroradjustup
		blt.b   .else                   ;if (errorterm >= 0)
		add.l   a1,d0                   ;d_aspancount += d_countextrastep
		sub.l   d3,d1                   ;errorterm -= error adjustdown
		bra.b   .next
.else
		add.l   d5,d0                   ;d_aspancount += ubasestep
.next
		subq.l  #1,d7
		blt.b   .loopend

*                        lpdest = pspanpackage->pdest;
*                        lptex = pspanpackage->ptex;
*                        lpz = pspanpackage->pz;
*                        lsfrac = pspanpackage->sfrac;
*                        ltfrac = pspanpackage->tfrac;
*                        llight = pspanpackage->light;
*                        lzi = pspanpackage->zi;

		movem.l d0-d3/d5/a0/a1,-(sp)
		move.l  (a0)+,d2                ;lpdest = pspanpackage->dest
		move.l  (a0)+,a1                ;lpz = pspanpackage->pz
		addq.l  #4,a0
		move.l  (a0)+,a2                ;lptex = pspanpackage->ptex
		move.l  (a0)+,d3                ;lsfrac = pspanpackage->sfrac
		move.l  (a0)+,d5                ;tsfrac = pspanpackage->tfrac
		swap    d5
		move.l  (a0)+,d1                ;llight = pspanpackage->light
		move.l  (a0)+,d0                ;lzi = pspanpackage->zi
		move.l  d2,a0
		moveq   #0,d2

****** main drawing loop

****** d0 = lzi
****** d1 = llight
****** d3 = lsfrac
****** d4 = r_zistepx
****** d5 = ltfrac
****** d6 = a_tstepxfrac [high] and r_affinetridesc.skinwidth [low]
****** d7 = lcount-1
****** a0 -> lpdest
****** a1 -> lpz
****** a2 -> lptex
****** a3 -> acolormap
****** a4 = a_sstepxfrac
****** a5 = a_ststepxwhole
****** a6 = r_lstepx

*                        do
*                        {
*                                if (*lptex != 0)
*                                {
*                                        if ((lzi >> 16) >= *lpz)
*                                        {
*                                                *lpdest = ((byte *)acolormap)[*lptex + (llight & 0xFF00)];
*                                                *lpz = lzi >> 16;
*                                        }
*                                }
*                                lpdest++;
*                                lzi += r_zistepx;
*                                lpz++;
*                                llight += r_lstepx;
*                                lptex += a_ststepxwhole;
*                                lsfrac += a_sstepxfrac;
*                                lptex += lsfrac >> 16;
*                                lsfrac &= 0xFFFF;
*                                ltfrac += a_tstepxfrac;
*                                if (ltfrac & 0x10000)
*                                {
*                                        lptex += r_affinetridesc.skinwidth;
*                                        ltfrac &= 0xFFFF;
*                                }
*                        } while (--lcount);

.loop2
		move.b  (a2),d2                 ;d2 = *lptex
		tst.b   d2                      ;if (*lptex != 0)
		beq.b   .cont3
		swap    d0
		cmp     (a1),d0                 ;if ((lzi >> 16) >= *lpz)
		blt.b   .cont
		move    d1,d2
		move.b  (a2),d2                 ;d2 = *lptex + (llight & $ff00)
		move.b  0(a3,d2.l),(a0)         ;*lpdest = ((byte *)acolormap[d2]
		move    d0,(a1)                 ;*lpz = lzi >> 16
.cont
		swap    d0
.cont3
		addq.l  #1,a0                   ;lpdest++
		add.l   d4,d0                   ;lzi += r_zistepx
		addq.l  #2,a1                   ;lpz++;
		add.l   a6,d1                   ;llight += r_lstepx
		add.l   a5,a2                   ;lptex += a_ststepxwhole
		add.l   a4,d3                   ;lsfrac += a_sstepxfrac
		swap    d3
		add     d3,a2                   ;lptex += lsfrac >> 16
		clr     d3
		swap    d3                      ;lsfrac &= $ffff
		add.l   d6,d5                   ;ltfrac += a_tstepxfrac
		bcc.b   .cont2                  ;if (ltfrac & $10000)
		add     d6,a2                   ;lptex += r_affine...
.cont2
		dbra    d7,.loop2               ;while (--lcount)
		movem.l (sp)+,d0-d3/d5/a0/a1
.loopend

*                pspanpackage++;
*        } while (pspanpackage->count != -999999);

		lea     PSPANP_SIZEOF(a0),a0    ;pspanpackage++
		cmp.l   #-999999,PSPANP_COUNT(a0) ; while (pspanpackage->count...)
		bne.b   .loop
		movem.l (sp)+,d2-d7/a2-a6
		rts






******************************************************************************
*
*       void D_PolysetDrawSpans8T5 (spanpackage_t *pspanpackage)
*
*       special translucent scan drawing function for alias models
*
******************************************************************************

		cnop    0,4
_D_PolysetDrawSpans8T5


*****   stackframe

		rsreset
.intregs        rs.l    10
		rs.l    1
.pspan          rs.l    1


****** prologue

		movem.l d2-d7/a2-a5,-(sp)
		move.l  .pspan(sp),a0
		move.l  _d_aspancount,d0
		move.l  _errorterm,d1
		move.l  _erroradjustup,d2
		move.l  _erroradjustdown,d3
		move.l  _d_countextrastep,a1
		move.l  _ubasestep,d5
		move.l  _transTable,a3
		move.l  _r_zistepx,d4
		move.l  _a_ststepxwhole,a5
		move.l  _a_sstepxfrac,a4
		move.l  _a_tstepxfrac,d6
		swap    d6
		move.l  _r_affinetridesc+R_SKINWIDTH,d7
		move    d7,d6
.loop

*                lcount = d_aspancount - pspanpackage->count;
*
*                errorterm += erroradjustup;
*                if (errorterm >= 0)
*                {
*                        d_aspancount += d_countextrastep;
*                        errorterm -= erroradjustdown;
*                }
*                else
*                {
*                        d_aspancount += ubasestep;
*                }

		move.l  d0,d7
		sub.l   PSPANP_COUNT(a0),d7     ;lcount = d_aspancount-pspa...
		add.l   d2,d1                   ;errorterm += erroradjustup
		blt.b   .else                   ;if (errorterm >= 0)
		add.l   a1,d0                   ;d_aspancount += d_countextrastep
		sub.l   d3,d1                   ;errorterm -= error adjustdown
		bra.b   .next
.else
		add.l   d5,d0                   ;d_aspancount += ubasestep
.next
		subq.l  #1,d7
		blt.b   .loopend

*                        lpdest = pspanpackage->pdest;
*                        lptex = pspanpackage->ptex;
*                        lpz = pspanpackage->pz;
*                        lsfrac = pspanpackage->sfrac;
*                        ltfrac = pspanpackage->tfrac;
*                        llight = pspanpackage->light;
*                        lzi = pspanpackage->zi;

		movem.l d0-d3/d5/a0/a1,-(sp)
		move.l  (a0)+,d2                ;lpdest = pspanpackage->dest
		move.l  (a0)+,a1                ;lpz = pspanpackage->pz
		addq.l  #4,a0
		move.l  (a0)+,a2                ;lptex = pspanpackage->ptex
		move.l  (a0)+,d3                ;lsfrac = pspanpackage->sfrac
		move.l  (a0)+,d5                ;tsfrac = pspanpackage->tfrac
		swap    d5
		;move.l  (a0)+,d1                ;llight = pspanpackage->light
		addq.l  #4,a0
		move.l  (a0)+,d0                ;lzi = pspanpackage->zi
		move.l  d2,a0
		moveq   #0,d2

****** main drawing loop

****** d0 = lzi
****** d1 = scratch
****** d3 = lsfrac
****** d4 = r_zistepx
****** d5 = ltfrac
****** d6 = a_tstepxfrac [high] and r_affinetridesc.skinwidth [low]
****** d7 = lcount-1
****** a0 -> lpdest
****** a1 -> lpz
****** a2 -> lptex
****** a3 -> transTable
****** a4 = a_sstepxfrac
****** a5 = a_ststepxwhole

*                        do
*                        {
*                                if (*lptex != 0)
*                                {
*                                        if ((lzi >> 16) >= *lpz)
*                                        {
*                                                *lpdest = transTable[(*lptex << 8) + (*lpdest)];
*                                                *lpz = lzi >> 16;
*                                        }
*                                }
*                                lpdest++;
*                                lzi += r_zistepx;
*                                lpz++;
*                                llight += r_lstepx;
*                                lptex += a_ststepxwhole;
*                                lsfrac += a_sstepxfrac;
*                                lptex += lsfrac >> 16;
*                                lsfrac &= 0xFFFF;
*                                ltfrac += a_tstepxfrac;
*                                if (ltfrac & 0x10000)
*                                {
*                                        lptex += r_affinetridesc.skinwidth;
*                                        ltfrac &= 0xFFFF;
*                                }
*                        } while (--lcount);

.loop2
		move.b  (a2),d2                 ;d2 = *lptex
		tst.b   d2                      ;if (*lptex != 0)
		beq.b   .cont3
		swap    d0
		cmp     (a1),d0                 ;if ((lzi >> 16) >= *lpz)
		blt.b   .cont
		lsl.w   #8,d2
		move.b  (a0),d2                 ;d2 = (btemp<<8) + (*lpdest)
		move.b  0(a3,d2.l),(a0)         ;*lpdest = transTable[d2]
		move    d0,(a1)                 ;*lpz = lzi >> 16
.cont
		swap    d0
.cont3
		addq.l  #1,a0                   ;lpdest++
		add.l   d4,d0                   ;lzi += r_zistepx
		addq.l  #2,a1                   ;lpz++;
		add.l   a5,a2                   ;lptex += a_ststepxwhole
		add.l   a4,d3                   ;lsfrac += a_sstepxfrac
		swap    d3
		add     d3,a2                   ;lptex += lsfrac >> 16
		clr     d3
		swap    d3                      ;lsfrac &= $ffff
		add.l   d6,d5                   ;ltfrac += a_tstepxfrac
		bcc.b   .cont2                  ;if (ltfrac & $10000)
		add     d6,a2                   ;lptex += r_affine...
.cont2
		dbra    d7,.loop2               ;while (--lcount)
		movem.l (sp)+,d0-d3/d5/a0/a1
.loopend

*                pspanpackage++;
*        } while (pspanpackage->count != -999999);

		lea     PSPANP_SIZEOF(a0),a0    ;pspanpackage++
		cmp.l   #-999999,PSPANP_COUNT(a0) ; while (pspanpackage->count...)
		bne.b   .loop
		movem.l (sp)+,d2-d7/a2-a5
		rts








******************************************************************************
*
*       void D_PolysetRecursiveTriangle (int *lp1, int *lp2, int *lp3)
*
******************************************************************************

		cnop    0,4
_D_PolysetRecursiveTriangle


*****   stackframe

		rsreset
.new            rs.l    6
.intregs        rs.l    11
		rs.l    1
.lp1            rs.l    1
.lp2            rs.l    1
.lp3            rs.l    1


		movem.l d2-d7/a2-a6,-(sp)
		sub.l   #.intregs,sp
		move.l  .lp1(sp),a0
		move.l  .lp2(sp),a1
		move.l  .lp3(sp),a2
		lea     _zspantable,a5
		move.l  _d_viewbuffer,a6
		bsr     DoRecursion
		add.l   #.intregs,sp
		movem.l (sp)+,d2-d7/a2-a6
		rts

****** a0 -> lp1
****** a1 -> lp2
****** a2 -> lp3

DoRecursion
		lea     -6*4(sp),sp
		move.l  sp,a4

*        d = lp2[0] - lp1[0];
*        if (d < -1 || d > 1)
*                goto split;
*        d = lp2[1] - lp1[1];
*        if (d < -1 || d > 1)
*                goto split;
*
*        d = lp3[0] - lp2[0];
*        if (d < -1 || d > 1)
*                goto split2;
*        d = lp3[1] - lp2[1];
*        if (d < -1 || d > 1)
*                goto split2;
*
*        d = lp1[0] - lp3[0];
*        if (d < -1 || d > 1)
*                goto split3;
*        d = lp1[1] - lp3[1];
*        if (d < -1 || d > 1)

		move.l  (a1),d0
		move.l  (a0),d1
		move.l  1*4(a1),d2
		move.l  1*4(a0),d3
		move.l  d0,d4
		sub.l   d1,d4                   ;d = lp2[0] - lp1[0]
		addq.l  #1,d4
		cmp.l   #2,d4                   ;if (d < -1 || d > 1)
		bhi.b   .split                  ;goto split
		move.l  d2,d5
		sub.l   d3,d5                   ;d = lp2[1] - lp1[1]
		addq.l  #1,d5
		cmp.l   #2,d5                   ;if (d < -1 || d > 1)
		bhi.b   .split                  ;goto split
		move.l  (a2),d4
		move.l  d4,d5
		sub.l   d0,d4                   ;d = lp3[0] - lp2[0]
		addq.l  #1,d4
		cmp.l   #2,d4                   ;if (d < -1 || d > 1)
		bhi.b   .split2                 ;goto split2
		move.l  1*4(a2),d4
		move.l  d4,d6
		sub.l   d2,d4                   ;d = lp3[1] - lp2[1]
		addq.l  #1,d4
		cmp.l   #2,d4                   ;if (d < -1 || d > 1)
		bhi.b   .split2                 ;goto split2
		sub.l   d1,d5                   ;d = lp1[0] - lp3[0]
		addq.l  #1,d5
		cmp.l   #2,d5                   ;if (d < -1 || d > 1)
		bhi.b   .split3                 ;goto split3
		sub.l   d3,d6                   ;d = lp1[1] - lp3[1]
		addq.l  #1,d6
		cmp.l   #2,d6                   ;if (d < -1 || d > 1)
		bls.b   .exit

*split3:
*                temp = lp1;
*                lp1 = lp3;
*                lp3 = lp2;
*                lp2 = temp;
*
*                goto split;
*
*split2:
*        temp = lp1;
*        lp1 = lp2;
*        lp2 = lp3;
*        lp3 = temp;

.split3
		exg     a1,a2                   ;rotate forward
		exg     a1,a0
		move.l  (a1),d0
		move.l  (a0),d1
		move.l  1*4(a1),d2
		move.l  1*4(a0),d3
		bra.b   .split
.split2
		exg     a1,a0                   ;rotate backward
		exg     a1,a2
		move.l  (a1),d0
		move.l  (a0),d1
		move.l  1*4(a1),d2
		move.l  1*4(a0),d3
.split

*        new[0] = (lp1[0] + lp2[0]) >> 1;
*        new[1] = (lp1[1] + lp2[1]) >> 1;
*        new[2] = (lp1[2] + lp2[2]) >> 1;
*        new[3] = (lp1[3] + lp2[3]) >> 1;
*        new[5] = (lp1[5] + lp2[5]) >> 1;

		move.l  d0,d4
		move.l  d0,a3
		add.l   d1,d4
		asr.l   #1,d4                   ;d4 = new[0]
		move.l  d2,d5
		add.l   d3,d5
		asr.l   #1,d5                   ;d5 = new[1]
		move.l  2*4(a0),d6
		add.l   2*4(a1),d6
		asr.l   #1,d6                   ;d6 = new[2]
		move.l  3*4(a0),d7
		add.l   3*4(a1),d7
		asr.l   #1,d7                   ;d7 = new[3]
		move.l  5*4(a0),d0
		add.l   5*4(a1),d0
		asr.l   #1,d0                   ;d0 = new[5]

*        if (lp2[1] > lp1[1])
*                goto nodraw;
*        if ((lp2[1] == lp1[1]) && (lp2[0] < lp1[0]))
*                goto nodraw;

		move.l  d4,(a4)+                ;store new[]
		move.l  d5,(a4)+
		move.l  d6,(a4)+
		move.l  d7,(a4)+
		move.l  d0,4(a4)
		lea     -16(a4),a4
		swap    d0                      ;z = new[5]>>16
		cmp.l   d3,d2                   ;if (lp2[1] > lp1[1])
		bgt.b   .nodraw                 ;goto nodraw
		cmp.l   d2,d3                   ;if (lp2[1] == lp1[1)
		bne.b   .draw
		cmp.l   d1,a3                   ;&& (lp2[0] < lp1[0])
		blt.b   .nodraw                 ;goto nodraw
.draw

*        z = new[5]>>16;
*        zbuf = zspantable[new[1]] + new[0];
*        if (z >= *zbuf)
*        {
*                int             pix;
*
*                *zbuf = z;
*                pix = d_pcolormap[skintable[new[3]>>16][new[2]>>16]];
*                d_viewbuffer[d_scantable[new[1]] + new[0]] = pix;
*        }

		move.l  0(a5,d5.l*4),a3
		cmp     0(a3,d4.l*2),d0         ;if (z >= *zbuf)
		blt.b   .nodraw
		move    d0,0(a3,d4.l*2)         ;*zbuf = z
		swap    d6
		swap    d7
		lea     _skintable,a3
		move.l  0(a3,d7.w*4),a3         ;skintable[new[3]]
		move.b  0(a3,d6.w),d2           ;d2 = skintable[new[3]][new[2]>>16]
		and     #$ff,d2
		move.l  _d_pcolormap,a3
		move.b  0(a3,d2.w),d2           ;pix = d_pcolormap[d2]
		lea     _d_scantable,a3
		move.l  0(a3,d5.l*4),d1         ;d_scantable[new[1]]
		add.l   d4,d1                   ;+new[0]
		move.b  d2,0(a6,d1.l)           ;d_viewbuffer[d1] = pix
.nodraw

*// recursively continue
*        D_PolysetRecursiveTriangle (lp3, lp1, new);
*        D_PolysetRecursiveTriangle (lp3, new, lp2);

		movem.l a1/a2/a4,-(sp)
		move.l  a0,a1
		move.l  a2,a0
		move.l  a4,a2
		bsr     DoRecursion             ;DRT (lp3, lp1, new)
		movem.l (sp)+,a1/a2/a4
		move.l  a2,a0
		move.l  a1,a2
		move.l  a4,a1
		bsr     DoRecursion             ;DRT (lp3, new, lp2)
.exit
		lea     6*4(sp),sp
		rts







******************************************************************************
*
*       void D_PolysetRecursiveTriangleT (int *lp1, int *lp2, int *lp3)
*
******************************************************************************

		cnop    0,4
_D_PolysetRecursiveTriangleT


*****   stackframe

		rsreset
.new            rs.l    6
.intregs        rs.l    11
		rs.l    1
.lp1            rs.l    1
.lp2            rs.l    1
.lp3            rs.l    1


		movem.l d2-d7/a2-a6,-(sp)
		sub.l   #.intregs,sp
		move.l  .lp1(sp),a0
		move.l  .lp2(sp),a1
		move.l  .lp3(sp),a2
		lea     _zspantable,a5
		move.l  _d_viewbuffer,a6
		bsr     DoRecursionT
		add.l   #.intregs,sp
		movem.l (sp)+,d2-d7/a2-a6
		rts

****** a0 -> lp1
****** a1 -> lp2
****** a2 -> lp3

DoRecursionT
		lea     -6*4(sp),sp
		move.l  sp,a4

*        d = lp2[0] - lp1[0];
*        if (d < -1 || d > 1)
*                goto split;
*        d = lp2[1] - lp1[1];
*        if (d < -1 || d > 1)
*                goto split;
*
*        d = lp3[0] - lp2[0];
*        if (d < -1 || d > 1)
*                goto split2;
*        d = lp3[1] - lp2[1];
*        if (d < -1 || d > 1)
*                goto split2;
*
*        d = lp1[0] - lp3[0];
*        if (d < -1 || d > 1)
*                goto split3;
*        d = lp1[1] - lp3[1];
*        if (d < -1 || d > 1)

		move.l  (a1),d0
		move.l  (a0),d1
		move.l  1*4(a1),d2
		move.l  1*4(a0),d3
		move.l  d0,d4
		sub.l   d1,d4                   ;d = lp2[0] - lp1[0]
		addq.l  #1,d4
		cmp.l   #2,d4                   ;if (d < -1 || d > 1)
		bhi.b   .split                  ;goto split
		move.l  d2,d5
		sub.l   d3,d5                   ;d = lp2[1] - lp1[1]
		addq.l  #1,d5
		cmp.l   #2,d5                   ;if (d < -1 || d > 1)
		bhi.b   .split                  ;goto split
		move.l  (a2),d4
		move.l  d4,d5
		sub.l   d0,d4                   ;d = lp3[0] - lp2[0]
		addq.l  #1,d4
		cmp.l   #2,d4                   ;if (d < -1 || d > 1)
		bhi.b   .split2                 ;goto split2
		move.l  1*4(a2),d4
		move.l  d4,d6
		sub.l   d2,d4                   ;d = lp3[1] - lp2[1]
		addq.l  #1,d4
		cmp.l   #2,d4                   ;if (d < -1 || d > 1)
		bhi.b   .split2                 ;goto split2
		sub.l   d1,d5                   ;d = lp1[0] - lp3[0]
		addq.l  #1,d5
		cmp.l   #2,d5                   ;if (d < -1 || d > 1)
		bhi.b   .split3                 ;goto split3
		sub.l   d3,d6                   ;d = lp1[1] - lp3[1]
		addq.l  #1,d6
		cmp.l   #2,d6                   ;if (d < -1 || d > 1)
		bls.b   .exit

*split3:
*                temp = lp1;
*                lp1 = lp3;
*                lp3 = lp2;
*                lp2 = temp;
*
*                goto split;
*
*split2:
*        temp = lp1;
*        lp1 = lp2;
*        lp2 = lp3;
*        lp3 = temp;

.split3
		exg     a1,a2                   ;rotate forward
		exg     a1,a0
		move.l  (a1),d0
		move.l  (a0),d1
		move.l  1*4(a1),d2
		move.l  1*4(a0),d3
		bra.b   .split
.split2
		exg     a1,a0                   ;rotate backward
		exg     a1,a2
		move.l  (a1),d0
		move.l  (a0),d1
		move.l  1*4(a1),d2
		move.l  1*4(a0),d3
.split

*        new[0] = (lp1[0] + lp2[0]) >> 1;
*        new[1] = (lp1[1] + lp2[1]) >> 1;
*        new[2] = (lp1[2] + lp2[2]) >> 1;
*        new[3] = (lp1[3] + lp2[3]) >> 1;
*        new[5] = (lp1[5] + lp2[5]) >> 1;

		move.l  d0,d4
		move.l  d0,a3
		add.l   d1,d4
		asr.l   #1,d4                   ;d4 = new[0]
		move.l  d2,d5
		add.l   d3,d5
		asr.l   #1,d5                   ;d5 = new[1]
		move.l  2*4(a0),d6
		add.l   2*4(a1),d6
		asr.l   #1,d6                   ;d6 = new[2]
		move.l  3*4(a0),d7
		add.l   3*4(a1),d7
		asr.l   #1,d7                   ;d7 = new[3]
		move.l  5*4(a0),d0
		add.l   5*4(a1),d0
		asr.l   #1,d0                   ;d0 = new[5]

*        if (lp2[1] > lp1[1])
*                goto nodraw;
*        if ((lp2[1] == lp1[1]) && (lp2[0] < lp1[0]))
*                goto nodraw;

		move.l  d4,(a4)+                ;store new[]
		move.l  d5,(a4)+
		move.l  d6,(a4)+
		move.l  d7,(a4)+
		move.l  d0,4(a4)
		lea     -16(a4),a4
		swap    d0                      ;z = new[5]>>16
		cmp.l   d3,d2                   ;if (lp2[1] > lp1[1])
		bgt.b   .nodraw                 ;goto nodraw
		cmp.l   d2,d3                   ;if (lp2[1] == lp1[1)
		bne.b   .draw
		cmp.l   d1,a3                   ;&& (lp2[0] < lp1[0])
		blt.b   .nodraw                 ;goto nodraw
.draw



*        z = new[5]>>16;
*        zbuf = zspantable[new[1]] + new[0];
*        if (z >= *zbuf)
*        {
*            color_map_idx = skintable[new_p[3]>>16][new_p[2]>>16];
*
*            if (color_map_idx != 0)
*            {
*                unsigned int    pix, pix2;
*
*                *zbuf = z;
*                pix = d_pcolormap[color_map_idx];
*                pix2 = d_viewbuffer[d_scantable[new_p[1]] + new_p[0]];
*                pix = mainTransTable[(pix<<8) + pix2];
*                d_viewbuffer[d_scantable[new_p[1]] + new_p[0]] = pix;
*            }
*        }

		move.l  0(a5,d5.l*4),a3
		cmp     0(a3,d4.l*2),d0         ;if (z >= *zbuf)
		blt.b   .nodraw
		move    d0,0(a3,d4.l*2)         ;*zbuf = z
		swap    d6
		swap    d7
		lea     _skintable,a3
		move.l  0(a3,d7.w*4),a3         ;skintable[new[3]]
		move.b  0(a3,d6.w),d2           ;d2 = skintable[new[3]][new[2]>>16]
		tst.b   d2                      ;if (color_map_idx != 0)
		beq.b   .nodraw
		and     #$ff,d2
		move.l  _d_pcolormap,a3
		move.b  0(a3,d2.w),d2           ;pix = d_pcolormap[d2]
		lsl.w   #8,d2
		move.b  0(a6,d1.l),d2           ;d2 = (pix<<8) + pix2
		lea     _d_scantable,a3
		move.l  0(a3,d5.l*4),d1         ;d_scantable[new[1]]
		add.l   d4,d1                   ;+new[0]
		;move.b  d2,0(a6,d1.l)           ;d_viewbuffer[d1] = pix
		move.l  _mainTransTable,a3
		move.b  0(a3,d2.l),0(a6,d1.l)   ;d_viewbuffer[d1] = mainTransTable[d2]
.nodraw

*// recursively continue
*        D_PolysetRecursiveTriangleT (lp3, lp1, new);
*        D_PolysetRecursiveTriangleT (lp3, new, lp2);

		movem.l a1/a2/a4,-(sp)
		move.l  a0,a1
		move.l  a2,a0
		move.l  a4,a2
		bsr     DoRecursionT             ;DRT (lp3, lp1, new)
		movem.l (sp)+,a1/a2/a4
		move.l  a2,a0
		move.l  a1,a2
		move.l  a4,a1
		bsr     DoRecursionT             ;DRT (lp3, new, lp2)
.exit
		lea     6*4(sp),sp
		rts







******************************************************************************
*
*       void D_PolysetRecursiveTriangleT2 (int *lp1, int *lp2, int *lp3)
*
******************************************************************************

		cnop    0,4
_D_PolysetRecursiveTriangleT2


*****   stackframe

		rsreset
.new            rs.l    6
.intregs        rs.l    11
		rs.l    1
.lp1            rs.l    1
.lp2            rs.l    1
.lp3            rs.l    1


		movem.l d2-d7/a2-a6,-(sp)
		sub.l   #.intregs,sp
		move.l  .lp1(sp),a0
		move.l  .lp2(sp),a1
		move.l  .lp3(sp),a2
		lea     _zspantable,a5
		move.l  _d_viewbuffer,a6
		bsr     DoRecursionT2
		add.l   #.intregs,sp
		movem.l (sp)+,d2-d7/a2-a6
		rts

****** a0 -> lp1
****** a1 -> lp2
****** a2 -> lp3

DoRecursionT2
		lea     -6*4(sp),sp
		move.l  sp,a4

*        d = lp2[0] - lp1[0];
*        if (d < -1 || d > 1)
*                goto split;
*        d = lp2[1] - lp1[1];
*        if (d < -1 || d > 1)
*                goto split;
*
*        d = lp3[0] - lp2[0];
*        if (d < -1 || d > 1)
*                goto split2;
*        d = lp3[1] - lp2[1];
*        if (d < -1 || d > 1)
*                goto split2;
*
*        d = lp1[0] - lp3[0];
*        if (d < -1 || d > 1)
*                goto split3;
*        d = lp1[1] - lp3[1];
*        if (d < -1 || d > 1)

		move.l  (a1),d0
		move.l  (a0),d1
		move.l  1*4(a1),d2
		move.l  1*4(a0),d3
		move.l  d0,d4
		sub.l   d1,d4                   ;d = lp2[0] - lp1[0]
		addq.l  #1,d4
		cmp.l   #2,d4                   ;if (d < -1 || d > 1)
		bhi.b   .split                  ;goto split
		move.l  d2,d5
		sub.l   d3,d5                   ;d = lp2[1] - lp1[1]
		addq.l  #1,d5
		cmp.l   #2,d5                   ;if (d < -1 || d > 1)
		bhi.b   .split                  ;goto split
		move.l  (a2),d4
		move.l  d4,d5
		sub.l   d0,d4                   ;d = lp3[0] - lp2[0]
		addq.l  #1,d4
		cmp.l   #2,d4                   ;if (d < -1 || d > 1)
		bhi.b   .split2                 ;goto split2
		move.l  1*4(a2),d4
		move.l  d4,d6
		sub.l   d2,d4                   ;d = lp3[1] - lp2[1]
		addq.l  #1,d4
		cmp.l   #2,d4                   ;if (d < -1 || d > 1)
		bhi.b   .split2                 ;goto split2
		sub.l   d1,d5                   ;d = lp1[0] - lp3[0]
		addq.l  #1,d5
		cmp.l   #2,d5                   ;if (d < -1 || d > 1)
		bhi.b   .split3                 ;goto split3
		sub.l   d3,d6                   ;d = lp1[1] - lp3[1]
		addq.l  #1,d6
		cmp.l   #2,d6                   ;if (d < -1 || d > 1)
		bls.b   .exit

*split3:
*                temp = lp1;
*                lp1 = lp3;
*                lp3 = lp2;
*                lp2 = temp;
*
*                goto split;
*
*split2:
*        temp = lp1;
*        lp1 = lp2;
*        lp2 = lp3;
*        lp3 = temp;

.split3
		exg     a1,a2                   ;rotate forward
		exg     a1,a0
		move.l  (a1),d0
		move.l  (a0),d1
		move.l  1*4(a1),d2
		move.l  1*4(a0),d3
		bra.b   .split
.split2
		exg     a1,a0                   ;rotate backward
		exg     a1,a2
		move.l  (a1),d0
		move.l  (a0),d1
		move.l  1*4(a1),d2
		move.l  1*4(a0),d3
.split

*        new[0] = (lp1[0] + lp2[0]) >> 1;
*        new[1] = (lp1[1] + lp2[1]) >> 1;
*        new[2] = (lp1[2] + lp2[2]) >> 1;
*        new[3] = (lp1[3] + lp2[3]) >> 1;
*        new[5] = (lp1[5] + lp2[5]) >> 1;

		move.l  d0,d4
		move.l  d0,a3
		add.l   d1,d4
		asr.l   #1,d4                   ;d4 = new[0]
		move.l  d2,d5
		add.l   d3,d5
		asr.l   #1,d5                   ;d5 = new[1]
		move.l  2*4(a0),d6
		add.l   2*4(a1),d6
		asr.l   #1,d6                   ;d6 = new[2]
		move.l  3*4(a0),d7
		add.l   3*4(a1),d7
		asr.l   #1,d7                   ;d7 = new[3]
		move.l  5*4(a0),d0
		add.l   5*4(a1),d0
		asr.l   #1,d0                   ;d0 = new[5]

*        if (lp2[1] > lp1[1])
*                goto nodraw;
*        if ((lp2[1] == lp1[1]) && (lp2[0] < lp1[0]))
*                goto nodraw;

		move.l  d4,(a4)+                ;store new[]
		move.l  d5,(a4)+
		move.l  d6,(a4)+
		move.l  d7,(a4)+
		move.l  d0,4(a4)
		lea     -16(a4),a4
		swap    d0                      ;z = new[5]>>16
		cmp.l   d3,d2                   ;if (lp2[1] > lp1[1])
		bgt.b   .nodraw                 ;goto nodraw
		cmp.l   d2,d3                   ;if (lp2[1] == lp1[1)
		bne.b   .draw
		cmp.l   d1,a3                   ;&& (lp2[0] < lp1[0])
		blt.b   .nodraw                 ;goto nodraw
.draw



*        z = new[5]>>16;
*        zbuf = zspantable[new[1]] + new[0];
*        if (z >= *zbuf)
*        {
*            color_map_idx = skintable[new_p[3]>>16][new_p[2]>>16];
*
*            if (color_map_idx != 0)
*            {
*                unsigned int    pix, pix2;
*
*                *zbuf = z;
*                pix = d_pcolormap[color_map_idx];
*
*				if (color_map_idx % 2 == 0)
*				{
*					d_viewbuffer[d_scantable[new_p[1]] + new_p[0]] = pix;
*				}
*				else
*				{
*					pix2 = d_viewbuffer[d_scantable[new_p[1]] + new_p[0]];
*					pix = mainTransTable[(pix<<8) + pix2];
*					d_viewbuffer[d_scantable[new_p[1]] + new_p[0]] = pix;
*				}
*            }
*        }

		move.l  0(a5,d5.l*4),a3
		cmp     0(a3,d4.l*2),d0         ;if (z >= *zbuf)
		blt.b   .nodraw
		move    d0,0(a3,d4.l*2)         ;*zbuf = z
		swap    d6
		swap    d7
		lea     _skintable,a3
		move.l  0(a3,d7.w*4),a3         ;skintable[new[3]]
		move.b  0(a3,d6.w),d2           ;d2 = skintable[new[3]][new[2]>>16]
		tst.b   d2                      ;if (color_map_idx != 0)
		beq.b   .nodraw
		and     #$ff,d2
		btst    #0,d2                   ;if (color_map_idx % 2 == 0)
		bne.b   .transluc
		move.l  _d_pcolormap,a3
		move.b  0(a3,d2.w),d2           ;pix = d_pcolormap[d2]
		lea     _d_scantable,a3
		move.l  0(a3,d5.l*4),d1         ;d_scantable[new[1]]
		add.l   d4,d1                   ;+new[0]
		move.b  d2,0(a6,d1.l)           ;d_viewbuffer[d1] = pix
		bra.b .nodraw
.transluc
		move.l  _d_pcolormap,a3
		move.b  0(a3,d2.w),d2           ;pix = d_pcolormap[d2]
		lsl.w   #8,d2
		move.b  0(a6,d1.l),d2           ;d2 = (pix<<8) + pix2
		lea     _d_scantable,a3
		move.l  0(a3,d5.l*4),d1         ;d_scantable[new[1]]
		add.l   d4,d1                   ;+new[0]
		move.l  _mainTransTable,a3
		move.b  0(a3,d2.l),0(a6,d1.l)   ;d_viewbuffer[d1] = mainTransTable[d2]
.nodraw

*// recursively continue
*        D_PolysetRecursiveTriangleT2 (lp3, lp1, new);
*        D_PolysetRecursiveTriangleT2 (lp3, new, lp2);

		movem.l a1/a2/a4,-(sp)
		move.l  a0,a1
		move.l  a2,a0
		move.l  a4,a2
		bsr     DoRecursionT2             ;DRT (lp3, lp1, new)
		movem.l (sp)+,a1/a2/a4
		move.l  a2,a0
		move.l  a1,a2
		move.l  a4,a1
		bsr     DoRecursionT2             ;DRT (lp3, new, lp2)
.exit
		lea     6*4(sp),sp
		rts







******************************************************************************
*
*       void D_PolysetRecursiveTriangleT3 (int *lp1, int *lp2, int *lp3)
*
******************************************************************************

		cnop    0,4
_D_PolysetRecursiveTriangleT3


*****   stackframe

		rsreset
.new            rs.l    6
.intregs        rs.l    11
		rs.l    1
.lp1            rs.l    1
.lp2            rs.l    1
.lp3            rs.l    1


		movem.l d2-d7/a2-a6,-(sp)
		sub.l   #.intregs,sp
		move.l  .lp1(sp),a0
		move.l  .lp2(sp),a1
		move.l  .lp3(sp),a2
		lea     _zspantable,a5
		move.l  _d_viewbuffer,a6
		bsr     DoRecursionT3
		add.l   #.intregs,sp
		movem.l (sp)+,d2-d7/a2-a6
		rts

****** a0 -> lp1
****** a1 -> lp2
****** a2 -> lp3

DoRecursionT3
		lea     -6*4(sp),sp
		move.l  sp,a4

*        d = lp2[0] - lp1[0];
*        if (d < -1 || d > 1)
*                goto split;
*        d = lp2[1] - lp1[1];
*        if (d < -1 || d > 1)
*                goto split;
*
*        d = lp3[0] - lp2[0];
*        if (d < -1 || d > 1)
*                goto split2;
*        d = lp3[1] - lp2[1];
*        if (d < -1 || d > 1)
*                goto split2;
*
*        d = lp1[0] - lp3[0];
*        if (d < -1 || d > 1)
*                goto split3;
*        d = lp1[1] - lp3[1];
*        if (d < -1 || d > 1)

		move.l  (a1),d0
		move.l  (a0),d1
		move.l  1*4(a1),d2
		move.l  1*4(a0),d3
		move.l  d0,d4
		sub.l   d1,d4                   ;d = lp2[0] - lp1[0]
		addq.l  #1,d4
		cmp.l   #2,d4                   ;if (d < -1 || d > 1)
		bhi.b   .split                  ;goto split
		move.l  d2,d5
		sub.l   d3,d5                   ;d = lp2[1] - lp1[1]
		addq.l  #1,d5
		cmp.l   #2,d5                   ;if (d < -1 || d > 1)
		bhi.b   .split                  ;goto split
		move.l  (a2),d4
		move.l  d4,d5
		sub.l   d0,d4                   ;d = lp3[0] - lp2[0]
		addq.l  #1,d4
		cmp.l   #2,d4                   ;if (d < -1 || d > 1)
		bhi.b   .split2                 ;goto split2
		move.l  1*4(a2),d4
		move.l  d4,d6
		sub.l   d2,d4                   ;d = lp3[1] - lp2[1]
		addq.l  #1,d4
		cmp.l   #2,d4                   ;if (d < -1 || d > 1)
		bhi.b   .split2                 ;goto split2
		sub.l   d1,d5                   ;d = lp1[0] - lp3[0]
		addq.l  #1,d5
		cmp.l   #2,d5                   ;if (d < -1 || d > 1)
		bhi.b   .split3                 ;goto split3
		sub.l   d3,d6                   ;d = lp1[1] - lp3[1]
		addq.l  #1,d6
		cmp.l   #2,d6                   ;if (d < -1 || d > 1)
		bls.b   .exit

*split3:
*                temp = lp1;
*                lp1 = lp3;
*                lp3 = lp2;
*                lp2 = temp;
*
*                goto split;
*
*split2:
*        temp = lp1;
*        lp1 = lp2;
*        lp2 = lp3;
*        lp3 = temp;

.split3
		exg     a1,a2                   ;rotate forward
		exg     a1,a0
		move.l  (a1),d0
		move.l  (a0),d1
		move.l  1*4(a1),d2
		move.l  1*4(a0),d3
		bra.b   .split
.split2
		exg     a1,a0                   ;rotate backward
		exg     a1,a2
		move.l  (a1),d0
		move.l  (a0),d1
		move.l  1*4(a1),d2
		move.l  1*4(a0),d3
.split

*        new[0] = (lp1[0] + lp2[0]) >> 1;
*        new[1] = (lp1[1] + lp2[1]) >> 1;
*        new[2] = (lp1[2] + lp2[2]) >> 1;
*        new[3] = (lp1[3] + lp2[3]) >> 1;
*        new[5] = (lp1[5] + lp2[5]) >> 1;

		move.l  d0,d4
		move.l  d0,a3
		add.l   d1,d4
		asr.l   #1,d4                   ;d4 = new[0]
		move.l  d2,d5
		add.l   d3,d5
		asr.l   #1,d5                   ;d5 = new[1]
		move.l  2*4(a0),d6
		add.l   2*4(a1),d6
		asr.l   #1,d6                   ;d6 = new[2]
		move.l  3*4(a0),d7
		add.l   3*4(a1),d7
		asr.l   #1,d7                   ;d7 = new[3]
		move.l  5*4(a0),d0
		add.l   5*4(a1),d0
		asr.l   #1,d0                   ;d0 = new[5]

*        if (lp2[1] > lp1[1])
*                goto nodraw;
*        if ((lp2[1] == lp1[1]) && (lp2[0] < lp1[0]))
*                goto nodraw;

		move.l  d4,(a4)+                ;store new[]
		move.l  d5,(a4)+
		move.l  d6,(a4)+
		move.l  d7,(a4)+
		move.l  d0,4(a4)
		lea     -16(a4),a4
		swap    d0                      ;z = new[5]>>16
		cmp.l   d3,d2                   ;if (lp2[1] > lp1[1])
		bgt.b   .nodraw                 ;goto nodraw
		cmp.l   d2,d3                   ;if (lp2[1] == lp1[1)
		bne.b   .draw
		cmp.l   d1,a3                   ;&& (lp2[0] < lp1[0])
		blt.b   .nodraw                 ;goto nodraw
.draw

*        z = new[5]>>16;
*        zbuf = zspantable[new[1]] + new[0];
*        if (z >= *zbuf)
*        {
*                int             pix;
*
*                *zbuf = z;
*				if (color_map_idx != 0)
*				{
*					unsigned int	pix;
*
*					*zbuf = z;
*					pix = d_pcolormap[color_map_idx];
*					d_viewbuffer[d_scantable[new_p[1]] + new_p[0]] = pix;
*				}
*        }

		move.l  0(a5,d5.l*4),a3
		cmp     0(a3,d4.l*2),d0         ;if (z >= *zbuf)
		blt.b   .nodraw
		move    d0,0(a3,d4.l*2)         ;*zbuf = z
		swap    d6
		swap    d7
		lea     _skintable,a3
		move.l  0(a3,d7.w*4),a3         ;skintable[new[3]]
		move.b  0(a3,d6.w),d2           ;d2 = skintable[new[3]][new[2]>>16]
		tst.b   d2                      ;if (color_map_idx != 0)
		beq.b   .nodraw
		and     #$ff,d2
		move.l  _d_pcolormap,a3
		move.b  0(a3,d2.w),d2           ;pix = d_pcolormap[d2]
		lea     _d_scantable,a3
		move.l  0(a3,d5.l*4),d1         ;d_scantable[new[1]]
		add.l   d4,d1                   ;+new[0]
		move.b  d2,0(a6,d1.l)           ;d_viewbuffer[d1] = pix
.nodraw

*// recursively continue
*        D_PolysetRecursiveTriangleT3 (lp3, lp1, new);
*        D_PolysetRecursiveTriangleT3 (lp3, new, lp2);

		movem.l a1/a2/a4,-(sp)
		move.l  a0,a1
		move.l  a2,a0
		move.l  a4,a2
		bsr     DoRecursionT3             ;DRT (lp3, lp1, new)
		movem.l (sp)+,a1/a2/a4
		move.l  a2,a0
		move.l  a1,a2
		move.l  a4,a1
		bsr     DoRecursionT3             ;DRT (lp3, new, lp2)
.exit
		lea     6*4(sp),sp
		rts







******************************************************************************
*
*       void D_PolysetRecursiveTriangleT5 (int *lp1, int *lp2, int *lp3)
*
******************************************************************************

		cnop    0,4
_D_PolysetRecursiveTriangleT5


*****   stackframe

		rsreset
.new            rs.l    6
.intregs        rs.l    11
		rs.l    1
.lp1            rs.l    1
.lp2            rs.l    1
.lp3            rs.l    1


		movem.l d2-d7/a2-a6,-(sp)
		sub.l   #.intregs,sp
		move.l  .lp1(sp),a0
		move.l  .lp2(sp),a1
		move.l  .lp3(sp),a2
		lea     _zspantable,a5
		move.l  _d_viewbuffer,a6
		bsr     DoRecursionT5
		add.l   #.intregs,sp
		movem.l (sp)+,d2-d7/a2-a6
		rts

****** a0 -> lp1
****** a1 -> lp2
****** a2 -> lp3

DoRecursionT5
		lea     -6*4(sp),sp
		move.l  sp,a4

*        d = lp2[0] - lp1[0];
*        if (d < -1 || d > 1)
*                goto split;
*        d = lp2[1] - lp1[1];
*        if (d < -1 || d > 1)
*                goto split;
*
*        d = lp3[0] - lp2[0];
*        if (d < -1 || d > 1)
*                goto split2;
*        d = lp3[1] - lp2[1];
*        if (d < -1 || d > 1)
*                goto split2;
*
*        d = lp1[0] - lp3[0];
*        if (d < -1 || d > 1)
*                goto split3;
*        d = lp1[1] - lp3[1];
*        if (d < -1 || d > 1)

		move.l  (a1),d0
		move.l  (a0),d1
		move.l  1*4(a1),d2
		move.l  1*4(a0),d3
		move.l  d0,d4
		sub.l   d1,d4                   ;d = lp2[0] - lp1[0]
		addq.l  #1,d4
		cmp.l   #2,d4                   ;if (d < -1 || d > 1)
		bhi.b   .split                  ;goto split
		move.l  d2,d5
		sub.l   d3,d5                   ;d = lp2[1] - lp1[1]
		addq.l  #1,d5
		cmp.l   #2,d5                   ;if (d < -1 || d > 1)
		bhi.b   .split                  ;goto split
		move.l  (a2),d4
		move.l  d4,d5
		sub.l   d0,d4                   ;d = lp3[0] - lp2[0]
		addq.l  #1,d4
		cmp.l   #2,d4                   ;if (d < -1 || d > 1)
		bhi.b   .split2                 ;goto split2
		move.l  1*4(a2),d4
		move.l  d4,d6
		sub.l   d2,d4                   ;d = lp3[1] - lp2[1]
		addq.l  #1,d4
		cmp.l   #2,d4                   ;if (d < -1 || d > 1)
		bhi.b   .split2                 ;goto split2
		sub.l   d1,d5                   ;d = lp1[0] - lp3[0]
		addq.l  #1,d5
		cmp.l   #2,d5                   ;if (d < -1 || d > 1)
		bhi.b   .split3                 ;goto split3
		sub.l   d3,d6                   ;d = lp1[1] - lp3[1]
		addq.l  #1,d6
		cmp.l   #2,d6                   ;if (d < -1 || d > 1)
		bls.b   .exit

*split3:
*                temp = lp1;
*                lp1 = lp3;
*                lp3 = lp2;
*                lp2 = temp;
*
*                goto split;
*
*split2:
*        temp = lp1;
*        lp1 = lp2;
*        lp2 = lp3;
*        lp3 = temp;

.split3
		exg     a1,a2                   ;rotate forward
		exg     a1,a0
		move.l  (a1),d0
		move.l  (a0),d1
		move.l  1*4(a1),d2
		move.l  1*4(a0),d3
		bra.b   .split
.split2
		exg     a1,a0                   ;rotate backward
		exg     a1,a2
		move.l  (a1),d0
		move.l  (a0),d1
		move.l  1*4(a1),d2
		move.l  1*4(a0),d3
.split

*        new[0] = (lp1[0] + lp2[0]) >> 1;
*        new[1] = (lp1[1] + lp2[1]) >> 1;
*        new[2] = (lp1[2] + lp2[2]) >> 1;
*        new[3] = (lp1[3] + lp2[3]) >> 1;
*        new[5] = (lp1[5] + lp2[5]) >> 1;

		move.l  d0,d4
		move.l  d0,a3
		add.l   d1,d4
		asr.l   #1,d4                   ;d4 = new[0]
		move.l  d2,d5
		add.l   d3,d5
		asr.l   #1,d5                   ;d5 = new[1]
		move.l  2*4(a0),d6
		add.l   2*4(a1),d6
		asr.l   #1,d6                   ;d6 = new[2]
		move.l  3*4(a0),d7
		add.l   3*4(a1),d7
		asr.l   #1,d7                   ;d7 = new[3]
		move.l  5*4(a0),d0
		add.l   5*4(a1),d0
		asr.l   #1,d0                   ;d0 = new[5]

*        if (lp2[1] > lp1[1])
*                goto nodraw;
*        if ((lp2[1] == lp1[1]) && (lp2[0] < lp1[0]))
*                goto nodraw;

		move.l  d4,(a4)+                ;store new[]
		move.l  d5,(a4)+
		move.l  d6,(a4)+
		move.l  d7,(a4)+
		move.l  d0,4(a4)
		lea     -16(a4),a4
		swap    d0                      ;z = new[5]>>16
		cmp.l   d3,d2                   ;if (lp2[1] > lp1[1])
		bgt.b   .nodraw                 ;goto nodraw
		cmp.l   d2,d3                   ;if (lp2[1] == lp1[1)
		bne.b   .draw
		cmp.l   d1,a3                   ;&& (lp2[0] < lp1[0])
		blt.b   .nodraw                 ;goto nodraw
.draw



*        z = new[5]>>16;
*        zbuf = zspantable[new[1]] + new[0];
*        if (z >= *zbuf)
*        {
*            color_map_idx = skintable[new_p[3]>>16][new_p[2]>>16];
*
*            if (color_map_idx != 0)
*            {
*                unsigned int    pix, pix2;
*
*                *zbuf = z;
*                pix = color_map_idx;
*                pix2 = d_viewbuffer[d_scantable[new_p[1]] + new_p[0]];
*                pix = transTable[(pix<<8) + pix2];
*                d_viewbuffer[d_scantable[new_p[1]] + new_p[0]] = pix;
*            }
*        }

		move.l  0(a5,d5.l*4),a3
		cmp     0(a3,d4.l*2),d0         ;if (z >= *zbuf)
		blt.b   .nodraw
		move    d0,0(a3,d4.l*2)         ;*zbuf = z
		swap    d6
		swap    d7
		lea     _skintable,a3
		move.l  0(a3,d7.w*4),a3         ;skintable[new[3]]
		move.b  0(a3,d6.w),d2           ;d2 = skintable[new[3]][new[2]>>16]
		tst.b   d2                      ;if (color_map_idx != 0)
		beq.b   .nodraw
		and     #$ff,d2
		;move.l  _d_pcolormap,a3
		;move.b  0(a3,d2.w),d2           ;pix = d_pcolormap[d2]
		lsl.w   #8,d2
		move.b  0(a6,d1.l),d2           ;d2 = (pix<<8) + pix2
		lea     _d_scantable,a3
		move.l  0(a3,d5.l*4),d1         ;d_scantable[new[1]]
		add.l   d4,d1                   ;+new[0]
		move.l  _transTable,a3
		move.b  0(a3,d2.l),0(a6,d1.l)   ;d_viewbuffer[d1] = transTable[d2]
.nodraw

*// recursively continue
*        D_PolysetRecursiveTriangleT5 (lp3, lp1, new);
*        D_PolysetRecursiveTriangleT5 (lp3, new, lp2);

		movem.l a1/a2/a4,-(sp)
		move.l  a0,a1
		move.l  a2,a0
		move.l  a4,a2
		bsr     DoRecursionT5             ;DRT (lp3, lp1, new)
		movem.l (sp)+,a1/a2/a4
		move.l  a2,a0
		move.l  a1,a2
		move.l  a4,a1
		bsr     DoRecursionT5             ;DRT (lp3, new, lp2)
.exit
		lea     6*4(sp),sp
		rts







******************************************************************************
*
*       void D_PolysetSetUpForLineScan (fixed8_t startvertu,
*               fixed8_t startvertv, fixed8_t endvertu, fixed8_t endvertv)
*
*       Parameters are transferred in registers d0-d3
******************************************************************************

		cnop    0,4
D_PolysetSetUpForLineScan


*****   stackframe

		rsreset
.intregs        rs.l    2
		rs.l    1
.startvertu     rs.l    1
.startvertv     rs.l    1
.endvertu       rs.l    1
.endvertv       rs.l    1


*        errorterm = -1;
*
*        tm = endvertu - startvertu;
*        tn = endvertv - startvertv;

		move.l  #-1,_errorterm
		sub.l   d0,d2
		sub.l   d1,d3
		add.l   #15,d2
		add.l   #15,d3

*        if (((tm <= 16) && (tm >= -15)) &&
*                ((tn <= 16) && (tn >= -15)))
*        {
*                ptemp = &adivtab[((tm+15) << 5) + (tn+15)];
*                ubasestep = ptemp->quotient;
*                erroradjustup = ptemp->remainder;
*                erroradjustdown = tn;
*        }

		cmp.l   #31,d2
		bhi.b   .else
		cmp.l   #31,d3
		bhi.b   .else
		lea     _adivtab,a0
		lsl.l   #5,d2
		add.l   d3,d2
		lea     0(a0,d2.l*8),a0
		move.l  PTEMP_QUOTIENT(a0),_ubasestep
		move.l  PTEMP_REMAINDER(a0),_erroradjustup
		sub.l   #15,d3
		move.l  d3,_erroradjustdown
		bra.b   .exit

*        {
*                dm = (double)tm;
*                dn = (double)tn;
*
*                FloorDivMod (dm, dn, &ubasestep, &erroradjustup);
*
*                erroradjustdown = dn;
*        }

.else
		sub.l   #15,d2
		sub.l   #15,d3
		fmove.l d2,fp0
		fmove.l d3,fp1
		move.l  #_erroradjustup,-(sp)
		move.l  #_ubasestep,-(sp)
		fmove.d fp1,-(sp)
		fmove.d fp0,-(sp)
		jsr     _FloorDivMod
		add     #24,sp
		move.l  d3,_erroradjustdown
.exit
		rts








*****************************************************************************
*
*       void D_PolysetCalcGradients (int skinwidth)
*
******************************************************************************

		cnop    0,4
_D_PolysetCalcGradients


*****   stackframe
		rsreset
.fpuregs        rs.x    6
.intregs        rs.l    3
		rs.l    1
.skinwidth      rs.l    1

***** prologue

		movem.l d2/d3/a2,-(sp)
		fmovem.x        fp2-fp7,-(sp)

*        p00_minus_p20 = r_p0[0] - r_p2[0];
*        p01_minus_p21 = r_p0[1] - r_p2[1];
*        p10_minus_p20 = r_p1[0] - r_p2[0];
*        p11_minus_p21 = r_p1[1] - r_p2[1];
*
*        xstepdenominv = 1.0 / (float)d_xdenom;
*
*        ystepdenominv = -xstepdenominv;

		lea     _r_p2,a0
		lea     _r_p1,a1
		lea     _r_p0,a2
		move.l  (a2),d0
		move.l  1*4(a2),d1
		move.l  (a0),d2
		sub.l   d2,d0
		fmove.l d0,fp0                  ;fp0 = p00_minus_p20
		move.l  1*4(a0),d3
		sub.l   d3,d1
		fmove.l d1,fp1                  ;fp1 = p01_minus_p21
		sub.l   (a1),d2
		neg.l   d2
		fmove.l d2,fp2                  ;fp2 = p10_minus_p20
		sub.l   4(a1),d3
		neg.l   d3
		fmove.l d3,fp3                  ;fp3 = p11_minus_p21

		fmove.l _d_xdenom,fp4
		fmove.s #1,fp5
		fdiv    fp4,fp5                 ;fp5 = 1.0 / (float)d_xdenom
		fneg    fp5,fp6                 ;fp6 = ystepdenominv
		fmul    fp5,fp1
		fmul    fp5,fp3
		fmul    fp6,fp0
		fmul    fp6,fp2

*        t0 = r_p0[4] - r_p2[4];
*        t1 = r_p1[4] - r_p2[4];
*        r_lstepx = (int)
*                        ceil((t1 * p01_minus_p21 - t0 * p11_minus_p21) * xstepdenominv);
*        r_lstepy = (int)
*                        ceil((t1 * p00_minus_p20 - t0 * p10_minus_p20) * ystepdenominv);

		fmove.l fpcr,d2
		fmove.l #$000000b0,fpcr
		move.l  4*4(a0),d0
		move.l  d0,d1
		sub.l   4*4(a2),d0
		neg.l   d0
		fmove.l d0,fp4                  ;fp4 = t0
		sub.l   4*4(a1),d1
		neg.l   d1
		fmove.l d1,fp5                  ;fp5 = t1
		fmove   fp4,fp6
		fmul    fp3,fp6
		fmove   fp5,fp7
		fmul    fp1,fp7
		fsub    fp6,fp7
		fmove.l fp7,_r_lstepx
		fmul    fp2,fp4
		fmul    fp0,fp5
		fsub    fp4,fp5
		fmove.l fp5,_r_lstepy
		fmove.l d2,fpcr

*        t0 = r_p0[2] - r_p2[2];
*        t1 = r_p1[2] - r_p2[2];
*        r_sstepx = (int)((t1 * p01_minus_p21 - t0 * p11_minus_p21) *
*                        xstepdenominv);
*        r_sstepy = (int)((t1 * p00_minus_p20 - t0* p10_minus_p20) *
*                        ystepdenominv);

		move.l  2*4(a0),d0
		move.l  d0,d1
		sub.l   2*4(a2),d0
		neg.l   d0
		fmove.l d0,fp4                  ;fp4 = t0
		sub.l   2*4(a1),d1
		neg.l   d1
		fmove.l d1,fp5                  ;fp5 = t1
		fmove   fp4,fp6
		fmul    fp3,fp6
		fmove   fp5,fp7
		fmul    fp1,fp7
		fsub    fp6,fp7
		fmove.l fp7,d2
		move.l  d2,_r_sstepx
		fmul    fp2,fp4
		fmul    fp0,fp5
		fsub    fp4,fp5
		fmove.l fp5,_r_sstepy

*        t0 = r_p0[3] - r_p2[3];
*        t1 = r_p1[3] - r_p2[3];
*        r_tstepx = (int)((t1 * p01_minus_p21 - t0 * p11_minus_p21) *
*                        xstepdenominv);
*        r_tstepy = (int)((t1 * p00_minus_p20 - t0 * p10_minus_p20) *
*                        ystepdenominv);

		move.l  3*4(a0),d0
		move.l  d0,d1
		sub.l   3*4(a2),d0
		neg.l   d0
		fmove.l d0,fp4                  ;fp4 = t0
		sub.l   3*4(a1),d1
		neg.l   d1
		fmove.l d1,fp5                  ;fp5 = t1
		fmove   fp4,fp6
		fmul    fp3,fp6
		fmove   fp5,fp7
		fmul    fp1,fp7
		fsub    fp6,fp7
		fmove.l fp7,d3
		move.l  d3,_r_tstepx
		fmul    fp2,fp4
		fmul    fp0,fp5
		fsub    fp4,fp5
		fmove.l fp5,_r_tstepy

*        t0 = r_p0[5] - r_p2[5];
*        t1 = r_p1[5] - r_p2[5];
*        r_zistepx = (int)((t1 * p01_minus_p21 - t0 * p11_minus_p21) *
*                        xstepdenominv);
*        r_zistepy = (int)((t1 * p00_minus_p20 - t0 * p10_minus_p20) *
*                        ystepdenominv);

		move.l  5*4(a0),d0
		move.l  d0,d1
		sub.l   5*4(a2),d0
		neg.l   d0
		fmove.l d0,fp4                  ;fp4 = t0
		sub.l   5*4(a1),d1
		neg.l   d1
		fmove.l d1,fp5                  ;fp5 = t1
		fmove   fp4,fp6
		fmul    fp3,fp6
		fmove   fp5,fp7
		fmul    fp1,fp7
		fsub    fp6,fp7
		fmove.l fp7,_r_zistepx
		fmul    fp2,fp4
		fmul    fp0,fp5
		fsub    fp4,fp5
		fmove.l fp5,_r_zistepy

*        a_sstepxfrac = r_sstepx & 0xFFFF;
*        a_tstepxfrac = r_tstepx & 0xFFFF;
*        a_ststepxwhole = skinwidth * (r_tstepx >> 16) + (r_sstepx >> 16);

		move.l  .skinwidth(sp),d0
		swap    d3
		muls    d3,d0
		clr     d3
		swap    d3
		move.l  d2,d1
		swap    d1
		ext.l   d1
		add.l   d1,d0
		and.l   #$ffff,d2
		move.l  d2,_a_sstepxfrac
		move.l  d3,_a_tstepxfrac
		move.l  d0,_a_ststepxwhole
		fmovem.x        (sp)+,fp2-fp7
		movem.l (sp)+,d2/d3/a2
		rts








******************************************************************************
*
*       void D_RasterizeAliasPolySmooth (void)
*
******************************************************************************
		cnop    0,4
_D_RasterizeAliasPolySmooth

*****   stackframe
		rsreset
.pdestx         rs.l    1
.pzx            rs.l    1
.aspanx         rs.l    1
.ptexx          rs.l    1
.sfracx         rs.l    1
.tfracx         rs.l    1
.lightx         rs.l    1
.zix            rs.l    1
.pdestb         rs.l    1
.pzb            rs.l    1
.aspanb         rs.l    1
.ptexb          rs.l    1
.sfracb         rs.l    1
.tfracb         rs.l    1
.lightb         rs.l    1
.zib            rs.l    1
.r_affine       rs.l    1
.height         rs.l    1
.initLH         rs.l    1
.initRH         rs.l    1
.savearea       rs.l    11
.intregs        rs.l    11
		rs.l    1

		movem.l d2-d7/a2-a6,-(sp)
		sub     #.intregs,sp

*        plefttop = pedgetable->pleftedgevert0;
*        prighttop = pedgetable->prightedgevert0;
*
*        pleftbottom = pedgetable->pleftedgevert1;
*        prightbottom = pedgetable->prightedgevert1;
*
*        initialleftheight = pleftbottom[1] - plefttop[1];
*        initialrightheight = prightbottom[1] - prighttop[1];

		move.l  _pedgetable,a0
		move.l  ETAB_PLEV0(a0),a6       ;a6 = plefttop
		move.l  ETAB_PREV0(a0),a2       ;a2 = prighttop
		move.l  ETAB_PLEV1(a0),a3       ;a3 = pleftbottom
		move.l  ETAB_PREV1(a0),a4       ;a4 = prightbottom
		lea     _r_affinetridesc,a5
		move.l  4(a3),d6
		move.l  4(a6),d7                ;d7 = plefttop[1]
		move.l  d6,d2
		sub.l   d7,d2
		move.l  d2,.initLH(sp)          ;initialleftheight
		move.l  4(a4),d3
		move.l  4(a2),d4
		move.l  d3,d5
		sub.l   d4,d5
		move.l  d5,.initRH(sp)          ;initialrightheight
		move.l  R_SKINWIDTH(a5),d4
		move.l  d4,.r_affine(sp)
		move.l  d4,-(sp)
		bsr     _D_PolysetCalcGradients
		addq    #4,sp
		move.l  d6,d3
		move.l  (a3),d2
		move.l  d7,d1
		move.l  (a6),d0
		bsr     D_PolysetSetUpForLineScan

*        d_pedgespanpackage = a_spans;
*
*        ystart = plefttop[1];
*        d_aspancount = plefttop[0] - prighttop[0];
*
*        d_ptex = (byte *)r_affinetridesc.pskin + (plefttop[2] >> 16) +
*                        (plefttop[3] >> 16) * r_affinetridesc.skinwidth;
*        d_sfrac = plefttop[2] & 0xFFFF;
*        d_tfrac = plefttop[3] & 0xFFFF;
*        d_pzbasestep = d_zwidth + ubasestep;
*        d_pzextrastep = d_pzbasestep + 1;
*        d_light = plefttop[4];
*        d_zi = plefttop[5];
*
*        d_pdestbasestep = screenwidth + ubasestep;
*        d_pdestextrastep = d_pdestbasestep + 1;
*        d_pdest = (byte *)d_viewbuffer +
*                        ystart * screenwidth + plefttop[0];
*        d_pz = d_pzbuffer + ystart * d_zwidth + plefttop[0];

		move.l  _a_spans,_d_pedgespanpackage
		move.l  (a6),d2
		sub.l   (a2),d2
		move.l  d2,_d_aspancount        ;d_aspancount = plefttop[0] - ...
		move.l  2*4(a6),d0
		move.l  3*4(a6),d1
		move.l  d0,d3                   ;d3 = plefttop[2]
		move.l  d1,d6                   ;d6 = plefttop[3]
		swap    d0
		swap    d1
		and.l   #$ffff,d0
		and.l   #$ffff,d1
		mulu    d4,d1
		add.l   d0,d1
		add.l   R_PSKIN(a5),d1
		move.l  d1,_d_ptex              ;d_ptex = ...
		and.l   #$ffff,d3
		and.l   #$ffff,d6
		move.l  d3,_d_sfrac             ;d_sfrac = plefttop[2]&$ffff
		move.l  d6,_d_tfrac             ;d_tfrac = plefttop[3]&$ffff
		move.l  _d_zwidth,d0
		move.l  d0,d3                   ;d3 = d_zwidth
		move.l  _ubasestep,d1
		move.l  d1,.aspanb(sp)
		move.l  d1,d5                   ;d5 = ubasestep
		add.l   d1,d0
		addq.l  #1,d1
		move.l  d1,_d_countextrastep
		move.l  d1,.aspanx(sp)
		add.l   d0,d0
		move.l  d0,.pzb(sp)             ;d_pzbasestep = d_zwidth + ubasestep
		addq.l  #2,d0
		move.l  d0,.pzx(sp)             ;d_pzextrastep = d_pzbasestep + 1
		move.l  4*4(a6),_d_light        ;d_light = plefttop[4]
		move.l  5*4(a6),_d_zi           ;d_zi = plefttop[5]
		move.l  _screenwidth,d0
		add.l   d0,d1
		move.l  d1,.pdestx(sp)          ;d_pdestextrastep = ...
		subq.l  #1,d1
		move.l  d1,.pdestb(sp)          ;d_pdestbasestep = screenwidth + ..
		mulu    d7,d0                   ;screenwidth * ystart
		move.l  (a6),d6
		add.l   d6,d0                   ;+ plefttop[0]
		add.l   _d_viewbuffer,d0
		move.l  d0,_d_pdest             ;d_pdest + d_viewbuffer + ...
		mulu    d7,d3                   ;d_zwidth * ystart
		add.l   d6,d3                   ;+ plefttop[0]
		add.l   d3,d3
		add.l   _d_pzbuffer,d3
		move.l  d3,_d_pz                ;d_pz = d_pzbuffer + ...

*        if (ubasestep < 0)
*                working_lstepx = r_lstepx - 1;
*        else
*                working_lstepx = r_lstepx;
*
*        d_countextrastep = ubasestep + 1;
*        d_ptexbasestep = ((r_sstepy + r_sstepx * ubasestep) >> 16) +
*                        ((r_tstepy + r_tstepx * ubasestep) >> 16) *
*                        r_affinetridesc.skinwidth;
*        d_sfracbasestep = (r_sstepy + r_sstepx * ubasestep) & 0xFFFF;
*        d_tfracbasestep = (r_tstepy + r_tstepx * ubasestep) & 0xFFFF;
*        d_lightbasestep = r_lstepy + working_lstepx * ubasestep;
*        d_zibasestep = r_zistepy + r_zistepx * ubasestep;
*
*        d_ptexextrastep = ((r_sstepy + r_sstepx * d_countextrastep) >> 16) +
*                        ((r_tstepy + r_tstepx * d_countextrastep) >> 16) *
*                        r_affinetridesc.skinwidth;
*        d_sfracextrastep = (r_sstepy + r_sstepx*d_countextrastep) & 0xFFFF;
*        d_tfracextrastep = (r_tstepy + r_tstepx*d_countextrastep) & 0xFFFF;
*        d_lightextrastep = d_lightbasestep + working_lstepx;
*        d_ziextrastep = d_zibasestep + r_zistepx;
*

		move.l  _r_sstepy,a0
		move.l  _r_sstepx,d1
		move.l  _r_tstepy,a1
		move.l  _r_tstepx,d6
		move.l  d1,d2
		muls.l  d5,d2
		move.l  d6,d7
		muls.l  d5,d7
		add.l   a0,d2
		add.l   a1,d7
		move.l  d2,d0
		move.l  d7,d3
		and.l   #$ffff,d0
		move.l  d0,.sfracb(sp)
		and.l   #$ffff,d3
		swap    d3
		move.l  d3,.tfracb(sp)
		swap    d3
		add.l   d1,d0
		add.l   d6,d3
		and.l   #$ffff,d0
		move.l  d0,.sfracx(sp)
		and.l   #$ffff,d3
		swap    d3
		move.l  d3,.tfracx(sp)
		swap    d3
		move.l  d2,d0
		move.l  d7,d3
		swap    d2
		ext.l   d2
		swap    d7
		muls    d4,d7
		add.l   d2,d7
		move.l  d7,.ptexb(sp)
		add.l   d1,d0
		add.l   d6,d3
		swap    d0
		ext.l   d0
		swap    d3
		muls    d4,d3
		add.l   d0,d3
		move.l  d3,.ptexx(sp)
		move.l  _r_lstepy,d0
		move.l  _r_lstepx,d1
		tst.l   d5                      ;(if ubasestep < 0)
		bge.b   .ge
		subq.l  #1,d1                   ;working_lstepx = r_lstepx - 1
.ge
		move.l  d1,d2
		muls.l  d5,d1
		add.l   d0,d1
		move.l  d1,.lightb(sp)
		add.l   d2,d1
		move.l  d1,.lightx(sp)
		move.l  _r_zistepy,d0
		move.l  _r_zistepx,d1
		move.l  d1,d2
		muls.l  d5,d1
		add.l   d0,d1
		move.l  d1,.zib(sp)
		add.l   d2,d1
		move.l  d1,.zix(sp)



*****   D_PolysetScanLeftEdge (inlined)


		movem.l d2-d7/a2-a6,.savearea(sp)
		move.l  _d_pedgespanpackage,a0
		move.l  _d_pdest,a1
		move.l  _d_pz,a2
		move.l  _d_aspancount,d6
		move.l  _d_ptex,a3
		move.l  _d_sfrac,d4
		move.l  _d_tfrac,d5
		swap    d5
		move.l  _d_light,a5
		move.l  _d_zi,a4
		move.l  _errorterm,d2
		move.l  _erroradjustup,d1
		move.l  _erroradjustdown,d0
		move.l  .initLH(sp),d7
		subq    #1,d7
.loopA

*                d_pedgespanpackage->pdest = d_pdest;
*                d_pedgespanpackage->pz = d_pz;
*                d_pedgespanpackage->count = d_aspancount;
*                d_pedgespanpackage->ptex = d_ptex;
*
*                d_pedgespanpackage->sfrac = d_sfrac;
*                d_pedgespanpackage->tfrac = d_tfrac;
*
*                d_pedgespanpackage->light = d_light;
*                d_pedgespanpackage->zi = d_zi;
*
*                d_pedgespanpackage++;
*
*                errorterm += erroradjustup;
*                if (errorterm >= 0)

		move.l  a1,(a0)+
		move.l  a2,(a0)+
		move.l  d6,(a0)+
		move.l  a3,(a0)+
		move.l  d4,(a0)+
		swap    d5
		clr     (a0)+
		move    d5,(a0)+
		swap    d5
		move.l  a5,(a0)+
		move.l  a4,(a0)+
		add.l   d1,d2
		blt.b   .elseA

*                        d_pdest += d_pdestextrastep;
*                        d_pz += d_pzextrastep;
*                        d_aspancount += d_countextrastep;
*                        d_ptex += d_ptexextrastep;
*                        d_sfrac += d_sfracextrastep;
*                        d_ptex += d_sfrac >> 16;
*
*                        d_sfrac &= 0xFFFF;
*                        d_tfrac += d_tfracextrastep;
*                        if (d_tfrac & 0x10000)
*                        {
*                                d_ptex += r_affinetridesc.skinwidth;
*                                d_tfrac &= 0xFFFF;
*                        }
*                        d_light += d_lightextrastep;
*                        d_zi += d_ziextrastep;
*                        errorterm -= erroradjustdown;

		lea     .pdestx(sp),a6
		add.l   (a6)+,a1
		add.l   (a6)+,a2
		add.l   (a6)+,d6
		add.l   (a6)+,a3
		add.l   (a6)+,d4
		swap    d4
		add     d4,a3
		clr     d4
		swap    d4
		add.l   (a6)+,d5
		bcc.b   .contA
		add.l   .r_affine(sp),a3
.contA
		add.l   (a6)+,a5
		add.l   (a6)+,a4
		sub.l   d0,d2
		bra.b   .nextA
.elseA

*                        d_pdest += d_pdestbasestep;
*                        d_pz += d_pzbasestep;
*                        d_aspancount += ubasestep;
*                        d_ptex += d_ptexbasestep;
*                        d_sfrac += d_sfracbasestep;
*                        d_ptex += d_sfrac >> 16;
*                        d_sfrac &= 0xFFFF;
*                        d_tfrac += d_tfracbasestep;
*                        if (d_tfrac & 0x10000)
*                        {
*                                d_ptex += r_affinetridesc.skinwidth;
*                                d_tfrac &= 0xFFFF;
*                        }
*                        d_light += d_lightbasestep;
*                        d_zi += d_zibasestep;

		lea     .pdestb(sp),a6
		add.l   (a6)+,a1
		add.l   (a6)+,a2
		add.l   (a6)+,d6
		add.l   (a6)+,a3
		add.l   (a6)+,d4
		swap    d4
		add     d4,a3
		clr     d4
		swap    d4
		add.l   (a6)+,d5
		bcc.b   .cont2A
		add.l   .r_affine(sp),a3
.cont2A
		add.l   (a6)+,a5
		add.l   (a6)+,a4
.nextA
		dbra    d7,.loopA
		move.l  a0,_d_pedgespanpackage
		movem.l .savearea(sp),d2-d7/a2-a6

******  End of D_PolysetScanLeftEdge




*        if (pedgetable->numleftedges == 2)
*        {
*                int             height;
*
*                plefttop = pleftbottom;
*                pleftbottom = pedgetable->pleftedgevert2;
*
*                D_PolysetSetUpForLineScan(plefttop[0], plefttop[1],
*                                                          pleftbottom[0], pleftbottom[1]);
*
*                height = pleftbottom[1] - plefttop[1];
*
*// TODO: make this a function; modularize this function in general
*
*                ystart = plefttop[1];
*                d_aspancount = plefttop[0] - prighttop[0];
*                d_ptex = (byte *)r_affinetridesc.pskin + (plefttop[2] >> 16) +
*                                (plefttop[3] >> 16) * r_affinetridesc.skinwidth;
*                d_sfrac = 0;
*                d_tfrac = 0;
*                d_light = plefttop[4];
*                d_zi = plefttop[5];
*
*                d_pdestbasestep = screenwidth + ubasestep;
*                d_pdestextrastep = d_pdestbasestep + 1;
*                d_pdest = (byte *)d_viewbuffer + ystart * screenwidth + plefttop[0];
*                d_pzbasestep = d_zwidth + ubasestep;
*                d_pzextrastep = d_pzbasestep + 1;
*                d_pz = d_pzbuffer + ystart * d_zwidth + plefttop[0];
*
*                if (ubasestep < 0)
*                        working_lstepx = r_lstepx - 1;
*                else
*                        working_lstepx = r_lstepx;
*
*                d_countextrastep = ubasestep + 1;
*                d_ptexbasestep = ((r_sstepy + r_sstepx * ubasestep) >> 16) +
*                                ((r_tstepy + r_tstepx * ubasestep) >> 16) *
*                                r_affinetridesc.skinwidth;
*                d_sfracbasestep = (r_sstepy + r_sstepx * ubasestep) & 0xFFFF;
*                d_tfracbasestep = (r_tstepy + r_tstepx * ubasestep) & 0xFFFF;
*                d_lightbasestep = r_lstepy + working_lstepx * ubasestep;
*                d_zibasestep = r_zistepy + r_zistepx * ubasestep;
*
*                d_ptexextrastep = ((r_sstepy + r_sstepx * d_countextrastep) >> 16) +
*                                ((r_tstepy + r_tstepx * d_countextrastep) >> 16) *
*                                r_affinetridesc.skinwidth;
*                d_sfracextrastep = (r_sstepy+r_sstepx*d_countextrastep) & 0xFFFF;
*                d_tfracextrastep = (r_tstepy+r_tstepx*d_countextrastep) & 0xFFFF;
*                d_lightextrastep = d_lightbasestep + working_lstepx;
*                d_ziextrastep = d_zibasestep + r_zistepx;


		move.l  _pedgetable,a0
		cmp.l   #2,ETAB_NUMLEFTEDGES(a0)
		bne.b   .cont
		move.l  a3,a6
		move.l  ETAB_PLEV2(a0),a3
		move.l  4(a3),d5
		move.l  d5,d3
		move.l  (a3),d2
		move.l  4(a6),d7
		move.l  d7,d1
		move.l  (a6),d6
		move.l  d6,d0
		bsr     D_PolysetSetUpForLineScan
		sub.l   d7,d5
		move.l  d5,.height(sp)
		sub.l   (a2),d6
		move.l  d6,_d_aspancount        ;d_aspancount = plefttop[0] - ...

		move.l  2*4(a6),d0
		move.l  3*4(a6),d1
		move.l  d0,d3                   ;d3 = plefttop[2]
		move.l  d1,d6                   ;d6 = plefttop[3]
		swap    d0
		swap    d1
		and.l   #$ffff,d0
		and.l   #$ffff,d1
		mulu    d4,d1
		add.l   d0,d1
		add.l   R_PSKIN(a5),d1
		move.l  d1,_d_ptex              ;d_ptex = ...
		clr.l   _d_sfrac
		clr.l   _d_tfrac
		move.l  _d_zwidth,d0
		move.l  d0,d3                   ;d3 = d_zwidth
		move.l  _ubasestep,d1
		move.l  d1,.aspanb(sp)
		move.l  d1,d5                   ;d5 = ubasestep
		add.l   d1,d0
		addq.l  #1,d1
		move.l  d1,_d_countextrastep
		move.l  d1,.aspanx(sp)
		add.l   d0,d0
		move.l  d0,.pzb(sp)             ;d_pzbasestep = d_zwidth + ubasestep
		addq.l  #2,d0
		move.l  d0,.pzx(sp)             ;d_pzextrastep = d_pzbasestep + 1
		move.l  4*4(a6),_d_light        ;d_light = plefttop[4]
		move.l  5*4(a6),_d_zi           ;d_zi = plefttop[5]
		move.l  _screenwidth,d0
		add.l   d0,d1
		move.l  d1,.pdestx(sp)          ;d_pdestextrastep = ...
		subq.l  #1,d1
		move.l  d1,.pdestb(sp)          ;d_pdestbasestep = screenwidth + ..
		mulu    d7,d0                   ;screenwidth * ystart
		move.l  (a6),d6
		add.l   d6,d0                   ;+ plefttop[0]
		add.l   _d_viewbuffer,d0
		move.l  d0,_d_pdest             ;d_pdest + d_viewbuffer + ...
		mulu    d7,d3                   ;d_zwidth * ystart
		add.l   d6,d3                   ;+ plefttop[0]
		add.l   d3,d3
		add.l   _d_pzbuffer,d3
		move.l  d3,_d_pz                ;d_pz = d_pzbuffer + ...

		move.l  _r_sstepy,a0
		move.l  _r_sstepx,d1
		move.l  _r_tstepy,a1
		move.l  _r_tstepx,d6
		move.l  d1,d2
		muls.l  d5,d2
		move.l  d6,d7
		muls.l  d5,d7
		add.l   a0,d2
		add.l   a1,d7
		move.l  d2,d0
		move.l  d7,d3
		and.l   #$ffff,d0
		move.l  d0,.sfracb(sp)
		and.l   #$ffff,d3
		swap    d3
		move.l  d3,.tfracb(sp)
		swap    d3
		add.l   d1,d0
		add.l   d6,d3
		and.l   #$ffff,d0
		move.l  d0,.sfracx(sp)
		and.l   #$ffff,d3
		swap    d3
		move.l  d3,.tfracx(sp)
		swap    d3
		move.l  d2,d0
		move.l  d7,d3
		swap    d2
		ext.l   d2
		swap    d7
		muls    d4,d7
		add.l   d2,d7
		move.l  d7,.ptexb(sp)
		add.l   d1,d0
		add.l   d6,d3
		swap    d0
		ext.l   d0
		swap    d3
		muls    d4,d3
		add.l   d0,d3
		move.l  d3,.ptexx(sp)
		move.l  _r_lstepy,d0
		move.l  _r_lstepx,d1
		tst.l   d5                      ;(if ubasestep < 0)
		bge.b   .ge2
		subq.l  #1,d1                   ;working_lstepx = r_lstepx - 1
.ge2
		move.l  d1,d2
		muls.l  d5,d1
		add.l   d0,d1
		move.l  d1,.lightb(sp)
		add.l   d2,d1
		move.l  d1,.lightx(sp)
		move.l  _r_zistepy,d0
		move.l  _r_zistepx,d1
		move.l  d1,d2
		muls.l  d5,d1
		add.l   d0,d1
		move.l  d1,.zib(sp)
		add.l   d2,d1
		move.l  d1,.zix(sp)


*****   D_PolysetScanLeftEdge (inlined)


		movem.l d2-d7/a2-a6,.savearea(sp)
		move.l  _d_pedgespanpackage,a0
		move.l  _d_pdest,a1
		move.l  _d_pz,a2
		move.l  _d_aspancount,d6
		move.l  _d_ptex,a3
		move.l  _d_sfrac,d4
		move.l  _d_tfrac,d5
		swap    d5
		move.l  _d_light,a5
		move.l  _d_zi,a4
		move.l  _errorterm,d2
		move.l  _erroradjustup,d1
		move.l  _erroradjustdown,d0
		move.l  .height(sp),d7
		subq    #1,d7
.loopB

*                d_pedgespanpackage->pdest = d_pdest;
*                d_pedgespanpackage->pz = d_pz;
*                d_pedgespanpackage->count = d_aspancount;
*                d_pedgespanpackage->ptex = d_ptex;
*
*                d_pedgespanpackage->sfrac = d_sfrac;
*                d_pedgespanpackage->tfrac = d_tfrac;
*
*                d_pedgespanpackage->light = d_light;
*                d_pedgespanpackage->zi = d_zi;
*
*                d_pedgespanpackage++;
*
*                errorterm += erroradjustup;
*                if (errorterm >= 0)

		move.l  a1,(a0)+
		move.l  a2,(a0)+
		move.l  d6,(a0)+
		move.l  a3,(a0)+
		move.l  d4,(a0)+
		swap    d5
		clr     (a0)+
		move    d5,(a0)+
		swap    d5
		move.l  a5,(a0)+
		move.l  a4,(a0)+
		add.l   d1,d2
		blt.b   .elseB

*                        d_pdest += d_pdestextrastep;
*                        d_pz += d_pzextrastep;
*                        d_aspancount += d_countextrastep;
*                        d_ptex += d_ptexextrastep;
*                        d_sfrac += d_sfracextrastep;
*                        d_ptex += d_sfrac >> 16;
*
*                        d_sfrac &= 0xFFFF;
*                        d_tfrac += d_tfracextrastep;
*                        if (d_tfrac & 0x10000)
*                        {
*                                d_ptex += r_affinetridesc.skinwidth;
*                                d_tfrac &= 0xFFFF;
*                        }
*                        d_light += d_lightextrastep;
*                        d_zi += d_ziextrastep;
*                        errorterm -= erroradjustdown;

		lea     .pdestx(sp),a6
		add.l   (a6)+,a1
		add.l   (a6)+,a2
		add.l   (a6)+,d6
		add.l   (a6)+,a3
		add.l   (a6)+,d4
		swap    d4
		add     d4,a3
		clr     d4
		swap    d4
		add.l   (a6)+,d5
		bcc.b   .contB
		add.l   .r_affine(sp),a3
.contB
		add.l   (a6)+,a5
		add.l   (a6)+,a4
		sub.l   d0,d2
		bra.b   .nextB
.elseB

*                        d_pdest += d_pdestbasestep;
*                        d_pz += d_pzbasestep;
*                        d_aspancount += ubasestep;
*                        d_ptex += d_ptexbasestep;
*                        d_sfrac += d_sfracbasestep;
*                        d_ptex += d_sfrac >> 16;
*                        d_sfrac &= 0xFFFF;
*                        d_tfrac += d_tfracbasestep;
*                        if (d_tfrac & 0x10000)
*                        {
*                                d_ptex += r_affinetridesc.skinwidth;
*                                d_tfrac &= 0xFFFF;
*                        }
*                        d_light += d_lightbasestep;
*                        d_zi += d_zibasestep;

		lea     .pdestb(sp),a6
		add.l   (a6)+,a1
		add.l   (a6)+,a2
		add.l   (a6)+,d6
		add.l   (a6)+,a3
		add.l   (a6)+,d4
		swap    d4
		add     d4,a3
		clr     d4
		swap    d4
		add.l   (a6)+,d5
		bcc.b   .cont2B
		add.l   .r_affine(sp),a3
.cont2B
		add.l   (a6)+,a5
		add.l   (a6)+,a4
.nextB
		dbra    d7,.loopB
		move.l  a0,_d_pedgespanpackage
		movem.l .savearea(sp),d2-d7/a2-a6


.cont

*        d_pedgespanpackage = a_spans;
*
*        D_PolysetSetUpForLineScan(prighttop[0], prighttop[1],
*                                                  prightbottom[0], prightbottom[1]);
*        d_aspancount = 0;
*        d_countextrastep = ubasestep + 1;
*        originalcount = a_spans[initialrightheight].count;
*        a_spans[initialrightheight].count = -999999; // mark end of the spanpackages
*        D_PolysetDrawSpans8 (a_spans);

		move.l  _a_spans,_d_pedgespanpackage
		move.l  4(a4),d3
		move.l  (a4),d2
		move.l  4(a2),d1
		move.l  (a2),d0
		bsr     D_PolysetSetUpForLineScan
		clr.l   _d_aspancount
		move.l  _ubasestep,d0
		addq.l  #1,d0
		move.l  d0,_d_countextrastep
		move.l  _a_spans,a3
		move.l  .initRH(sp),d6
		move.l  d6,d0
		asl.l   #PSPANP_SIZEOF_EXP,d0
		move.l  PSPANP_COUNT(a3,d0.l),d7
		move.l  #-999999,PSPANP_COUNT(a3,d0.l)
		move.l  a3,-(sp)
		;bsr     _D_PolysetDrawSpans8
		move.l  _d_polysetdrawspans,a0
		jsr     (a0)
		addq    #4,sp

*        if (pedgetable->numrightedges == 2)
*        {
*                int                             height;
*                spanpackage_t   *pstart;
*
*                pstart = a_spans + initialrightheight;
*                pstart->count = originalcount;
*
*                d_aspancount = prightbottom[0] - prighttop[0];
*
*                prighttop = prightbottom;
*                prightbottom = pedgetable->prightedgevert2;
*
*                height = prightbottom[1] - prighttop[1];
*
*                D_PolysetSetUpForLineScan(prighttop[0], prighttop[1],
*                                                          prightbottom[0], prightbottom[1]);
*
*                d_countextrastep = ubasestep + 1;
*                a_spans[initialrightheight + height].count = -999999;
*                                                                                        // mark end of the spanpackages
*                D_PolysetDrawSpans8 (pstart);

		move.l  _pedgetable,a0
		cmp.l   #2,ETAB_NUMRIGHTEDGES(a0)
		bne.b   .exit
		move.l  a3,a6
		move.l  d6,d0
		asl.l   #PSPANP_SIZEOF_EXP,d0
		add.l   d0,a6
		move.l  d7,PSPANP_COUNT(a6)
		move.l  (a4),d0
		sub.l   (a2),d0
		move.l  d0,_d_aspancount
		move.l  a4,a2
		move.l  ETAB_PREV2(a0),a4
		move.l  4(a4),d5
		move.l  d5,d3
		move.l  4(a2),d0
		sub.l   d0,d5
		move.l  (a4),d2
		move.l  d0,d1
		move.l  (a2),d0
		bsr     D_PolysetSetUpForLineScan
		move.l  _ubasestep,d4
		addq.l  #1,d4
		move.l  d4,_d_countextrastep
		add.l   d6,d5
		asl.l   #PSPANP_SIZEOF_EXP,d5
		move.l  #-999999,PSPANP_COUNT(a3,d5.l)
		move.l  a6,-(sp)
		;bsr     _D_PolysetDrawSpans8
		move.l  _d_polysetdrawspans,a0
		jsr     (a0)
		addq    #4,sp
.exit
		add     #.intregs,sp
		movem.l (sp)+,d2-d7/a2-a6
		rts








******************************************************************************
*
*       void D_DrawNonSubdiv (void)
*
******************************************************************************
		cnop    0,4
_D_DrawNonSubdiv

		movem.l d2-d7/a2-a6,-(sp)

*        pfv = r_affinetridesc.pfinalverts;
*        ptri = r_affinetridesc.ptriangles;
*        lnumtriangles = r_affinetridesc.numtriangles;

		lea     _r_affinetridesc,a0
		move.l  R_PFINALVERTS(a0),a2    ;pfv = f_affinetridesc.pfinalverts
		move.l  R_PTRIANGLES(a0),a3     ;ptri = f_affinetridesc.ptriangles
		move.l  R_NUMTRIANGLES(a0),d3   ;lnumtriangles = r_affine...
		subq    #1,d3
.loop

*                index0 = pfv + ptri->vertindex[0];
*                index1 = pfv + ptri->vertindex[1];
*                index2 = pfv + ptri->vertindex[2];
*
*                d_xdenom = (index0->v[1]-index1->v[1]) *
*                                (index0->v[0]-index2->v[0]) -
*                                (index0->v[0]-index1->v[0])*(index0->v[1]-index2->v[1]);

		clr.l   d0
		move.w  MT_VERTINDEX+0*2(a3),d0
		asl.l   #FV_SIZEOF_EXP,d0
		lea     0(a2,d0.l),a0           ;a0 = index0
		clr.l   d0
		move.w  MT_VERTINDEX+1*2(a3),d0
		asl.l   #FV_SIZEOF_EXP,d0
		lea     0(a2,d0.l),a1           ;a1 = index1
		clr.l   d0
		move.w  MT_VERTINDEX+2*2(a3),d0
		asl.l   #FV_SIZEOF_EXP,d0
		lea     0(a2,d0.l),a4           ;a2 = index2
		movem.l d3/a3,-(sp)
		move.l  (a0)+,d0                ;d0 = index0->v[0]
		move.l  (a0)+,d1                ;d1 = index0->v[1]
		move.l  (a1)+,a6                ;d2 = index1->v[0]
		move.l  (a1)+,d2                ;a6 = index1->v[1]
		move.l  (a4)+,d4                ;d4 = index2->v[0]
		move.l  (a4)+,d5                ;d5 = index2->v[1]
		move.l  d0,d6
		move.l  d1,d7
		sub.l   d4,d6                   ;index0->v[0]-index2->v[0]
		sub.l   d2,d7                   ;index0->v[1]-index1->v[1]
		move.l  d6,d3
		muls.l  d7,d3
		move.l  d0,d6
		move.l  d1,d7
		sub.l   a6,d6                   ;index0->v[0]-index1->v[0]
		sub.l   d5,d7                   ;index0->v[1]-index2->v[1]
		muls.l  d6,d7

*                if (d_xdenom >= 0)
*                {
*                        continue;
*                }

		sub.l   d7,d3
		bge.w   .next                   ;if (d_xdenom >= 0)
		move.l  d3,_d_xdenom            ;d_xdenom = ...

*                r_p0[0] = index0->v[0];         // u
*                r_p0[1] = index0->v[1];         // v
*                r_p0[2] = index0->v[2];         // s
*                r_p0[3] = index0->v[3];         // t
*                r_p0[4] = index0->v[4];         // light
*                r_p0[5] = index0->v[5];         // iz
*
*                r_p1[0] = index1->v[0];
*                r_p1[1] = index1->v[1];
*                r_p1[2] = index1->v[2];
*                r_p1[3] = index1->v[3];
*                r_p1[4] = index1->v[4];
*                r_p1[5] = index1->v[5];
*
*                r_p2[0] = index2->v[0];
*                r_p2[1] = index2->v[1];
*                r_p2[2] = index2->v[2];
*                r_p2[3] = index2->v[3];
*                r_p2[4] = index2->v[4];
*                r_p2[5] = index2->v[5];

		lea     _r_p1,a5
		move.l  a6,(a5)+                ;r_p1[0] = index1->v[0]
		move.l  d2,(a5)+                ;r_p1[1] = index1->v[1]
		move.l  (a1)+,(a5)+             ;...
		move.l  (a1)+,(a5)+
		move.l  (a1)+,(a5)+
		move.l  (a1)+,(a5)+
		lea     _r_p0,a6
		move.l  d0,(a6)+                ;r_p0[0] = index0->v[0]
		move.l  d1,(a6)+                ;...
		move.l  (a0)+,(a6)+
		move.l  (a0)+,(a6)+
		move.l  (a0)+,(a6)+
		move.l  (a0)+,(a6)+
		move.l  MT_FACESFRONT(a3),d6
		lea     _r_p2,a3                ;r_p2[0] = index2->v[0]
		move.l  d4,(a3)+                ;...
		move.l  d5,(a3)+
		move.l  (a4)+,(a3)+
		move.l  (a4)+,(a3)+
		move.l  (a4)+,(a3)+
		move.l  (a4)+,(a3)+

*                if (!ptri->facesfront)
*                {
*                        if (index0->flags & ALIAS_ONSEAM)
*                                r_p0[2] += r_affinetridesc.seamfixupX16;
*                        if (index1->flags & ALIAS_ONSEAM)
*                                r_p1[2] += r_affinetridesc.seamfixupX16;
*                        if (index2->flags & ALIAS_ONSEAM)
*                                r_p2[2] += r_affinetridesc.seamfixupX16;
*                }

		tst.l   d6                      ;if (!ptri->facesfront)
		bne.b   .cont
		move.l  _r_affinetridesc+R_SEAMFIXUP16,d3
		move.l  (a0),d0                 ;if (index0->flags & ALIAS_ONSEAM)
		and.l   #ALIAS_ONSEAM,d0
		beq.b   .1
		add.l   d3,-16(a6)              ;r_p0[2] += r_affinetridesc.se...
.1
		move.l  (a1),d0
		and.l   #ALIAS_ONSEAM,d0
		beq.b   .2
		add.l   d3,-16(a5)
.2
		move.l  (a4),d0
		and.l   #ALIAS_ONSEAM,d0
		beq.b   .cont
		add.l   d3,-16(a3)
.cont

******  D_PolysetSetEdgeTable (inlined)

		cmp.l   d2,d1
		blt.b   .lt1
		beq.b   .eq1
.gt1
		moveq   #ETAB_SIZEOF,d0
		cmp.l   d5,d1
		beq.b   .eq2
		cmp.l   d5,d2
		beq.b   .eq3
		cmp.l   d5,d1
		ble.b   .skip
		add.l   #2*ETAB_SIZEOF,d0
.skip
		cmp.l   d5,d2
		ble.b   .skip2
		add.l   #4*ETAB_SIZEOF,d0
.skip2
		add.l   #_edgetables,d0
		move.l  d0,_pedgetable
		bra.b   .done
.eq2
		move.l  #_edgetables+8*ETAB_SIZEOF,_pedgetable
		bra.b   .done
.eq3
		move.l  #_edgetables+10*ETAB_SIZEOF,_pedgetable
		bra.b   .done
.eq1
		cmp.l   d5,d1
		bge.b   .ge
		move.l  #_edgetables+2*ETAB_SIZEOF,_pedgetable
		bra.b   .done
.ge
		move.l  #_edgetables+5*ETAB_SIZEOF,_pedgetable
		bra.b   .done
.lt1
		moveq   #0,d0
		cmp.l   d5,d1
		beq.b   .eq4
		cmp.l   d5,d2
		beq.b   .eq5
		cmp.l   d5,d1
		ble.b   .skip3
		add.l   #2*ETAB_SIZEOF,d0
.skip3
		cmp.l   d5,d2
		ble.b   .skip4
		add.l   #4*ETAB_SIZEOF,d0
.skip4
		add.l   #_edgetables,d0
		move.l  d0,_pedgetable
		bra.b   .done
.eq4
		move.l  #_edgetables+9*ETAB_SIZEOF,_pedgetable
		bra.b   .done
.eq5
		move.l  #_edgetables+11*ETAB_SIZEOF,_pedgetable
.done

******  end of D_PolysetSetEdgeTable

		bsr     _D_RasterizeAliasPolySmooth
		;jsr     _D_RasterizeAliasPolySmooth
.next
		movem.l (sp)+,d3/a3
		lea     MT_SIZEOF(a3),a3        ;ptri++
		dbra    d3,.loop
		movem.l (sp)+,d2-d7/a2-a6
		rts







**
** 680x0 optimised Quake render routines by John Selck.
**

******************************************************************************
*
*       void D_PolysetDrawFinalVerts (finalvert_t *pv1, finalvert_t *pv2, finalvert_t *pv3)
*
*       entity polygon renderer for very small polygons (1 pixel)
*
******************************************************************************

		cnop    0,4
_D_PolysetDrawFinalVerts


*****   stackframe

		rsreset
.intregs        rs.l    9
		rs.l    1
.pv1            rs.l    1
.pv2            rs.l    1
.pv3            rs.l    1


****** prologue

		movem.l	d2-d5/a2-a6,-(sp) ; TODO d3 is not used

		lea		_zspantable,a1
		lea		_skintable,a2
		lea		_d_scantable,a3
		move.l	_acolormap,a4
		move.l	_d_viewbuffer,a5

		move.l	_r_refdef+REFDEF_VRECTRIGHT,d4
		move.l	_r_refdef+REFDEF_VRECTBOTTOM,d5

		moveq	#0,d2

		move.l	.pv1(sp),a6
		bsr.b	do_PolysetDrawFinalVerts
		move.l	.pv2(sp),a6
		bsr.b	do_PolysetDrawFinalVerts
		move.l	.pv3(sp),a6
		bsr.b	do_PolysetDrawFinalVerts

		movem.l	(sp)+,d2-d5/a2-a6
		rts

		cnop	0,4
do_PolysetDrawFinalVerts
		move.l	(a6),d0					;d0 = pv->v[0]
		cmp.l	d4,d0					;if (d0 < r_refdef.vrectright)
		bge.b	.fvskip
		move.l	4(a6),d1				;d1 = pv->v[1]
		cmp.l	d5,d1					;if (d1 < r_refdef.vrectbottom)
		bge.b	.fvskip

		move.l	(a1,d1.w*4),a0
		lea		0(a0,d0.w*2),a0			;a0 = zspantable[pv->v[1]] + pv->v[0]
		move.w	20(a6),d2				;d2 = pv->v[5]>>16
		cmp.w	(a0),d2					;if (d2 >= *zbuf)
		blt.b	.fvskip
		move.w	d2,(a0)					;*zbuf = d2

		move.w	12(a6),d0				;d0 = pv->v[3]>>16
		move.l	(a2,d0.w*4),a0			;a0 = skintable[d0]
		add.w	8(a6),a0				;a0 = a0[pv->v[2]>>16]
		move.w	18(a6),d2				;d2 = pv->v[4]
		move.b	(a0),d2					;d2 = *a0 + pv->v[4] & 0xFF00

		move.l	(a3,d1.w*4),d0			;d0 = d_scantable[pv->v[1]]
		add.l	(a6),d0					;d0 = d0 + pv->v[0]

		move.b	0(a4,d2.l),0(a5,d0.l)	;d_viewbuffer[d0] = acolormap[d2]
.fvskip
		rts


******************************************************************************
*
*       void D_PolysetDrawFinalVertsT (finalvert_t *pv1, finalvert_t *pv2, finalvert_t *pv3)
*
*       translucent entity polygon renderer for very small polygons (1 pixel)
*
******************************************************************************

		cnop    0,4
_D_PolysetDrawFinalVertsT


*****   stackframe

		rsreset
.intregs        rs.l    9
		rs.l    1
.pv1            rs.l    1
.pv2            rs.l    1
.pv3            rs.l    1


****** prologue

		movem.l	d2-d5/a2-a6,-(sp) ; TODO d3 is not used

		lea		_zspantable,a1
		lea		_skintable,a2
		lea		_d_scantable,a3
		move.l	_acolormap,a4
		move.l	_d_viewbuffer,a5

		move.l	_r_refdef+REFDEF_VRECTRIGHT,d4
		move.l	_r_refdef+REFDEF_VRECTBOTTOM,d5

		moveq	#0,d2

		move.l	.pv1(sp),a6
		bsr.b	do_PolysetDrawFinalVertsT
		move.l	.pv2(sp),a6
		bsr.b	do_PolysetDrawFinalVertsT
		move.l	.pv3(sp),a6
		bsr.b	do_PolysetDrawFinalVertsT

		movem.l	(sp)+,d2-d5/a2-a6
		rts

		cnop	0,4
do_PolysetDrawFinalVertsT
		move.l	(a6),d0					;d0 = pv->v[0]
		cmp.l	d4,d0					;if (d0 < r_refdef.vrectright)
		bge.b	.fvskip
		move.l	4(a6),d1				;d1 = pv->v[1]
		cmp.l	d5,d1					;if (d1 < r_refdef.vrectbottom)
		bge.b	.fvskip

		move.l	(a1,d1.w*4),a0
		lea		0(a0,d0.w*2),a0			;a0 = zspantable[pv->v[1]] + pv->v[0]
		move.w	20(a6),d2				;d2 = pv->v[5]>>16
		cmp.w	(a0),d2					;if (d2 >= *zbuf)
		blt.b	.fvskip
		move.w	d2,(a0)					;*zbuf = d2

		move.w	12(a6),d0				;d0 = pv->v[3]>>16
		move.l	(a2,d0.w*4),a0			;a0 = skintable[d0]
		add.w	8(a6),a0				;a0 = a0[pv->v[2]>>16]
		move.w	18(a6),d2				;d2 = pv->v[4]
		move.b	(a0),d2					;d2 = *a0 + pv->v[4] & 0xFF00
		tst.b	d2						;if (color_map_idx != 0)
		beq.b	.fvskip

		move.l	(a3,d1.w*4),d0			;d0 = d_scantable[pv->v[1]]
		add.l	(a6),d0					;d0 = d0 + pv->v[0]

		;move.b	0(a4,d2.l),0(a5,d0.l)	;d_viewbuffer[d0] = acolormap[d2]
		move.b	0(a4,d2.l),d2			;d2 = acolormap[d2]
		lsl.w   #8,d2
		move.b	0(a5,d0.l),d2			;d2 = (pix<<8) + pix2
		move.l	_mainTransTable,a0
		move.b  0(a0,d2.l),0(a5,d0.l)	;d_viewbuffer[d0] = mainTransTable[d2]
.fvskip
		rts


******************************************************************************
*
*       void D_PolysetDrawFinalVertsT2 (finalvert_t *pv1, finalvert_t *pv2, finalvert_t *pv3)
*
*       transparent entity polygon renderer for very small polygons (1 pixel)
*
******************************************************************************

		cnop    0,4
_D_PolysetDrawFinalVertsT2


*****   stackframe

		rsreset
.intregs        rs.l    9
		rs.l    1
.pv1            rs.l    1
.pv2            rs.l    1
.pv3            rs.l    1


****** prologue

		movem.l	d2-d5/a2-a6,-(sp) ; TODO d3 is not used

		lea		_zspantable,a1
		lea		_skintable,a2
		lea		_d_scantable,a3
		move.l	_acolormap,a4
		move.l	_d_viewbuffer,a5

		move.l	_r_refdef+REFDEF_VRECTRIGHT,d4
		move.l	_r_refdef+REFDEF_VRECTBOTTOM,d5

		moveq	#0,d2

		move.l	.pv1(sp),a6
		bsr.b	do_PolysetDrawFinalVertsT2
		move.l	.pv2(sp),a6
		bsr.b	do_PolysetDrawFinalVertsT2
		move.l	.pv3(sp),a6
		bsr.b	do_PolysetDrawFinalVertsT2

		movem.l	(sp)+,d2-d5/a2-a6
		rts

		cnop	0,4
do_PolysetDrawFinalVertsT2
		move.l	(a6),d0					;d0 = pv->v[0]
		cmp.l	d4,d0					;if (d0 < r_refdef.vrectright)
		bge.b	.fvskip
		move.l	4(a6),d1				;d1 = pv->v[1]
		cmp.l	d5,d1					;if (d1 < r_refdef.vrectbottom)
		bge.b	.fvskip

		move.l	(a1,d1.w*4),a0
		lea		0(a0,d0.w*2),a0			;a0 = zspantable[pv->v[1]] + pv->v[0]
		move.w	20(a6),d2				;d2 = pv->v[5]>>16
		cmp.w	(a0),d2					;if (d2 >= *zbuf)
		blt.b	.fvskip
		move.w	d2,(a0)					;*zbuf = d2

		move.w	12(a6),d0				;d0 = pv->v[3]>>16
		move.l	(a2,d0.w*4),a0			;a0 = skintable[d0]
		add.w	8(a6),a0				;a0 = a0[pv->v[2]>>16]
		move.w	18(a6),d2				;d2 = pv->v[4]
		move.b	(a0),d2					;d2 = *a0 + pv->v[4] & 0xFF00
		tst.b	d2						;if (color_map_idx != 0)
		beq.b	.fvskip

		move.l	(a3,d1.w*4),d0			;d0 = d_scantable[pv->v[1]]
		add.l	(a6),d0					;d0 = d0 + pv->v[0]

		btst	#0,d2					;if (color_map_idx % 2 == 0)
		bne.b	.transluc
		move.b	0(a4,d2.l),0(a5,d0.l)	;d_viewbuffer[d0] = acolormap[d2]
		rts
.transluc
		move.b	0(a4,d2.l),d2			;d2 = acolormap[d2]
		lsl.w   #8,d2
		move.b	0(a5,d0.l),d2			;d2 = (pix<<8) + pix2
		move.l	_mainTransTable,a0
		move.b  0(a0,d2.l),0(a5,d0.l)	;d_viewbuffer[d0] = mainTransTable[d2]
.fvskip
		rts


******************************************************************************
*
*       void D_PolysetDrawFinalVertsT3 (finalvert_t *pv1, finalvert_t *pv2, finalvert_t *pv3)
*
*       holey entity polygon renderer for very small polygons (1 pixel)
*
******************************************************************************

		cnop    0,4
_D_PolysetDrawFinalVertsT3


*****   stackframe

		rsreset
.intregs        rs.l    9
		rs.l    1
.pv1            rs.l    1
.pv2            rs.l    1
.pv3            rs.l    1


****** prologue

		movem.l	d2-d5/a2-a6,-(sp) ; TODO d3 is not used

		lea		_zspantable,a1
		lea		_skintable,a2
		lea		_d_scantable,a3
		move.l	_acolormap,a4
		move.l	_d_viewbuffer,a5

		move.l	_r_refdef+REFDEF_VRECTRIGHT,d4
		move.l	_r_refdef+REFDEF_VRECTBOTTOM,d5

		moveq	#0,d2

		move.l	.pv1(sp),a6
		bsr.b	do_PolysetDrawFinalVertsT3
		move.l	.pv2(sp),a6
		bsr.b	do_PolysetDrawFinalVertsT3
		move.l	.pv3(sp),a6
		bsr.b	do_PolysetDrawFinalVertsT3

		movem.l	(sp)+,d2-d5/a2-a6
		rts

		cnop	0,4
do_PolysetDrawFinalVertsT3
		move.l	(a6),d0					;d0 = pv->v[0]
		cmp.l	d4,d0					;if (d0 < r_refdef.vrectright)
		bge.b	.fvskip
		move.l	4(a6),d1				;d1 = pv->v[1]
		cmp.l	d5,d1					;if (d1 < r_refdef.vrectbottom)
		bge.b	.fvskip

		move.l	(a1,d1.w*4),a0
		lea		0(a0,d0.w*2),a0			;a0 = zspantable[pv->v[1]] + pv->v[0]
		move.w	20(a6),d2				;d2 = pv->v[5]>>16
		cmp.w	(a0),d2					;if (d2 >= *zbuf)
		blt.b	.fvskip
		move.w	d2,(a0)					;*zbuf = d2

		move.w	12(a6),d0				;d0 = pv->v[3]>>16
		move.l	(a2,d0.w*4),a0			;a0 = skintable[d0]
		add.w	8(a6),a0				;a0 = a0[pv->v[2]>>16]
		move.w	18(a6),d2				;d2 = pv->v[4]
		move.b	(a0),d2					;d2 = *a0 + pv->v[4] & 0xFF00
		tst.b	d2						;if (color_map_idx != 0)
		beq.b	.fvskip

		move.l	(a3,d1.w*4),d0			;d0 = d_scantable[pv->v[1]]
		add.l	(a6),d0					;d0 = d0 + pv->v[0]

		move.b	0(a4,d2.l),0(a5,d0.l)	;d_viewbuffer[d0] = acolormap[d2]
.fvskip
		rts


******************************************************************************
*
*       void D_PolysetDrawFinalVertsT5 (finalvert_t *pv1, finalvert_t *pv2, finalvert_t *pv3)
*
*       translucent entity polygon renderer for very small polygons (1 pixel)
*
******************************************************************************

		cnop    0,4
_D_PolysetDrawFinalVertsT5


*****   stackframe

		rsreset
.intregs        rs.l    9
		rs.l    1
.pv1            rs.l    1
.pv2            rs.l    1
.pv3            rs.l    1


****** prologue

		movem.l	d2-d5/a2-a6,-(sp) ; TODO d3 and a4 are not used

		lea		_zspantable,a1
		lea		_skintable,a2
		lea		_d_scantable,a3
		;move.l	_acolormap,a4
		move.l	_d_viewbuffer,a5

		move.l	_r_refdef+REFDEF_VRECTRIGHT,d4
		move.l	_r_refdef+REFDEF_VRECTBOTTOM,d5

		moveq	#0,d2

		move.l	.pv1(sp),a6
		bsr.b	do_PolysetDrawFinalVertsT5
		move.l	.pv2(sp),a6
		bsr.b	do_PolysetDrawFinalVertsT5
		move.l	.pv3(sp),a6
		bsr.b	do_PolysetDrawFinalVertsT5

		movem.l	(sp)+,d2-d5/a2-a6
		rts

		cnop	0,4
do_PolysetDrawFinalVertsT5
		move.l	(a6),d0					;d0 = pv->v[0]
		cmp.l	d4,d0					;if (d0 < r_refdef.vrectright)
		bge.b	.fvskip
		move.l	4(a6),d1				;d1 = pv->v[1]
		cmp.l	d5,d1					;if (d1 < r_refdef.vrectbottom)
		bge.b	.fvskip

		move.l	(a1,d1.w*4),a0
		lea		0(a0,d0.w*2),a0			;a0 = zspantable[pv->v[1]] + pv->v[0]
		move.w	20(a6),d2				;d2 = pv->v[5]>>16
		cmp.w	(a0),d2					;if (d2 >= *zbuf)
		blt.b	.fvskip
		move.w	d2,(a0)					;*zbuf = d2

		move.w	12(a6),d0				;d0 = pv->v[3]>>16
		move.l	(a2,d0.w*4),a0			;a0 = skintable[d0]
		add.w	8(a6),a0				;a0 = a0[pv->v[2]>>16]
		;move.w	18(a6),d2				;d2 = pv->v[4]
		move.b	(a0),d2					;d2 = *a0 + pv->v[4] & 0xFF00
		tst.b	d2						;if (color_map_idx != 0)
		beq.b	.fvskip

		move.l	(a3,d1.w*4),d0			;d0 = d_scantable[pv->v[1]]
		add.l	(a6),d0					;d0 = d0 + pv->v[0]

		;move.b	0(a4,d2.l),0(a5,d0.l)	;d_viewbuffer[d0] = acolormap[d2]
		;move.b	0(a4,d2.l),d2			;d2 = acolormap[d2]
		lsl.w   #8,d2
		move.b	0(a5,d0.l),d2			;d2 = (pix<<8) + pix2
		move.l	_transTable,a0
		move.b  0(a0,d2.l),0(a5,d0.l)	;d_viewbuffer[d0] = transTable[d2]
.fvskip
		rts
