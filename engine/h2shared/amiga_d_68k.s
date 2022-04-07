; 680x0 optimised Quake render routines by John Selck.

	; d_sprite.c:

		XDEF	_D_SpriteDrawSpans

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

		SECTION	CODE,CODE

; void D_SpriteDrawSpans (sspan_t *pspan);

		cnop	0,4
_D_SpriteDrawSpans:
		move.l	4(sp),a0
		fmovem.x	fp2-fp5,-(sp)

		fmove.s	_d_ziorigin,fp2
		fmove.s	#65536,fp3
		fdiv.x	fp2,fp3		; z

		movem.l	d2-d7/a2-a6,-(sp)
		move.l	a0,a6
		move.l	_cacheblock,a0	; pbase

		fmul.s	#32768,fp2

		move.l	_bbextents,a4
		move.l	_sadjust,a5

		fmove.l	fp2,d6		; izi
.ls1
		move.l	8(a6),d7	; count
		ble.w	.skip

		move.l	(a6),d0
		fmove.l	d0,fp4	; du

		fmove.s	_d_sdivzstepu,fp0
		fmul.x	fp4,fp0
	move.l	4(a6),d1
	fmove.l	d1,fp5	; dv
		fmove.s	_d_sdivzstepv,fp1
		fmul.x	fp5,fp1
	move.l	_d_viewbuffer,a1
	move.l	d1,d2
	add.l	d0,a1
		fadd.x	fp1,fp0
		fadd.s	_d_sdivzorigin,fp0	; sdivz

		fmove.s	_d_tdivzstepu,fp1
		fmul.x	fp4,fp1
	mulu.l	_screenwidth,d2
	add.l	d2,a1	; pdest
		fmove.s	_d_tdivzstepv,fp2
		fmul.x	fp5,fp2
	move.l	_d_pzbuffer,a2
	move.l	d1,d2
	mulu.l	_d_zwidth,d2
		fadd.x	fp2,fp1
		fadd.s	_d_tdivzorigin,fp1	; tdivz

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
		move.b	(a3,d0.w),d3
		cmp.b	d1,d3
		beq.b	.ss
		cmp.w	(a2),d6
		ble.b	.ss
		move.w	d6,(a2)
		move.b	d3,(a1)
.ss
		add.l	d4,d2
		addq.l	#2,a2
		addq.l	#1,a1
		subq.l	#1,d7
		bgt.b	.ls0
.skip
		add.w	#12,a6
		moveq	#-128,d0
		cmp.l	8(a6),d0
		bne.w	.ls1

		movem.l	(sp)+,d2-d7/a2-a6
		fmovem.x	(sp)+,fp2-fp5
		rts

		END
