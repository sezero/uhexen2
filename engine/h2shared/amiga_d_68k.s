; 680x0 optimised Quake render routines by John Selck.

	; d_scan.c:

		XDEF	_D_DrawTurbulent8Span
		XDEF	_D_DrawSpans8
		XDEF	_D_DrawSpans16
		XDEF	_D_DrawZSpans

	; d_sprite.c:

		XDEF	_D_SpriteDrawSpans

	; d_part.c:

		XDEF	_D_DrawParticle_fixme

	; external defs:

		XREF	_cacheblock
		XREF	_d_sdivzorigin
		XREF	_d_sdivzstepu
		XREF	_d_sdivzstepv
		XREF	_d_tdivzorigin
		XREF	_d_tdivzstepu
		XREF	_d_tdivzstepv
		XREF	_d_ziorigin
		XREF	_d_zistepu
		XREF	_d_zistepv
		XREF	_sadjust
		XREF	_tadjust
		XREF	_bbextents
		XREF	_bbextentt
		XREF	_screenwidth
		XREF	_r_turb_s
		XREF	_r_turb_t
		XREF	_r_turb_sstep
		XREF	_r_turb_tstep
		XREF	_r_turb_spancount
		XREF	_r_turb_pdest
		XREF	_r_turb_turb

		SECTION	"Span",CODE

; void D_DrawTurbulent8Span (void);

;	turbulent polygon span renderer

		cnop	0,4
_D_DrawTurbulent8Span:
		movem.l	d2-d7/a2,-(sp)
		moveq	#$7f,d7
		move.l	_r_turb_s,d2
		move.l	_r_turb_t,d3
		move.l	_r_turb_sstep,d4
		move.l	_r_turb_tstep,d5
		move.l	_r_turb_spancount,d6
		move.l	_cacheblock,a0
		move.l	_r_turb_pdest,a1
		move.l	_r_turb_turb,a2

.ti
		move.l	d2,d0
		move.l	d3,d1
		swap	d0
		swap	d1
		and.w	d7,d0
		and.w	d7,d1
		move.l	(a2,d0.w*4),d0
		move.l	(a2,d1.w*4),d1

		add.l	d3,d0
		add.l	d2,d1
		swap	d0
		swap	d1
		and.w	#$3f,d0
		and.w	#$3f,d1
		lsl.w	#6,d0
		add.l	d4,d2
		add.w	d1,d0
		add.l	d5,d3
		move.b	(a0,d0.w),(a1)+

		subq.l	#1,d6
		bgt.b	.ti

		move.l	a1,_r_turb_pdest
		movem.l	(sp)+,d2-d7/a2
		rts

; void D_DrawSpans8 (espan_t *pspan);

;	perspective texture mapped polygon span renderer

		cnop	0,4
_D_DrawSpans8
_D_DrawSpans16
		move.l	4(sp),a0
		fmovem.x	fp2-fp7,-(sp)

		fmove.s	#16,fp4
		fmove.x	fp4,fp5
		fmove.x	fp4,fp6
		fmul.s	_d_sdivzstepu,fp4
	movem.l	d2-d7/a2-a6,-(sp)
		fmul.s	_d_tdivzstepu,fp5
	move.l	_cacheblock,a1
		fmul.s	_d_zistepu,fp6
	move.l	_sadjust,a6
