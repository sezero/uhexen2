; 680x0 optimised Quake render routines by John Selck.
; Adapted for Hexen II by Szilard Biro

	; d_sprite.c:

		XDEF	_D_SpriteDrawSpans
		XDEF	_D_SpriteDrawSpansT
		XDEF	_D_SpriteDrawSpansT2

	; external defs:

		XREF	_cacheblock
		XREF	_d_sdivzorigin
		XREF	_d_sdivzstepu
		XREF	_d_sdivzstepv
		XREF	_d_tdivzorigin
		XREF	_d_tdivzstepu
		XREF	_d_tdivzstepv
		XREF	_d_ziorigin
		XREF	_sadjust
		XREF	_tadjust
		XREF	_bbextents
		XREF	_bbextentt
		XREF	_screenwidth
		XREF	_mainTransTable

SSPAN_U	equ	0
SSPAN_V	equ	4
SSPAN_COUNT	equ	8
DS_SPAN_LIST_END	equ	-128

		SECTION	CODE,CODE


*****************************************************************************
*
*       void D_SpriteDrawSpans (sspan_t *pspan)
*
******************************************************************************

		cnop    0,4
_D_SpriteDrawSpans


*****   stackframe
		rsreset
.fpuregs        rs.x    4
.intregs        rs.l    11
		rs.l    1
.pspan          rs.l    1

***** prologue


		movem.l	d2-d7/a2-a6,-(sp)
		fmovem.x	fp2-fp5,-(sp)

		fmove.s	_d_ziorigin,fp2
		fmove.s	#65536,fp3
		fdiv.x	fp2,fp3				;fp3 = (float)0x10000 / d_ziorigin

		move.l	.pspan(sp),a6
		move.l	_cacheblock,a0		;a0 = pbase

		fmul.s	#32768,fp2			;fp2 = d_ziorigin * (float)0x8000

		move.l	_bbextents,a4
		move.l	_sadjust,a5

		fmove.l	fp2,d6				;d6 = (int)(d_ziorigin * 0x8000);
.ls1
		move.l	SSPAN_COUNT(a6),d7
		ble.w	.skip

		move.l	SSPAN_U(a6),d0		;d0 = pspan->u
		fmove.l	d0,fp4				;fp4 = du = (float)psan->u
		fmove.s	_d_sdivzstepu,fp0
		fmul.x	fp4,fp0				;fp0 = du*d_sdivzstepu

		move.l	SSPAN_V(a6),d1		;d1 = pspan->v
		fmove.l	d1,fp5				;fp5 = dv = (float)pspan->v
		fmove.s	_d_sdivzstepv,fp1
		fmul.x	fp5,fp1				;fp1 = dv*d_sdivzstepv

		move.l	_d_viewbuffer,a1
		move.l	d1,d2				;d2 = pspan->v
		add.l	d0,a1				;a1 = d_viewbuffer + pspan->u
		fadd.x	fp1,fp0				;fp0 = dv*d_sdivzstepv + du*d_sdivzstepu
		fadd.s	_d_sdivzorigin,fp0	;fp0 = sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu

		fmove.s	_d_tdivzstepu,fp1
		fmul.x	fp4,fp1				;fp1 = du*d_tdivzstepu
		mulu.l	_screenwidth,d2		;d2 = screenwidth * pspan->v
		add.l	d2,a1				;a1 = d_viewbuffer + pspan->u + (screenwidth * pspan->v)
		fmove.s	_d_tdivzstepv,fp2
		fmul.x	fp5,fp2				;fp2 = dv*d_tdivzstepv
		move.l	_d_pzbuffer,a2
		move.l	d1,d2				;d2 = pspan->v
		mulu.l	_d_zwidth,d2		;d2 = d_zwidth * pspan->v
		fadd.x	fp2,fp1				;fp1 = dv*d_tdivzstepv + du*d_tdivzstepu
		fadd.s	_d_tdivzorigin,fp1	;fp1 = tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu

		fmove.x	fp0,fp4
		fmul.x	fp3,fp4
		add.l	d0,d2
		moveq	#-1,d1
		add.l	d2,d2
		move.l	d7,d0
		add.l	d2,a2	; pz
		subq.l	#1,d0
		fmove.l	fp4,d2
		fmove.l	d0,fp4		; spancountminus1
		fmul.x	fp3,fp1

		add.l	a5,d2	; s

		cmp.l	a4,d2
		ble.b	.s0
		move.l	a4,d2
		bra.b	.s1
.s0
		tst.l	d2
		bpl.b	.s1
		moveq	#0,d2
