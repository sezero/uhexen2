**
** Quake for AMIGA
** d_sky.c assembler implementations by Frank Wille <frank@phoenix.owl.de>
** Adapted for Hexen II by Szilard Biro
**

;		INCLUDE	"quakedef68k.i"

		XREF    _d_viewbuffer
		XREF    _screenwidth
		XREF    _r_skysource
		XREF    _vright
		XREF    _vpn
		XREF    _vup
		XREF    _skytime
		XREF    _skyspeed
		XREF    _xcenter
		XREF    _xscale
		XREF    _ycenter
		XREF    _yscale

		XDEF    _D_DrawSkyScans8

SKYSHIFT                =       7
SKYSIZE                 =       (1 << SKYSHIFT)
SKYMASK                 =       (SKYSIZE - 1)
SKY_SPAN_SHIFT          =       5
SKY_SPAN_MAX            =       (1 << SKY_SPAN_SHIFT)
R_SKY_SMASK             =       $007f
R_SKY_TMASK             =       $007f



******************************************************************************
*
*       void D_DrawSkyScans8 (espan_t *pspan)
*
*       standard scan drawing function for the sky
*
*       D_Sky_uv_To_st is inlined.
*
*       IMPORTANT!! SKY_SPAN_SHIFT must *NOT* exceed 5 (The ReciprocTable
*       has to be extended)
*
******************************************************************************

		cnop    0,4
_D_DrawSkyScans8


*****   stackframe

		rsreset
.savefp1        rs.x    1
.saved4         rs.l    1
.saved5         rs.l    1
.vr0            rs.s    1
.vr1            rs.s    1
.vr2            rs.s    1
.fpuregs        rs.x    6
.intregs        rs.l    11
		rs.l    1
.pspan          rs.l    1


******  Prologue. Global variables are put into registers or onto the stackframe

		movem.l d2-d7/a2-a6,-(sp)
		fmovem.x        fp2-fp7,-(sp)
		sub.l   #.fpuregs,sp

******  First loop. In every iteration one complete span is drawn


		move.l  .pspan(sp),a6           ;get function parameter

		fmove.s #1,fp1
		fdiv.s  _xscale,fp1             ;fp1 = 1 / xscale
		move.l  _r_skysource,a5
.loop
		fmove.x fp1,.savefp1(sp)
		move.l  _d_viewbuffer,a0
		move.l  _screenwidth,d0
		move.l  (a6)+,d3                ;d3 = pspan->u
		move.l  (a6)+,d2                ;d2 = pspan->v
		fmove.l d2,fp4                  ;fp4 = v
		muls    d2,d0                   ;d0 = screenwidth * pspan->v
		add.l   d3,d0
		add.l   d0,a0                   ;pdest = d_viewbuffer + pspan->u + d0
		lea     _vpn,a2
		fmove.s (a2)+,fp5               ;fp5 = vpn[0]
		fmove.s (a2)+,fp6               ;fp6 = vpn[1]
		fmove.s (a2)+,fp7               ;fp7 = vpn[2]
		;fmove.l _xcenter,d0
		fmove.s _xcenter,fp0
		fmove.l  fp0,d0
		move.l  d0,a2                   ;a2 = (int)xcenter
		fmove.s _ycenter,fp0
		fsub    fp4,fp0                 ;fp0 = (ycenter-v)
		fdiv.s  _yscale,fp0             ;fp0 = wv = (ycenter-v) / yscale
		lea     _vup,a1
		fmove.s (a1)+,fp2               ;fp2 = vup[0]
		fmul    fp0,fp2                 ;fp2 = wv*vup[0]
		fadd    fp2,fp5                 ;fp5 = vpn[0] + wv*vup[0]
		fmove.s (a1)+,fp3               ;fp3 = vup[1]
		fmul    fp0,fp3                 ;fp3 = wv*vup[1]
		fadd    fp3,fp6                 ;fp6 = vpn[1] + wv*vup[1]
		fmul.s  (a1)+,fp0               ;fp0 = wv*vup[2]
		fadd    fp0,fp7                 ;fp7 = vpn[2] + wv*vup[2]
		lea     _vright,a1
		fmove.s (a1)+,fp2               ;fp2 = vright[0]
		fmove.s (a1)+,fp3               ;fp3 = vright[1]
		fmove.s (a1)+,fp4               ;fp4 = vright[2]
		fmul    fp1,fp2                 ;fp2 = vright[0] / xscale
		fmul    fp1,fp3                 ;fp3 = vright[1] / xscale
		fmul    fp1,fp4                 ;fp4 = vright[2] / xscale
		fmove.s fp2,.vr0(sp)
		fmove.s fp3,.vr1(sp)
		fmove.s #3,fp1
		fmul    fp1,fp7                 ;fp7 = (vpn[2] + wv*vup[2]) * 3
		fmul    fp1,fp4                 ;fp4 = (vright[2] / xscale) * 3
		fmove.s fp4,.vr2(sp)
		move.l  (a6)+,d1                ;d1 = count = pspan->count
		fmove.s _skytime,fp0
		fmul.s  _skyspeed,fp0
		fmul.s  #65536,fp0
		fmove.l fp0,d0                  ;d0 = skytime*skyspeed*$10000
		move.l  d0,a4                   ;a4 = skytime*skyspeed*$10000

