**
** Quake for AMIGA
** r_bsp.c assembler implementations by Frank Wille <frank@phoenix.owl.de>
** Adapted for Hexen II by Szilard Biro
**

		;INCLUDE	"quakedef68k.i"
CLIP_SIZEOF	equ	24
MPLANE_SIZEOF	equ	20

		XREF    _modelorg
		XREF    _vpn
		XREF    _vright
		XREF    _vup
		XREF    _view_clipplanes
		XREF    _screenedge

		XDEF    _R_TransformFrustum



******************************************************************************
*
*       void R_TransformFrustum (void)
*
******************************************************************************

		cnop    0,4
_R_TransformFrustum
		movem.l a2-a5,-(sp)
		fmovem.x        fp2-fp7,-(sp)

		moveq   #4-1,d0
		;lea     _screenedge,a4
		;lea     _view_clipplanes,a5
		lea     _screenedge,a0
		lea     _view_clipplanes,a1
		lea     _modelorg,a3
		fmove.s (a3)+,fp7
.loop
;		move.l  a4,a0
;		move.l  a5,a1
		fmove.s (a0)+,fp1
		fneg    fp1
		fmove.s (a0)+,fp2
		fmove.s (a0)+,fp0
		lea     _vright,a2
		fmove.s (a2)+,fp3
		fmove.s (a2)+,fp4
		fmove.s (a2)+,fp5
		fmul    fp1,fp3
		fmul    fp1,fp4
		fmul    fp1,fp5
		lea     _vup,a2
		fmove.s (a2)+,fp6
		fmul    fp2,fp6
		fadd    fp6,fp3
		fmove.s (a2)+,fp6
		fmul    fp2,fp6
		fadd    fp6,fp4
		fmove.s (a2)+,fp6
		fmul    fp2,fp6
		fadd    fp6,fp5
		lea     _vpn,a2
		fmove.s (a2)+,fp6
		fmul    fp0,fp6
		fadd    fp6,fp3
		fmove.s fp3,(a1)+
		fmove.s (a2)+,fp6
		fmul    fp0,fp6
		fadd    fp6,fp4
		fmove.s fp4,(a1)+
		fmove.s (a2)+,fp6
		fmul    fp0,fp6
		fadd    fp6,fp5
		fmove.s fp5,(a1)+

		fmul    fp7,fp3
		fmul.s  (a3),fp4
		fadd    fp4,fp3
		fmul.s  4(a3),fp5
		fadd    fp5,fp3
		fmove.s fp3,(a1)+
		lea     CLIP_SIZEOF-16(a1),a1
		lea     MPLANE_SIZEOF-12(a0),a0
		dbra    d0,.loop

		fmovem.x        (sp)+,fp2-fp7
		movem.l (sp)+,a2-a5
		rts
