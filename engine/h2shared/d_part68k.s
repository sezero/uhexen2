**
** Quake for AMIGA
** d_part.c assembler implementations by Frank Wille <frank@phoenix.owl.de>
** Transparent version for Hexen II by Szilard Biro
**

;		INCLUDE	"quakedef68k.i"
PART_COLOR	equ	12

		XREF    _r_origin
		XREF    _r_pright
		XREF    _r_pup
		XREF    _r_ppn
		XREF    _xcenter
		XREF    _ycenter
		XREF    _d_vrectbottom_particle
		XREF    _d_vrectright_particle
		XREF    _d_vrecty
		XREF    _d_vrectx
		XREF    _d_pzbuffer
		XREF    _d_zwidth
		XREF    _d_viewbuffer
		XREF    _d_scantable
		XREF    _d_y_aspect_shift
		XREF    _d_pix_min
		XREF    _d_pix_max
		XREF    _d_pix_shift
		XREF    _screenwidth
		XREF    _transTable;

		XDEF    _D_DrawParticle

PARTICLE_Z_CLIP         equ.s   8.0             ;must match the values in d_iface.h!


******************************************************************************
*
*       void _D_DrawParticle (particle_t *pparticle)
*
******************************************************************************

		cnop    0,4
_D_DrawParticle

		rsreset
.fpuregs        rs.x    4
.regs           rs.l    9
		rs.l    1
.pparticle      rs.l    1

		movem.l d2-d7/a2-a4,-(sp)
		fmovem.x        fp2-fp5,-(sp)
		move.l  .pparticle(sp),a2
		lea     _r_origin,a0
		fmove.s (a2)+,fp0
		fsub.s  (a0)+,fp0               ;fp0 = local[0]
		fmove.s (a2)+,fp1
		fsub.s  (a0)+,fp1               ;fp1 = local[1]
		fmove.s (a2)+,fp2
		fsub.s  (a0)+,fp2               ;fp2 = local[2]
		lea     -12(a2),a2
		lea     _r_pright,a1
		fmove.s (a1)+,fp3
		fmul    fp0,fp3
		fmove.s (a1)+,fp4
		fmul    fp1,fp4
		fadd    fp4,fp3
		fmove.s (a1)+,fp4
		fmul    fp2,fp4
		fadd    fp4,fp3                 ;fp3 = transformed[0]
		lea     _r_pup,a1
		fmove.s (a1)+,fp4
		fmul    fp0,fp4
		fmove.s (a1)+,fp5
		fmul    fp1,fp5
		fadd    fp5,fp4
		fmove.s (a1)+,fp5
		fmul    fp2,fp5
		fadd    fp5,fp4                 ;fp4 = transformed[1]
		lea     _r_ppn,a1
		fmul.s  (a1)+,fp0
		fmul.s  (a1)+,fp1
		fadd    fp0,fp1
		fmul.s  (a1)+,fp2
		fadd    fp1,fp2                 ;fp2 = transformed[2]
		fcmp.s  #PARTICLE_Z_CLIP,fp2
		fblt.w  .exit
		fmove.s #1,fp0
		fdiv    fp2,fp0                 ;zi = 1.0 / transformed[2]
		fmove.s #0.5,fp1
		fmul    fp0,fp3
		fmul    fp0,fp4
		fadd    fp1,fp3
		fadd    fp1,fp4
		fadd.s  _xcenter,fp3
		fmove.l fp3,d0                  ;d0 = u
		fsub.s  _ycenter,fp4
		fneg    fp4
		fmove.l fp4,d1                  ;d1 = v
		cmp.l   _d_vrectbottom_particle,d1
		bgt.w   .exit
		cmp.l   _d_vrectright_particle,d0
		bgt.w   .exit
		cmp.l   _d_vrecty,d1
		blt.w   .exit
		cmp.l   _d_vrectx,d0
		blt.w   .exit
		move.l  _d_pzbuffer,a0
		lea     _d_scantable,a1
		move.l  0(a1,d1.l*4),a1
		move.l  _d_zwidth,d2            ;d2 = d_zwidth
		muls    d2,d1
		add.l   d0,d1
		lea     0(a0,d1.l*2),a0         ;a0 = pz
		add.l   d0,a1
		add.l   _d_viewbuffer,a1        ;a1 = pdest
		fmul.s  #32768,fp0
		fmove.l fp0,d0                  ;izi = (int)(zi * 0x8000)
		move.l  d0,d5                   ;d5 = izi
		move.l  _d_pix_shift,d1
		asr.l   d1,d0                   ;d0 = pix
		move.l  _d_pix_min,d1
		cmp.l   d1,d0
		bgt.b   .cont
		move.l  d1,d0
