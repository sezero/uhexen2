;
; 2000-04-17
;
; c2p1x1_8_c5_030
;
; 1.31vbl [all dma off] on Blizzard1230-IV@50MHz
;
; 2000-04-17: added bplsize modifying init (smcinit)
; 1999-01-08: initial version
;
; bplsize must be less than or equal to 16kB!
;

	IFND	BPLX
BPLX	EQU	320
	ENDC
	IFND	BPLY
BPLY	EQU	256
	ENDC
	IFND	BPLSIZE
BPLSIZE	EQU	BPLX*BPLY/8
	ENDC
	IFND	CHUNKYXMAX
CHUNKYXMAX EQU	BPLX
	ENDC
	IFND	CHUNKYYMAX
CHUNKYYMAX EQU	BPLY
	ENDC

	xdef	_c2p1x1_8_c5_030_smcinit
	xdef	_c2p1x1_8_c5_030_init
	xdef	_c2p1x1_8_c5_030

;	incdir	include:
;	include	lvo/exec_lib.i


	section	code,code

; d0.w	chunkyx [chunky-pixels]
; d1.w	chunkyy [chunky-pixels]
; d2.w	(scroffsx) [screen-pixels]
; d3.w	scroffsy [screen-pixels]
; d4.l	(rowlen) [bytes] -- offset between one row and the next in a bpl
; d5.l	bplsize [bytes] -- offset between one row in one bpl and the next bpl
; d6.l	(chunkylen) [bytes] -- offset between one row and the next in chunkybuf

_c2p1x1_8_c5_030_smcinit
c2p1x1_8_c5_030_smcinit
	movem.l	d2-d3/d5/a6,-(sp)
	andi.l	#$ffff,d0
	mulu.w	d0,d3
	lsr.l	#3,d3
	move.l	d3,c2p1x1_8_c5_030_scroffs
	mulu.w	d0,d1
	move.l	d1,c2p1x1_8_c5_030_pixels

	move.w	d5,c2p1x1_8_c5_030_smc1
	move.w	d5,c2p1x1_8_c5_030_smc2
	move.w	d5,c2p1x1_8_c5_030_smc5
	move.w	d5,c2p1x1_8_c5_030_smc8
	move.w	d5,c2p1x1_8_c5_030_smc11

	move.w	d5,d0
	neg.w	d0
	subq.w	#4,d0
	move.w	d0,c2p1x1_8_c5_030_smc3
	move.w	d0,c2p1x1_8_c5_030_smc6
	move.w	d0,c2p1x1_8_c5_030_smc9
	move.w	d0,c2p1x1_8_c5_030_smc12

	add.l	d5,d5
	move.w	d5,c2p1x1_8_c5_030_smc4
	move.w	d5,c2p1x1_8_c5_030_smc10
	add.l	d5,d5
	move.l	d5,c2p1x1_8_c5_030_smc7

	move.l	$4.w,a6
	jsr	_LVOCacheClearU(a6)
	movem.l	(sp)+,d2-d3/d5/a6
	rts

; d0.w	chunkyx [chunky-pixels]
; d1.w	chunkyy [chunky-pixels]
; d2.w	(scroffsx) [screen-pixels]
; d3.w	scroffsy [screen-pixels]
; d4.l	(rowlen) [bytes] -- offset between one row and the next in a bpl
; d5.l	(bplsize) [bytes] -- offset between one row in one bpl and the next bpl
; d6.l	(chunkylen) [bytes] -- offset between one row and the next in chunkybuf

_c2p1x1_8_c5_030_init
c2p1x1_8_c5_030_init
	movem.l	d2-d3,-(sp)
	andi.l	#$ffff,d0
	mulu.w	d0,d3
	lsr.l	#3,d3
	move.l	d3,c2p1x1_8_c5_030_scroffs
	mulu.w	d0,d1
	move.l	d1,c2p1x1_8_c5_030_pixels
	movem.l	(sp)+,d2-d3
	rts

; a0	c2pscreen
; a1	bitplanes

_c2p1x1_8_c5_030
c2p1x1_8_c5_030
	movem.l	d2-d7/a2-a6,-(sp)

	move.l	#$33333333,d5
	move.l	#$55555555,d6
	move.l	#$00ff00ff,a6

	add.w	#BPLSIZE,a1
