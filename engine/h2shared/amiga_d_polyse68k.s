; 680x0 optimised Quake render routines by John Selck.

	; d_polyse.c:

		XDEF	_D_PolysetDrawFinalVerts
		XDEF	_D_PolysetRecursiveTriangle
		XDEF	_D_PolysetDrawSpans8
		XDEF	_D_PolysetCalcGradients
		XDEF	_D_PolysetCalcGradientsT
		XDEF	_D_PolysetCalcGradientsT2
		XDEF	_D_PolysetCalcGradientsT3
		XDEF	_D_PolysetCalcGradientsT5

	; external defs:

		XREF	_erroradjustup
		XREF	_erroradjustdown
		XREF	_errorterm
		XREF	_ubasestep
		XREF	_acolormap
		XREF	_r_affinetridesc

		XREF	_d_pcolormap
		XREF	_a_sstepxfrac
		XREF	_a_tstepxfrac
		XREF	_r_lstepx
		XREF	_a_ststepxwhole
		XREF	_r_zistepx
		XREF	_d_aspancount
		XREF	_d_countextrastep
		XREF	_skintable

		XREF	_zspantable
		XREF	_d_scantable
		XREF	_d_viewbuffer

		XREF	_r_refdef

		XREF	_r_p0
		XREF	_r_p1
		XREF	_r_p2
		XREF	_d_xdenom
		XREF	_r_sstepx
		XREF	_r_tstepx
		XREF	_r_sstepy
		XREF	_r_tstepy
		XREF	_r_zistepy
		XREF	_r_lstepy

		SECTION	"Poly",CODE

; void D_PolysetDrawFinalVerts (finalvert_t *fv, int numverts);

;	entity polygon renderer for very small polygons (1 pixel)

		cnop	0,4
_D_PolysetDrawFinalVerts:
		movem.l	d2-d5/a2-a6,-(sp)

		move.l	40(sp),a6
		bsr.b	_do_PolysetDrawFinalVerts
		move.l	44(sp),a6
		bsr.b	_do_PolysetDrawFinalVerts
		move.l	48(sp),a6
		bsr.b	_do_PolysetDrawFinalVerts

		movem.l	(sp)+,d2-d5/a2-a6
		rts

		cnop	0,4
_do_PolysetDrawFinalVerts:
		;movem.l	d2-d5/a2-a6,-(sp)

		lea	_zspantable,a1
		lea	_skintable,a2
		lea	_d_scantable,a3
		move.l	_acolormap,a4
		move.l	_d_viewbuffer,a5

		move.l	_r_refdef+$28,d4
		move.l	_r_refdef+$2c,d5

		moveq	#0,d2
		;move.l	d0,d3
		;move.l	a0,a6
		;bra.b	.fvin
.fvl0
		move.l	(a6),d0
		cmp.l	d4,d0
		bge.b	.fvskip
		move.l	4(a6),d1
		cmp.l	d5,d1
		bge.b	.fvskip

		move.l	(a1,d1.w*4),a0
		move.w	20(a6),d2
		cmp.w	(a0,d0.w*2),d2
		blt.b	.fvskip
		move.w	d2,(a0,d0.w*2)

		move.w	12(a6),d0
		move.l	(a2,d0.w*4),a0
		add.w	8(a6),a0

		move.w	18(a6),d2
		move.l	(a3,d1.w*4),d0
		move.b	(a0),d2
		add.l	(a6),d0
		move.b	0(a4,d2.l),0(a5,d0.l)
.fvskip
		add.w	#32,a6
.fvin
		;subq.l	#1,d3
		;bpl.b	.fvl0

		;movem.l	(sp)+,d2-d5/a2-a6
		rts

; void D_PolysetRecursiveTriangle (int *lp1, int *lp2, int *lp3);

;	entity polygon renderer for small polys (few pixels)

		cnop	0,4