.l1
		move.l	(a0),d0		; pspan->u
		fmove.l	d0,fp3

		fmove.s	_d_sdivzstepu,fp0
		fmul.x	fp3,fp0
	move.l	4(a0),d1	; pspan->v
	fmove.l	d1,fp7
		fmove.s	_d_sdivzstepv,fp1
		fmul.x	fp7,fp1
		fadd.x	fp1,fp0
		fadd.s	_d_sdivzorigin,fp0

		fmove.s	_d_tdivzstepu,fp1
		fmul.x	fp3,fp1
		fmove.s	_d_tdivzstepv,fp2
		fmul.x	fp7,fp2
	move.l	8(a0),d7	; pspan->count
		fadd.x	fp2,fp1
		fadd.s	_d_tdivzorigin,fp1

		fmove.s	_d_zistepu,fp2
		fmul.x	fp3,fp2
	move.l	_d_viewbuffer,a2
		fmove.s	_d_zistepv,fp3
		fmul.x	fp7,fp3
	muls.l	_screenwidth,d1
		fadd.x	fp3,fp2
		fadd.s	_d_ziorigin,fp2

		fmove.s	#65536,fp3
		fdiv.x	fp2,fp3

	add.l	d0,a2
	add.l	d1,a2

		; fp0 = sdivz
		; fp1 = tdivz
		; fp2 = zi
		; fp3 = z

	move.l	_bbextents,d0
	move.l	_bbextentt,d1

		fmove.x	fp0,fp7
		fmul.x	fp3,fp7
		fmove.l	fp7,d2

		fmul.x	fp1,fp3

		add.l	a6,d2

		cmp.l	d0,d2
		ble.b	.ss0
		move.l	d0,d2
		bra.b	.ss1
.ss0
		tst.l	d2
		bpl.b	.ss1
		moveq	#0,d2
.ss1
		fmove.l	fp3,d3

		cmp.w	#16,d7		;
		blt.b	.sdiv		;
		fmove.s	#65536,fp3	;
		fadd.x	fp6,fp2		;
		fdiv.x	fp2,fp3		;
.sdiv
		add.l	_tadjust,d3

		cmp.l	d1,d3
		ble.b	.ss2
		move.l	d1,d3
		bra.b	.ss3
.ss2
		tst.l	d3
		bpl.b	.ss3
		moveq	#0,d3
.ss3

.here
		moveq	#16,d1
		cmp.l	d1,d7
		blt.w	.sx1

		fadd.x	fp4,fp0
		fadd.x	fp5,fp1

		move.l	d7,a3
		move.l	d1,d6
		sub.l	d1,a3

		fmove.x	fp0,fp7
		fmul.x	fp3,fp7
	move.l	_bbextents,d0
	move.l	_bbextentt,d7
		fmove.l	fp7,d4
		add.l	a6,d4

		fmul.x	fp1,fp3

		cmp.l	d0,d4
		ble.b	.ss4
		move.l	d0,d4
		bra.b	.ss5
.ss4
		cmp.l	d1,d4
		bge.b	.ss5
		move.l	d1,d4
.ss5
		fmove.l	fp3,d5
		add.l	_tadjust,d5

		cmp.l	d1,a3		;
		blt.b	.abc		;
		fadd.x	fp6,fp2		;
		fmove.s	#65536,fp3	;
		fdiv.x	fp2,fp3		;
.abc
		cmp.l	d7,d5
		ble.b	.ss6
		move.l	d7,d5
		bra.b	.ss7
.ss6
		cmp.l	d1,d5
		bge.b	.ss7
		move.l	d1,d5
.ss7
		move.l	d4,a4
		move.l	d5,a5
		sub.l	d2,d4
		sub.l	d3,d5
		asr.l	#4,d4
		asr.l	#4,d5

		moveq	#16,d6
		move.l	_cachewidth,d7
iter	MACRO
		move.l	d2,d1
		move.l	d3,d0
		lsr.l	d6,d1
		lsr.l	d6,d0
		mulu.w	d7,d0
		add.l	d4,d2
		add.w	d1,d0
		move.b	(a1,d0.l),(a2)+
		add.l	d5,d3
	ENDM
		iter
		iter
		iter
		iter
		iter
		iter
		iter
		iter
		iter
		iter
		iter
		iter
		iter
		iter
		iter
		move.l	d2,d1
		move.l	d3,d0
		lsr.l	d6,d1
		lsr.l	d6,d0
		mulu.w	d7,d0
		move.l	a4,d2
		add.w	d1,d0
		move.b	(a1,d0.l),(a2)+
		move.l	a5,d3

		move.l	a3,d7
		bgt.w	.here

		move.l	12(a0),a0
		move.l	a0,d0
		bne.w	.l1
		bra.w	.dexit