c2p1x1_8_c5_030_smc1 EQU *-2
	add.l	c2p1x1_8_c5_030_scroffs,a1

	move.l	c2p1x1_8_c5_030_pixels,a2
	add.l	a0,a2
	cmp.l	a0,a2
	beq	.none
	addq.l	#4,a2

	movem.l	a0-a1,-(sp)

	move.l	(a0)+,d0
	move.l	(a0)+,d2
	move.l	(a0)+,d1
	move.l	(a0)+,d3

	move.l	#$0f0f0f0f,d4		; Merge 4x1, part 1
	and.l	d4,d0
	and.l	d4,d2
	lsl.l	#4,d0
	or.l	d2,d0

	and.l	d4,d1
	and.l	d4,d3
	move.l	(a0)+,d2
	lsl.l	#4,d1
	or.l	d3,d1

	move.l	d1,a3
	move.l	(a0)+,d1
	move.l	(a0)+,d3
	move.l	(a0)+,d7

	and.l	d4,d2			; Merge 4x1, part 2
	and.l	d4,d1
	lsl.l	#4,d2
	or.l	d1,d2

	and.l	d4,d3
	and.l	d4,d7
	lsl.l	#4,d3
	or.l	d7,d3

	move.l	a3,d1

	move.w	d2,d7			; Swap 16x2
	move.w	d0,d2
	swap	d2
	move.w	d2,d0
	move.w	d7,d2

	move.w	d3,d7
	move.w	d1,d3
	swap	d3
	move.w	d3,d1
	move.w	d7,d3

	bra.s	.start1
.x1
	move.l	(a0)+,d2
	move.l	(a0)+,d1
	move.l	(a0)+,d3
	move.l	d7,BPLSIZE(a1)
c2p1x1_8_c5_030_smc2 EQU *-2

	move.l	#$0f0f0f0f,d4		; Merge 4x1, part 1
	and.l	d4,d0
	and.l	d4,d2
	lsl.l	#4,d0
	or.l	d2,d0

	and.l	d4,d1
	and.l	d4,d3
	lsl.l	#4,d1
	or.l	d3,d1

	move.l	d1,a3
	move.l	(a0)+,d2
	and.l	d4,d2			; Merge 4x1, part 2
	lsl.l	#4,d2
	move.l	(a0)+,d1
	move.l	(a0)+,d3
	move.l	(a0)+,d7

	move.l	a4,(a1)+

	and.l	d4,d1
	or.l	d1,d2

	and.l	d4,d3
	and.l	d4,d7
	lsl.l	#4,d3
	or.l	d7,d3

	move.l	a3,d1

	move.w	d2,d7			; Swap 16x2
	move.w	d0,d2
	swap	d2
	move.w	d2,d0
	move.w	d7,d2

	move.w	d3,d7
	move.w	d1,d3
	swap	d3
	move.w	d3,d1
	move.w	d7,d3

	move.l	a5,-BPLSIZE-4(a1)
c2p1x1_8_c5_030_smc3 EQU *-2
.start1
	move.l	a6,d4

	move.l	d2,d7			; Swap 2x2
	lsr.l	#2,d7
	eor.l	d0,d7
	and.l	d5,d7
	eor.l	d7,d0
	lsl.l	#2,d7
	eor.l	d7,d2

	move.l	d3,d7
	lsr.l	#2,d7
	eor.l	d1,d7
	and.l	d5,d7
	eor.l	d7,d1
	lsl.l	#2,d7
	eor.l	d7,d3

	move.l	d1,d7
	lsr.l	#8,d7
	eor.l	d0,d7
	and.l	d4,d7
	eor.l	d7,d0
	lsl.l	#8,d7
	eor.l	d7,d1

	move.l	d1,d7
	lsr.l	#1,d7
	eor.l	d0,d7
	and.l	d6,d7
	eor.l	d7,d0
	move.l	d0,BPLSIZE*2(a1)
c2p1x1_8_c5_030_smc4 EQU *-2
	add.l	d7,d7
	eor.l	d1,d7

	move.l	d3,d1
	lsr.l	#8,d1
	eor.l	d2,d1
	and.l	d4,d1
	eor.l	d1,d2
	lsl.l	#8,d1
	eor.l	d1,d3

	move.l	d3,d1
	lsr.l	#1,d1
	eor.l	d2,d1
	and.l	d6,d1
	eor.l	d1,d2
	add.l	d1,d1
	eor.l	d1,d3

	move.l	(a0)+,d0
	move.l	d2,a4
	move.l	d3,a5

	cmpa.l	a0,a2
	bne	.x1

	move.l	d7,BPLSIZE(a1)
c2p1x1_8_c5_030_smc5 EQU *-2
	move.l	a4,(a1)+
	move.l	a5,-BPLSIZE-4(a1)
c2p1x1_8_c5_030_smc6 EQU *-2

	movem.l	(sp)+,a0-a1

	move.l	(a0)+,d0
	add.l	#BPLSIZE*4,a1