.cont
		move.l  _d_pix_max,d1
		cmp.l   d1,d0
		blt.b   .cont2
		move.l  d1,d0
.cont2
		fmove.s PART_COLOR(a2),fp0
		fmove.l fp0,d4
		move.l  _screenwidth,d1
		move.l  _d_y_aspect_shift,d3
		and.l   #511,d4            ; clamp color to 0-511
		cmp.l   #255,d4
		bgt.w   .is_trans
		cmp.l   #4,d0                   ;switch (pix)
		bgt.b   .more
		beq.b   .four
		cmp.l   #2,d0
		bgt.b   .three
		beq.b   .two
.one                                            ;case 1
		lsl     d3,d0
		subq    #1,d0
.one_loop
		cmp     (a0),d5                 ;if (pz[0] <= izi)
		blt.b   .one_next
		move    d5,(a0)                 ;pz[0] = izi
		move.b  d4,(a1)                 ;pdest[0] = pparticle->color
.one_next
		add.l   d1,a1                   ;pdest += screenwidth
		lea     0(a0,d2.l*2),a0         ;pz += d_zwidth
		dbra    d0,.one_loop            ;count--
		bra.w   .exit
.two                                            ;case 2
		lsl     d3,d0
		subq    #1,d0
.two_loop
		cmp     (a0),d5                 ;if (pz[0] <= izi)
		blt.b   .two_2
		move    d5,(a0)                 ;pz[0] = izi
		move.b  d4,(a1)                 ;pdest[0] = pparticle->color
.two_2
		cmp     2(a0),d5                ;if (pz[1] <= izi)
		blt.b   .two_next
		move    d5,2(a0)                ;pz[1] = izi
		move.b  d4,1(a1)                ;pdest[1] = pparticle->color
.two_next
		add.l   d1,a1                   ;pdest += screenwidth
		lea     0(a0,d2.l*2),a0         ;pz += d_zwidth
		dbra    d0,.two_loop            ;count--
		bra.w   .exit
.three                                          ;case3
		lsl     d3,d0
		subq    #1,d0
.three_loop
		cmp     (a0),d5                 ;if (pz[0] <= izi)
		blt.b   .three_2
		move    d5,(a0)                 ;pz[0] = izi
		move.b  d4,(a1)                 ;pdest[0] = pparticle->color
.three_2
		cmp     2(a0),d5                ;if (pz[1] <= izi)
		blt.b   .three_3
		move    d5,2(a0)                ;pz[1] = izi
		move.b  d4,1(a1)                ;pdest[1] = pparticle->color
.three_3
		cmp     4(a0),d5                ;if (pz[2] <= izi)
		blt.b   .three_next
		move    d5,4(a0)                ;pz[2] = izi
		move.b  d4,2(a1)                ;pdest[2] = pparticle->color
.three_next
		add.l   d1,a1                   ;pdest += screenwidth
		lea     0(a0,d2.l*2),a0         ;pz += d_zwidth
		dbra    d0,.three_loop          ;count--
		bra.b   .exit
.four                                           ;case4
		lsl     d3,d0
		subq    #1,d0