.sx1
		move.l	d7,d6
		sub.l	a3,a3

		move.l	d6,d0
		subq.l	#1,d0
		ble.w	.ssrend
	;	fmove.l	d0,fp7
		lea	l_to_s(pc),a4
		fmove.s	(a4,d0.w*4),fp7

		fmove.s	_d_sdivzstepu,fp3
		fmul.x	fp7,fp3
		fadd.x	fp3,fp0
		fmove.s	_d_tdivzstepu,fp3
		fmul.x	fp7,fp3
		fadd.x	fp3,fp1
		fmove.s	_d_zistepu,fp3
		fmul.x	fp7,fp3
		fadd.x	fp3,fp2
		fmove.s	#65536,fp3
		fdiv.x	fp2,fp3

		move.l	_bbextents,d0
		move.l	_bbextentt,d7

		fmove.x	fp0,fp7
		fmul.x	fp3,fp7
		fmove.l	fp7,d4
		add.l	a6,d4

		fmul.x	fp1,fp3

		cmp.l	d0,d4
		ble.b	.ss4b
		move.l	d0,d4
		bra.b	.ss5b
.ss4b
		cmp.l	d1,d4
		bge.b	.ss5b
		move.l	d1,d4
.ss5b
		fmove.l	fp3,d5
		add.l	_tadjust,d5

		cmp.l	d7,d5
		ble.b	.ss6b
		move.l	d7,d5
		bra.b	.ss7b
.ss6b
		cmp.l	d1,d5
		bge.b	.ss7b
		move.l	d1,d5
.ss7b
		move.l	d6,d0
		sub.l	d2,d4
		subq.l	#1,d0
		sub.l	d3,d5
		lsl.l	#8,d0
		divs.w	d0,d4
		divs.w	d0,d5
.ssrend
		lsr.l	#8,d2
		lsr.l	#8,d3

		move.l	_cachewidth,d7

.l0		move.w	d2,d1
		move.w	d3,d0
		lsr.w	#8,d1
		lsr.w	#8,d0
		mulu.w	d7,d0
		add.w	d4,d2
		add.w	d1,d0
		move.b	(a1,d0.l),(a2)+
		add.w	d5,d3
		subq.l	#1,d6
		bgt.b	.l0

		move.l	12(a0),a0
		move.l	a0,d0
		bne.w	.l1
.dexit
		movem.l	(sp)+,d2-d7/a2-a6
		fmovem.x	(sp)+,fp2-fp7
		rts


		cnop	0,8
l_to_s:
		dc.l	$00000000	; 0
		dc.l	$3f800000	; 1
		dc.l	$40000000	; 2
		dc.l	$40400000	; 3
		dc.l	$40800000	; 4
		dc.l	$40a00000	; 5
		dc.l	$40c00000	; 6
		dc.l	$40e00000	; 7
		dc.l	$41000000	; 8
		dc.l	$41100000	; 9
		dc.l	$41200000	; 10
		dc.l	$41300000	; 11
		dc.l	$41400000	; 12
		dc.l	$41500000	; 13
		dc.l	$41600000	; 14
		dc.l	$41700000	; 15


		;	z-buffer render loop
		cnop	0,4
_D_DrawZSpans:
		move.l	4(sp),a0
		fmovem.x	fp2/fp5-fp7,-(sp)
		movem.l	d2/d5-d7/a2/a3/a5,-(sp)
		fmove.s	(_d_zistepu).l,fp7
		fmove.x	fp7,fp1
		fmul.s	#2147483648,fp1
		move.l	a0,a5
		move.l	(_d_pzbuffer).l,a3
		fmove.s	(_d_ziorigin).l,fp5
		fmove.l	fp1,d7
		fmove.s	(_d_zistepv).l,fp6
.j4
		move.l	(a5)+,d0
		fmove.l	d0,fp1
		fmul.x	fp7,fp1
		move.l	(a5)+,d1
		fmove.l	d1,fp2
		fmul.x	fp6,fp2
		mulu.l	_d_zwidth,d1
		add.l	d1,d0
		fadd.x	fp5,fp2
		fadd.x	fp1,fp2
		fmul.s	#2147483648,fp2
		lea	(a3,d0.l*2),a1
		move.l	(a5)+,d6
		fmove.l	fp2,d5

		move.l	a1,d0
		btst	#1,d0
		beq.b	.j3
		move.l	d5,d0
		swap	d0
		subq.l	#1,d6
		add.l	d7,d5
		move.w	d0,(a1)+