******  D_Sky_uv_To_st (inlined)

		move.l  d3,d0
		sub.l   a2,d0
		fmove.l d0,fp0                  ;fp0 = (float(u-(int)xcenter))
		fmove   fp0,fp2
		fmul.s  .vr0(sp),fp2            ;fp2 = wu*vright[0]
		fadd    fp5,fp2                 ;fp2 = end[0] = wu*vright[0] + vpn[0] + wv*vup[0]
		fmove   fp2,fp1                 ;fp1 = end[0]
		fmul    fp2,fp2                 ;fp2 = end[0]*end[0]
		fmove   fp0,fp3                 ;fp3 = u - xcenter
		fmul.s  .vr1(sp),fp3            ;fp3 = wu*vright[1]
		fadd    fp6,fp3                 ;fp3 = end[1] = wu*vright[1] + vpn[1] + wv*vup[1]
		fmove   fp3,fp4                 ;fp4 = end[1]
		fmul    fp3,fp3                 ;fp3 = end[1]*end[1]
		fadd    fp3,fp2                 ;fp2 = end[0]*end[0] + end[1]*end[1]
		fmul.s  .vr2(sp),fp0            ;fp0 = wu*vright[2] * 3
		fadd    fp7,fp0                 ;fp0 = end[2] = wu*vright[2] * 3 + (vpn[2] + wv*vup[2]) * 3
		fmul    fp0,fp0                 ;fp2 = end[2] * end[2]
		fadd    fp0,fp2                 ;fp2 = end[0]*end[0] + end[1]*end[1] + end[2] * end[2]
		;fsqrt   fp2                     ;fp2 = length(end)
; fast inverse square root
		fmove.s fp2,d0                  ;d0 = -(t.i >> 1)
		lsr.l   #1,d0
		neg.l   d0
		add.l   #1597463007,d0          ;d0 = d0 + 0x5f3759df
		fmul.s  #0.5,fp2                ;fp2 = fp2 * 0.5f
		fmul.s  d0,fp2                  ;fp2 = fp2 * t.f * t.f
		fmul.s  d0,fp2
		fneg.x  fp2                     ;fp2 = 1.5f - fp2
		fadd.s  #1.5,fp2
		fmul.s  d0,fp2                  ;fp2 = fp2 * t.f = 1/length(end)
; fast inverse square root
		fmove.s #(65536*6*(SKYSIZE/2-1)),fp0
		;fdiv    fp2,fp0                 ;6*(SKYSIZE/2-1) * 0x10000 / length(end)
		fmul    fp2,fp0                 ;6*(SKYSIZE/2-1) * 0x10000 * (1/length(end))
		fmul    fp0,fp1                 ;fp1 = 6*(SKYSIZE/2-1)*end[0]
		fmul    fp0,fp4                 ;fp4 = 6*(SKYSIZE/2-1)*end[1]
		fmove.l fp1,d6
		add.l   a4,d6                   ;d6 = s
		fmove.l fp4,d7
		add.l   a4,d7                   ;d7 = t

******  end of D_Sky_uv_To_st

******  Second loop. In every iteration one part of the whole span is drawn
******  d2 gets the value (spancount-1)! [NOT spancount]

******  d1 = count