.four_loop
		cmp     (a0),d5                 ;if (pz[0] <= izi)
		blt.b   .four_2
		move    d5,(a0)                 ;pz[0] = izi
		move.b  d4,(a1)                 ;pdest[0] = pparticle->color
.four_2
		cmp     2(a0),d5                ;if (pz[1] <= izi)
		blt.b   .four_3
		move    d5,2(a0)                ;pz[1] = izi
		move.b  d4,1(a1)                ;pdest[1] = pparticle->color
.four_3
		cmp     4(a0),d5                ;if (pz[2] <= izi)
		blt.b   .four_4
		move    d5,4(a0)                ;pz[2] = izi
		move.b  d4,2(a1)                ;pdest[2] = pparticle->color
.four_4
		cmp     6(a0),d5                ;if (pz[3] <= izi)
		blt.b   .four_next
		move    d5,6(a0)                ;pz[3] = izi
		move.b  d4,3(a1)                ;pdest[3] = pparticle->color
.four_next
		add.l   d1,a1                   ;pdest += screenwidth
		lea     0(a0,d2.l*2),a0         ;pz += d_zwidth
		dbra    d0,.four_loop           ;count--
		bra.b   .exit
.more
		move    d0,d6
		subq    #1,d6
		lsl     d3,d0
		subq    #1,d0
.more_loop
		move.l  a0,a2
		move.l  a1,a3
		move    d6,d7
.more_loop2
		addq.l  #1,a3
		cmp     (a2)+,d5                ;if (pz[i] <= izi)
		blt.b   .more_next
		move    d5,-2(a2)               ;pz[i] = izi
		move.b  d4,-1(a3)               ;pdest[i] = pparticle->color
.more_next
		dbra    d7,.more_loop2
		add.l   d1,a1                   ;pdest += screenwidth
		lea     0(a0,d2.l*2),a0         ;pz += d_zwidth
		dbra    d0,.more_loop
.is_trans
		add.w   #-256,d4
		lsl.w   #8,d4                   ; << 8 for transTable idx
		move.l  _transTable,a4
		cmp.l   #4,d0                   ;switch (pix)
		bgt.b   .t_more
		beq.b   .t_four
		cmp.l   #2,d0
		bgt.b   .t_three
		beq.b   .t_two
.t_one                                            ;case 1
		lsl     d3,d0
		subq    #1,d0
.t_one_loop
		cmp     (a0),d5                 ;if (pz[0] <= izi)
		blt.b   .t_one_next
		move    d5,(a0)                 ;pz[0] = izi
		move.b  (a1),d4
		move.b  (a4,d4.l),(a1)          ;pdest[0] = transTable[pparticle->color + pdest[0]];
.t_one_next
		add.l   d1,a1                   ;pdest += screenwidth
		lea     0(a0,d2.l*2),a0         ;pz += d_zwidth
		dbra    d0,.t_one_loop            ;count--
		bra.w   .exit
.t_two                                            ;case 2
		lsl     d3,d0
		subq    #1,d0
.t_two_loop
		cmp     (a0),d5                 ;if (pz[0] <= izi)
		blt.b   .t_two_2
		move    d5,(a0)                 ;pz[0] = izi
		move.b  (a1),d4
		move.b  (a4,d4.l),(a1)          ;pdest[0] = transTable[pparticle->color + pdest[0]];
.t_two_2
		cmp     2(a0),d5                ;if (pz[1] <= izi)
		blt.b   .t_two_next
		move    d5,2(a0)                ;pz[1] = izi
		move.b  1(a1),d4
		move.b  (a4,d4.l),1(a1)          ;pdest[1] = transTable[pparticle->color + pdest[1]];
.t_two_next
		add.l   d1,a1                   ;pdest += screenwidth
		lea     0(a0,d2.l*2),a0         ;pz += d_zwidth
		dbra    d0,.t_two_loop            ;count--
		bra.w   .exit