_D_PolysetRecursiveTriangle:
		movem.l	d2-d7/a2-a6,-(sp)

		;move.l	a2,a3
		;move.l	a1,a2
		;move.l	a0,a1
		move.l	56(sp),a3
		move.l	52(sp),a2
		move.l	48(sp),a1
		bsr.b	D_PolysetRecursiveTriangle

		movem.l	(sp)+,d2-d7/a2-a6
		rts

D_PolysetRecursiveTriangle:
		sub.w	#24,sp

		moveq	#-1,d0
		moveq	#1,d1

		move.l	(a2),d2
		sub.l	(a1),d2
		cmp.l	d0,d2
		blt.b	.split
		cmp.l	d1,d2
		bgt.b	.split

		move.l	4(a2),d2
		sub.l	4(a1),d2
		cmp.l	d0,d2
		blt.b	.split
		cmp.l	d1,d2
		bgt.b	.split

		move.l	(a3),d2
		sub.l	(a2),d2
		cmp.l	d0,d2
		blt.b	.split2
		cmp.l	d1,d2
		bgt.b	.split2

		move.l	4(a3),d2
		sub.l	4(a2),d2
		cmp.l	d0,d2
		blt.b	.split2
		cmp.l	d1,d2
		bgt.b	.split2

		move.l	(a1),d2
		sub.l	(a3),d2
		cmp.l	d0,d2
		blt.b	.split3
		cmp.l	d1,d2
		bgt.b	.split3

		move.l	4(a1),d2
		sub.l	4(a3),d2
		cmp.l	d0,d2
		blt.b	.split3
		cmp.l	d1,d2
		ble.w	.endpr
.split3
		move.l	a1,d2
		move.l	a3,a1
		move.l	a2,a3
		move.l	d2,a2
		bra.b	.split
.split2
		move.l	a1,d2
		move.l	a2,a1
		move.l	a3,a2
		move.l	d2,a3
.split
		move.l	(a1),d0
		move.l	4(a1),d1
		add.l	(a2),d0
		add.l	4(a2),d1
		asr.l	#1,d0
		asr.l	#1,d1
		move.l	d0,(sp)
		move.l	d1,4(sp)

		move.l	8(a1),d0
		move.l	12(a1),d1
		add.l	8(a2),d0
		add.l	12(a2),d1
		asr.l	#1,d0
		asr.l	#1,d1
		move.l	d0,8(sp)
		move.l	d1,12(sp)

		move.l	20(a1),d0
		add.l	20(a2),d0
		asr.l	#1,d0
		move.l	d0,20(sp)

		move.l	4(a2),d2
		cmp.l	4(a1),d2
		bgt.b	.nodraw
		bne.b	.skp
		move.l	(a2),d2
		cmp.l	(a1),d2
		blt.b	.nodraw
.skp
		lea	_zspantable,a0
		move.l	4(sp),d0
		move.l	(a0,d0.w*4),a0
		move.l	(sp),d0
		add.l	d0,a0
		add.l	d0,a0

		move.w	20(sp),d2
		cmp.w	(a0),d2
		blt.b	.nodraw
		move.w	d2,(a0)

		lea	_skintable,a4
		move.w	12(sp),d0
		move.l	(a4,d0.w*4),a4
		move.w	8(sp),d0
		add.w	d0,a4

		moveq	#0,d0
		move.b	(a4),d0
		move.l	_d_pcolormap,a4

		lea	_d_scantable,a5
		move.l	4(sp),d1
		move.l	(a5,d1.w*4),d1
		add.l	(sp),d1
		move.l	_d_viewbuffer,a5
		move.b	(a4,d0.w),0(a5,d1.l)
.nodraw
		move.l	a2,-(sp)
		move.l	a3,-(sp)
		move.l	a1,a2
		move.l	a3,a1
		move.l	sp,a3
		addq.l	#8,a3
		bsr.w	D_PolysetRecursiveTriangle
		move.l	(sp)+,a1
		move.l	(sp)+,a3
		move.l	sp,a2
		bsr.w	D_PolysetRecursiveTriangle