*                do
*                {
*                        if (count >= SKY_SPAN_MAX)
*                                spancount = SKY_SPAN_MAX;
*                        else
*                                spancount = count;
*
*                        count -= spancount;
*
*                        if (count)
*                        {

.loop2
		move.l  #SKY_SPAN_MAX-1,d2      ;spancount = SKY_SPAN_MAX
		cmp.l   #SKY_SPAN_MAX,d1        ;if (count >= SKY_SPAN_MAX)
		bgt.b   .cont
		move.l  d1,d2                   ;spancount = count
		subq.l  #1,d2
		moveq   #0,d1                   ;count -= spancount
		bra.w   .finalpart
.cont
		sub.l   #SKY_SPAN_MAX,d1        ;count -= spancount;


******  Evaluation of the values for the inner loop. This version is used for
******  span size = SKY_SPAN_MAX

*                        // calculate s and t at far end of span,
*                        // calculate s and t steps across span by shifting
*                                u += spancount;
*
*                                D_Sky_uv_To_st (u, v, &snext, &tnext);
*
*                                sstep = (snext - s) >> SKY_SPAN_SHIFT;
*                                tstep = (tnext - t) >> SKY_SPAN_SHIFT;
*                        }


		add.l   d2,d3
		addq.l  #1,d3

******  D_Sky_uv_To_st (inlined)

		move.l  d3,d0
		sub.l   a2,d0
		fmove.l d0,fp0                  ;fp0 = (float(u-(int)xcenter))
		fmove   fp0,fp2                 ;fp2 = u - xcenter
		fmul.s  .vr0(sp),fp2            ;fp2 = wu*vright[0]
		fadd    fp5,fp2                 ;fp2 = end[0]
		fmove   fp2,fp1
		fmul    fp2,fp2
		fmove   fp0,fp3
		fmul.s  .vr1(sp),fp3            ;fp3 = wu*vright[1]
		fadd    fp6,fp3                 ;fp3 = end[1]
		fmove   fp3,fp4
		fmul    fp3,fp3
		fadd    fp3,fp2
		fmul.s  .vr2(sp),fp0            ;fp0 = wu*vright[2]
		fadd    fp7,fp0                 ;fp0 = end[2]
		fmul    fp0,fp0
		fadd    fp0,fp2
		;fsqrt   fp2                     ;fp2 = length(end)
; fast inverse square root
		fmove.s fp2,d0                  ;d0 = -(t.i >> 1)
		lsr.l   #1,d0
		neg.l   d0
		add.l   #1597463007,d0          ;d0 = d0 + 0x5f3759df
		fmul.s  #0.5,fp2                ;fp2 = fp2 * 0.5f
		fmul.s  d0,fp2                  ;fp2 = fp2 * t.f * t.f
		fmul.s  d0,fp2
		fneg.x  fp2                     ;fp2 = 1.5f - fp2
		fadd.s  #1.5,fp2
		fmul.s  d0,fp2                  ;fp2 = fp2 * t.f = 1/length(end)
; fast inverse square root
		fmove.s #(65536*6*(SKYSIZE/2-1)),fp0
		;fdiv    fp2,fp0
		fmul    fp2,fp0
		fmul    fp0,fp1                 ;fp1 = 6*(SKYSIZE/2-1)*end[0]
		fmul    fp0,fp4                 ;fp4 = 6*(SKYSIZE/2-1)*end[1]
		fmove.l fp1,d4
		add.l   a4,d4                   ;d6 = snext
		fmove.l fp4,d5
		add.l   a4,d5                   ;d7 = tnext

******  end of D_Sky_uv_To_st

		move.l  d4,.saved4(sp)          ;save snext
		move.l  d5,.saved5(sp)          ;save tnext
		sub.l   d6,d4                   ;d4 = snext - s
		sub.l   d7,d5                   ;d5 = tnext - t
		asr.l   #SKY_SPAN_SHIFT,d4      ;sstep = d4 >> SKY_SPAN_SHIFT
		asr.l   #SKY_SPAN_SHIFT,d5      ;tstep = d5 >> SKY_SPAN_SHIFT
		bra.w   .mainloop


.finalpart
		add.l   d2,d3

******  D_Sky_uv_To_st (inlined)

		move.l  d3,d0
		sub.l   a2,d0
		fmove.l d0,fp0                  ;fp0 = (float(u-(int)xcenter))
		fmove   fp0,fp2                 ;fp2 = u - xcenter
		fmul.s  .vr0(sp),fp2            ;fp2 = wu*vright[0]
		fadd    fp5,fp2                 ;fp2 = end[0]
		fmove   fp2,fp1
		fmul    fp2,fp2
		fmove   fp0,fp3
		fmul.s  .vr1(sp),fp3            ;fp3 = wu*vright[1]
		fadd    fp6,fp3                 ;fp3 = end[1]
		fmove   fp3,fp4
		fmul    fp3,fp3
		fadd    fp3,fp2
		fmul.s  .vr2(sp),fp0            ;fp0 = wu*vright[2]
		fadd    fp7,fp0                 ;fp0 = end[2]
		fmul    fp0,fp0
		fadd    fp0,fp2
		;fsqrt   fp2                     ;fp2 = length(end)
; fast inverse square root
		fmove.s fp2,d0                  ;d0 = -(t.i >> 1)
		lsr.l   #1,d0
		neg.l   d0
		add.l   #1597463007,d0          ;d0 = d0 + 0x5f3759df
		fmul.s  #0.5,fp2                ;fp2 = fp2 * 0.5f
		fmul.s  d0,fp2                  ;fp2 = fp2 * t.f * t.f
		fmul.s  d0,fp2
		fneg.x  fp2                     ;fp2 = 1.5f - fp2
		fadd.s  #1.5,fp2
		fmul.s  d0,fp2                  ;fp2 = fp2 * t.f = 1/length(end)
; fast inverse square root
		fmove.s #(65536*6*(SKYSIZE/2-1)),fp0
		;fdiv    fp2,fp0
		fmul    fp2,fp0
		fmul    fp0,fp1                 ;fp1 = 6*(SKYSIZE/2-1)*end[0]
		fmul    fp0,fp4                 ;fp4 = 6*(SKYSIZE/2-1)*end[1]
		fmove.l fp1,d4
		add.l   a4,d4                   ;d6 = snext
		fmove.l fp4,d5
		add.l   a4,d5                   ;d7 = tnext

******  end of D_Sky_uv_To_st

		move.l  d4,.saved4(sp)          ;save snext
		move.l  d5,.saved5(sp)          ;save tnext
		sub.l   d6,d4                   ;d4 = snext - s
		sub.l   d7,d5                   ;d5 = tnext - t
		cmp     #5,d2                   ;(spancount-1) < 5?
		blt.b   .special                ;yes -> special case
.qdiv
		asr.l   #7,d4                   ;d4 >> 7
		asr.l   #7,d5                   ;d5 >> 7
		lea     ReciprocTable,a3        ;a3 -> reciprocal table
		move    0(a3,d2.w*2),d0         ;d0 = (1/(spancount-1))<<16
		muls    d0,d4                   ;d4 = d4 / (spancount-1)
		asr.l   #7,d4                   ;sstep = d4 >> 7
		muls    d0,d5                   ;d5 = d5 / (spancount-1)
		asr.l   #7,d5                   ;tstep = d5 >> 7
		bra.b   .mainloop
.special
		cmp     #1,d2                   ;switch (spancount-1)
		ble.b   .mainloop               ;0,1 -> no scaling needed
		cmp     #3,d2                   ;3 -> standard qdiv
		beq.b   .qdiv
		blt.b   .spec_2
		asr.l   #2,d4                   ;4 -> scale by shifting right
		asr.l   #2,d5
		bra.b   .mainloop
.spec_2
		asr.l   #1,d4                   ;2 -> scale by shifting right
		asr.l   #1,d5


******  Main drawing loop.

******  d2 : spancount
******  d4 : sstep
******  d5 : tstep
******  d6 : s
******  d7 : t
******  a0 : pdest
******  a5 : r_skysource

*                        do
*                        {
*                                *pdest++ = r_skysource[((t & R_SKY_TMASK) >> 8) +
*                                                ((s & R_SKY_SMASK) >> 16)];
*                                s += sstep;
*                                t += tstep;
*                        } while (--spancount > 0);


.mainloop
		move.l  d1,-(sp)
		swap    d4
		swap    d5
		swap    d6
		swap    d7
		move    d5,d1                   ;d2 = tstep integer part
		move    d4,d0                   ;d0 = sstep integer part
		clr     d5                      ;d5 = tstep fractional part
		clr     d4                      ;d4 = sstep fractional part
.loop3
		and     #R_SKY_TMASK,d7
		asl     #8,d7
		lea     0(a5,d7.w),a3
		asr     #8,d7
		and     #R_SKY_SMASK,d6
		move.b  0(a3,d6.w),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d5,d7
		addx.w  d1,d7
		dbra    d2,.loop3
		move.l  (sp)+,d1

******  loop terminations


		move.l   .saved5(sp),d7         ;t = tnext
		move.l   .saved4(sp),d6         ;s = snext

		tst.l   d1                      ;while (count > 0)
		bgt.w   .loop2
		fmove.x .savefp1(sp),fp1

		move.l  (a6)+,a6
		tst.l   a6
		bne.w   .loop
		add.l   #.fpuregs,sp
		fmovem.x        (sp)+,fp2-fp7
		movem.l (sp)+,d2-d7/a2-a6
		rts

ReciprocTable
		dc.w    0
		dc.w    0
		dc.w    0
		dc.w    16384/3
		dc.w    0
		dc.w    16384/5
		dc.w    16384/6
		dc.w    16384/7
		dc.w    16384/8
		dc.w    16384/9
		dc.w    16384/10
		dc.w    16384/11
		dc.w    16384/12
		dc.w    16384/13
		dc.w    16384/14
		dc.w    16384/15
		dc.w    16384/16
		dc.w    16384/17
		dc.w    16384/18
		dc.w    16384/19
		dc.w    16384/20
		dc.w    16384/21
		dc.w    16384/22
		dc.w    16384/23
		dc.w    16384/24
		dc.w    16384/25
		dc.w    16384/26
		dc.w    16384/27
		dc.w    16384/28
		dc.w    16384/29
		dc.w    16384/30
		dc.w    16384/31

