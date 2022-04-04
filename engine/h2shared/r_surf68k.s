**
** Quake for AMIGA
** r_surf.c assembler implementations by Frank Wille <frank@phoenix.owl.de>
**

;	INCLUDE	"quakedef68k.i"
VID_COLORMAP	equ	4

	code

	xref	_pbasesource
	xref	_prowdestbase
	xref	_r_numvblocks
	xref	_r_lightptr
	xref	_r_lightwidth
	xref	_sourcetstep
	xref	_surfrowbytes
	xref	_r_sourcemax
	xref	_r_stepback
	xref	_vid


	xdef	_R_DrawSurfaceBlock8_mip0
	cnop	0,4
_R_DrawSurfaceBlock8_mip0:
	movem.l	d1-d7/a2-a6,-(sp)
	setso	0
.v	so.l	1			; saved d1-field is used as .v

	move.l	_vid+VID_COLORMAP,a4
	move.l	_sourcetstep,a5
	move.l	_surfrowbytes,a6

; psource = pbasesource;
; prowdest = prowdestbase;
	move.l	_pbasesource,a2		; a2 psource
	move.l	_prowdestbase,a3	; a3 prowdest

; for (v=0 ; v<r_numvblocks ; v++) {
	move.l	_r_numvblocks,(sp)	; .v
	beq	.quit

.vloop:
; lightleft = r_lightptr[0];
; lightright = r_lightptr[1];
	move.l	_r_lightptr,a0
	move.l	(a0)+,d6		; d6 lightleft
	move.l	(a0),d7			; d7 lightright

; r_lightptr += r_lightwidth;
; lightleftstep = (r_lightptr[0] - lightleft) >> 4;
; lightrightstep = (r_lightptr[1] - lightright) >> 4;
	move.l	_r_lightwidth,d0
	lea	-4(a0,d0.l*4),a0
	move.l	a0,_r_lightptr
	move.l	(a0)+,d1
	sub.l	d6,d1
	lsr.l	#4,d1			; d1 lightleftstep
	move.l	(a0),d2
	sub.l	d7,d2
	lsr.l	#4,d2			; d2 lightrightstep

; for (i=0 ; i<16 ; i++) {
	moveq	#15,d5

; lighttemp = lightleft - lightright;
; lightstep = lighttemp >> 4;
; light = lightright;
.1:	move.l	d6,d4
	sub.l	d7,d4
	lsr.l	#4,d4			; d4 lightstep
	move.l	d7,d3			; d3 light

; for (b=15; b>=0; b--) {
	moveq	#16,d0
	add.w	d0,a2
	add.w	d0,a3

; pix = psource[b];
; prowdest[b] = ((unsigned char *)vid.colormap)[(light & 0xFF00) + pix];
; light += lightstep;
	REPT	16
	move.w	d3,d0
	move.b	-(a2),d0
	move.b	(a4,d0.l),-(a3)
	add.l	d4,d3
	ENDR

; psource += sourcetstep;
; lightright += lightrightstep;
; lightleft += lightleftstep;
; prowdest += surfrowbytes;
	add.l	a5,a2
	add.l	a6,a3
	add.l	d1,d6
	add.l	d2,d7
	dbf	d5,.1

; if (psource >= r_sourcemax) psource -= r_stepback;
	cmp.l	_r_sourcemax,a2
	blo.b	.2
	sub.l	_r_stepback,a2

.2:	subq.l	#1,(sp)			; .v
	bne	.vloop
.quit:	movem.l	(sp)+,d1-d7/a2-a6
	rts


	xdef	_R_DrawSurfaceBlock8_mip1
	cnop	0,4
_R_DrawSurfaceBlock8_mip1:
	movem.l	d1-d7/a2-a6,-(sp)
	setso	0
.v	so.l	1			; saved d1-field is used as .v

	move.l	_vid+VID_COLORMAP,a4
	move.l	_sourcetstep,a5
	move.l	_surfrowbytes,a6

; psource = pbasesource;
; prowdest = prowdestbase;
	move.l	_pbasesource,a2		; a2 psource
	move.l	_prowdestbase,a3	; a3 prowdest

; for (v=0 ; v<r_numvblocks ; v++) {
	move.l	_r_numvblocks,(sp)	; .v
	beq	.quit

.vloop:
; lightleft = r_lightptr[0];
; lightright = r_lightptr[1];
	move.l	_r_lightptr,a0
	move.l	(a0)+,d6		; d6 lightleft
	move.l	(a0),d7			; d7 lightright

; r_lightptr += r_lightwidth;
; lightleftstep = (r_lightptr[0] - lightleft) >> 3;
; lightrightstep = (r_lightptr[1] - lightright) >> 3;
	move.l	_r_lightwidth,d0
	lea	-4(a0,d0.l*4),a0
	move.l	a0,_r_lightptr
	move.l	(a0)+,d1
	sub.l	d6,d1
	lsr.l	#3,d1			; d1 lightleftstep
	move.l	(a0),d2
	sub.l	d7,d2
	lsr.l	#3,d2			; d2 lightrightstep

; for (i=0 ; i<8 ; i++) {
	moveq	#7,d5

; lighttemp = lightleft - lightright;
; lightstep = lighttemp >> 3;
; light = lightright;
.1:	move.l	d6,d4
	sub.l	d7,d4
	lsr.l	#3,d4			; d4 lightstep
	move.l	d7,d3			; d3 light

; for (b=7; b>=0; b--) {
	moveq	#8,d0
	add.w	d0,a2
	add.w	d0,a3

; pix = psource[b];
; prowdest[b] = ((unsigned char *)vid.colormap)[(light & 0xFF00) + pix];
; light += lightstep;
	REPT	8
	move.w	d3,d0
	move.b	-(a2),d0
	move.b	(a4,d0.l),-(a3)
	add.l	d4,d3
	ENDR

; psource += sourcetstep;
; lightright += lightrightstep;
; lightleft += lightleftstep;
; prowdest += surfrowbytes;
	add.l	a5,a2
	add.l	a6,a3
	add.l	d1,d6
	add.l	d2,d7
	dbf	d5,.1

; if (psource >= r_sourcemax) psource -= r_stepback;
	cmp.l	_r_sourcemax,a2
	blo.b	.2
	sub.l	_r_stepback,a2

.2:	subq.l	#1,(sp)			; .v
	bne	.vloop
.quit:	movem.l	(sp)+,d1-d7/a2-a6
	rts


	xdef	_R_DrawSurfaceBlock8_mip2
	cnop	0,4
_R_DrawSurfaceBlock8_mip2:
	movem.l	d1-d7/a2-a6,-(sp)
	setso	0
.v	so.l	1			; saved d1-field is used as .v

	move.l	_vid+VID_COLORMAP,a4
	move.l	_sourcetstep,a5
	move.l	_surfrowbytes,a6

; psource = pbasesource;
; prowdest = prowdestbase;
	move.l	_pbasesource,a2		; a2 psource
	move.l	_prowdestbase,a3	; a3 prowdest

; for (v=0 ; v<r_numvblocks ; v++) {
	move.l	_r_numvblocks,(sp)	; .v
	beq	.quit

.vloop:
; lightleft = r_lightptr[0];
; lightright = r_lightptr[1];
	move.l	_r_lightptr,a0
	move.l	(a0)+,d6		; d6 lightleft
	move.l	(a0),d7			; d7 lightright

; r_lightptr += r_lightwidth;
; lightleftstep = (r_lightptr[0] - lightleft) >> 2;
; lightrightstep = (r_lightptr[1] - lightright) >> 2;
	move.l	_r_lightwidth,d0
	lea	-4(a0,d0.l*4),a0
	move.l	a0,_r_lightptr
	move.l	(a0)+,d1
	sub.l	d6,d1
	lsr.l	#2,d1			; d1 lightleftstep
	move.l	(a0),d2
	sub.l	d7,d2
	lsr.l	#2,d2			; d2 lightrightstep

; for (i=0 ; i<4 ; i++) {
	moveq	#3,d5

; lighttemp = lightleft - lightright;
; lightstep = lighttemp >> 3;
; light = lightright;
.1:	move.l	d6,d4
	sub.l	d7,d4
	lsr.l	#2,d4			; d4 lightstep
	move.l	d7,d3			; d3 light

; for (b=3; b>=0; b--) {
	moveq	#4,d0
	add.w	d0,a2
	add.w	d0,a3

; pix = psource[b];
; prowdest[b] = ((unsigned char *)vid.colormap)[(light & 0xFF00) + pix];
; light += lightstep;
	REPT	4
	move.w	d3,d0
	move.b	-(a2),d0
	move.b	(a4,d0.l),-(a3)
	add.l	d4,d3
	ENDR

; psource += sourcetstep;
; lightright += lightrightstep;
; lightleft += lightleftstep;
; prowdest += surfrowbytes;
	add.l	a5,a2
	add.l	a6,a3
	add.l	d1,d6
	add.l	d2,d7
	dbf	d5,.1

; if (psource >= r_sourcemax) psource -= r_stepback;
	cmp.l	_r_sourcemax,a2
	blo.b	.2
	sub.l	_r_stepback,a2

.2:	subq.l	#1,(sp)			; .v
	bne	.vloop
.quit:	movem.l	(sp)+,d1-d7/a2-a6
	rts


	xdef	_R_DrawSurfaceBlock8_mip3
	cnop	0,4
_R_DrawSurfaceBlock8_mip3:
	movem.l	d1-d7/a2-a6,-(sp)
	setso	0
.v	so.l	1			; saved d1-field is used as .v

	move.l	_vid+VID_COLORMAP,a4
	move.l	_sourcetstep,a5
	move.l	_surfrowbytes,a6

; psource = pbasesource;
; prowdest = prowdestbase;
	move.l	_pbasesource,a2		; a2 psource
	move.l	_prowdestbase,a3	; a3 prowdest

; for (v=0 ; v<r_numvblocks ; v++) {
	move.l	_r_numvblocks,(sp)	; .v
	beq	.quit

.vloop:
; lightleft = r_lightptr[0];
; lightright = r_lightptr[1];
	move.l	_r_lightptr,a0
	move.l	(a0)+,d6		; d6 lightleft
	move.l	(a0),d7			; d7 lightright

; r_lightptr += r_lightwidth;
; lightleftstep = (r_lightptr[0] - lightleft) >> 1;
; lightrightstep = (r_lightptr[1] - lightright) >> 1;
	move.l	_r_lightwidth,d0
	lea	-4(a0,d0.l*4),a0
	move.l	a0,_r_lightptr
	move.l	(a0)+,d1
	sub.l	d6,d1
	lsr.l	#1,d1			; d1 lightleftstep
	move.l	(a0),d2
	sub.l	d7,d2
	lsr.l	#1,d2			; d2 lightrightstep

; for (i=0 ; i<2 ; i++) {
	moveq	#1,d5

; lighttemp = lightleft - lightright;
; lightstep = lighttemp >> 1;
; light = lightright;
.1:	move.l	d6,d4
	sub.l	d7,d4
	lsr.l	#1,d4			; d4 lightstep
	move.l	d7,d3			; d3 light

; for (b=1; b>=0; b--) {
	moveq	#2,d0
	add.w	d0,a2
	add.w	d0,a3

; pix = psource[b];
; prowdest[b] = ((unsigned char *)vid.colormap)[(light & 0xFF00) + pix];
; light += lightstep;
	REPT	2
	move.w	d3,d0
	move.b	-(a2),d0
	move.b	(a4,d0.l),-(a3)
	add.l	d4,d3
	ENDR

; psource += sourcetstep;
; lightright += lightrightstep;
; lightleft += lightleftstep;
; prowdest += surfrowbytes;
	add.l	a5,a2
	add.l	a6,a3
	add.l	d1,d6
	add.l	d2,d7
	dbf	d5,.1

; if (psource >= r_sourcemax) psource -= r_stepback;
	cmp.l	_r_sourcemax,a2
	blo.b	.2
	sub.l	_r_stepback,a2

.2:	subq.l	#1,(sp)			; .v
	bne	.vloop
.quit:	movem.l	(sp)+,d1-d7/a2-a6
	rts
