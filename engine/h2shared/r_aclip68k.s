**
** Quake for AMIGA
** r_aclip.c assembler implementations by Frank Wille <frank@phoenix.owl.de>
**

		;INCLUDE	"quakedef68k.i"
REFDEF_ALIASVRECT	equ	20
REFDEF_ALIASVRECTRIGHT	equ	48
REFDEF_ALIASVRECTBOTTOM	equ	52
VRECT_X	equ	0
VRECT_Y	equ	4

		XREF    _r_refdef

		XDEF    _R_Alias_clip_left
		XDEF    _R_Alias_clip_right
		XDEF    _R_Alias_clip_top
		XDEF    _R_Alias_clip_bottom

ALIAS_LEFT_CLIP         =       1
ALIAS_TOP_CLIP          =       2
ALIAS_RIGHT_CLIP        =       4
ALIAS_BOTTOM_CLIP       =       8



		fpu

******************************************************************************
*
*       void _R_Alias_clip_left (finalvert_t *pfv0, finalvert_t *pfv1,
*                                finalvert_t *out)
*
******************************************************************************

		cnop    0,4
_R_Alias_clip_left

*****   stackframe

		rsreset
.fpuregs        rs.x    2
.intregs        rs.l    5
		rs.l    1
.pfv0           rs.l    1
.pfv1           rs.l    1
.out            rs.l    1


		movem.l d2-d4/a2/a3,-(sp)
		fmovem.x        fp2/fp3,-(sp)
		move.l  .pfv0(sp),a0
		move.l  .pfv1(sp),a1
		move.l  .out(sp),a2
		lea     _r_refdef,a3
		fmove.s #0.5,fp2
		move.l  (a0)+,d2
		move.l  (a0)+,d3
		move.l  (a1)+,d0
		move.l  (a1)+,d1
		move.l  REFDEF_ALIASVRECT+VRECT_X(a3),d4
		cmp.l   d1,d3
		blt.b   .cont
		sub.l   d2,d4
		fmove.l d4,fp0
		fmove   fp0,fp3
		fadd    fp2,fp3
		move.l  d0,d4
		sub.l   d2,d4
		fmove.l d4,fp1
		fdiv    fp1,fp0
		fmove.l fp3,d0
		add.l   d2,d0
		move.l  d0,(a2)+
		moveq   #5-1,d2
		bra.b   .entry
.loop
		move.l  (a0)+,d3
		move.l  (a1)+,d1
.entry
		sub.l   d3,d1
		fmove.l d1,fp1
		fmul    fp0,fp1
		fadd    fp2,fp1
		fmove.l fp1,d1
		add.l   d3,d1
		move.l  d1,(a2)+
		dbra    d2,.loop
		bra.b   .exit
.cont
		sub.l   d0,d4
		fmove.l d4,fp0
		fmove   fp0,fp3
		fadd    fp2,fp3
		move.l  d2,d4
		sub.l   d0,d4
		fmove.l d4,fp1
		fdiv    fp1,fp0
		fmove.l fp3,d2
		add.l   d0,d2
		move.l  d2,(a2)+
		moveq   #5-1,d2
		bra.b   .entry2
.loop2
		move.l  (a0)+,d3
		move.l  (a1)+,d1
.entry2
		sub.l   d1,d3
		fmove.l d3,fp1
		fmul    fp0,fp1
		fadd    fp2,fp1
		fmove.l fp1,d3
		add.l   d1,d3
		move.l  d3,(a2)+
		dbra    d2,.loop2
.exit
		fmovem.x        (sp)+,fp2/fp3
		movem.l (sp)+,d2-d4/a2/a3
		rts






******************************************************************************
*
*       void _R_Alias_clip_right (finalvert_t *pfv0, finalvert_t *pfv1,
*                                 finalvert_t *out)
*
******************************************************************************

		cnop    0,4
_R_Alias_clip_right

*****   stackframe

		rsreset
.fpuregs        rs.x    2
.intregs        rs.l    5
		rs.l    1