.s1
		fmove.l	fp1,d3
		fmove.s	_d_sdivzstepu,fp5
		fmul.x	fp4,fp5
		add.l	_tadjust,d3	; t

		move.l	_bbextentt,d0
		cmp.l	d0,d3
		ble.b	.s2
		move.l	d0,d3
		bra.b	.s3
.s2
		tst.l	d3
		bpl.b	.s3
		moveq	#0,d3
.s3
		fadd.x	fp5,fp0
		fmul.x	fp3,fp0
		swap	d3
		muls.w	_cachewidth+2,d3
		move.l	a0,a3
		move.l	a4,d0
		add.l	d3,a3
		fmove.l	fp0,d4
		add.l	a5,d4

		cmp.l	d0,d4
		bgt.b	.s4
		moveq	#8,d0
		cmp.l	d0,d4
		bge.b	.s5
.s4
		move.l	d0,d4
.s5

		move.l	d7,d0
		subq.l	#1,d0
		ble.b	.sd
		sub.l	d2,d4
		divs.l	d0,d4
.sd
.ls0
		move.l	d2,d0
		swap	d0
		move.b	(a3,d0.w),d3		;btemp = *(a3 + (s >> 16) + (t >> 16) * cachewidth)
		cmp.b	d1,d3				;if (btemp != 255)
		beq.b	.ss
		cmp.w	(a2),d6				;if (*pz <= (izi >> 16))
		ble.b	.ss
		move.w	d6,(a2)				;*pz = izi >> 16
		move.b	d3,(a1)				;*pdest = btemp
.ss
		add.l	d4,d2
		addq.l	#2,a2
		addq.l	#1,a1
		subq.l	#1,d7
		bgt.b	.ls0
.skip
		add.w	#12,a6
		moveq	#DS_SPAN_LIST_END,d0
		cmp.l	8(a6),d0
		bne.w	.ls1

		fmovem.x	(sp)+,fp2-fp5
		movem.l	(sp)+,d2-d7/a2-a6
		rts



*****************************************************************************
*
*       void D_SpriteDrawSpansT (sspan_t *pspan)
*
******************************************************************************

		cnop    0,4
_D_SpriteDrawSpansT


*****   stackframe
		rsreset
.savea6         rs.l    1
.fpuregs        rs.x    4
.intregs        rs.l    11
		rs.l    1
.pspan          rs.l    1

***** prologue


		movem.l	d2-d7/a2-a6,-(sp)
		fmovem.x	fp2-fp5,-(sp)
		sub	#.fpuregs,sp

		fmove.s	_d_ziorigin,fp2
		fmove.s	#65536,fp3
		fdiv.x	fp2,fp3				;fp3 = (float)0x10000 / d_ziorigin

		move.l	.pspan(sp),a6
		move.l	_cacheblock,a0		;a0 = pbase

		fmul.s	#32768,fp2			;fp2 = d_ziorigin * (float)0x8000

		move.l	_bbextents,a4
		move.l	_sadjust,a5

		fmove.l	fp2,d6				;d6 = (int)(d_ziorigin * 0x8000);
.ls1
		move.l	SSPAN_COUNT(a6),d7
		ble.w	.skip

		move.l	SSPAN_U(a6),d0		;d0 = pspan->u
		fmove.l	d0,fp4				;fp4 = du = (float)psan->u
		fmove.s	_d_sdivzstepu,fp0
		fmul.x	fp4,fp0				;fp0 = du*d_sdivzstepu

		move.l	SSPAN_V(a6),d1		;d1 = pspan->v
		fmove.l	d1,fp5				;fp5 = dv = (float)pspan->v
		fmove.s	_d_sdivzstepv,fp1
		fmul.x	fp5,fp1				;fp1 = dv*d_sdivzstepv

		move.l	_d_viewbuffer,a1
		move.l	d1,d2				;d2 = pspan->v
		add.l	d0,a1				;a1 = d_viewbuffer + pspan->u
		fadd.x	fp1,fp0				;fp0 = dv*d_sdivzstepv + du*d_sdivzstepu
		fadd.s	_d_sdivzorigin,fp0	;fp0 = sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu

		fmove.s	_d_tdivzstepu,fp1
		fmul.x	fp4,fp1				;fp1 = du*d_tdivzstepu
		mulu.l	_screenwidth,d2		;d2 = screenwidth * pspan->v
		add.l	d2,a1				;a1 = d_viewbuffer + pspan->u + (screenwidth * pspan->v)
		fmove.s	_d_tdivzstepv,fp2
		fmul.x	fp5,fp2				;fp2 = dv*d_tdivzstepv
		move.l	_d_pzbuffer,a2
		move.l	d1,d2				;d2 = pspan->v
		mulu.l	_d_zwidth,d2		;d2 = d_zwidth * pspan->v
		fadd.x	fp2,fp1				;fp1 = dv*d_tdivzstepv + du*d_tdivzstepu
		fadd.s	_d_tdivzorigin,fp1	;fp1 = tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu

		fmove.x	fp0,fp4
		fmul.x	fp3,fp4
		add.l	d0,d2
		moveq	#-1,d1
		add.l	d2,d2
		move.l	d7,d0
		add.l	d2,a2	; pz
		subq.l	#1,d0
		fmove.l	fp4,d2
		fmove.l	d0,fp4		; spancountminus1
		fmul.x	fp3,fp1

		add.l	a5,d2	; s

		cmp.l	a4,d2
		ble.b	.s0
		move.l	a4,d2
		bra.b	.s1