.t_three                                          ;case3
		lsl     d3,d0
		subq    #1,d0
.t_three_loop
		cmp     (a0),d5                 ;if (pz[0] <= izi)
		blt.b   .t_three_2
		move    d5,(a0)                 ;pz[0] = izi
		move.b  (a1),d4
		move.b  (a4,d4.l),(a1)          ;pdest[0] = transTable[pparticle->color + pdest[0]];
.t_three_2
		cmp     2(a0),d5                ;if (pz[1] <= izi)
		blt.b   .t_three_3
		move    d5,2(a0)                ;pz[1] = izi
		move.b  1(a1),d4
		move.b  (a4,d4.l),1(a1)          ;pdest[1] = transTable[pparticle->color + pdest[1]];
.t_three_3
		cmp     4(a0),d5                ;if (pz[2] <= izi)
		blt.b   .t_three_next
		move    d5,4(a0)                ;pz[2] = izi
		move.b  2(a1),d4
		move.b  (a4,d4.l),2(a1)          ;pdest[2] = transTable[pparticle->color + pdest[2]];
.t_three_next
		add.l   d1,a1                   ;pdest += screenwidth
		lea     0(a0,d2.l*2),a0         ;pz += d_zwidth
		dbra    d0,.t_three_loop          ;count--
		bra.b   .exit
.t_four                                           ;case4
		lsl     d3,d0
		subq    #1,d0
.t_four_loop
		cmp     (a0),d5                 ;if (pz[0] <= izi)
		blt.b   .t_four_2
		move    d5,(a0)                 ;pz[0] = izi
		move.b  (a1),d4
		move.b  (a4,d4.l),(a1)          ;pdest[0] = transTable[pparticle->color + pdest[0]];
.t_four_2
		cmp     2(a0),d5                ;if (pz[1] <= izi)
		blt.b   .t_four_3
		move    d5,2(a0)                ;pz[1] = izi
		move.b  1(a1),d4
		move.b  (a4,d4.l),1(a1)          ;pdest[1] = transTable[pparticle->color + pdest[1]];
.t_four_3
		cmp     4(a0),d5                ;if (pz[2] <= izi)
		blt.b   .t_four_4
		move    d5,4(a0)                ;pz[2] = izi
		move.b  2(a1),d4
		move.b  (a4,d4.l),2(a1)          ;pdest[2] = transTable[pparticle->color + pdest[2]];
.t_four_4
		cmp     6(a0),d5                ;if (pz[3] <= izi)
		blt.b   .t_four_next
		move    d5,6(a0)                ;pz[3] = izi
		move.b  3(a1),d4
		move.b  (a4,d4.l),3(a1)          ;pdest[3] = transTable[pparticle->color + pdest[3]];
.t_four_next
		add.l   d1,a1                   ;pdest += screenwidth
		lea     0(a0,d2.l*2),a0         ;pz += d_zwidth
		dbra    d0,.t_four_loop           ;count--
		bra.b   .exit
.t_more
		move    d0,d6
		subq    #1,d6
		lsl     d3,d0
		subq    #1,d0
.t_more_loop
		move.l  a0,a2
		move.l  a1,a3
		move    d6,d7
.t_more_loop2
		addq.l  #1,a3
		cmp     (a2)+,d5                ;if (pz[i] <= izi)
		blt.b   .t_more_next
		move    d5,-2(a2)               ;pz[i] = izi
		move.b  -1(a3),d4
		move.b  (a4,d4.l),-1(a3)          ;pdest[i] = transTable[pparticle->color + pdest[i]];
.t_more_next
		dbra    d7,.t_more_loop2
		add.l   d1,a1                   ;pdest += screenwidth
		lea     0(a0,d2.l*2),a0         ;pz += d_zwidth
		dbra    d0,.t_more_loop
.exit
		fmovem.x        (sp)+,fp2-fp5
		movem.l (sp)+,d2-d7/a2-a4
		rts