.endpr
		add.w	#24,sp
		rts

; void D_PolysetDrawSpans8 (spanpackage_t *pspanpackage);

;	entity polygon renderer for big polys

		cnop	0,4
_D_PolysetDrawSpans8:
		move.l	4(sp),a0
		movem.l	d2-d7/a2-a6,-(sp)

		move.l	a0,-(sp)
		move.l	a0,a5
.l1
		move.l	_d_aspancount,d7
		sub.l	8(a5),d7
		move.l	_erroradjustup,d0
		add.l	d0,_errorterm
		blt.b	.s0

		move.l	_erroradjustdown,d0
		sub.l	d0,_errorterm
		move.l	_d_countextrastep,d0
		bra.b	.s1
.s0
		move.l	_ubasestep,d0
.s1
		add.l	d0,_d_aspancount

		move.l	d7,d0
		beq.b	.nospan

		lea	$0c(a5),a1
		move.l	(a5)+,a3	; $00
		move.l	(a5),a6		; $04 ...
		move.l	(a1)+,a2	; $0c
		move.l	(a1)+,d6	; $10
		move.l	(a1)+,d7	; $14
		move.l	(a1)+,d5	; $18
		move.l	(a1)+,d4	; $1c

		move.l	_acolormap,a0
		move.l	_a_ststepxwhole,a1
		move.l	_r_zistepx,a4
		move.l	_r_lstepx,a5

		move.l	_a_sstepxfrac,d2
		move.l	_a_tstepxfrac,d3
.l0
		move.l	d4,d1
		swap	d1
		cmp.w	(a6),d1
		blt.b	.no
		move.w	d1,(a6)

		moveq	#0,d1
		move.w	d5,d1
		move.b	(a2),d1
		move.b	0(a0,d1.l),(a3)
.no
		addq.l	#1,a3
		addq.l	#2,a6

		add.l	a1,a2

		add.l	d2,d6
		;add.l	d3,d7

		add.l	a4,d4
		add.l	a5,d5

		move.l	d6,d1
		and.l	#$0000ffff,d6
		swap	d1
		add.w	d1,a2

		add.w	d3,d7
		bcc.b	.np
		add.l	_r_affinetridesc+$08,a2
.np
		subq.l	#1,d0
		bne.b	.l0
.nospan
		moveq	#$20,d0
		add.l	d0,(sp)
		move.l	(sp),a5
		cmp.l	#-999999,8(a5)
		bne.w	.l1

		addq.l	#4,sp
		movem.l	(sp)+,d2-d7/a2-a6
		rts

; void D_PolysetCalcGradients (int skinwidth);

;	entity polygon gradient calculation

		cnop	0,4
