;
;
; Date: 2000-04-11			Mikael Kalms (Scout/C-Lous & more)
;					Email: mikael@kalms.org
;
; About:
;   1x1 8bpl cpu5 C2P for contigous bitplanes and no horizontal modulo
;
;   This routine is intended for use on all 68040 and 68060 based systems.
;   It is not designed to perform well on 68020-030.
;
;   This routine is released into the public domain. It may be freely used
;   for non-commercial as well as commercial purposes. A short notice via
;   email is always appreciated, though.
;
; Timings:
;   Estimated to run at copyspeed on 040-40 and 060
;
; Features:
;   Handles bitplanes of virtually any size (4GB)
;
; Restrictions:
;   Chunky-buffer must be an even multiple of 32 pixels wide
;   If incorrect/invalid parameters are specified, the routine will
;   most probably crash.
;
; c2p1x1_8_c5_040_init			sets chunkybuffer size/pos & bplsize
; c2p1x1_8_c5_040			performs the actual c2p conversion
;


	XDEF	_c2p1x1_8_c5_040_init
	XDEF	_c2p1x1_8_c5_040


	section	code,code


; d0.w	chunkyx [chunky-pixels]
; d1.w	chunkyy [chunky-pixels]
; d2.w	(scroffsx) [screen-pixels]
; d3.w	scroffsy [screen-pixels]
; d4.l	(rowlen) [bytes] -- offset between one row and the next in a bpl
; d5.l	bplsize [bytes] -- offset between one row in one bpl and the next bpl
; d6.l	(chunkylen) [bytes] -- offset between one row and the next in chunkybuf

_c2p1x1_8_c5_040_init
c2p1x1_8_c5_040_init
	move.l	d3,-(sp)
	mulu.w	d0,d3
	lsr.l	#3,d3
	move.l	d3,c2p1x1_8_c5_040_scroffs
	mulu.w	d0,d1
	move.l	d1,c2p1x1_8_c5_040_pixels
	move.l	d5,d0
	lsl.l	#3,d0
	sub.l	d5,d0
	move.l	d0,c2p1x1_8_c5_040_delta0
	addq.l	#4,d0
	move.l	d0,c2p1x1_8_c5_040_delta4
	move.l	d5,d0
	lsl.l	#2,d0
	move.l	d0,c2p1x1_8_c5_040_delta1
	move.l	d0,c2p1x1_8_c5_040_delta3
	move.l	d0,c2p1x1_8_c5_040_delta5
	move.l	d0,c2p1x1_8_c5_040_delta7
	sub.l	d5,d0
	move.l	d0,c2p1x1_8_c5_040_delta2
	move.l	d0,c2p1x1_8_c5_040_delta6
	move.l	d0,c2p1x1_8_c5_040_delta8
	move.l	(sp)+,d3
	rts



; a0	c2pscreen
; a1	bitplanes