.pfv0           rs.l    1
.pfv1           rs.l    1
.out            rs.l    1


		movem.l d2-d4/a2/a3,-(sp)
		fmovem.x        fp2/fp3,-(sp)
		move.l  .pfv0(sp),a0
		move.l  .pfv1(sp),a1
		move.l  .out(sp),a2
		lea     _r_refdef,a3
		fmove.s #0.5,fp2
		move.l  (a0)+,d2
		move.l  (a0)+,d3
		move.l  (a1)+,d0
		move.l  (a1)+,d1
		move.l  REFDEF_ALIASVRECTRIGHT(a3),d4
		cmp.l   d1,d3
		blt.b   .cont
		sub.l   d2,d4
		fmove.l d4,fp0
		fmove   fp0,fp3
		fadd    fp2,fp3
		move.l  d0,d4
		sub.l   d2,d4
		fmove.l d4,fp1
		fdiv    fp1,fp0
		fmove.l fp3,d0
		add.l   d2,d0
		move.l  d0,(a2)+
		moveq   #5-1,d2
		bra.b   .entry
.loop
		move.l  (a0)+,d3
		move.l  (a1)+,d1
.entry
		sub.l   d3,d1
		fmove.l d1,fp1
		fmul    fp0,fp1
		fadd    fp2,fp1
		fmove.l fp1,d1
		add.l   d3,d1
		move.l  d1,(a2)+
		dbra    d2,.loop
		bra.b   .exit
.cont
		sub.l   d0,d4
		fmove.l d4,fp0
		fmove   fp0,fp3
		fadd    fp2,fp3
		move.l  d2,d4
		sub.l   d0,d4
		fmove.l d4,fp1
		fdiv    fp1,fp0
		fmove.l fp3,d2
		add.l   d0,d2
		move.l  d2,(a2)+
		moveq   #5-1,d2
		bra.b   .entry2
.loop2
		move.l  (a0)+,d3
		move.l  (a1)+,d1
.entry2
		sub.l   d1,d3
		fmove.l d3,fp1
		fmul    fp0,fp1
		fadd    fp2,fp1
		fmove.l fp1,d3
		add.l   d1,d3
		move.l  d3,(a2)+
		dbra    d2,.loop2
.exit
		fmovem.x        (sp)+,fp2/fp3
		movem.l (sp)+,d2-d4/a2/a3
		rts







******************************************************************************
*
*       void _R_Alias_clip_top (finalvert_t *pfv0, finalvert_t *pfv1,
*                               finalvert_t *out)
*
******************************************************************************

		cnop    0,4
_R_Alias_clip_top

*****   stackframe

		rsreset
.fpuregs        rs.x    2
.intregs        rs.l    5
		rs.l    1
.pfv0           rs.l    1
.pfv1           rs.l    1
.out            rs.l    1


		movem.l d2-d4/a2/a3,-(sp)
		fmovem.x        fp2/fp3,-(sp)
		move.l  .pfv0(sp),a0
		move.l  .pfv1(sp),a1
		move.l  .out(sp),a2
		lea     _r_refdef,a3
		fmove.s #0.5,fp2
		move.l  (a0)+,d2
		move.l  (a0)+,d3
		move.l  (a1)+,d0
		move.l  (a1)+,d1
		move.l  REFDEF_ALIASVRECT+VRECT_Y(a3),d4
		cmp.l   d1,d3
		blt.b   .cont
		sub.l   d3,d4
		fmove.l d4,fp0
		fmove   fp0,fp3
		fadd    fp2,fp3
		move.l  d1,d4
		sub.l   d3,d4
		fmove.l d4,fp1
		fdiv    fp1,fp0
		fmove.l fp3,d1
		add.l   d3,d1
		move.l  d1,(a2)+
		moveq   #5-1,d3
		bra.b   .entry
.loop
		move.l  (a0)+,d2
		move.l  (a1)+,d0
.entry
		sub.l   d2,d0
		fmove.l d0,fp1
		fmul    fp0,fp1
		fadd    fp2,fp1
		fmove.l fp1,d0
		add.l   d2,d0
		move.l  d0,(a2)+
		dbra    d3,.loop
		bra.b   .exit
