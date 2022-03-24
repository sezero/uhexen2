**
**  Sound mixing routines for Amiga 68k
**  Written by Frank Wille <frank@phoenix.owl.de>
**
** This implementation of S_TransferPaintBuffer() handles the following
** two formats:
**
** 1. 8-bits STEREO for Amiga native Paula sound-chip.
**    DMA buffer layout (8 bits signed samples):
**    <-- shm.samples bytes left ch. --><-- shm.samples bytes right ch. -->
**
** 2. 16-bits STEREO for AHI.
**    DMA buffer layout (16 bits signed big-endian samples):
**    <-- shm.samples * (<16bits left ch.>|<16 bits right ch.>) -->
**

;	INCLUDE	"quakedef68k.i"
dma_samples	equ	4
dma_samplebits	equ	16
dma_buffer	equ	28

	code

	xref	_shm
	xref	_paintbuffer
	xref	_paintedtime
;	xref	_volume



	xdef	_S_TransferStereoAmiga
	cnop	0,4
_S_TransferStereoAmiga:
	movem.l	d2-d7/a2-a3,-(sp)
	setso	4+8*4
.endtim so.l	1

	move.l	_shm,a1			; a1 shm (struct dma_t)
	lea	_paintbuffer,a0		; a0 paintbuffer (int left,int right)
;	fmove.s	_volume+16,fp0		; volume.value * 256
	move.l	.endtim(sp),d2
;	fmul.s	#256.0,fp0
	move.l	_paintedtime,d6
	sub.l	d6,d2			; d2 count
	beq	.exit
	move.l	dma_buffer(a1),a3	; a3 dma buffer start address
;	fmove.l	fp0,d3			; d3 snd_vol
	move.l	dma_samples(a1),d0
	cmp.l	#8,dma_samplebits(a1)
	beq	.paula8bit

; 16-bit AHI transfer
	lea	(a3,d0.l*2),a2		; a2 dma buffer end address
	lsr.l	#1,d0
	subq.l	#1,d0
	and.l	d0,d6
	lea	(a3,d6.l*4),a1		; a1 out
	move.l	#$7fff,d4		; d4 max val
	move.l	d4,d5
	not.l	d5			; d5 min val
	move.l	a2,d6
	sub.l	a3,d6			; d6 buffer size
.loop16:
	move.l	(a0)+,d0
;	muls.l	d3,d0
	move.l	(a0)+,d1
	asr.l	#8,d0
;	muls.l	d3,d1
	cmp.l	d4,d0
	ble.b	.161
	move.l	d4,d0
	bra.b	.162
.161:	cmp.l	d5,d0
	bge.b	.162
	move.l	d5,d0
.162:	asr.l	#8,d1
	swap	d0
	cmp.l	d4,d1
	ble.b	.163
	move.l	d4,d1
	bra.b	.164
.163:	cmp.l	d5,d1
	bge.b	.164
	move.l	d5,d1
.164:	move.w	d1,d0			; d0 leftCh16.W | rightCh16.W
	move.l	d0,(a1)+
	cmp.l	a2,a1
	blo.b	.165
	sub.l	d6,a1
.165:	subq.l	#1,d2
	bne.b	.loop16
	movem.l	(sp)+,d2-d7/a2-a3
	rts

.paula8bit:
	lsr.l	#1,d0			; dma_samples / 2
	lea	(a3,d0.l),a2		; a2 dma buffer end address
	move.l	d0,d1
	subq.l	#1,d0
	and.l	d0,d6
	lea	(a3,d6.l),a1		; a1 out
	moveq	#$7f,d4			; d4 max val
	moveq	#-$80,d5		; d5 min val
	move.l	a2,d6
	sub.l	a3,d6			; d6 buffer size
	move.l	d1,a3			; a3 stereo buffer offset
.loop8:
	move.l	a1,d0
	and.b	#$fc,d0			; 32-bit aligned output?
	beq.b	.loop8aligned
.loop8unaligned:
	move.l	(a0)+,d0
;	muls.l	d3,d0
	move.l	(a0)+,d1
	swap	d0