_c2p1x1_8_c5_040
c2p1x1_8_c5_040
	movem.l	d2-d7/a2-a6,-(sp)

	add.l	c2p1x1_8_c5_040_delta0(pc),a1
	add.l	c2p1x1_8_c5_040_scroffs(pc),a1

	move.l	c2p1x1_8_c5_040_pixels(pc),d0
	beq	.none
	add.l	a0,d0
	move.l	d0,-(sp)

	tst.b	16(a0)
	move.l	(a0)+,d0
	move.l	(a0)+,d1
	move.l	(a0)+,d2
	move.l	(a0)+,d3
	tst.b	16(a0)
	move.l	(a0)+,d4
	move.l	(a0)+,d5
	move.l	(a0)+,a5
	move.l	(a0)+,a6

	swap	d4			; Swap 16x4, part 1
	swap	d5
	eor.w	d0,d4
	eor.w	d1,d5
	eor.w	d4,d0
	eor.w	d5,d1
	eor.w	d0,d4
	eor.w	d1,d5
	swap	d4
	swap	d5

	move.l	d4,d6			; Swap 2x4, part 1
	move.l	d5,d7
	lsr.l	#2,d6
	lsr.l	#2,d7
	eor.l	d0,d6
	eor.l	d1,d7
	and.l	#$33333333,d6
	and.l	#$33333333,d7
	eor.l	d6,d0
	eor.l	d7,d1
	lsl.l	#2,d6
	lsl.l	#2,d7
	eor.l	d6,d4
	eor.l	d7,d5

	exg	d4,a5
	exg	d5,a6

	swap	d4			; Swap 16x4, part 2
	swap	d5
	eor.w	d2,d4
	eor.w	d3,d5
	eor.w	d4,d2
	eor.w	d5,d3
	eor.w	d2,d4
	eor.w	d3,d5
	swap	d4
	swap	d5

	move.l	d4,d6			; Swap 2x4, part 1
	move.l	d5,d7
	lsr.l	#2,d6
	lsr.l	#2,d7
	eor.l	d2,d6
	eor.l	d3,d7
	and.l	#$33333333,d6
	and.l	#$33333333,d7
	eor.l	d6,d2
	eor.l	d7,d3
	lsl.l	#2,d6
	lsl.l	#2,d7
	eor.l	d6,d4
	eor.l	d7,d5

	move.l	d1,d6			; Swap 4x1, part 1
	move.l	d3,d7
	lsr.l	#4,d6
	lsr.l	#4,d7
	eor.l	d0,d6
	eor.l	d2,d7
	and.l	#$0f0f0f0f,d6
	and.l	#$0f0f0f0f,d7
	eor.l	d6,d0
	eor.l	d7,d2
	lsl.l	#4,d6
	lsl.l	#4,d7
	eor.l	d6,d1
	eor.l	d7,d3

	move.l	d2,d6			; Swap 8x2, part 1
	move.l	d3,d7
	lsr.l	#8,d6
	lsr.l	#8,d7
	eor.l	d0,d6
	eor.l	d1,d7
	and.l	#$00ff00ff,d6
	and.l	#$00ff00ff,d7
	eor.l	d6,d0
	eor.l	d7,d1
	lsl.l	#8,d6
	lsl.l	#8,d7
	eor.l	d6,d2
	eor.l	d7,d3

	bra	.start

	cnop	0,4
.x
	tst.b	32(a0)
	move.l	(a0)+,d0
	move.l	(a0)+,d1
	move.l	(a0)+,d2
	move.l	(a0)+,d3
	tst.b	32(a0)
	move.l	(a0)+,d4
	move.l	(a0)+,d5
	move.l	(a0)+,a5
	move.l	(a0)+,a6

	move.l	d6,(a1)

	swap	d4			; Swap 16x4, part 1
	swap	d5
	eor.w	d0,d4
	eor.w	d1,d5
	eor.w	d4,d0
	eor.w	d5,d1
	eor.w	d0,d4
	sub.l	c2p1x1_8_c5_040_delta1(pc),a1
	eor.w	d1,d5
	swap	d4
	swap	d5

	move.l	d4,d6			; Swap 2x4, part 1
	move.l	d7,(a1)
	move.l	d5,d7
	lsr.l	#2,d6
	lsr.l	#2,d7
	eor.l	d0,d6
	eor.l	d1,d7
	and.l	#$33333333,d6
	and.l	#$33333333,d7
	eor.l	d6,d0
	eor.l	d7,d1
	lsl.l	#2,d6
	lsl.l	#2,d7
	eor.l	d6,d4
	eor.l	d7,d5

	exg	d4,a5
	add.l	c2p1x1_8_c5_040_delta2(pc),a1
	exg	d5,a6

	swap	d4			; Swap 16x4, part 2
	swap	d5
	eor.w	d2,d4
	eor.w	d3,d5
	eor.w	d4,d2
	eor.w	d5,d3
	eor.w	d2,d4
	eor.w	d3,d5
	swap	d4
	swap	d5

	move.l	a3,(a1)
	move.l	d4,d6			; Swap 2x4, part 2
	move.l	d5,d7
	lsr.l	#2,d6
	lsr.l	#2,d7
	eor.l	d2,d6
	eor.l	d3,d7
	and.l	#$33333333,d6
	and.l	#$33333333,d7
	eor.l	d6,d2
	eor.l	d7,d3
	lsl.l	#2,d6
	lsl.l	#2,d7
	eor.l	d6,d4
	eor.l	d7,d5

	move.l	d1,d6			; Swap 4x1, part 1
	move.l	d3,d7
	lsr.l	#4,d6
	lsr.l	#4,d7
	eor.l	d0,d6
	eor.l	d2,d7
	and.l	#$0f0f0f0f,d6
	and.l	#$0f0f0f0f,d7
	sub.l	c2p1x1_8_c5_040_delta3(pc),a1
	eor.l	d6,d0
	eor.l	d7,d2
	lsl.l	#4,d6
	lsl.l	#4,d7
	eor.l	d6,d1
	move.l	a4,(a1)
	eor.l	d7,d3

	move.l	d2,d6			; Swap 8x2, part 1
	move.l	d3,d7
	lsr.l	#8,d6
	lsr.l	#8,d7
	eor.l	d0,d6
	eor.l	d1,d7
	and.l	#$00ff00ff,d6
	and.l	#$00ff00ff,d7
	eor.l	d6,d0
	eor.l	d7,d1
	lsl.l	#8,d6
	lsl.l	#8,d7
	eor.l	d6,d2
	add.l	c2p1x1_8_c5_040_delta4(pc),a1
	eor.l	d7,d3