.cont
		sub.l   d1,d4
		fmove.l d4,fp0
		fmove   fp0,fp3
		fadd    fp2,fp3
		move.l  d3,d4
		sub.l   d1,d4
		fmove.l d4,fp1
		fdiv    fp1,fp0
		fmove.l fp3,d3
		add.l   d1,d3
		move.l  d3,(a2)+
		moveq   #5-1,d3
		bra.b   .entry2
.loop2
		move.l  (a0)+,d2
		move.l  (a1)+,d0
.entry2
		sub.l   d0,d2
		fmove.l d2,fp1
		fmul    fp0,fp1
		fadd    fp2,fp1
		fmove.l fp1,d2
		add.l   d0,d2
		move.l  d2,(a2)+
		dbra    d3,.loop2
.exit
		move.l  -24(a2),d0
		move.l  -20(a2),-24(a2)
		move.l  d0,-20(a2)
		fmovem.x        (sp)+,fp2/fp3
		movem.l (sp)+,d2-d4/a2/a3
		rts






******************************************************************************
*
*       void _R_Alias_clip_bottom (finalvert_t *pfv0, finalvert_t *pfv1,
*                                  finalvert_t *out)
*
******************************************************************************

		cnop    0,4
_R_Alias_clip_bottom

*****   stackframe

		rsreset
.fpuregs        rs.x    2
.intregs        rs.l    5
		rs.l    1
.pfv0           rs.l    1
.pfv1           rs.l    1
.out            rs.l    1


		movem.l d2-d4/a2/a3,-(sp)
		fmovem.x        fp2/fp3,-(sp)
		move.l  .pfv0(sp),a0
		move.l  .pfv1(sp),a1
		move.l  .out(sp),a2
		lea     _r_refdef,a3
		fmove.s #0.5,fp2
		move.l  (a0)+,d2
		move.l  (a0)+,d3
		move.l  (a1)+,d0
		move.l  (a1)+,d1
		move.l  REFDEF_ALIASVRECTBOTTOM(a3),d4
		cmp.l   d1,d3
		blt.b   .cont
		sub.l   d3,d4
		fmove.l d4,fp0
		fmove   fp0,fp3
		fadd    fp2,fp3
		move.l  d1,d4
		sub.l   d3,d4
		fmove.l d4,fp1
		fdiv    fp1,fp0
		fmove.l fp3,d1
		add.l   d3,d1
		move.l  d1,(a2)+
		moveq   #5-1,d3
		bra.b   .entry
.loop
		move.l  (a0)+,d2
		move.l  (a1)+,d0
.entry
		sub.l   d2,d0
		fmove.l d0,fp1
		fmul    fp0,fp1
		fadd    fp2,fp1
		fmove.l fp1,d0
		add.l   d2,d0
		move.l  d0,(a2)+
		dbra    d3,.loop
		bra.b   .exit
.cont
		sub.l   d1,d4
		fmove.l d4,fp0
		fmove   fp0,fp3
		fadd    fp2,fp3
		move.l  d3,d4
		sub.l   d1,d4
		fmove.l d4,fp1
		fdiv    fp1,fp0
		fmove.l fp3,d3
		add.l   d1,d3
		move.l  d3,(a2)+
		moveq   #5-1,d3
		bra.b   .entry2
.loop2
		move.l  (a0)+,d2
		move.l  (a1)+,d0
.entry2
		sub.l   d0,d2
		fmove.l d2,fp1
		fmul    fp0,fp1
		fadd    fp2,fp1
		fmove.l fp1,d2
		add.l   d0,d2
		move.l  d2,(a2)+
		dbra    d3,.loop2
.exit
		move.l  -24(a2),d0
		move.l  -20(a2),-24(a2)
		move.l  d0,-20(a2)
		fmovem.x        (sp)+,fp2/fp3
		movem.l (sp)+,d2-d4/a2/a3
		rts