;	muls.l	d3,d1
	cmp.w	d4,d0
	ble.b	.1
	move.w	d4,d0
	bra.b	.2
.1:	cmp.w	d5,d0
	bge.b	.2
	move.w	d5,d0
.2:	move.b	d0,(a1)+		; left channel
	swap	d1
	cmp.w	d4,d1
	ble.b	.3
	move.w	d4,d1
	bra.b	.4
.3:	cmp.w	d5,d1
	bge.b	.4
	move.w	d5,d1
.4:	move.b	d1,-1(a1,a3.l)		; right channel
	cmp.l	a2,a1
	blo.b	.5
	sub.l	d6,a1
.5:	subq.l	#1,d2
	bne.b	.loop8
	bra	.exit
.loop8aligned:
	cmp.l	#4,d2
	blo.b	.loop8unaligned
	move.l	(a0)+,d0
;	muls.l	d3,d0
	swap	d0
	cmp.w	d4,d0
	ble.b	.10
	move.w	d4,d0
	bra.b	.11
.10:	cmp.w	d5,d0
	bge.b	.11
	move.w	d5,d0
.11:	lsl.w	#8,d0			; left ch. byte 0
	move.l	(a0)+,d1
;	muls.l	d3,d1
	swap	d1
	cmp.w	d4,d1
	ble.b	.12
	move.w	d4,d1
	bra.b	.13
.12:	cmp.w	d5,d1
	bge.b	.13
	move.w	d5,d1
.13:	lsl.w	#8,d1			; right ch. byte 0
	move.l	(a0)+,d7
;	muls.l	d3,d7
	swap	d7
	cmp.w	d4,d7
	ble.b	.14
	move.w	d4,d7
	bra.b	.15
.14:	cmp.w	d5,d7
	bge.b	.15
	move.w	d5,d7
.15:	move.b	d7,d0			; left ch. byte 1
	swap	d0
	move.l	(a0)+,d7
;	muls.l	d3,d7
	swap	d7
	cmp.w	d4,d7
	ble.b	.16
	move.w	d4,d7
	bra.b	.17
.16:	cmp.w	d5,d7
	bge.b	.17
	move.w	d5,d7
.17:	move.b	d7,d1			; right ch. byte 1
	swap	d1
	move.l	(a0)+,d7
;	muls.l	d3,d7
	swap	d7
	cmp.w	d4,d7
	ble.b	.18
	move.w	d4,d7
	bra.b	.19
.18:	cmp.w	d5,d7
	bge.b	.19
	move.w	d5,d7
.19:	lsl.w	#8,d7
	move.w	d7,d0			; left ch. byte 2
	move.l	(a0)+,d7
;	muls.l	d3,d7
	swap	d7
	cmp.w	d4,d7
	ble.b	.20
	move.w	d4,d7
	bra.b	.21
.20:	cmp.w	d5,d7
	bge.b	.21
	move.w	d5,d7
.21:	lsl.w	#8,d7
	move.w	d7,d1			; right ch. byte 2
	move.l	(a0)+,d7
;	muls.l	d3,d7
	swap	d7
	cmp.w	d4,d7
	ble.b	.22
	move.w	d4,d7
	bra.b	.23
.22:	cmp.w	d5,d7
	bge.b	.23
	move.w	d5,d7
.23:	move.b	d7,d0			; left ch. byte 3
	move.l	(a0)+,d7
;	muls.l	d3,d7
	swap	d7
	move.l	d0,(a1)+		; write left channel
	cmp.w	d4,d7
	ble.b	.24
	move.w	d4,d7
	bra.b	.25
.24:	cmp.w	d5,d7
	bge.b	.25
	move.w	d5,d7
.25:	move.b	d7,d1			; right ch. byte 3
	move.l	d1,-4(a1,a3.l)		; write right channel
	cmp.l	a2,a1
	blo.b	.30
	sub.l	d6,a1
.30:	subq.l	#4,d2
	bne	.loop8aligned

.exit:	movem.l	(sp)+,d2-d7/a2-a3
	rts