_D_PolysetCalcGradientsT:
_D_PolysetCalcGradientsT2:
_D_PolysetCalcGradientsT3:
_D_PolysetCalcGradientsT5:
_D_PolysetCalcGradients:
		move.l	4(sp),d0
		fmovem.x	fp2-fp7,-(sp)

		fmove.s	#-1.0,fp0
		fdiv.l	_d_xdenom,fp0

		movem.l	d2-d4/d7,-(sp)
		move.l	d0,d7

		move.l	_r_p0,d0
		sub.l	_r_p2,d0
		move.l	_r_p0+4,d1
		sub.l	_r_p2+4,d1
		move.l	_r_p1,d2
		sub.l	_r_p2,d2
		move.l	_r_p1+4,d3
		sub.l	_r_p2+4,d3
		fmove.l	d0,fp4		;p00_minus_p20
		fmove.l	d1,fp5		;p01_minus_p21
		fmove.l	d2,fp6		;p10_minus_p20
		fmove.l	d3,fp7		;p11_minus_p21

		move.l	_r_p1+16,d0
		sub.l	_r_p2+16,d0
		fmove.l	d0,fp1

		fmove.x	fp1,fp2
		fmul.x	fp5,fp2
	move.l	_r_p0+16,d0
	sub.l	_r_p2+16,d0
	fmove.l	d0,fp3
	fmove.s	fp3,t0
		fmul.x	fp7,fp3
		fsub.x	fp3,fp2
		fmul.x	fp0,fp2
		fmove.l	fp2,d0

		fmove.x	fp1,fp2
		fmove.s	t0,fp3
		fmul.x	fp4,fp2
	neg.l	d0
	addq.l	#1,d0
		fmul.x	fp6,fp3
	move.l	d0,_r_lstepx
		fsub.x	fp3,fp2
		fmul.x	fp0,fp2
	move.l	_r_p1+8,d0
	sub.l	_r_p2+8,d0
	fmove.l	d0,fp1
		fmove.l	fp2,d2


		fmove.x	fp1,fp2
		fmul.x	fp5,fp2
	move.l	_r_p0+8,d0
	sub.l	_r_p2+8,d0
	fmove.l	d0,fp3
	fmove.s	fp3,t0
		fmul.x	fp7,fp3
	addq.l	#1,d2
	move.l	d2,_r_lstepy
		fsub.x	fp3,fp2
		fmul.x	fp0,fp2
		fmove.l	fp2,d0

		fmove.x	fp1,fp2
		fmove.s	t0,fp3
		fmul.x	fp4,fp2
	neg.l	d0
	move.l	d0,_r_sstepx
		fmul.x	fp6,fp3
	and.l	#$ffff,d0
	move.l	d0,_a_sstepxfrac
		fsub.x	fp3,fp2
		fmul.x	fp0,fp2
	move.l	_r_p1+12,d0
	sub.l	_r_p2+12,d0
	fmove.l	d0,fp1
		fmove.l	fp2,_r_sstepy

		fmove.x	fp1,fp2
		fmul.x	fp5,fp2
	move.l	_r_p0+12,d0
	sub.l	_r_p2+12,d0
	fmove.l	d0,fp3
	fmove.s	fp3,t0
		fmul.x	fp7,fp3
		fsub.x	fp3,fp2
		fmul.x	fp0,fp2
		fmove.l	fp2,d0

		fmove.x	fp1,fp2
		fmove.s	t0,fp3
		fmul.x	fp4,fp2
	neg.l	d0
	move.l	d0,_r_tstepx
		fmul.x	fp6,fp3
	and.l	#$ffff,d0
	move.l	d0,_a_tstepxfrac
		fsub.x	fp3,fp2
		fmul.x	fp0,fp2
	move.l	_r_p1+20,d0
	sub.l	_r_p2+20,d0
	fmove.l	d0,fp1
		fmove.l	fp2,_r_tstepy

		fmove.x	fp1,fp2
		fmul.x	fp5,fp2
	move.l	_r_p0+20,d0
	sub.l	_r_p2+20,d0
	fmove.l	d0,fp3
	fmove.s	fp3,t0
		fmul.x	fp7,fp3
	move.w	_r_sstepx,d2
	move.w	_r_tstepx,d3
		fsub.x	fp3,fp2
		fmul.x	fp0,fp2
	ext.l	d2
	ext.l	d3
		fmove.l	fp2,d0

		fmove.x	fp1,fp2
		fmove.s	t0,fp3
		fmul.x	fp4,fp2
	neg.l	d0
	move.l	d0,_r_zistepx
		fmul.x	fp6,fp3
	muls.l	d7,d3
	add.l	d3,d2
		fsub.x	fp3,fp2
		fmul.x	fp0,fp2
	move.l	d2,_a_ststepxwhole
	movem.l	(sp)+,d2-d4/d7
		fmove.l	fp2,_r_zistepy

		fmovem.x	(sp)+,fp2-fp7
		rts

		cnop	0,8
t0:		dc.l	0


		END