.s0
		tst.l	d2
		bpl.b	.s1
		moveq	#0,d2
.s1
		fmove.l	fp1,d3
		fmove.s	_d_sdivzstepu,fp5
		fmul.x	fp4,fp5
		add.l	_tadjust,d3	; t

		move.l	_bbextentt,d0
		cmp.l	d0,d3
		ble.b	.s2
		move.l	d0,d3
		bra.b	.s3
.s2
		tst.l	d3
		bpl.b	.s3
		moveq	#0,d3
.s3
		fadd.x	fp5,fp0
		fmul.x	fp3,fp0
		swap	d3
		muls.w	_cachewidth+2,d3
		move.l	a0,a3
		move.l	a4,d0
		add.l	d3,a3
		fmove.l	fp0,d4
		add.l	a5,d4

		cmp.l	d0,d4
		bgt.b	.s4
		moveq	#8,d0
		cmp.l	d0,d4
		bge.b	.s5
.s4
		move.l	d0,d4
.s5

		move.l	d7,d0
		subq.l	#1,d0
		ble.b	.sd
		sub.l	d2,d4
		divs.l	d0,d4
.sd
		move.l	a6,.savea6(sp)		;save actual ptr to pspan
		move.l	_mainTransTable,a6
.ls0
		move.l	d2,d0
		swap	d0
		move.b	(a3,d0.w),d3		;btemp = *(a3 + (s >> 16) + (t >> 16) * cachewidth)
		cmp.b	d1,d3				;if (btemp != 255)
		beq.b	.ss
		cmp.w	(a2),d6				;if (*pz <= (izi >> 16))
		ble.b	.ss
		lsl.w	#8,d3
		move.b	(a1),d3				;d3 = (btemp<<8) + (*pdest)
		;move.w	d6,(a2)				;*pz = izi >> 16
		;move.b	d3,(a1)				;*pdest = btemp
		move.b	(a6,d3.l),(a1)		;*pdest = mainTransTable[(btemp<<8) + (*pdest)]
.ss
		add.l	d4,d2
		addq.l	#2,a2
		addq.l	#1,a1
		subq.l	#1,d7
		bgt.b	.ls0
		move.l	.savea6(sp),a6		;restore pspan
.skip
		add.w	#12,a6
		moveq	#DS_SPAN_LIST_END,d0
		cmp.l	8(a6),d0
		bne.w	.ls1

		add.l	#.fpuregs,sp
		fmovem.x	(sp)+,fp2-fp5
		movem.l	(sp)+,d2-d7/a2-a6
		rts




*****************************************************************************
*
*       void D_SpriteDrawSpansT2 (sspan_t *pspan)
*
******************************************************************************

		cnop    0,4
_D_SpriteDrawSpansT2


*****   stackframe
		rsreset
.savea6         rs.l    1
.fpuregs        rs.x    4
.intregs        rs.l    11
		rs.l    1
.pspan          rs.l    1

***** prologue


		movem.l	d2-d7/a2-a6,-(sp)
		fmovem.x	fp2-fp5,-(sp)
		sub	#.fpuregs,sp

		fmove.s	_d_ziorigin,fp2
		fmove.s	#65536,fp3
		fdiv.x	fp2,fp3				;fp3 = (float)0x10000 / d_ziorigin

		move.l	.pspan(sp),a6
		move.l	_cacheblock,a0		;a0 = pbase

		fmul.s	#32768,fp2			;fp2 = d_ziorigin * (float)0x8000

		move.l	_bbextents,a4
		move.l	_sadjust,a5

		fmove.l	fp2,d6				;d6 = (int)(d_ziorigin * 0x8000);