.start

	move.l	d2,d6			; Swap 1x2, part 1
	move.l	d3,d7
	lsr.l	#1,d6
	lsr.l	#1,d7
	eor.l	d0,d6
	eor.l	d1,d7
	and.l	#$55555555,d6
	and.l	#$55555555,d7
	eor.l	d6,d0
	eor.l	d7,d1
	move.l	d0,(a1)
	add.l	d6,d6
	add.l	d7,d7
	eor.l	d6,d2
	eor.l	d7,d3

	move.l	a5,d6
	move.l	a6,d7
	move.l	d2,a3
	move.l	d3,a4

	move.l	d5,d2			; Swap 4x1, part 2
	move.l	d7,d3
	lsr.l	#4,d2
	lsr.l	#4,d3
	sub.l	c2p1x1_8_c5_040_delta5(pc),a1
	eor.l	d4,d2
	eor.l	d6,d3
	and.l	#$0f0f0f0f,d2
	and.l	#$0f0f0f0f,d3
	eor.l	d2,d4
	move.l	d1,(a1)
	eor.l	d3,d6

	lsl.l	#4,d2
	lsl.l	#4,d3
	eor.l	d2,d5
	eor.l	d3,d7

	move.l	d4,d2			; Swap 8x2, part 2
	move.l	d5,d3
	lsr.l	#8,d2
	lsr.l	#8,d3
	add.l	c2p1x1_8_c5_040_delta6(pc),a1
	eor.l	d6,d2
	eor.l	d7,d3
	and.l	#$00ff00ff,d2
	and.l	#$00ff00ff,d3
	eor.l	d2,d6
	move.l	a3,(a1)
	eor.l	d3,d7

	lsl.l	#8,d2
	lsl.l	#8,d3
	eor.l	d2,d4
	eor.l	d3,d5

	move.l	d4,d2			; Swap 1x2, part 2
	move.l	d5,d3
	sub.l	c2p1x1_8_c5_040_delta7(pc),a1
	lsr.l	#1,d2
	lsr.l	#1,d3
	eor.l	d6,d2
	eor.l	d7,d3
	and.l	#$55555555,d2
	move.l	a4,(a1)
	and.l	#$55555555,d3

	eor.l	d2,d6
	eor.l	d3,d7
	add.l	d2,d2
	add.l	d3,d3
	eor.l	d2,d4
	eor.l	d3,d5

	add.l	c2p1x1_8_c5_040_delta8(pc),a1
	move.l	d4,a3
	move.l	d5,a4

	cmp.l	(sp),a0
	bne	.x

	move.l	d6,(a1)
	sub.l	c2p1x1_8_c5_040_delta1(pc),a1
	move.l	d7,(a1)
	add.l	c2p1x1_8_c5_040_delta2(pc),a1
	move.l	a3,(a1)
	sub.l	c2p1x1_8_c5_040_delta3(pc),a1
	move.l	a4,(a1)

	addq.l	#4,sp

.none	movem.l	(sp)+,d2-d7/a2-a6
	rts

			cnop	0,4

c2p1x1_8_c5_040_data
c2p1x1_8_c5_040_scroffs	ds.l	1
c2p1x1_8_c5_040_pixels	ds.l	1
c2p1x1_8_c5_040_delta0	ds.l	1
c2p1x1_8_c5_040_delta1	ds.l	1
c2p1x1_8_c5_040_delta2	ds.l	1
c2p1x1_8_c5_040_delta3	ds.l	1
c2p1x1_8_c5_040_delta4	ds.l	1
c2p1x1_8_c5_040_delta5	ds.l	1
c2p1x1_8_c5_040_delta6	ds.l	1
c2p1x1_8_c5_040_delta7	ds.l	1
c2p1x1_8_c5_040_delta8	ds.l	1