.j3
		move.l	d6,d2
		asr.l	#1,d2
		ble.b	.j1


		; d5=izi
		; d7=izistep
.j0
		move.l	d5,d0
		add.l	d7,d5
		move.l	d5,d1
		swap	d1
		add.l	d7,d5
		move.w	d1,d0
		move.l	d0,(a1)+
		subq.l	#1,d2
		bgt.b	.j0

.j1		btst	#0,d6
		beq.b	.j2
		swap	d5
		move.w	d5,(a1)
.j2
		move.l	(a5),a5
		move.l	a5,d0
		bne.b	.j4
		movem.l	(sp)+,d2/d5-d7/a2/a3/a5
		fmovem.x	(sp)+,fp2/fp5-fp7
		rts

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

; void D_DrawParticle (particle_t *pparticle);

;	Draws a single particle.

		cnop	0,4
_D_DrawParticle_fixme:
		; TODO transparency
		move.l	4(sp),a0
		fmovem.x	fp2-fp7,-(sp)
		movem.l	d4-d7/a2-a3/a5,-(sp)

		move.l	a0,a5

		fmove.s	(a5),fp5
		fsub.s	_r_origin,fp5
		fmove.s	4(a5),fp6
		fsub.s	_r_origin+4,fp6
		fmove.s	8(a5),fp7
		fsub.s	_r_origin+8,fp7

		fmove.s	_r_ppn,fp0
		fmul.x	fp5,fp0
		fmove.s	_r_ppn+4,fp1
		fmul.x	fp6,fp1
		fmove.s	_r_ppn+8,fp2
		fmul.x	fp7,fp2
		fadd.x	fp1,fp0
		fadd.x	fp2,fp0

		fcmp.s	#8,fp0
		fbult.w	.end

		fmove.s	#1,fp4
		fdiv.x	fp0,fp4

		fmove.s	_r_pright,fp0
		fmul.x	fp5,fp0
		fmove.s	_r_pright+4,fp1
		fmul.x	fp6,fp1
		fmove.s	_r_pright+8,fp2
		fmul.x	fp7,fp2
		fadd.x	fp1,fp0
		fadd.x	fp2,fp0
		fmul.x	fp4,fp0
		fadd.s	_xcenter,fp0
		fadd.s	#0.5,fp0
		fmove.l	fp0,d7

		cmp.l	_d_vrectright_particle,d7
		bgt.w	.end
		cmp.l	_d_vrectx,d7
		blt.w	.end

		fmove.s	_r_pup,fp0
		fmul.x	fp5,fp0
		fmove.s	_r_pup+4,fp1
		fmul.x	fp6,fp1
		fmove.s	_r_pup+8,fp2
		fmul.x	fp7,fp2
		fadd.x	fp1,fp0
		fadd.x	fp2,fp0
		fmul.x	fp4,fp0
		fmove.s	_ycenter,fp1
		fsub.x	fp0,fp1
		fadd.s	#0.5,fp1
		fmove.l	fp1,d6

		cmp.l	_d_vrectbottom_particle,d6
		bgt.w	.end
		cmp.l	_d_vrecty,d6
		blt.w	.end

		fmul.s	#32768,fp4

		move.l	_d_zwidth,d0
		mulu.w	d6,d0
		move.l	_d_pzbuffer,a3
		add.l	d7,d0
		add.l	d0,a3
		add.l	d0,a3

		lea	_d_scantable,a1
		move.l	_d_viewbuffer,a2
		add.l	(a1,d6.l*4),a2
		add.l	d7,a2

		fmove.l	fp4,d7	; z
		move.l	_d_pix_shift,d0
		move.l	d7,d6
		asr.l	d0,d6	; size

		move.l	_d_pix_min,d0
		cmp.l	d0,d6
		blt.b	.r0p
		move.l	_d_pix_max,d0
		cmp.l	d0,d6
		ble.b	.r1p