.ls1
		move.l	SSPAN_COUNT(a6),d7
		ble.w	.skip

		move.l	SSPAN_U(a6),d0		;d0 = pspan->u
		fmove.l	d0,fp4				;fp4 = du = (float)psan->u
		fmove.s	_d_sdivzstepu,fp0
		fmul.x	fp4,fp0				;fp0 = du*d_sdivzstepu

		move.l	SSPAN_V(a6),d1		;d1 = pspan->v
		fmove.l	d1,fp5				;fp5 = dv = (float)pspan->v
		fmove.s	_d_sdivzstepv,fp1
		fmul.x	fp5,fp1				;fp1 = dv*d_sdivzstepv

		move.l	_d_viewbuffer,a1
		move.l	d1,d2				;d2 = pspan->v
		add.l	d0,a1				;a1 = d_viewbuffer + pspan->u
		fadd.x	fp1,fp0				;fp0 = dv*d_sdivzstepv + du*d_sdivzstepu
		fadd.s	_d_sdivzorigin,fp0	;fp0 = sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu

		fmove.s	_d_tdivzstepu,fp1
		fmul.x	fp4,fp1				;fp1 = du*d_tdivzstepu
		mulu.l	_screenwidth,d2		;d2 = screenwidth * pspan->v
		add.l	d2,a1				;a1 = d_viewbuffer + pspan->u + (screenwidth * pspan->v)
		fmove.s	_d_tdivzstepv,fp2
		fmul.x	fp5,fp2				;fp2 = dv*d_tdivzstepv
		move.l	_d_pzbuffer,a2
		move.l	d1,d2				;d2 = pspan->v
		mulu.l	_d_zwidth,d2		;d2 = d_zwidth * pspan->v
		fadd.x	fp2,fp1				;fp1 = dv*d_tdivzstepv + du*d_tdivzstepu
		fadd.s	_d_tdivzorigin,fp1	;fp1 = tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu

		fmove.x	fp0,fp4
		fmul.x	fp3,fp4
		add.l	d0,d2
		moveq	#-1,d1
		add.l	d2,d2
		move.l	d7,d0
		add.l	d2,a2	; pz
		subq.l	#1,d0
		fmove.l	fp4,d2
		fmove.l	d0,fp4		; spancountminus1
		fmul.x	fp3,fp1

		add.l	a5,d2	; s

		cmp.l	a4,d2
		ble.b	.s0
		move.l	a4,d2
		bra.b	.s1
.s0
		tst.l	d2
		bpl.b	.s1
		moveq	#0,d2
.s1
		fmove.l	fp1,d3
		fmove.s	_d_sdivzstepu,fp5
		fmul.x	fp4,fp5
		add.l	_tadjust,d3	; t

		move.l	_bbextentt,d0
		cmp.l	d0,d3
		ble.b	.s2
		move.l	d0,d3
		bra.b	.s3
.s2
		tst.l	d3
		bpl.b	.s3
		moveq	#0,d3
.s3
		fadd.x	fp5,fp0
		fmul.x	fp3,fp0
		swap	d3
		muls.w	_cachewidth+2,d3
		move.l	a0,a3
		move.l	a4,d0
		add.l	d3,a3
		fmove.l	fp0,d4
		add.l	a5,d4

		cmp.l	d0,d4
		bgt.b	.s4
		moveq	#8,d0
		cmp.l	d0,d4
		bge.b	.s5
.s4
		move.l	d0,d4
.s5

		move.l	d7,d0
		subq.l	#1,d0
		ble.b	.sd
		sub.l	d2,d4
		divs.l	d0,d4
.sd
		move.l	a6,.savea6(sp)		;save actual ptr to pspan
		move.l	_mainTransTable,a6
.ls0
		move.l	d2,d0
		swap	d0
		move.b	(a3,d0.w),d3		;btemp = *(a3 + (s >> 16) + (t >> 16) * cachewidth)
		cmp.b	d1,d3				;if (btemp != 255)
		beq.b	.ss
		cmp.w	(a2),d6				;if (*pz <= (izi >> 16))
		ble.b	.ss
		btst	#0,d3				;if (btemp % 2 == 0)
		bne.b	.transluc
		move.w	d6,(a2)				;*pz = izi >> 16
		move.b	d3,(a1)				;*pdest = btemp
		bra.b	.ss
.transluc
		lsl.w	#8,d3
		move.b	(a1),d3				;d3 = (btemp<<8) + (*pdest)
		move.b	(a6,d3.l),(a1)		;*pdest = mainTransTable[(btemp<<8) + (*pdest)]
.ss
		add.l	d4,d2
		addq.l	#2,a2
		addq.l	#1,a1
		subq.l	#1,d7
		bgt.b	.ls0
		move.l	.savea6(sp),a6		;restore pspan
.skip
		add.w	#12,a6
		moveq	#DS_SPAN_LIST_END,d0
		cmp.l	8(a6),d0
		bne.w	.ls1

		add.l	#.fpuregs,sp
		fmovem.x	(sp)+,fp2-fp5
		movem.l	(sp)+,d2-d7/a2-a6
		rts

		END