c2p1x1_8_c5_030_smc7 EQU *-4
	move.l	(a0)+,d2
	move.l	(a0)+,d1
	move.l	(a0)+,d3

	move.l	#$f0f0f0f0,d4		; Merge 4x1, part 1
	and.l	d4,d0
	and.l	d4,d2
	lsr.l	#4,d2
	or.l	d2,d0

	and.l	d4,d1
	and.l	d4,d3
	move.l	(a0)+,d2
	lsr.l	#4,d3
	or.l	d3,d1

	move.l	d1,a3
	move.l	(a0)+,d1
	move.l	(a0)+,d3
	move.l	(a0)+,d7

	and.l	d4,d2			; Merge 4x1, part 2
	and.l	d4,d1
	lsr.l	#4,d1
	or.l	d1,d2

	and.l	d4,d3
	and.l	d4,d7
	lsr.l	#4,d7
	or.l	d7,d3

	move.l	a3,d1

	move.w	d2,d7			; Swap 16x2
	move.w	d0,d2
	swap	d2
	move.w	d2,d0
	move.w	d7,d2

	move.w	d3,d7
	move.w	d1,d3
	swap	d3
	move.w	d3,d1
	move.w	d7,d3

	bra.s	.start2
.x2
	move.l	(a0)+,d2
	move.l	(a0)+,d1
	move.l	(a0)+,d3
	move.l	d7,BPLSIZE(a1)
c2p1x1_8_c5_030_smc8 EQU *-2

	move.l	#$f0f0f0f0,d4		; Merge 4x1, part 1
	and.l	d4,d0
	and.l	d4,d2
	lsr.l	#4,d2
	or.l	d2,d0

	and.l	d4,d1
	and.l	d4,d3
	lsr.l	#4,d3
	or.l	d3,d1

	move.l	d1,a3
	move.l	(a0)+,d2
	and.l	d4,d2			; Merge 4x1, part 2
	move.l	(a0)+,d1
	move.l	(a0)+,d3
	move.l	(a0)+,d7

	move.l	a4,(a1)+

	and.l	d4,d1
	lsr.l	#4,d1
	or.l	d1,d2

	and.l	d4,d3
	and.l	d4,d7
	lsr.l	#4,d7
	or.l	d7,d3

	move.l	a3,d1

	move.w	d2,d7			; Swap 16x2
	move.w	d0,d2
	swap	d2
	move.w	d2,d0
	move.w	d7,d2

	move.w	d3,d7
	move.w	d1,d3
	swap	d3
	move.w	d3,d1
	move.w	d7,d3

	move.l	a5,-BPLSIZE-4(a1)
c2p1x1_8_c5_030_smc9 EQU *-2
.start2
	move.l	a6,d4

	move.l	d2,d7			; Swap 2x2
	lsr.l	#2,d7
	eor.l	d0,d7
	and.l	d5,d7
	eor.l	d7,d0
	lsl.l	#2,d7
	eor.l	d7,d2

	move.l	d3,d7
	lsr.l	#2,d7
	eor.l	d1,d7
	and.l	d5,d7
	eor.l	d7,d1
	lsl.l	#2,d7
	eor.l	d7,d3

	move.l	d1,d7
	lsr.l	#8,d7
	eor.l	d0,d7
	and.l	d4,d7
	eor.l	d7,d0
	lsl.l	#8,d7
	eor.l	d7,d1

	move.l	d1,d7
	lsr.l	#1,d7
	eor.l	d0,d7
	and.l	d6,d7
	eor.l	d7,d0
	move.l	d0,BPLSIZE*2(a1)
c2p1x1_8_c5_030_smc10 EQU *-2
	add.l	d7,d7
	eor.l	d1,d7

	move.l	d3,d1
	lsr.l	#8,d1
	eor.l	d2,d1
	and.l	d4,d1
	eor.l	d1,d2
	lsl.l	#8,d1
	eor.l	d1,d3

	move.l	d3,d1
	lsr.l	#1,d1
	eor.l	d2,d1
	and.l	d6,d1
	eor.l	d1,d2
	add.l	d1,d1
	eor.l	d1,d3

	move.l	(a0)+,d0
	move.l	d2,a4
	move.l	d3,a5

	cmpa.l	a0,a2
	bne	.x2

	move.l	d7,BPLSIZE(a1)
c2p1x1_8_c5_030_smc11 EQU *-2
	move.l	a4,(a1)+
	move.l	a5,-BPLSIZE-4(a1)
c2p1x1_8_c5_030_smc12 EQU *-2

.none
	movem.l	(sp)+,d2-d7/a2-a6
	rts

	section	bss,bss

c2p1x1_8_c5_030_scroffs	ds.l	1
c2p1x1_8_c5_030_pixels	ds.l	1