.r0p
		move.l	d0,d6
.r1p
		fmove.s	12(a5),fp0
		fmove.l	fp0,d1
		move.l	_d_zwidth,a5
		add.l	a5,a5

		move.l	d6,d0
		subq.l	#1,d0
		beq.b	.part0
		subq.l	#1,d0
		beq.b	.part1
		subq.l	#1,d0
		beq.b	.part2
		subq.l	#1,d0
		beq.w	.part3
		bra.w	.partx
.part0
		move.l	_d_y_aspect_shift,d0
		moveq	#0,d6
		bset	d0,d6
		tst.l	d6
		beq.w	.end
.l0p
		cmp.w	(a3),d7
		ble.b	.s0p
		move.w	d7,(a3)
		move.b	d1,(a2)
.s0p
		add.l	_screenwidth,a2
		add.l	a5,a3
		subq.l	#1,d6
		bne.b	.l0p
		bra.w	.end

.part1
		move.l	_d_y_aspect_shift,d0
		moveq	#2,d6
		lsl.l	d0,d6
		tst.l	d6
		beq.w	.end
.l1p
		cmp.w	(a3),d7
		ble.b	.s1a
		move.w	d7,(a3)
		move.b	d1,(a2)
.s1a
		cmp.w	2(a3),d7
		ble.b	.s1b
		move.w	d7,2(a3)
		move.b	d1,1(a2)
.s1b
		add.l	_screenwidth,a2
		add.l	a5,a3
		subq.l	#1,d6
		bne.b	.l1p
		bra.w	.end

.part2
		move.l	_d_y_aspect_shift,d0
		moveq	#3,d6
		lsl.l	d0,d6
		tst.l	d6
		beq.w	.end
.l2p
		cmp.w	(a3),d7
		ble.b	.s2a
		move.w	d7,(a3)
		move.b	d1,(a2)
.s2a
		cmp.w	2(a3),d7
		ble.b	.s2b
		move.w	d7,2(a3)
		move.b	d1,1(a2)
.s2b
		cmp.w	4(a3),d7
		ble.b	.s2c
		move.w	d7,4(a3)
		move.b	d1,2(a2)
.s2c
		add.l	_screenwidth,a2
		add.l	a5,a3
		subq.l	#1,d6
		bne.b	.l2p
		bra.b	.end

.part3
		move.l	_d_y_aspect_shift,d0
		moveq	#4,d6
		lsl.l	d0,d6
		tst.l	d6
		beq.b	.end
.l3p
		cmp.w	(a3),d7
		ble.b	.s3a
		move.w	d7,(a3)
		move.b	d1,(a2)
.s3a
		cmp.w	2(a3),d7
		ble.b	.s3b
		move.w	d7,2(a3)
		move.b	d1,1(a2)
.s3b
		cmp.w	4(a3),d7
		ble.b	.s3c
		move.w	d7,4(a3)
		move.b	d1,2(a2)
.s3c
		cmp.w	6(a3),d7
		ble.b	.s3d
		move.w	d7,6(a3)
		move.b	d1,3(a2)
.s3d
		add.l	_screenwidth,a2
		add.l	a5,a3
		subq.l	#1,d6
		bne.b	.l3p
		bra.b	.end

.partx
		move.l	_d_y_aspect_shift,d0
		move.l	d6,d5
		beq.b	.end
		lsl.l	d0,d5
		tst.l	d5
		beq.b	.end
.l4p
		move.l	a3,a0
		move.l	d6,d4
.l5p
		cmp.w	(a0),d7
		ble.b	.s4p
		move.w	d7,(a0)
		move.b	d1,(a2,d4.w)
.s4p
		addq.l	#2,a0
		subq.l	#1,d4
		bne.b	.l5p

		add.l	_screenwidth,a2
		add.l	a5,a3
		subq.l	#1,d5
		bne.b	.l4p
.end
		movem.l	(sp)+,d4-d7/a2-a3/a5
		fmovem.x	(sp)+,fp2-fp7
		rts


		END
