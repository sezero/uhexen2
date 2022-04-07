**
** Quake for AMIGA
** d_scan.c assembler implementations by Frank Wille <frank@phoenix.owl.de>
** Translucent version for Hexen II by Szilard Biro
**

;		INCLUDE	"quakedef68k.i"
SPAN_PNEXT	equ	12

		XREF    _cacheblock
		XREF    _d_sdivzorigin
		XREF    _d_sdivzstepu
		XREF    _d_sdivzstepv
		XREF    _d_tdivzorigin
		XREF    _d_tdivzstepu
		XREF    _d_tdivzstepv
		XREF    _d_ziorigin
		XREF    _d_zistepu
		XREF    _d_zistepv
		XREF    _sadjust
		XREF    _tadjust
		XREF    _bbextents
		XREF    _bbextentt
		XREF    _d_viewbuffer
		XREF    _screenwidth
		XREF    _cachewidth
		XREF    _d_zwidth
		XREF    _d_pzbuffer
		;XREF    _sintable
		XREF    _r_turb_turb
		XREF    _mainTransTable
		XREF    _scanList
		XREF    _ZScanCount

		XDEF    _D_DrawTurbulent8
		XDEF    _D_DrawTurbulent8T
		XDEF    _D_DrawSpans8
		XDEF	_D_DrawSpans16
		XDEF	_D_DrawSpans16T
		XDEF    _D_DrawZSpans

QDIV                    =       1
NICE_DIV                =       1

CYCLE                   =       128             ;MUST match the #define in d_iface.h!
AMP2                    =       3               ;--
SPEED                   =       20              ;--
SCAN_SIZE               =       2048            ;MUST match the #define in d_local.h!



******************************************************************************
*
*       void D_DrawTurbulent8 (espan_t *pspan)
*
*       standard scan drawing function for animated textures
*       Note: The function D_DrawTurbulent8Span was inlined into this
*       function, because it's never used anywhere else.
*       This function expects r_turb_turb to be set by Turbulent8.
*
******************************************************************************

		cnop    0,4
_D_DrawTurbulent8


*****   stackframe

		rsreset
.saved4         rs.l    1
.saved5         rs.l    1
.savea1         rs.l    1
.szstpu         rs.s    1
.szstpv         rs.s    1
.szorg          rs.s    1
.tzstpu         rs.s    1
.tzstpv         rs.s    1
.tzorg          rs.s    1
.zistpu         rs.s    1
.zistpv         rs.s    1
.ziorg          rs.s    1
.fpuregs        rs.x    6
.intregs        rs.l    11
		rs.l    1
.pspan          rs.l    1


******  Prologue. Global variables are put into registers or onto the stackframe

		movem.l d2-d7/a2-a6,-(sp)
		fmovem.x        fp2-fp7,-(sp)
		move.l  _bbextentt,a2
		move.l  _tadjust,a3
		move.l  _bbextents,a4
		move.l  _sadjust,a5
		move.l  _d_ziorigin,-(sp)
		move.l  _d_zistepv,-(sp)
		move.l  _d_zistepu,-(sp)
		move.l  _d_tdivzorigin,-(sp)
		move.l  _d_tdivzstepv,-(sp)
		move.l  _d_tdivzstepu,-(sp)
		move.l  _d_sdivzorigin,-(sp)
		move.l  _d_sdivzstepv,-(sp)
		move.l  _d_sdivzstepu,-(sp)
		sub.l   #.szstpu,sp

******  First loop. In every iteration one complete span is drawn

*        r_turb_turb = sintable + ((int)(cl.time*SPEED)&(CYCLE-1));
*
*        r_turb_pbase = (unsigned char *)cacheblock;
*
*        sdivz16stepu = d_sdivzstepu * 16;
*        tdivz16stepu = d_tdivzstepu * 16;
*        zi16stepu = d_zistepu * 16;
*
*        do
*        {
*                r_turb_pdest = (unsigned char *)((byte *)d_viewbuffer +
*                                (screenwidth * pspan->v) + pspan->u);
*
*                count = pspan->count;
*
*        // calculate the initial s/z, t/z, 1/z, s, and t and clamp
*                du = (float)pspan->u;
*                dv = (float)pspan->v;
*
*                sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
*                tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
*                zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
*                z = (float)0x10000 / zi;        // prescale to 16.16 fixed-point
*

		;fmove.d _cl+CL_TIME,fp0         ;get cl.time
		;fmul.s  #SPEED,fp0              ;fp0 = cl.time*SPEED
		;fmove.l fp0,d0                  ;(int)(cl.time*SPEED)
		;and.l   #CYCLE-1,d0             ;(int)(cl.time*SPEED)&(CYCLE-1)
		;lsl.l   #2,d0
		;add.l   #_sintable,d0           ;r_turb_turb = _sintable + 4*d0
		;move.l  d0,a6
		move.l  _r_turb_turb,a6
		fmove.s #16,fp7
		fmove.s .szstpu(sp),fp3
		fmul    fp7,fp3                 ;sdivz16stepu = d_sdivzstepu * 16
		fmove.s .tzstpu(sp),fp4
		fmul    fp7,fp4                 ;tdivz16stepu = d_tdivzstepu * 16
		fmove.s .zistpu(sp),fp5
		fmul    fp7,fp5                 ;zi16stepu = d_zistepu * 16
		move.l  .pspan(sp),a1           ;get function parameter
.loop
		move.l  a1,.savea1(sp)          ;save actual ptr to pspan
		move.l  _d_viewbuffer,a0
		move.l  _screenwidth,d0
		move.l  (a1)+,d1
		fmove.l d1,fp2                  ;du = (float)pspan->u
		move.l  (a1)+,d2
		fmove.l d2,fp7                  ;dv = (float)pspan->v
		move.l  (a1)+,d4
		muls    d2,d0                   ;d0 = screenwidth * pspan->v
		add.l   d1,d0
		add.l   d0,a0                   ;pdest = d_viewbuffer + pspan->u + d0
		lea     .szstpu(sp),a1          ;a1 -> stackframe
		fmove.s (a1)+,fp0
		fmul    fp2,fp0                 ;fp0 = du * d_sdivzstepu
		fmove.s (a1)+,fp1
		fmul    fp7,fp1                 ;fp1 = dv * d_sdivzstepv
		fadd    fp1,fp0
		fadd.s  (a1)+,fp0               ;sdivz = d_sdivzorigin + fp0 + fp1
		fmove.s (a1)+,fp1
		fmul    fp2,fp1                 ;fp1 = du * d_tdivzstepu
		fmove.s (a1)+,fp6
		fmul    fp7,fp6                 ;fp6 = dv * d_tdivzstepv
		fadd    fp6,fp1
		fadd.s  (a1)+,fp1               ;tdivz = d_tdivzorigin + fp1 + fp6
		fmul.s  (a1)+,fp2               ;fp2 = du * d_zistepu
		fmul.s  (a1)+,fp7               ;fp7 = dv * d_zistepv
		fadd    fp7,fp2
		fadd.s  (a1)+,fp2               ;zi = d_ziorigin + fp2 + fp7
		fmove.s #65536,fp6
		fdiv    fp2,fp6                 ;z = (float)0x10000 / zi

*                s = (int)(sdivz * z) + sadjust;
*                if (s > bbextents)
*                        s = bbextents;
*                else if (s < 0)
*                        s = 0;
*
*                t = (int)(tdivz * z) + tadjust;
*                if (t > bbextentt)
*                        t = bbextentt;
*                else if (t < 0)
*                        t = 0;


		fmove   fp6,fp7
		fmul    fp0,fp7                 ;fp7 = sdivz * z
		fmove.l fp7,d6                  ;convert to integer
		add.l   a5,d6                   ;s = d6 + sadjust
		cmp.l   a4,d6                   ;if (s > bbextents)
		bgt.b   .down
		tst.l   d6                      ;if (s < 0)
		bge.b   .keep
.up
		moveq   #0,d6                   ;s = 0
		bra.b   .keep
.down
		move.l  a4,d6                   ;s = bbextents
.keep
		fmul    fp1,fp6                 ;fp6 = tdivz * z
		fmove.l fp6,d7                  ;convert to integer
		add.l   a3,d7                   ;t = d7 + tadjust
		cmp.l   a2,d7                   ;if (t > bbextentt)
		bgt.b   .down2
		tst.l   d7                      ;if (t < 0)
		bge.b   .keep2
.up2
		moveq   #0,d7                   ;t = 0
		bra.b   .keep2
.down2
		move.l  a2,d7                   ;t = bbextentt
.keep2
		move.l  d4,d1


******  Second loop. In every iteration one part of the whole span is drawn
******  d2 gets the value (spancount-1)! [NOT spancount]

******  d1 = count

*                do
*                {
*                // calculate s and t at the far end of the span
*                        if (count >= 16)
*                                spancount = 16;
*                        else
*                                spancount = count;
*
*                        count -= spancount;
*
*                        if (count)
*                        {

.loop2
		moveq   #16-1,d2                ;spancount = 16
		cmp.l   #16,d1                  ;if (count >= 16)
		bgt.b   .cont
		move.l  d1,d2                   ;spancount = count
		subq.l  #1,d2
		moveq   #0,d1                   ;count -= spancount
		bra.w   .finalpart
.cont
		sub.l   #16,d1                  ;count -= spancount;


******  Evaluation of the values for the inner loop. This version is used for
******  span size = 16

******  a2  : bbextentt
******  a3  : tadjust
******  a4  : bbextents
******  a5  : sadjust
******  fp0 : sdivz
******  fp1 : tdivz
******  fp2 : zi
******  fp3 : sdivz16stepu
******  fp4 : tdivz16stepu
******  fp5 : zi16stepu

*                        // calculate s/z, t/z, zi->fixed s and t at far end of span,
*                        // calculate s and t steps across span by shifting
*                                sdivz += sdivz16stepu;
*                                tdivz += tdivz16stepu;
*                                zi += zi16stepu;
*                                z = (float)0x10000 / zi;        // prescale to 16.16 fixed-point
*
*                                snext = (int)(sdivz * z) + sadjust;
*                                if (snext > bbextents)
*                                        snext = bbextents;
*                                else if (snext < 16)
*                                        snext = 16;     // prevent round-off error on <0 steps from
*                                                                //  from causing overstepping & running off the
*                                                                //  edge of the texture
*
*                                tnext = (int)(tdivz * z) + tadjust;
*                                if (tnext > bbextentt)
*                                        tnext = bbextentt;
*                                else if (tnext < 16)
*                                        tnext = 16;     // guard against round-off error on <0 steps
*
*                                r_turb_sstep = (snext - r_turb_s) >> 4;
*                                r_turb_tstep = (tnext - r_turb_t) >> 4;
*                        }

		fadd    fp3,fp0                 ;sdivz += sdivz16stepu
		fadd    fp4,fp1                 ;tdivz += tdivz16stepu
		fadd    fp5,fp2                 ;zi += zi16stepu
		fmove.s #65536,fp7
		fdiv    fp2,fp7                 ;z = (float)0x10000 / zi;
		fmove   fp7,fp6
		fmul    fp0,fp6                 ;fp2 = sdivz * z
		fmove.l fp6,d4                  ;convert to integer
		add.l   a5,d4                   ;snext = d4 + sadjust
		cmp.l   a4,d4                   ;if (snext > bbextents)
		bgt.b   .down3
		cmp.l   #16,d4                  ;if (snext < 16)
		bge.b   .keep3
.up3
		moveq   #16,d4                  ;snext = 16
		bra.b   .keep3
.down3
		move.l  a4,d4                   ;snext = bbextents
.keep3
		fmul    fp1,fp7                 ;fp7 = tdivz * z
		fmove.l fp7,d5                  ;convert to integer
		add.l   a3,d5                   ;tnext = d5 + tadjust
		cmp.l   a2,d5                   ;if (tnext > bbextentt)
		bgt.b   .down4
		cmp.l   #16,d5                  ;if (tnext < 16)
		bge.b   .keep4
.up4
		moveq   #16,d5                  ;tnext = 16
		bra.b   .keep4
.down4
		move.l  a2,d5                   ;tnext = bbextentt
.keep4
		move.l  d4,.saved4(sp)          ;save snext
		move.l  d5,.saved5(sp)          ;save tnext
		sub.l   d6,d4                   ;d4 = snext - s
		sub.l   d7,d5                   ;d5 = tnext - t
		asr.l   #4,d4                   ;r_turb_sstep = d4 >> 4
		asr.l   #4,d5                   ;r_turb_tstep = d5 >> 4
		bra.w   .mainloop


******  Evaluation of the values for the inner loop. This version is used for
******  span size < 16

******  The original algorithm has two ugly divisions at the end of this part.
******  These are removed by the following optimization:
******  First, the divisors 1,2 and 4 are handled specially to gain speed. The
******  other divisors are handled using a reciprocal table.

******  a2  : bbextentt
******  a3  : tadjust
******  a4  : bbextents
******  a5  : sadjust
******  fp0 : sdivz
******  fp1 : tdivz
******  fp2 : zi

*                        // calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
*                        // can't step off polygon), clamp, calculate s and t steps across
*                        // span by division, biasing steps low so we don't run off the
*                        // texture
*                                spancountminus1 = (float)(r_turb_spancount - 1);
*                                sdivz += d_sdivzstepu * spancountminus1;
*                                tdivz += d_tdivzstepu * spancountminus1;
*                                zi += d_zistepu * spancountminus1;
*                                z = (float)0x10000 / zi;        // prescale to 16.16 fixed-point
*                                snext = (int)(sdivz * z) + sadjust;
*                                if (snext > bbextents)
*                                        snext = bbextents;
*                                else if (snext < 16)
*                                        snext = 16;     // prevent round-off error on <0 steps from
*                                                                //  from causing overstepping & running off the
*                                                                //  edge of the texture
*
*                                tnext = (int)(tdivz * z) + tadjust;
*                                if (tnext > bbextentt)
*                                        tnext = bbextentt;
*                                else if (tnext < 16)
*                                        tnext = 16;     // guard against round-off error on <0 steps
*
*                                if (r_turb_spancount > 1)
*                                {
*                                        r_turb_sstep = (snext - r_turb_s) / (r_turb_spancount - 1);
*                                        r_turb_tstep = (tnext - r_turb_t) / (r_turb_spancount - 1);
*                                }
*                        }

.finalpart
		fmove.l d2,fp7                  ;spancountminus1 = (float)(r_turb_spancount-1)
		fmove   fp7,fp6
		fmul.s  .szstpu(sp),fp6         ;fp6 = d_sdivzstepu * spancountminus1
		fadd    fp6,fp0                 ;sdivz += fp6
		fmove   fp7,fp6
		fmul.s  .tzstpu(sp),fp6         ;fp6 = d_tdivzstepu * spancountminus1
		fadd    fp6,fp1                 ;tdivz += fp6
		fmul.s  .zistpu(sp),fp7         ;fp7 = d_zistepu * spancountminus1
		fadd    fp7,fp2                 ;zi += fp7
		fmove.s #65536,fp7
		fdiv    fp2,fp7                 ;z = (float)0x10000 / zi;
		fmove   fp7,fp6
		fmul    fp0,fp6                 ;fp6 = sdivz * z
		fmove.l fp6,d4                  ;convert to integer
		add.l   a5,d4                   ;snext = d4 + sadjust
		cmp.l   a4,d4                   ;if (snext > bbextents)
		bgt.b   .down5
		cmp.l   #16,d4                  ;if (snext < 16)
		bge.b   .keep5
.up5
		moveq   #16,d4                  ;snext = 16
		bra.b   .keep5
.down5
		move.l  a4,d4                   ;snext = bbextents
.keep5
		fmul    fp1,fp7                 ;fp7 = tdivz * z
		fmove.l fp7,d5                  ;convert to integer
		add.l   a3,d5                   ;tnext = d5 + tadjust
		cmp.l   a2,d5                   ;if (tnext > bbextentt)
		bgt.b   .down6
		cmp.l   #16,d5                  ;if (tnext < 16)
		bge.b   .keep6
.up6
		moveq   #16,d5                  ;tnext = 16
		bra.b   .keep6
.down6
		move.l  a2,d5                   ;tnext = bbextentt
.keep6
		move.l  d4,.saved4(sp)          ;save snext
		move.l  d5,.saved5(sp)          ;save tnext
		sub.l   d6,d4                   ;d4 = snext - r_turb_s
		sub.l   d7,d5                   ;d5 = tnext - r_turb_t
		IFEQ    QDIV
		tst.l   d2
		beq.w   .mainloop
		divs.l  d2,d4
		divs.l  d2,d5
		ELSEIF
		cmp     #5,d2                   ;(r_turb_spancount-1) < 5?
		blt.b   .special                ;yes -> special case
		cmp     #8,d2
		beq.b   .spec_8
.qdiv
		IFNE    NICE_DIV
		lsl.l   #2,d4
		lsl.l   #2,d5
		lea     ReciprocTable,a1
		move    0(a1,d2.w*2),d0
		move.l  d4,d3
		mulu    d0,d3
		clr     d3
		swap    d3
		swap    d4
		muls    d0,d4
		add.l   d3,d4
		move.l  d5,d3
		mulu    d0,d3
		clr     d3
		swap    d3
		swap    d5
		muls    d0,d5
		add.l   d3,d5
		bra.b   .mainloop
		ELSEIF
		asr.l   #7,d4                   ;d4 >> 7
		asr.l   #7,d5                   ;d5 >> 7
		lea     ReciprocTable,a1        ;a1 -> reciprocal table
		move    0(a1,d2.w*2),d0         ;d0 = (1/(r_turb_spancount-1))<<16
		muls    d0,d4                   ;d4 = d4 / (r_turb_spancount-1)
		asr.l   #7,d4                   ;sstep = d4 >> 7
		muls    d0,d5                   ;d5 = d5 / (r_turb_spancount-1)
		asr.l   #7,d5                   ;tstep = d5 >> 7
		bra.b   .mainloop
		ENDC
.special
		cmp     #1,d2                   ;switch (r_turb_spancount-1)
		ble.b   .mainloop               ;0,1 -> no scaling needed
		cmp     #3,d2                   ;3 -> standard qdiv
		beq.b   .qdiv
		blt.b   .spec_2
		asr.l   #2,d4                   ;4 -> scale by shifting right
		asr.l   #2,d5
		bra.b   .mainloop
.spec_8
		asr.l   #3,d4                   ;8 -> scale by shifting right
		asr.l   #3,d5
		bra.b   .mainloop
.spec_2
		asr.l   #1,d4                   ;2 -> scale by shifting right
		asr.l   #1,d5
		ENDC

******  D_DrawTurbulent8Span (inlined)
******  Main drawing loop.

******  d2 : r_turb_spancount
******  d4 : r_turb_sstep
******  d5 : r_turb_tstep
******  d6 : r_turb_s
******  d7 : r_turb_t
******  a0 : r_turb_pdest
******  a6 : r_turb_turb

*        do
*        {
*                sturb = ((r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)])>>16)&63;
*                tturb = ((r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)])>>16)&63;
*                *r_turb_pdest++ = *(r_turb_pbase + (tturb<<6) + sturb);
*                r_turb_s += r_turb_sstep;
*                r_turb_t += r_turb_tstep;
*        } while (--r_turb_spancount > 0);

.mainloop
		move.l  d1,-(sp)
		move.l  _cacheblock,a1          ;pbase = (unsigned char *)cacheblock
		moveq   #10,d1
.draw
		swap    d6                      ;r_turb_s >> 16
		swap    d7                      ;r_turb_t >> 16
		and     #CYCLE-1,d6             ;(r_turb_s >> 16) & (CYCLE-1)
		and     #CYCLE-1,d7             ;(r_turb_t >> 16) & (CYCLE-1)
		move.l  0(a6,d7.w*4),d0         ;r_turb_turb [d7]
		move.l  0(a6,d6.w*4),d3         ;r_turb_turb [d6]
		swap    d6
		swap    d7
		add.l   d6,d0                   ;r_turb_s + r_turb_turb []
		add.l   d7,d3                   ;r_turb_t + r_turb_turb []
		swap    d0                      ;d0 >> 16
		and.l   #$3f,d0                 ;sturb = (d0 >> 16) & 63
		lsr.l   d1,d3                   ;(d3 >> (16-6))
		and.l   #$fc0,d3                ;tturb<<6 = (d3 >> (16-6)) & (63 << 6)
		add.l   d3,d0                   ;sturb + tturb << 6
		move.b  0(a1,d0.l),(a0)+        ;*r_turb_pdest++ = *(r_turb_pbase + d0)
		add.l   d4,d6                   ;r_turb_s += r_turb_sstep
		add.l   d5,d7                   ;r_turb_t += r_turb_tstep
		dbra    d2,.draw                ;while (--r_turb_spancount > 0)
		move.l  (sp)+,d1

******  loop terminations


		move.l   .saved5(sp),d7         ;r_turb_t = tnext
		move.l   .saved4(sp),d6         ;r_turb_s = snext

		tst.l   d1                      ;while (count > 0)
		bgt.w   .loop2

		move.l  .savea1(sp),a1          ;while ((pspan = pspan->next) != NULL)
		move.l  SPAN_PNEXT(a1),a1
		tst.l   a1
		bne.w   .loop
		add.l   #.fpuregs,sp
		fmovem.x        (sp)+,fp2-fp7
		movem.l (sp)+,d2-d7/a2-a6
		rts







******************************************************************************
*
*       void D_DrawTurbulent8T (espan_t *pspan)
*
*       translucent scan drawing function for animated textures
*       Note: The functions D_DrawSingleZSpans, D_DrawTurbulent8TSpan
*       and D_DrawTurbulent8TQuickSpan were inlined into this
*       function, because they are never used anywhere else.
*       This function expects r_turb_turb to be set by the caller.
*
******************************************************************************

		cnop    0,4
_D_DrawTurbulent8T


*****   stackframe

		rsreset
;.origscanList   rs.b    SCAN_SIZE
;.ZScanCount     rs.l    1
.saved4         rs.l    1
.saved5         rs.l    1
.savea1         rs.l    1
.szstpu         rs.s    1
.szstpv         rs.s    1
.szorg          rs.s    1
.tzstpu         rs.s    1
.tzstpv         rs.s    1
.tzorg          rs.s    1
.zistpu         rs.s    1
.zistpv         rs.s    1
.ziorg          rs.s    1
.fpuregs        rs.x    6
.intregs        rs.l    11
		rs.l    1
.pspan          rs.l    1


******  Prologue. Global variables are put into registers or onto the stackframe

		movem.l d2-d7/a2-a6,-(sp)
		fmovem.x        fp2-fp7,-(sp)
		move.l  _bbextentt,a2
		move.l  _tadjust,a3
		move.l  _bbextents,a4
		move.l  _sadjust,a5
		move.l  _d_ziorigin,-(sp)
		move.l  _d_zistepv,-(sp)
		move.l  _d_zistepu,-(sp)
		move.l  _d_tdivzorigin,-(sp)
		move.l  _d_tdivzstepv,-(sp)
		move.l  _d_tdivzstepu,-(sp)
		move.l  _d_sdivzorigin,-(sp)
		move.l  _d_sdivzstepv,-(sp)
		move.l  _d_sdivzstepu,-(sp)
		sub.l   #.szstpu,sp

******  First loop. In every iteration one complete span is drawn

*        r_turb_turb = sintable + ((int)(cl.time*SPEED)&(CYCLE-1));
*
*        r_turb_pbase = (unsigned char *)cacheblock;
*
*        sdivz16stepu = d_sdivzstepu * 16;
*        tdivz16stepu = d_tdivzstepu * 16;
*        zi16stepu = d_zistepu * 16;
*
*        do
*        {
*                r_turb_pdest = (unsigned char *)((byte *)d_viewbuffer +
*                                (screenwidth * pspan->v) + pspan->u);
*
*                count = pspan->count;
*
*        // calculate the initial s/z, t/z, 1/z, s, and t and clamp
*                du = (float)pspan->u;
*                dv = (float)pspan->v;
*
*                sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
*                tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
*                zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
*                z = (float)0x10000 / zi;        // prescale to 16.16 fixed-point
*

		;fmove.d _cl+CL_TIME,fp0         ;get cl.time
		;fmul.s  #SPEED,fp0              ;fp0 = cl.time*SPEED
		;fmove.l fp0,d0                  ;(int)(cl.time*SPEED)
		;and.l   #CYCLE-1,d0             ;(int)(cl.time*SPEED)&(CYCLE-1)
		;and.l   #CYCLE-1,d0             ;(int)(cl.time*SPEED)&(CYCLE-1)
		;lsl.l   #2,d0
		;add.l   #_sintable,d0           ;r_turb_turb = _sintable + 4*d0
		;move.l  d0,a6
		move.l  _r_turb_turb,a6
		fmove.s #16,fp7
		fmove.s .szstpu(sp),fp3
		fmul    fp7,fp3                 ;sdivz16stepu = d_sdivzstepu * 16
		fmove.s .tzstpu(sp),fp4
		fmul    fp7,fp4                 ;tdivz16stepu = d_tdivzstepu * 16
		fmove.s .zistpu(sp),fp5
		fmul    fp7,fp5                 ;zi16stepu = d_zistepu * 16
		move.l  .pspan(sp),a1           ;get function parameter
.loop
		;move.l  a1,-(sp)
		;jsr _D_DrawSingleZSpans             ; this should probably be inlined
		;addq.l  #4,sp
		move.l  a1,.savea1(sp)          ;save actual ptr to pspan
		move.l  (a1)+,d1
		fmove.l d1,fp2                  ;du = (float)pspan->u
		move.l  (a1)+,d2
		fmove.l d2,fp7                  ;dv = (float)pspan->v
		move.l  (a1)+,d4                ;d4 = pspan->count

******  D_DrawSingleZSpans (inlined)

******  d0  : scratch
******  d1  : pspan->u
******  d2  : pspan->v
******  d3  : scratch
******  d4  : pspan->count
******  d5  : scratch
******  d6  : scratch
******  d7  : scratch
******  a0  : scratch
******  a1  : scratch
******  fp0 : scratch
******  fp1 : scratch
******  fp2 : du = (float)pspan->u
******  fp6 : scratch
******  fp7 : dv = (float)pspan->v

*				ZScanCount = 0;
*				izistep = (int)(d_zistepu * 0x8000 * 0x10000);
*				pdest = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u;
*				zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
*				izi = (int)(zi * 0x8000 * 0x10000);

		clr.l    d0                     ;d0 = ZScanCount
		fmove.s #32768*65536,fp0
		fmove.s .zistpu(sp),fp6         ;fp6 = d_zistepu
		fmove   fp6,fp1                 ;fp1 = d_zistepu
		fmul    fp0,fp1                 ;multiply by $8000*$10000
		fmove.l fp1,d3                  ;d3 = izistep

		fmove   fp6,fp1                 ;fp1 = d_zistepu
		fmul    fp2,fp1                 ;fp1 = du * d_zistepu
		fmove.s .zistpv(sp),fp6         ;fp6 = d_zistepv
		fmul    fp7,fp6                 ;fp6 = dv * d_zistepv
		fadd    fp6,fp1                 ;fp1 = dv * d_zistepv + du * d_zistepu

		move.l  _d_zwidth,d7
		muls    d2,d7                   ;d7 = pspan->v * d_zwidth
		fmove.s _d_ziorigin,fp6         ;fp6 = d_ziorigin
		fadd    fp6,fp1                 ;fp1 = zi = d_ziorigin + dv * d_zistepv + du * d_zistepu
		add.l   d1,d7                   ;d7 = d7 + pspan->u
		move.l  _d_pzbuffer,a0
		lea     0(a0,d7.l*2),a1         ;a1 = pdest
		fmul    fp0,fp1                 ;izi = zi * $8000 * $10000
		fmove.l fp1,d5                  ;d5 = izi

*				if (count > 0)
*				{

		tst.l   d4
		ble.w   .szsend

*					do
*					{
*						if (*pdest > (short)(izi >> 16))
*						{
*							ZScanCount++;
*							scanList[count-1] = 0;
*						}
*						else
*						{
*							scanList[count-1] = 1;
*							*pdest = (short)(izi >> 16);
*						}
*						izi += izistep;
*						pdest++;
*						count--;
*					} while (count > 0);

******  d0 = ZScanCount
******  d3 = izistep
******  d4 = count
******  d5 = izi
******  d6 = scratch
******  d7 = scratch
******  a1 = scratch
******  a1 = pdest

		lea     _scanList,a0
		move.l  d4,d6                    ;d6 = count
		subq.l  #1,d6                    ;d6 = count--
.szsloop
		move.l  d5,d7
		swap    d7                       ;d7 = izi >> 16 (upper 16-bits contain garbage)
		cmp.w   (a1),d7
		bge.w   .szsvisible              ;if (*pdest > (izi >> 16))
		addq.l  #1,d0                    ;ZScanCount++
		clr.b   (a0,d6.l)                ;scanList[count-1] = 0
		jmp     .szsincvars              ;TODO this could probably be done better
.szsvisible
		move.b  #1,(a0,d6.l)             ;scanList[count-1] = 1;
		move.w  d7,(a1)                  ;*pdest = (izi >> 16)
.szsincvars
		add.l   d3,d5                    ;izi += izistep
		addq.l  #2,a1                    ;pdest++
		move.l  d6,d7                    ;d7 = count
		subq.l  #1,d6                    ;count--
		tst.l   d7
		bne.w   .szsloop                 ;while (count > 0);
.szsend
*				}
****** D_DrawSingleZSpans return
******  d0 : ZScanCount
******  d4 : count
; translucent

*				if (ZScanCount == count)  // fully blocked
*					continue;

		cmp.l   d0,d4
		beq.w   .skiploop
		;move.l  d0,.ZScanCount(sp)      ;save ZScanCount
		move.l  d0,_ZScanCount
; translucent
		move.l  _d_viewbuffer,a0
		move.l  _screenwidth,d0
		muls    d2,d0                   ;d0 = screenwidth * pspan->v
		add.l   d1,d0
		add.l   d0,a0                   ;pdest = d_viewbuffer + pspan->u + d0
		lea     .szstpu(sp),a1          ;a1 -> stackframe
		fmove.s (a1)+,fp0
		fmul    fp2,fp0                 ;fp0 = du * d_sdivzstepu
		fmove.s (a1)+,fp1
		fmul    fp7,fp1                 ;fp1 = dv * d_sdivzstepv
		fadd    fp1,fp0
		fadd.s  (a1)+,fp0               ;sdivz = d_sdivzorigin + fp0 + fp1
		fmove.s (a1)+,fp1
		fmul    fp2,fp1                 ;fp1 = du * d_tdivzstepu
		fmove.s (a1)+,fp6
		fmul    fp7,fp6                 ;fp6 = dv * d_tdivzstepv
		fadd    fp6,fp1
		fadd.s  (a1)+,fp1               ;tdivz = d_tdivzorigin + fp1 + fp6
		fmul.s  (a1)+,fp2               ;fp2 = du * d_zistepu
		fmul.s  (a1)+,fp7               ;fp7 = dv * d_zistepv
		fadd    fp7,fp2
		fadd.s  (a1)+,fp2               ;zi = d_ziorigin + fp2 + fp7
		fmove.s #65536,fp6
		fdiv    fp2,fp6                 ;z = (float)0x10000 / zi

*                s = (int)(sdivz * z) + sadjust;
*                if (s > bbextents)
*                        s = bbextents;
*                else if (s < 0)
*                        s = 0;
*
*                t = (int)(tdivz * z) + tadjust;
*                if (t > bbextentt)
*                        t = bbextentt;
*                else if (t < 0)
*                        t = 0;


		fmove   fp6,fp7
		fmul    fp0,fp7                 ;fp7 = sdivz * z
		fmove.l fp7,d6                  ;convert to integer
		add.l   a5,d6                   ;s = d6 + sadjust
		cmp.l   a4,d6                   ;if (s > bbextents)
		bgt.b   .down
		tst.l   d6                      ;if (s < 0)
		bge.b   .keep
.up
		moveq   #0,d6                   ;s = 0
		bra.b   .keep
.down
		move.l  a4,d6                   ;s = bbextents
.keep
		fmul    fp1,fp6                 ;fp6 = tdivz * z
		fmove.l fp6,d7                  ;convert to integer
		add.l   a3,d7                   ;t = d7 + tadjust
		cmp.l   a2,d7                   ;if (t > bbextentt)
		bgt.b   .down2
		tst.l   d7                      ;if (t < 0)
		bge.b   .keep2
.up2
		moveq   #0,d7                   ;t = 0
		bra.b   .keep2
.down2
		move.l  a2,d7                   ;t = bbextentt
.keep2
		move.l  d4,d1


******  Second loop. In every iteration one part of the whole span is drawn
******  d2 gets the value (spancount-1)! [NOT spancount]

******  d1 = count

*                do
*                {
*                // calculate s and t at the far end of the span
*                        if (count >= 16)
*                                spancount = 16;
*                        else
*                                spancount = count;
*
*                        count -= spancount;
*
*                        if (count)
*                        {

.loop2
		moveq   #16-1,d2                ;spancount = 16
		cmp.l   #16,d1                  ;if (count >= 16)
		bgt.b   .cont
		move.l  d1,d2                   ;spancount = count
		subq.l  #1,d2
		moveq   #0,d1                   ;count -= spancount
		bra.w   .finalpart
.cont
		sub.l   #16,d1                  ;count -= spancount;


******  Evaluation of the values for the inner loop. This version is used for
******  span size = 16

******  a2  : bbextentt
******  a3  : tadjust
******  a4  : bbextents
******  a5  : sadjust
******  fp0 : sdivz
******  fp1 : tdivz
******  fp2 : zi
******  fp3 : sdivz16stepu
******  fp4 : tdivz16stepu
******  fp5 : zi16stepu

*                        // calculate s/z, t/z, zi->fixed s and t at far end of span,
*                        // calculate s and t steps across span by shifting
*                                sdivz += sdivz16stepu;
*                                tdivz += tdivz16stepu;
*                                zi += zi16stepu;
*                                z = (float)0x10000 / zi;        // prescale to 16.16 fixed-point
*
*                                snext = (int)(sdivz * z) + sadjust;
*                                if (snext > bbextents)
*                                        snext = bbextents;
*                                else if (snext < 16)
*                                        snext = 16;     // prevent round-off error on <0 steps from
*                                                                //  from causing overstepping & running off the
*                                                                //  edge of the texture
*
*                                tnext = (int)(tdivz * z) + tadjust;
*                                if (tnext > bbextentt)
*                                        tnext = bbextentt;
*                                else if (tnext < 16)
*                                        tnext = 16;     // guard against round-off error on <0 steps
*
*                                r_turb_sstep = (snext - r_turb_s) >> 4;
*                                r_turb_tstep = (tnext - r_turb_t) >> 4;
*                        }

		fadd    fp3,fp0                 ;sdivz += sdivz16stepu
		fadd    fp4,fp1                 ;tdivz += tdivz16stepu
		fadd    fp5,fp2                 ;zi += zi16stepu
		fmove.s #65536,fp7
		fdiv    fp2,fp7                 ;z = (float)0x10000 / zi;
		fmove   fp7,fp6
		fmul    fp0,fp6                 ;fp2 = sdivz * z
		fmove.l fp6,d4                  ;convert to integer
		add.l   a5,d4                   ;snext = d4 + sadjust
		cmp.l   a4,d4                   ;if (snext > bbextents)
		bgt.b   .down3
		cmp.l   #16,d4                  ;if (snext < 16)
		bge.b   .keep3
.up3
		moveq   #16,d4                  ;snext = 16
		bra.b   .keep3
.down3
		move.l  a4,d4                   ;snext = bbextents
.keep3
		fmul    fp1,fp7                 ;fp7 = tdivz * z
		fmove.l fp7,d5                  ;convert to integer
		add.l   a3,d5                   ;tnext = d5 + tadjust
		cmp.l   a2,d5                   ;if (tnext > bbextentt)
		bgt.b   .down4
		cmp.l   #16,d5                  ;if (tnext < 16)
		bge.b   .keep4
.up4
		moveq   #16,d5                  ;tnext = 16
		bra.b   .keep4
.down4
		move.l  a2,d5                   ;tnext = bbextentt
.keep4
		move.l  d4,.saved4(sp)          ;save snext
		move.l  d5,.saved5(sp)          ;save tnext
		sub.l   d6,d4                   ;d4 = snext - s
		sub.l   d7,d5                   ;d5 = tnext - t
		asr.l   #4,d4                   ;r_turb_sstep = d4 >> 4
		asr.l   #4,d5                   ;r_turb_tstep = d5 >> 4
		bra.w   .mainloop


******  Evaluation of the values for the inner loop. This version is used for
******  span size < 16

******  The original algorithm has two ugly divisions at the end of this part.
******  These are removed by the following optimization:
******  First, the divisors 1,2 and 4 are handled specially to gain speed. The
******  other divisors are handled using a reciprocal table.

******  a2  : bbextentt
******  a3  : tadjust
******  a4  : bbextents
******  a5  : sadjust
******  fp0 : sdivz
******  fp1 : tdivz
******  fp2 : zi

*                        // calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
*                        // can't step off polygon), clamp, calculate s and t steps across
*                        // span by division, biasing steps low so we don't run off the
*                        // texture
*                                spancountminus1 = (float)(r_turb_spancount - 1);
*                                sdivz += d_sdivzstepu * spancountminus1;
*                                tdivz += d_tdivzstepu * spancountminus1;
*                                zi += d_zistepu * spancountminus1;
*                                z = (float)0x10000 / zi;        // prescale to 16.16 fixed-point
*                                snext = (int)(sdivz * z) + sadjust;
*                                if (snext > bbextents)
*                                        snext = bbextents;
*                                else if (snext < 16)
*                                        snext = 16;     // prevent round-off error on <0 steps from
*                                                                //  from causing overstepping & running off the
*                                                                //  edge of the texture
*
*                                tnext = (int)(tdivz * z) + tadjust;
*                                if (tnext > bbextentt)
*                                        tnext = bbextentt;
*                                else if (tnext < 16)
*                                        tnext = 16;     // guard against round-off error on <0 steps
*
*                                if (r_turb_spancount > 1)
*                                {
*                                        r_turb_sstep = (snext - r_turb_s) / (r_turb_spancount - 1);
*                                        r_turb_tstep = (tnext - r_turb_t) / (r_turb_spancount - 1);
*                                }
*                        }

.finalpart
		fmove.l d2,fp7                  ;spancountminus1 = (float)(r_turb_spancount-1)
		fmove   fp7,fp6
		fmul.s  .szstpu(sp),fp6         ;fp6 = d_sdivzstepu * spancountminus1
		fadd    fp6,fp0                 ;sdivz += fp6
		fmove   fp7,fp6
		fmul.s  .tzstpu(sp),fp6         ;fp6 = d_tdivzstepu * spancountminus1
		fadd    fp6,fp1                 ;tdivz += fp6
		fmul.s  .zistpu(sp),fp7         ;fp7 = d_zistepu * spancountminus1
		fadd    fp7,fp2                 ;zi += fp7
		fmove.s #65536,fp7
		fdiv    fp2,fp7                 ;z = (float)0x10000 / zi;
		fmove   fp7,fp6
		fmul    fp0,fp6                 ;fp6 = sdivz * z
		fmove.l fp6,d4                  ;convert to integer
		add.l   a5,d4                   ;snext = d4 + sadjust
		cmp.l   a4,d4                   ;if (snext > bbextents)
		bgt.b   .down5
		cmp.l   #16,d4                  ;if (snext < 16)
		bge.b   .keep5
.up5
		moveq   #16,d4                  ;snext = 16
		bra.b   .keep5
.down5
		move.l  a4,d4                   ;snext = bbextents
.keep5
		fmul    fp1,fp7                 ;fp7 = tdivz * z
		fmove.l fp7,d5                  ;convert to integer
		add.l   a3,d5                   ;tnext = d5 + tadjust
		cmp.l   a2,d5                   ;if (tnext > bbextentt)
		bgt.b   .down6
		cmp.l   #16,d5                  ;if (tnext < 16)
		bge.b   .keep6
.up6
		moveq   #16,d5                  ;tnext = 16
		bra.b   .keep6
.down6
		move.l  a2,d5                   ;tnext = bbextentt
.keep6
		move.l  d4,.saved4(sp)          ;save snext
		move.l  d5,.saved5(sp)          ;save tnext
		sub.l   d6,d4                   ;d4 = snext - r_turb_s
		sub.l   d7,d5                   ;d5 = tnext - r_turb_t
		IFEQ    QDIV
		tst.l   d2
		beq.w   .mainloop
		divs.l  d2,d4
		divs.l  d2,d5
		ELSEIF
		cmp     #5,d2                   ;(r_turb_spancount-1) < 5?
		blt.b   .special                ;yes -> special case
		cmp     #8,d2
		beq.b   .spec_8
.qdiv
		IFNE    NICE_DIV
		lsl.l   #2,d4
		lsl.l   #2,d5
		lea     ReciprocTable,a1
		move    0(a1,d2.w*2),d0
		move.l  d4,d3
		mulu    d0,d3
		clr     d3
		swap    d3
		swap    d4
		muls    d0,d4
		add.l   d3,d4
		move.l  d5,d3
		mulu    d0,d3
		clr     d3
		swap    d3
		swap    d5
		muls    d0,d5
		add.l   d3,d5
		bra.b   .mainloop
		ELSEIF
		asr.l   #7,d4                   ;d4 >> 7
		asr.l   #7,d5                   ;d5 >> 7
		lea     ReciprocTable,a1        ;a1 -> reciprocal table
		move    0(a1,d2.w*2),d0         ;d0 = (1/(r_turb_spancount-1))<<16
		muls    d0,d4                   ;d4 = d4 / (r_turb_spancount-1)
		asr.l   #7,d4                   ;sstep = d4 >> 7
		muls    d0,d5                   ;d5 = d5 / (r_turb_spancount-1)
		asr.l   #7,d5                   ;tstep = d5 >> 7
		bra.b   .mainloop
		ENDC
.special
		cmp     #1,d2                   ;switch (r_turb_spancount-1)
		ble.b   .mainloop               ;0,1 -> no scaling needed
		cmp     #3,d2                   ;3 -> standard qdiv
		beq.b   .qdiv
		blt.b   .spec_2
		asr.l   #2,d4                   ;4 -> scale by shifting right
		asr.l   #2,d5
		bra.b   .mainloop
.spec_8
		asr.l   #3,d4                   ;8 -> scale by shifting right
		asr.l   #3,d5
		bra.b   .mainloop
.spec_2
		asr.l   #1,d4                   ;2 -> scale by shifting right
		asr.l   #1,d5
		ENDC

******  D_DrawTurbulent8TQuickSpan (inlined)
******  Main drawing loop.

******  d2 : r_turb_spancount
******  d4 : r_turb_sstep;
******  d5 : r_turb_tstep;
******  d6 : r_turb_s
******  d7 : r_turb_t
******  a0 : r_turb_pdest
******  a6 : r_turb_turb

*        do
*        {
*                sturb = ((r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)])>>16)&63;
*                tturb = ((r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)])>>16)&63;
*                temp = *(r_turb_pbase + (tturb<<6) + sturb);
*                *r_turb_pdest = mainTransTable[(temp<<8) + (*r_turb_pdest)];
*                r_turb_pdest++;
*                r_turb_s += r_turb_sstep;
*                r_turb_t += r_turb_tstep;
*        } while (--r_turb_spancount > 0);

.mainloop

		;move.l  d1,-(sp)
		movem.l d1/a2-a5,-(sp)
		;move.l  .ZScanCount(sp),d3
		lea     _scanList,a5            ;a5 = scanList
		add.l   d1,a5                   ;TODO don't do this for the quick case
		move.l  _cacheblock,a1          ;pbase = (unsigned char *)cacheblock
		moveq   #10,d1
		move.l  _mainTransTable,a2
		move.l  d4,a3                   ;a3 = r_turb_sstep
		;clr.l   d4
		move.l  d5,a4                   ;a4 = r_turb_tstep
		clr.l   d5
		move.l  _ZScanCount,d3
		tst.l   d3
		bne.w   .draw                   ;if (ZScanCount)

.drawquick
		swap    d6                      ;r_turb_s >> 16
		swap    d7                      ;r_turb_t >> 16
		and     #CYCLE-1,d6             ;(r_turb_s >> 16) & (CYCLE-1)
		and     #CYCLE-1,d7             ;(r_turb_t >> 16) & (CYCLE-1)
		move.l  0(a6,d7.w*4),d0         ;r_turb_turb [d7]
		move.l  0(a6,d6.w*4),d3         ;r_turb_turb [d6]
		swap    d6
		swap    d7
		add.l   d6,d0                   ;r_turb_s + r_turb_turb []
		add.l   d7,d3                   ;r_turb_t + r_turb_turb []
		swap    d0                      ;d0 >> 16
		and.l   #$3f,d0                 ;sturb = (d0 >> 16) & 63
		lsr.l   d1,d3                   ;(d3 >> (16-6))
		and.l   #$fc0,d3                ;tturb<<6 = (d3 >> (16-6)) & (63 << 6)
		add.l   d3,d0                   ;sturb + tturb << 6
		;move.b  0(a1,d0.l),(a0)+        ;*r_turb_pdest++ = *(r_turb_pbase + d0)
		clr.l   d4
		move.b  0(a1,d0.l),d4           ;d4 = *(r_turb_pbase + d0)
		lsl.l   #8,d4
		move.b  (a0),d5                 ;d5 = *r_turb_pdest
		add.l   d5,d4
		move.b  0(a2,d4.l),(a0)+
		add.l   a3,d6                   ;r_turb_s += r_turb_sstep
		add.l   a4,d7                   ;r_turb_t += r_turb_tstep
		dbra    d2,.drawquick           ;while (--r_turb_spancount > 0)
		;move.l  (sp)+,d1
		movem.l (sp)+,d1/a2-a5

******  loop terminations

.terminateloop
		move.l   .saved5(sp),d7         ;r_turb_t = tnext
		move.l   .saved4(sp),d6         ;r_turb_s = snext

		tst.l   d1                      ;while (count > 0)
		bgt.w   .loop2

.skiploop
		move.l  .savea1(sp),a1          ;while ((pspan = pspan->next) != NULL)
		move.l  SPAN_PNEXT(a1),a1
		tst.l   a1
		bne.w   .loop
		add.l   #.fpuregs,sp
		fmovem.x        (sp)+,fp2-fp7
		movem.l (sp)+,d2-d7/a2-a6
		rts

.draw
		sub.w   #1,d2
		move.b  0(a5,d2.w),d0           ;d0 = scanList[r_turb_spancount-1]
		tst.b   d0
		beq.w   .skipdraw
		swap    d6                      ;r_turb_s >> 16
		swap    d7                      ;r_turb_t >> 16
		and     #CYCLE-1,d6             ;(r_turb_s >> 16) & (CYCLE-1)
		and     #CYCLE-1,d7             ;(r_turb_t >> 16) & (CYCLE-1)
		move.l  0(a6,d7.w*4),d0         ;r_turb_turb [d7]
		move.l  0(a6,d6.w*4),d3         ;r_turb_turb [d6]
		swap    d6
		swap    d7
		add.l   d6,d0                   ;r_turb_s + r_turb_turb []
		add.l   d7,d3                   ;r_turb_t + r_turb_turb []
		swap    d0                      ;d0 >> 16
		and.l   #$3f,d0                 ;sturb = (d0 >> 16) & 63
		lsr.l   d1,d3                   ;(d3 >> (16-6))
		and.l   #$fc0,d3                ;tturb<<6 = (d3 >> (16-6)) & (63 << 6)
		add.l   d3,d0                   ;sturb + tturb << 6
		;move.b  0(a1,d0.l),(a0)+        ;*r_turb_pdest++ = *(r_turb_pbase + d0)
		clr.l   d4
		move.b  0(a1,d0.l),d4           ;d4 = *(r_turb_pbase + d0)
		lsl.l   #8,d4
		move.b  (a0),d5                 ;d5 = *r_turb_pdest
		add.l   d5,d4
		move.b  0(a2,d4.l),(a0)
.skipdraw
		add.l   #1,a0                   ;r_turb_pdest++
		add.l   a3,d6                   ;r_turb_s += r_turb_sstep
		add.l   a4,d7                   ;r_turb_t += r_turb_tstep

		cmp.w   #-1,d2
		bne     .draw
		;dbra    d2,.draw                ;while (--r_turb_spancount > 0)
		;move.l  (sp)+,d1
		movem.l (sp)+,d1/a2-a5
		jmp     .terminateloop








******************************************************************************
*
*       void D_DrawSpans8 (espan_t *pspan)
*
*       standard scan drawing function (8 pixel subdivision)
*
******************************************************************************

		cnop    0,4
_D_DrawSpans8


*****   stackframe

		rsreset
.saved4         rs.l    1
.saved5         rs.l    1
.savea6         rs.l    1
.szstpu         rs.s    1
.szstpv         rs.s    1
.szorg          rs.s    1
.tzstpu         rs.s    1
.tzstpv         rs.s    1
.tzorg          rs.s    1
.zistpu         rs.s    1
.zistpv         rs.s    1
.ziorg          rs.s    1
.fpuregs        rs.x    6
.intregs        rs.l    11
		rs.l    1
.pspan          rs.l    1


******  Prologue. Global variables are put into registers or onto the stackframe

		; d_subdiv16 is handled in D_SetupFrame
		;fmove.s _d_subdiv16+CVAR_VALUE,fp0
		;fcmp.s  #0,fp0
		;fbne    _D_DrawSpans16
		movem.l d2-d7/a2-a6,-(sp)
		fmovem.x        fp2-fp7,-(sp)
		move.l  _bbextentt,a2
		move.l  _tadjust,a3
		move.l  _bbextents,a4
		move.l  _sadjust,a5
		move.l  _d_ziorigin,-(sp)
		move.l  _d_zistepv,-(sp)
		move.l  _d_zistepu,-(sp)
		move.l  _d_tdivzorigin,-(sp)
		move.l  _d_tdivzstepv,-(sp)
		move.l  _d_tdivzstepu,-(sp)
		move.l  _d_sdivzorigin,-(sp)
		move.l  _d_sdivzstepv,-(sp)
		move.l  _d_sdivzstepu,-(sp)
		sub.l   #.szstpu,sp

******  First loop. In every iteration one complete span is drawn

*        pbase = (unsigned char *)cacheblock;
*
*        sdivz8stepu = d_sdivzstepu * 8;
*        tdivz8stepu = d_tdivzstepu * 8;
*        zi8stepu = d_zistepu * 8;
*
*        do
*        {
*                pdest = (unsigned char *)((byte *)d_viewbuffer +
*                                (screenwidth * pspan->v) + pspan->u);
*
*                count = pspan->count;
*
*        // calculate the initial s/z, t/z, 1/z, s, and t and clamp
*                du = (float)pspan->u;
*                dv = (float)pspan->v;
*
*                sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
*                tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
*                zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
*                z = (float)0x10000 / zi;        // prescale to 16.16 fixed-point
*

		move.l  _cacheblock,a1          ;pbase = (unsigned char *)cacheblock
		fmove.s #8,fp7
		fmove.s .szstpu(sp),fp3
		fmul    fp7,fp3                 ;sdivz8stepu = d_sdivzstepu * 8
		fmove.s .tzstpu(sp),fp4
		fmul    fp7,fp4                 ;tdivz8stepu = d_tdivzstepu * 8
		fmove.s .zistpu(sp),fp5
		fmul    fp7,fp5                 ;zi8stepu = d_zistepu * 8
		move.l  .pspan(sp),a6           ;get function parameter
.loop
		move.l  a6,.savea6(sp)          ;save actual ptr to pspan
		move.l  _d_viewbuffer,a0
		move.l  _screenwidth,d0
		move.l  (a6)+,d1
		fmove.l d1,fp2                  ;du = (float)pspan->u
		move.l  (a6)+,d2
		fmove.l d2,fp7                  ;dv = (float)pspan->v
		move.l  (a6)+,d4
		muls    d2,d0                   ;d0 = screenwidth * pspan->v
		add.l   d1,d0
		add.l   d0,a0                   ;pdest = d_viewbuffer + pspan->u + d0
		lea     .szstpu(sp),a6          ;a6 -> stackframe
		fmove.s (a6)+,fp0
		fmul    fp2,fp0                 ;fp0 = du * d_sdivzstepu
		fmove.s (a6)+,fp1
		fmul    fp7,fp1                 ;fp1 = dv * d_sdivzstepv
		fadd    fp1,fp0
		fadd.s  (a6)+,fp0               ;sdivz = d_sdivzorigin + fp0 + fp1
		fmove.s (a6)+,fp1
		fmul    fp2,fp1                 ;fp1 = du * d_tdivzstepu
		fmove.s (a6)+,fp6
		fmul    fp7,fp6                 ;fp6 = dv * d_tdivzstepv
		fadd    fp6,fp1
		fadd.s  (a6)+,fp1               ;tdivz = d_tdivzorigin + fp1 + fp6
		fmul.s  (a6)+,fp2               ;fp2 = du * d_zistepu
		fmul.s  (a6)+,fp7               ;fp7 = dv * d_zistepv
		fadd    fp7,fp2
		fadd.s  (a6)+,fp2               ;zi = d_ziorigin + fp2 + fp7
		fmove.s #65536,fp6
		fdiv    fp2,fp6                 ;z = (float)0x10000 / zi

*                s = (int)(sdivz * z) + sadjust;
*                if (s > bbextents)
*                        s = bbextents;
*                else if (s < 0)
*                        s = 0;
*
*                t = (int)(tdivz * z) + tadjust;
*                if (t > bbextentt)
*                        t = bbextentt;
*                else if (t < 0)
*                        t = 0;


		fmove   fp6,fp7
		fmul    fp0,fp7                 ;fp7 = sdivz * z
		fmove.l fp7,d6                  ;convert to integer
		add.l   a5,d6                   ;s = d6 + sadjust
		cmp.l   a4,d6                   ;if (s > bbextents)
		bgt.b   .down
		tst.l   d6                      ;if (s < 0)
		bge.b   .keep
.up
		moveq   #0,d6                   ;s = 0
		bra.b   .keep
.down
		move.l  a4,d6                   ;s = bbextents
.keep
		fmul    fp1,fp6                 ;fp6 = tdivz * z
		fmove.l fp6,d7                  ;convert to integer
		add.l   a3,d7                   ;t = d7 + tadjust
		cmp.l   a2,d7                   ;if (t > bbextentt)
		bgt.b   .down2
		tst.l   d7                      ;if (t < 0)
		bge.b   .keep2
.up2
		moveq   #0,d7                   ;t = 0
		bra.b   .keep2
.down2
		move.l  a2,d7                   ;t = bbextentt
.keep2
		move.l  d4,d1

******  Second loop. In every iteration one part of the whole span is drawn
******  d2 gets the value (spancount-1)! [NOT spancount]

******  d1 = count

*                do
*                {
*                // calculate s and t at the far end of the span
*                        if (count >= 8)
*                                spancount = 8;
*                        else
*                                spancount = count;
*
*                        count -= spancount;
*
*                        if (count)
*                        {

.loop2
		moveq   #8-1,d2                 ;spancount = 8
		cmp.l   #8,d1                   ;if (count >= 8)
		bgt.b   .cont
		move.l  d1,d2                   ;spancount = count
		subq.l  #1,d2
		moveq   #0,d1                   ;count -= spancount
		bra.w   .finalpart
.cont
		subq.l  #8,d1                   ;count -= spancount;

******  Evaluation of the values for the inner loop. This version is used for
******  span size = 8

******  a2  : bbextentt
******  a3  : tadjust
******  a4  : bbextents
******  a5  : sadjust
******  fp0 : sdivz
******  fp1 : tdivz
******  fp2 : zi
******  fp3 : sdivz8stepu
******  fp4 : tdivz8stepu
******  fp5 : zi8stepu

*                        // calculate s/z, t/z, zi->fixed s and t at far end of span,
*                        // calculate s and t steps across span by shifting
*                                sdivz += sdivz8stepu;
*                                tdivz += tdivz8stepu;
*                                zi += zi8stepu;
*                                z = (float)0x10000 / zi;        // prescale to 16.16 fixed-point
*                                snext = (int)(sdivz * z) + sadjust;
*                                if (snext > bbextents)
*                                        snext = bbextents;
*                                else if (snext < 8)
*                                        snext = 8;      // prevent round-off error on <0 steps from
*                                                                //  from causing overstepping & running off the
*                                                                //  edge of the texture
*                                tnext = (int)(tdivz * z) + tadjust;
*                                if (tnext > bbextentt)
*                                        tnext = bbextentt;
*                                else if (tnext < 8)
*                                        tnext = 8;      // guard against round-off error on <0 steps
*                                sstep = (snext - s) >> 3;
*                                tstep = (tnext - t) >> 3;
*                        }

		fadd    fp3,fp0                 ;sdivz += sdivz8stepu
		fadd    fp4,fp1                 ;tdivz += tdivz8stepu
		fadd    fp5,fp2                 ;zi += zi8stepu
		fmove.s #65536,fp7
		fdiv    fp2,fp7                 ;z = (float)0x10000 / zi;
		fmove   fp7,fp6
		fmul    fp0,fp6                 ;fp2 = sdivz * z
		fmove.l fp6,d4                  ;convert to integer
		add.l   a5,d4                   ;snext = d4 + sadjust
		cmp.l   a4,d4                   ;if (snext > bbextents)
		bgt.b   .down3
		cmp.l   #8,d4                   ;if (snext < 8)
		bge.b   .keep3
.up3
		moveq   #8,d4                   ;snext = 8
		bra.b   .keep3
.down3
		move.l  a4,d4                   ;snext = bbextents
.keep3
		fmul    fp1,fp7                 ;fp7 = tdivz * z
		fmove.l fp7,d5                  ;convert to integer
		add.l   a3,d5                   ;tnext = d5 + tadjust
		cmp.l   a2,d5                   ;if (tnext > bbextentt)
		bgt.b   .down4
		cmp.l   #8,d5                   ;if (tnext < 8)
		bge.b   .keep4
.up4
		moveq   #8,d5                   ;tnext = 8
		bra.b   .keep4
.down4
		move.l  a2,d5                   ;tnext = bbextentt
.keep4
		move.l  d4,.saved4(sp)          ;save snext
		move.l  d5,.saved5(sp)          ;save tnext
		sub.l   d6,d4                   ;d4 = snext - s
		sub.l   d7,d5                   ;d5 = tnext - t
		asr.l   #3,d4                   ;sstep = d4 >> 3
		asr.l   #3,d5                   ;tstep = d5 >> 3
		bra.w   .mainloop


******  Evaluation of the values for the inner loop. This version is used for
******  span size < 8

******  The original algorithm has two ugly divisions at the end of this part.
******  These are removed by the following optimization:
******  First, the divisors 1,2 and 4 are handled specially to gain speed. The
******  other divisors are handled using a reciprocal table.

******  a2  : bbextentt
******  a3  : tadjust
******  a4  : bbextents
******  a5  : sadjust
******  fp0 : sdivz
******  fp1 : tdivz
******  fp2 : zi

*                        // calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
*                        // can't step off polygon), clamp, calculate s and t steps across
*                        // span by division, biasing steps low so we don't run off the
*                        // texture
*                                spancountminus1 = (float)(spancount - 1);
*                                sdivz += d_sdivzstepu * spancountminus1;
*                                tdivz += d_tdivzstepu * spancountminus1;
*                                zi += d_zistepu * spancountminus1;
*                                z = (float)0x10000 / zi;        // prescale to 16.16 fixed-point
*                                snext = (int)(sdivz * z) + sadjust;
*                                if (snext > bbextents)
*                                        snext = bbextents;
*                                else if (snext < 8)
*                                        snext = 8;      // prevent round-off error on <0 steps from
*                                                                //  from causing overstepping & running off the
*                                                                //  edge of the texture
*
*                                tnext = (int)(tdivz * z) + tadjust;
*                                if (tnext > bbextentt)
*                                        tnext = bbextentt;
*                                else if (tnext < 8)
*                                        tnext = 8;      // guard against round-off error on <0 steps
*
*                                if (spancount > 1)
*                                {
*                                        sstep = (snext - s) / (spancount - 1);
*                                        tstep = (tnext - t) / (spancount - 1);
*                                }
*                        }

.finalpart
		fmove.l d2,fp7                  ;spancountminus1 = (float)(spancount-1)
		fmove   fp7,fp6
		fmul.s  .szstpu(sp),fp6         ;fp6 = d_sdivzstepu * spancountminus1
		fadd    fp6,fp0                 ;sdivz += fp6
		fmove   fp7,fp6
		fmul.s  .tzstpu(sp),fp6         ;fp6 = d_tdivzstepu * spancountminus1
		fadd    fp6,fp1                 ;tdivz += fp6
		fmul.s  .zistpu(sp),fp7         ;fp7 = d_zistepu * spancountminus1
		fadd    fp7,fp2                 ;zi += fp7
		fmove.s #65536,fp7
		fdiv    fp2,fp7                 ;z = (float)0x10000 / zi;
		fmove   fp7,fp6
		fmul    fp0,fp6                 ;fp6 = sdivz * z
		fmove.l fp6,d4                  ;convert to integer
		add.l   a5,d4                   ;snext = d4 + sadjust
		cmp.l   a4,d4                   ;if (snext > bbextents)
		bgt.b   .down5
		cmp.l   #8,d4                   ;if (snext < 8)
		bge.b   .keep5
.up5
		moveq   #8,d4                   ;snext = 8
		bra.b   .keep5
.down5
		move.l  a4,d4                   ;snext = bbextents
.keep5
		fmul    fp1,fp7                 ;fp7 = tdivz * z
		fmove.l fp7,d5                  ;convert to integer
		add.l   a3,d5                   ;tnext = d5 + tadjust
		cmp.l   a2,d5                   ;if (tnext > bbextentt)
		bgt.b   .down6
		cmp.l   #8,d5                   ;if (tnext < 8)
		bge.b   .keep6
.up6
		moveq   #8,d5                   ;tnext = 8
		bra.b   .keep6
.down6
		move.l  a2,d5                   ;tnext = bbextentt
.keep6
		move.l  d4,.saved4(sp)          ;save snext
		move.l  d5,.saved5(sp)          ;save tnext
		sub.l   d6,d4                   ;d4 = snext - s
		sub.l   d7,d5                   ;d5 = tnext - t
		IFEQ    QDIV
		tst.l   d2
		beq.w   .mainloop
		divs.l  d2,d4
		divs.l  d2,d5
		ELSEIF
		cmp     #5,d2                   ;(spancount-1) < 5?
		blt.b   .special                ;yes -> special case
.qdiv
		IFNE    NICE_DIV
		lsl.l   #2,d4
		lsl.l   #2,d5
		lea     ReciprocTable,a6
		move    0(a6,d2.w*2),d0
		move.l  d4,d3
		mulu    d0,d3
		clr     d3
		swap    d3
		swap    d4
		muls    d0,d4
		add.l   d3,d4
		move.l  d5,d3
		mulu    d0,d3
		clr     d3
		swap    d3
		swap    d5
		muls    d0,d5
		add.l   d3,d5
		bra.b   .mainloop
		ELSEIF
		asr.l   #7,d4                   ;d4 >> 7
		asr.l   #7,d5                   ;d5 >> 7
		lea     ReciprocTable,a6        ;a6 -> reciprocal table
		move    0(a6,d2.w*2),d0         ;d0 = (1/(spancount-1))<<16
		muls    d0,d4                   ;d4 = d4 / (spancount-1)
		asr.l   #7,d4                   ;sstep = d4 >> 7
		muls    d0,d5                   ;d5 = d5 / (spancount-1)
		asr.l   #7,d5                   ;tstep = d5 >> 7
		bra.b   .mainloop
		ENDC
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
		ENDC

******  Main drawing loop. Here lies the speed.
******  Very optimized (removed multiplication from inner loop)

******  d2 : spancount
******  d4 : sstep
******  d5 : tstep
******  d6 : s
******  d7 : t
******  a0 : pdest
******  a1 : pbase

*                        do
*                        {
*                                *pdest++ = *(pbase + (s >> 16) + (t >> 16) * cachewidth);
*                                s += sstep;
*                                t += tstep;
*                        } while (--spancount > 0);

.mainloop
		move.l  d1,-(sp)
		lea     .PixTable,a6            ;a6 -> Functable
		move.l  _cachewidth,d3          ;read cachewidth
		move.l  0(a6,d2.w*4),a6         ;get pointer to function
		swap    d7
		swap    d4
		move.l  d7,d1
		swap    d5
		muls    d3,d7                   ;d7 = t integer part * cachewidth
		move    d5,d2
		clr     d1                      ;d1 = t fractional part
		muls    d3,d2                   ;tstep integer part * cachewidth
		move    d4,d0                   ;d0 = sstep integer part
		clr     d5                      ;d5 = tstep fractional part
		clr     d4                      ;d4 = sstep fractional part
		swap    d6                      ;d6 = s swapped
		jmp     (a6)
.Pix8
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6                   ;increment s fractional part
		addx.w  d0,d6                   ;increment s integer part
		add.l   d2,d7                   ;increment t integer part
		add.l   d5,d1                   ;increment t fractional part
		bcc.b   .Pix7                   ;check if carry
		add.l   d3,d7                   ;add cachewidth to t
.Pix7
		lea     0(a1,d6.w),a6           ;and so long...
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix6
		add.l   d3,d7
.Pix6
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix5
		add.l   d3,d7
.Pix5
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix4
		add.l   d3,d7
.Pix4
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix3
		add.l   d3,d7
.Pix3
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix2
		add.l   d3,d7
.Pix2
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix1
		add.l   d3,d7
.Pix1
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix0
		add.l   d3,d7
.Pix0
		move.l  (sp)+,d1

******  loop terminations


		move.l   .saved5(sp),d7         ;t = tnext
		move.l   .saved4(sp),d6         ;s = snext

		tst.l   d1                      ;while (count > 0)
		bgt.w   .loop2

		move.l  .savea6(sp),a6          ;while ((pspan = pspan->next) != NULL)
		move.l  SPAN_PNEXT(a6),a6
		tst.l   a6
		bne.w   .loop
		add.l   #.fpuregs,sp
		fmovem.x        (sp)+,fp2-fp7
		movem.l (sp)+,d2-d7/a2-a6
		rts

.PixTable
		dc.l    .Pix1
		dc.l    .Pix2
		dc.l    .Pix3
		dc.l    .Pix4
		dc.l    .Pix5
		dc.l    .Pix6
		dc.l    .Pix7
		dc.l    .Pix8








******************************************************************************
*
*       void D_DrawSpans16 (espan_t *pspan)
*
*       standard scan drawing function (16 pixel subdivision)
*
******************************************************************************

		cnop    0,4
_D_DrawSpans16


*****   stackframe

		rsreset
.saved4         rs.l    1
.saved5         rs.l    1
.savea6         rs.l    1
.szstpu         rs.s    1
.szstpv         rs.s    1
.szorg          rs.s    1
.tzstpu         rs.s    1
.tzstpv         rs.s    1
.tzorg          rs.s    1
.zistpu         rs.s    1
.zistpv         rs.s    1
.ziorg          rs.s    1
.fpuregs        rs.x    6
.intregs        rs.l    11
		rs.l    1
.pspan          rs.l    1

******  Prologue. Global variables are put into registers or onto the stackframe

		movem.l d2-d7/a2-a6,-(sp)
		fmovem.x        fp2-fp7,-(sp)
		move.l  _bbextentt,a2
		move.l  _tadjust,a3
		move.l  _bbextents,a4
		move.l  _sadjust,a5
		move.l  _d_ziorigin,-(sp)
		move.l  _d_zistepv,-(sp)
		move.l  _d_zistepu,-(sp)
		move.l  _d_tdivzorigin,-(sp)
		move.l  _d_tdivzstepv,-(sp)
		move.l  _d_tdivzstepu,-(sp)
		move.l  _d_sdivzorigin,-(sp)
		move.l  _d_sdivzstepv,-(sp)
		move.l  _d_sdivzstepu,-(sp)
		sub.l   #.szstpu,sp

******  First loop. In every iteration one complete span is drawn

*        pbase = (unsigned char *)cacheblock;
*
*        sdivz16stepu = d_sdivzstepu * 16;
*        tdivz16stepu = d_tdivzstepu * 16;
*        zi16stepu = d_zistepu * 16;
*
*        do
*        {
*                pdest = (unsigned char *)((byte *)d_viewbuffer +
*                                (screenwidth * pspan->v) + pspan->u);
*
*                count = pspan->count;
*
*        // calculate the initial s/z, t/z, 1/z, s, and t and clamp
*                du = (float)pspan->u;
*                dv = (float)pspan->v;
*
*                sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
*                tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
*                zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
*                z = (float)0x10000 / zi;        // prescale to 16.16 fixed-point
*

		move.l  _cacheblock,a1          ;pbase = (unsigned char *)cacheblock
		fmove.s #16,fp7
		fmove.s .szstpu(sp),fp3
		fmul    fp7,fp3                 ;sdivz16stepu = d_sdivzstepu * 16
		fmove.s .tzstpu(sp),fp4
		fmul    fp7,fp4                 ;tdivz16stepu = d_tdivzstepu * 16
		fmove.s .zistpu(sp),fp5
		fmul    fp7,fp5                 ;zi16stepu = d_zistepu * 16
		move.l  .pspan(sp),a6           ;get function parameter
.loop
		move.l  a6,.savea6(sp)          ;save actual ptr to pspan
		move.l  _d_viewbuffer,a0
		move.l  _screenwidth,d0
		move.l  (a6)+,d1
		fmove.l d1,fp2                  ;du = (float)pspan->u
		move.l  (a6)+,d2
		fmove.l d2,fp7                  ;dv = (float)pspan->v
		move.l  (a6)+,d4
		muls    d2,d0                   ;d0 = screenwidth * pspan->v
		add.l   d1,d0
		add.l   d0,a0                   ;pdest = d_viewbuffer + pspan->u + d0
		lea     .szstpu(sp),a6          ;a6 -> stackframe
		fmove.s (a6)+,fp0
		fmul    fp2,fp0                 ;fp0 = du * d_sdivzstepu
		fmove.s (a6)+,fp1
		fmul    fp7,fp1                 ;fp1 = dv * d_sdivzstepv
		fadd    fp1,fp0
		fadd.s  (a6)+,fp0               ;sdivz = d_sdivzorigin + fp0 + fp1
		fmove.s (a6)+,fp1
		fmul    fp2,fp1                 ;fp1 = du * d_tdivzstepu
		fmove.s (a6)+,fp6
		fmul    fp7,fp6                 ;fp6 = dv * d_tdivzstepv
		fadd    fp6,fp1
		fadd.s  (a6)+,fp1               ;tdivz = d_tdivzorigin + fp1 + fp6
		fmul.s  (a6)+,fp2               ;fp2 = du * d_zistepu
		fmul.s  (a6)+,fp7               ;fp7 = dv * d_zistepv
		fadd    fp7,fp2
		fadd.s  (a6)+,fp2               ;zi = d_ziorigin + fp2 + fp7
		fmove.s #65536,fp6
		fdiv    fp2,fp6                 ;z = (float)0x10000 / zi

*                s = (int)(sdivz * z) + sadjust;
*                if (s > bbextents)
*                        s = bbextents;
*                else if (s < 0)
*                        s = 0;
*
*                t = (int)(tdivz * z) + tadjust;
*                if (t > bbextentt)
*                        t = bbextentt;
*                else if (t < 0)
*                        t = 0;


		fmove   fp6,fp7
		fmul    fp0,fp7                 ;fp7 = sdivz * z
		fmove.l fp7,d6                  ;convert to integer
		add.l   a5,d6                   ;s = d6 + sadjust
		cmp.l   a4,d6                   ;if (s > bbextents)
		bgt.b   .down
		tst.l   d6                      ;if (s < 0)
		bge.b   .keep
.up
		moveq   #0,d6                   ;s = 0
		bra.b   .keep
.down
		move.l  a4,d6                   ;s = bbextents
.keep
		fmul    fp1,fp6                 ;fp6 = tdivz * z
		fmove.l fp6,d7                  ;convert to integer
		add.l   a3,d7                   ;t = d7 + tadjust
		cmp.l   a2,d7                   ;if (t > bbextentt)
		bgt.b   .down2
		tst.l   d7                      ;if (t < 0)
		bge.b   .keep2
.up2
		moveq   #0,d7                   ;t = 0
		bra.b   .keep2
.down2
		move.l  a2,d7                   ;t = bbextentt
.keep2
		move.l  d4,d1

******  Second loop. In every iteration one part of the whole span is drawn
******  d2 gets the value (spancount-1)! [NOT spancount]

******  d1 = count

*                do
*                {
*                // calculate s and t at the far end of the span
*                        if (count >= 16)
*                                spancount = 16;
*                        else
*                                spancount = count;
*
*                        count -= spancount;
*
*                        if (count)
*                        {

.loop2
		moveq   #16-1,d2                ;spancount = 16
		cmp.l   #16,d1                  ;if (count >= 16)
		bgt.b   .cont
		move.l  d1,d2                   ;spancount = count
		subq.l  #1,d2
		moveq   #0,d1                   ;count -= spancount
		bra.w   .finalpart
.cont
		sub.l   #16,d1                  ;count -= spancount;

******  Evaluation of the values for the inner loop. This version is used for
******  span size = 16

******  a2  : bbextentt
******  a3  : tadjust
******  a4  : bbextents
******  a5  : sadjust
******  fp0 : sdivz
******  fp1 : tdivz
******  fp2 : zi
******  fp3 : sdivz16stepu
******  fp4 : tdivz16stepu
******  fp5 : zi16stepu

*                        // calculate s/z, t/z, zi->fixed s and t at far end of span,
*                        // calculate s and t steps across span by shifting
*                                sdivz += sdivz16stepu;
*                                tdivz += tdivz16stepu;
*                                zi += zi16stepu;
*                                z = (float)0x10000 / zi;        // prescale to 16.16 fixed-point
*                                snext = (int)(sdivz * z) + sadjust;
*                                if (snext > bbextents)
*                                        snext = bbextents;
*                                else if (snext < 16)
*                                        snext = 16;      // prevent round-off error on <0 steps from
*                                                                //  from causing overstepping & running off the
*                                                                //  edge of the texture
*                                tnext = (int)(tdivz * z) + tadjust;
*                                if (tnext > bbextentt)
*                                        tnext = bbextentt;
*                                else if (tnext < 16)
*                                        tnext = 16;      // guard against round-off error on <0 steps
*                                sstep = (snext - s) >> 4;
*                                tstep = (tnext - t) >> 4;
*                        }

		fadd    fp3,fp0                 ;sdivz += sdivz16stepu
		fadd    fp4,fp1                 ;tdivz += tdivz16stepu
		fadd    fp5,fp2                 ;zi += zi16stepu
		fmove.s #65536,fp7
		fdiv    fp2,fp7                 ;z = (float)0x10000 / zi;
		fmove   fp7,fp6
		fmul    fp0,fp6                 ;fp2 = sdivz * z
		fmove.l fp6,d4                  ;convert to integer
		add.l   a5,d4                   ;snext = d4 + sadjust
		cmp.l   a4,d4                   ;if (snext > bbextents)
		bgt.b   .down3
		cmp.l   #16,d4                  ;if (snext < 16)
		bge.b   .keep3
.up3
		moveq   #16,d4                  ;snext = 16
		bra.b   .keep3
.down3
		move.l  a4,d4                   ;snext = bbextents
.keep3
		fmul    fp1,fp7                 ;fp7 = tdivz * z
		fmove.l fp7,d5                  ;convert to integer
		add.l   a3,d5                   ;tnext = d5 + tadjust
		cmp.l   a2,d5                   ;if (tnext > bbextentt)
		bgt.b   .down4
		cmp.l   #16,d5                  ;if (tnext < 16)
		bge.b   .keep4
.up4
		moveq   #16,d5                  ;tnext = 16
		bra.b   .keep4
.down4
		move.l  a2,d5                   ;tnext = bbextentt
.keep4
		move.l  d4,.saved4(sp)          ;save snext
		move.l  d5,.saved5(sp)          ;save tnext
		sub.l   d6,d4                   ;d4 = snext - s
		sub.l   d7,d5                   ;d5 = tnext - t
		asr.l   #4,d4                   ;sstep = d4 >> 4
		asr.l   #4,d5                   ;tstep = d5 >> 4
		bra.w   .mainloop


******  Evaluation of the values for the inner loop. This version is used for
******  span size < 16

******  The original algorithm has two ugly divisions at the end of this part.
******  These are removed by the following optimization:
******  First, the divisors 1,2 and 4 are handled specially to gain speed. The
******  other divisors are handled using a reciprocal table.

******  a2  : bbextentt
******  a3  : tadjust
******  a4  : bbextents
******  a5  : sadjust
******  fp0 : sdivz
******  fp1 : tdivz
******  fp2 : zi

*                        // calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
*                        // can't step off polygon), clamp, calculate s and t steps across
*                        // span by division, biasing steps low so we don't run off the
*                        // texture
*                                spancountminus1 = (float)(spancount - 1);
*                                sdivz += d_sdivzstepu * spancountminus1;
*                                tdivz += d_tdivzstepu * spancountminus1;
*                                zi += d_zistepu * spancountminus1;
*                                z = (float)0x10000 / zi;        // prescale to 16.16 fixed-point
*                                snext = (int)(sdivz * z) + sadjust;
*                                if (snext > bbextents)
*                                        snext = bbextents;
*                                else if (snext < 16)
*                                        snext = 16;      // prevent round-off error on <0 steps from
*                                                                //  from causing overstepping & running off the
*                                                                //  edge of the texture
*
*                                tnext = (int)(tdivz * z) + tadjust;
*                                if (tnext > bbextentt)
*                                        tnext = bbextentt;
*                                else if (tnext < 16)
*                                        tnext = 16;      // guard against round-off error on <0 steps
*
*                                if (spancount > 1)
*                                {
*                                        sstep = (snext - s) / (spancount - 1);
*                                        tstep = (tnext - t) / (spancount - 1);
*                                }
*                        }

.finalpart
		fmove.l d2,fp7                  ;spancountminus1 = (float)(spancount-1)
		fmove   fp7,fp6
		fmul.s  .szstpu(sp),fp6         ;fp6 = d_sdivzstepu * spancountminus1
		fadd    fp6,fp0                 ;sdivz += fp6
		fmove   fp7,fp6
		fmul.s  .tzstpu(sp),fp6         ;fp6 = d_tdivzstepu * spancountminus1
		fadd    fp6,fp1                 ;tdivz += fp6
		fmul.s  .zistpu(sp),fp7         ;fp7 = d_zistepu * spancountminus1
		fadd    fp7,fp2                 ;zi += fp7
		fmove.s #65536,fp7
		fdiv    fp2,fp7                 ;z = (float)0x10000 / zi;
		fmove   fp7,fp6
		fmul    fp0,fp6                 ;fp6 = sdivz * z
		fmove.l fp6,d4                  ;convert to integer
		add.l   a5,d4                   ;snext = d4 + sadjust
		cmp.l   a4,d4                   ;if (snext > bbextents)
		bgt.b   .down5
		cmp.l   #16,d4                  ;if (snext < 16)
		bge.b   .keep5
.up5
		moveq   #16,d4                  ;snext = 16
		bra.b   .keep5
.down5
		move.l  a4,d4                   ;snext = bbextents
.keep5
		fmul    fp1,fp7                 ;fp7 = tdivz * z
		fmove.l fp7,d5                  ;convert to integer
		add.l   a3,d5                   ;tnext = d5 + tadjust
		cmp.l   a2,d5                   ;if (tnext > bbextentt)
		bgt.b   .down6
		cmp.l   #16,d5                  ;if (tnext < 16)
		bge.b   .keep6
.up6
		moveq   #16,d5                  ;tnext = 16
		bra.b   .keep6
.down6
		move.l  a2,d5                   ;tnext = bbextentt
.keep6
		move.l  d4,.saved4(sp)          ;save snext
		move.l  d5,.saved5(sp)          ;save tnext
		sub.l   d6,d4                   ;d4 = snext - s
		sub.l   d7,d5                   ;d5 = tnext - t
		IFEQ    QDIV
		tst.l   d2
		beq.w   .mainloop
		divs.l  d2,d4
		divs.l  d2,d5
		ELSEIF
		cmp     #5,d2                   ;(spancount-1) < 5?
		blt.b   .special                ;yes -> special case
		cmp     #8,d2
		beq.b   .spec_8
.qdiv
		IFNE    NICE_DIV
		lsl.l   #2,d4
		lsl.l   #2,d5
		lea     ReciprocTable,a6
		move    0(a6,d2.w*2),d0
		move.l  d4,d3
		mulu    d0,d3
		clr     d3
		swap    d3
		swap    d4
		muls    d0,d4
		add.l   d3,d4
		move.l  d5,d3
		mulu    d0,d3
		clr     d3
		swap    d3
		swap    d5
		muls    d0,d5
		add.l   d3,d5
		bra.b   .mainloop
		ELSEIF
		asr.l   #7,d4                   ;d4 >> 7
		asr.l   #7,d5                   ;d5 >> 7
		lea     ReciprocTable,a6        ;a6 -> reciprocal table
		move    0(a6,d2.w*2),d0         ;d0 = (1/(spancount-1))<<16
		muls    d0,d4                   ;d4 = d4 / (spancount-1)
		asr.l   #7,d4                   ;sstep = d4 >> 7
		muls    d0,d5                   ;d5 = d5 / (spancount-1)
		asr.l   #7,d5                   ;tstep = d5 >> 7
		bra.b   .mainloop
		ENDC
.special
		cmp     #1,d2                   ;switch (spancount-1)
		ble.b   .mainloop               ;0,1 -> no scaling needed
		cmp     #3,d2                   ;3 -> standard qdiv
		beq.b   .qdiv
		blt.b   .spec_2
		asr.l   #2,d4                   ;4 -> scale by shifting right
		asr.l   #2,d5
		bra.b   .mainloop
.spec_8
		asr.l   #3,d4                   ;8 -> scale by shifting right
		asr.l   #3,d5
		bra.b   .mainloop
.spec_2
		asr.l   #1,d4                   ;2 -> scale by shifting right
		asr.l   #1,d5
		ENDC

******  Main drawing loop. Here lies the speed.
******  Very optimized (removed multiplication from inner loop)

******  d2 : spancount
******  d4 : sstep
******  d5 : tstep
******  d6 : s
******  d7 : t
******  a0 : pdest
******  a1 : pbase

*                        do
*                        {
*                                *pdest++ = *(pbase + (s >> 16) + (t >> 16) * cachewidth);
*                                s += sstep;
*                                t += tstep;
*                        } while (--spancount > 0);

.mainloop
		move.l  d1,-(sp)
		lea     .PixTable,a6            ;a6 -> Functable
		move.l  _cachewidth,d3          ;read cachewidth
		move.l  0(a6,d2.w*4),a6         ;get pointer to function
		swap    d7
		swap    d4
		move.l  d7,d1
		swap    d5
		muls    d3,d7                   ;d7 = t integer part * cachewidth
		move    d5,d2
		clr     d1                      ;d1 = t fractional part
		muls    d3,d2                   ;tstep integer part * cachewidth
		move    d4,d0                   ;d0 = sstep integer part
		clr     d5                      ;d5 = tstep fractional part
		clr     d4                      ;d4 = sstep fractional part
		swap    d6                      ;d6 = s swapped
		jmp     (a6)
.Pix16
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6                   ;increment s fractional part
		addx.w  d0,d6                   ;increment s integer part
		add.l   d2,d7                   ;increment t integer part
		add.l   d5,d1                   ;increment t fractional part
		bcc.b   .Pix15                   ;check if carry
		add.l   d3,d7                   ;add cachewidth to t
.Pix15
		lea     0(a1,d6.w),a6           ;and so long...
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix14
		add.l   d3,d7
.Pix14
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix13
		add.l   d3,d7
.Pix13
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix12
		add.l   d3,d7
.Pix12
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix11
		add.l   d3,d7
.Pix11
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix10
		add.l   d3,d7
.Pix10
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix9
		add.l   d3,d7
.Pix9
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix8
		add.l   d3,d7
.Pix8
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix7
		add.l   d3,d7
.Pix7
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix6
		add.l   d3,d7
.Pix6
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix5
		add.l   d3,d7
.Pix5
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix4
		add.l   d3,d7
.Pix4
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix3
		add.l   d3,d7
.Pix3
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix2
		add.l   d3,d7
.Pix2
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix1
		add.l   d3,d7
.Pix1
		lea     0(a1,d6.w),a6
		move.b  0(a6,d7.l),(a0)+
		add.l   d4,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix0
		add.l   d3,d7
.Pix0
		move.l  (sp)+,d1

******  loop terminations


		move.l   .saved5(sp),d7         ;t = tnext
		move.l   .saved4(sp),d6         ;s = snext

		tst.l   d1                      ;while (count > 0)
		bgt.w   .loop2

		move.l  .savea6(sp),a6          ;while ((pspan = pspan->next) != NULL)
		move.l  SPAN_PNEXT(a6),a6
		tst.l   a6
		bne.w   .loop
		add.l   #.fpuregs,sp
		fmovem.x        (sp)+,fp2-fp7
		movem.l (sp)+,d2-d7/a2-a6
		rts

.PixTable
		dc.l    .Pix1
		dc.l    .Pix2
		dc.l    .Pix3
		dc.l    .Pix4
		dc.l    .Pix5
		dc.l    .Pix6
		dc.l    .Pix7
		dc.l    .Pix8
		dc.l    .Pix9
		dc.l    .Pix10
		dc.l    .Pix11
		dc.l    .Pix12
		dc.l    .Pix13
		dc.l    .Pix14
		dc.l    .Pix15
		dc.l    .Pix16








******************************************************************************
*
*       void D_DrawSpans16T (espan_t *pspan)
*
*       translucent scan drawing function (16 pixel subdivision)
*
******************************************************************************

		cnop    0,4
_D_DrawSpans16T


*****   stackframe

		rsreset
.izistep        rs.l    1
.izi            rs.l    1
.pz             rs.l    1
.saved4         rs.l    1
.saved5         rs.l    1
.savea6         rs.l    1
.szstpu         rs.s    1
.szstpv         rs.s    1
.szorg          rs.s    1
.tzstpu         rs.s    1
.tzstpv         rs.s    1
.tzorg          rs.s    1
.zistpu         rs.s    1
.zistpv         rs.s    1
.ziorg          rs.s    1
.fpuregs        rs.x    6
.intregs        rs.l    11
		rs.l    1
.pspan          rs.l    1

******  Prologue. Global variables are put into registers or onto the stackframe

		movem.l d2-d7/a2-a6,-(sp)
		fmovem.x        fp2-fp7,-(sp)
		move.l  _bbextentt,a2
		move.l  _tadjust,a3
		move.l  _bbextents,a4
		move.l  _sadjust,a5
		move.l  _d_ziorigin,-(sp)
		move.l  _d_zistepv,-(sp)
		move.l  _d_zistepu,-(sp)
		move.l  _d_tdivzorigin,-(sp)
		move.l  _d_tdivzstepv,-(sp)
		move.l  _d_tdivzstepu,-(sp)
		move.l  _d_sdivzorigin,-(sp)
		move.l  _d_sdivzstepv,-(sp)
		move.l  _d_sdivzstepu,-(sp)
		sub.l   #.szstpu,sp

******  First loop. In every iteration one complete span is drawn

*        pbase = (unsigned char *)cacheblock;
*
*        sdivz16stepu = d_sdivzstepu * 16;
*        tdivz16stepu = d_tdivzstepu * 16;
*        zi16stepu = d_zistepu * 16;
*
*        izistep = (int)(d_zistepu * 0x8000 * 0x10000);
*
*        do
*        {
*                pz = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u;
*                pdest = (unsigned char *)((byte *)d_viewbuffer +
*                                (screenwidth * pspan->v) + pspan->u);
*
*                count = pspan->count;
*
*        // calculate the initial s/z, t/z, 1/z, s, and t and clamp
*                du = (float)pspan->u;
*                dv = (float)pspan->v;
*
*                sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
*                tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
*                zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
*                izi = (int)(zi * 0x8000 * 0x10000);
*                z = (float)0x10000 / zi;        // prescale to 16.16 fixed-point
*

		move.l  _cacheblock,a1          ;pbase = (unsigned char *)cacheblock
		fmove.s #16,fp7
		fmove.s .szstpu(sp),fp3
		fmul    fp7,fp3                 ;sdivz16stepu = d_sdivzstepu * 16
		fmove.s .tzstpu(sp),fp4
		fmul    fp7,fp4                 ;tdivz16stepu = d_tdivzstepu * 16
		fmove.s .zistpu(sp),fp5
		fmul    fp7,fp5                 ;zi16stepu = d_zistepu * 16
; translucent
		fmove.s #32768*65536,fp0
		fmove.s .zistpu(sp),fp7         ;fp7 = d_zistepu
		fmul    fp0,fp7                 ;multiply by $8000*$10000
		fmove.l fp7,d3                  ;izistep = d3
		move.l  d3,.izistep(sp)
; translucent
		move.l  .pspan(sp),a6           ;get function parameter
.loop
		move.l  a6,.savea6(sp)          ;save actual ptr to pspan
		move.l  _d_viewbuffer,a0
		move.l  _screenwidth,d0
		move.l  (a6)+,d1
		fmove.l d1,fp2                  ;du = (float)pspan->u
		move.l  (a6)+,d2
		fmove.l d2,fp7                  ;dv = (float)pspan->v
		move.l  (a6)+,d4
; translucent
		move.l  _d_pzbuffer,a6
		move.l  _d_zwidth,d7
		move.l  d2,d3                   ;d3 = pspan->v
		muls    d7,d3                   ;d3 = pspan->v * d_zwidth
		add.l   d1,d3                   ;d3 = d3 + pspan->u
		lea     0(a6,d3.l*2),a6         ;pdest = d_pzbuffer + d3
		move.l  a6,.pz(sp)
; translucent
		muls    d2,d0                   ;d0 = screenwidth * pspan->v
		add.l   d1,d0
		add.l   d0,a0                   ;pdest = d_viewbuffer + pspan->u + d0
		lea     .szstpu(sp),a6          ;a6 -> stackframe
		fmove.s (a6)+,fp0
		fmul    fp2,fp0                 ;fp0 = du * d_sdivzstepu
		fmove.s (a6)+,fp1
		fmul    fp7,fp1                 ;fp1 = dv * d_sdivzstepv
		fadd    fp1,fp0
		fadd.s  (a6)+,fp0               ;sdivz = d_sdivzorigin + fp0 + fp1
		fmove.s (a6)+,fp1
		fmul    fp2,fp1                 ;fp1 = du * d_tdivzstepu
		fmove.s (a6)+,fp6
		fmul    fp7,fp6                 ;fp6 = dv * d_tdivzstepv
		fadd    fp6,fp1
		fadd.s  (a6)+,fp1               ;tdivz = d_tdivzorigin + fp1 + fp6
		fmul.s  (a6)+,fp2               ;fp2 = du * d_zistepu
		fmul.s  (a6)+,fp7               ;fp7 = dv * d_zistepv
		fadd    fp7,fp2
		fadd.s  (a6)+,fp2               ;zi = d_ziorigin + fp2 + fp7
; translucent
		fmove.s #32768*65536,fp6
		fmove.l fp2,fp7                 ;fp7 = zi
		fmul    fp7,fp6                 ;izi = zi * $8000 * $10000
		fmove.l fp6,d3                  ;convert to integer
		move.l  d3,.izi(sp)
; translucent
		fmove.s #65536,fp6
		fdiv    fp2,fp6                 ;z = (float)0x10000 / zi



*                s = (int)(sdivz * z) + sadjust;
*                if (s > bbextents)
*                        s = bbextents;
*                else if (s < 0)
*                        s = 0;
*
*                t = (int)(tdivz * z) + tadjust;
*                if (t > bbextentt)
*                        t = bbextentt;
*                else if (t < 0)
*                        t = 0;


		fmove   fp6,fp7
		fmul    fp0,fp7                 ;fp7 = sdivz * z
		fmove.l fp7,d6                  ;convert to integer
		add.l   a5,d6                   ;s = d6 + sadjust
		cmp.l   a4,d6                   ;if (s > bbextents)
		bgt.b   .down
		tst.l   d6                      ;if (s < 0)
		bge.b   .keep
.up
		moveq   #0,d6                   ;s = 0
		bra.b   .keep
.down
		move.l  a4,d6                   ;s = bbextents
.keep
		fmul    fp1,fp6                 ;fp6 = tdivz * z
		fmove.l fp6,d7                  ;convert to integer
		add.l   a3,d7                   ;t = d7 + tadjust
		cmp.l   a2,d7                   ;if (t > bbextentt)
		bgt.b   .down2
		tst.l   d7                      ;if (t < 0)
		bge.b   .keep2
.up2
		moveq   #0,d7                   ;t = 0
		bra.b   .keep2
.down2
		move.l  a2,d7                   ;t = bbextentt
.keep2
		move.l  d4,d1

******  Second loop. In every iteration one part of the whole span is drawn
******  d2 gets the value (spancount-1)! [NOT spancount]

******  d1 = count

*                do
*                {
*                // calculate s and t at the far end of the span
*                        if (count >= 16)
*                                spancount = 16;
*                        else
*                                spancount = count;
*
*                        count -= spancount;
*
*                        if (count)
*                        {

.loop2
		moveq   #16-1,d2                ;spancount = 16
		cmp.l   #16,d1                  ;if (count >= 16)
		bgt.b   .cont
		move.l  d1,d2                   ;spancount = count
		subq.l  #1,d2
		moveq   #0,d1                   ;count -= spancount
		bra.w   .finalpart
.cont
		sub.l   #16,d1                  ;count -= spancount;

******  Evaluation of the values for the inner loop. This version is used for
******  span size = 16

******  a2  : bbextentt
******  a3  : tadjust
******  a4  : bbextents
******  a5  : sadjust
******  fp0 : sdivz
******  fp1 : tdivz
******  fp2 : zi
******  fp3 : sdivz16stepu
******  fp4 : tdivz16stepu
******  fp5 : zi16stepu

*                        // calculate s/z, t/z, zi->fixed s and t at far end of span,
*                        // calculate s and t steps across span by shifting
*                                sdivz += sdivz16stepu;
*                                tdivz += tdivz16stepu;
*                                zi += zi16stepu;
*                                z = (float)0x10000 / zi;        // prescale to 16.16 fixed-point
*                                snext = (int)(sdivz * z) + sadjust;
*                                if (snext > bbextents)
*                                        snext = bbextents;
*                                else if (snext < 16)
*                                        snext = 16;      // prevent round-off error on <0 steps from
*                                                                //  from causing overstepping & running off the
*                                                                //  edge of the texture
*                                tnext = (int)(tdivz * z) + tadjust;
*                                if (tnext > bbextentt)
*                                        tnext = bbextentt;
*                                else if (tnext < 16)
*                                        tnext = 16;      // guard against round-off error on <0 steps
*                                sstep = (snext - s) >> 4;
*                                tstep = (tnext - t) >> 4;
*                        }

		fadd    fp3,fp0                 ;sdivz += sdivz16stepu
		fadd    fp4,fp1                 ;tdivz += tdivz16stepu
		fadd    fp5,fp2                 ;zi += zi16stepu
		fmove.s #65536,fp7
		fdiv    fp2,fp7                 ;z = (float)0x10000 / zi;
		fmove   fp7,fp6
		fmul    fp0,fp6                 ;fp2 = sdivz * z
		fmove.l fp6,d4                  ;convert to integer
		add.l   a5,d4                   ;snext = d4 + sadjust
		cmp.l   a4,d4                   ;if (snext > bbextents)
		bgt.b   .down3
		cmp.l   #16,d4                  ;if (snext < 16)
		bge.b   .keep3
.up3
		moveq   #16,d4                  ;snext = 16
		bra.b   .keep3
.down3
		move.l  a4,d4                   ;snext = bbextents
.keep3
		fmul    fp1,fp7                 ;fp7 = tdivz * z
		fmove.l fp7,d5                  ;convert to integer
		add.l   a3,d5                   ;tnext = d5 + tadjust
		cmp.l   a2,d5                   ;if (tnext > bbextentt)
		bgt.b   .down4
		cmp.l   #16,d5                  ;if (tnext < 16)
		bge.b   .keep4
.up4
		moveq   #16,d5                  ;tnext = 16
		bra.b   .keep4
.down4
		move.l  a2,d5                   ;tnext = bbextentt
.keep4
		move.l  d4,.saved4(sp)          ;save snext
		move.l  d5,.saved5(sp)          ;save tnext
		sub.l   d6,d4                   ;d4 = snext - s
		sub.l   d7,d5                   ;d5 = tnext - t
		asr.l   #4,d4                   ;sstep = d4 >> 4
		asr.l   #4,d5                   ;tstep = d5 >> 4
		bra.w   .mainloop


******  Evaluation of the values for the inner loop. This version is used for
******  span size < 16

******  The original algorithm has two ugly divisions at the end of this part.
******  These are removed by the following optimization:
******  First, the divisors 1,2 and 4 are handled specially to gain speed. The
******  other divisors are handled using a reciprocal table.

******  a2  : bbextentt
******  a3  : tadjust
******  a4  : bbextents
******  a5  : sadjust
******  fp0 : sdivz
******  fp1 : tdivz
******  fp2 : zi

*                        // calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
*                        // can't step off polygon), clamp, calculate s and t steps across
*                        // span by division, biasing steps low so we don't run off the
*                        // texture
*                                spancountminus1 = (float)(spancount - 1);
*                                sdivz += d_sdivzstepu * spancountminus1;
*                                tdivz += d_tdivzstepu * spancountminus1;
*                                zi += d_zistepu * spancountminus1;
*                                z = (float)0x10000 / zi;        // prescale to 16.16 fixed-point
*                                snext = (int)(sdivz * z) + sadjust;
*                                if (snext > bbextents)
*                                        snext = bbextents;
*                                else if (snext < 16)
*                                        snext = 16;      // prevent round-off error on <0 steps from
*                                                                //  from causing overstepping & running off the
*                                                                //  edge of the texture
*
*                                tnext = (int)(tdivz * z) + tadjust;
*                                if (tnext > bbextentt)
*                                        tnext = bbextentt;
*                                else if (tnext < 16)
*                                        tnext = 16;      // guard against round-off error on <0 steps
*
*                                if (spancount > 1)
*                                {
*                                        sstep = (snext - s) / (spancount - 1);
*                                        tstep = (tnext - t) / (spancount - 1);
*                                }
*                        }

.finalpart
		fmove.l d2,fp7                  ;spancountminus1 = (float)(spancount-1)
		fmove   fp7,fp6
		fmul.s  .szstpu(sp),fp6         ;fp6 = d_sdivzstepu * spancountminus1
		fadd    fp6,fp0                 ;sdivz += fp6
		fmove   fp7,fp6
		fmul.s  .tzstpu(sp),fp6         ;fp6 = d_tdivzstepu * spancountminus1
		fadd    fp6,fp1                 ;tdivz += fp6
		fmul.s  .zistpu(sp),fp7         ;fp7 = d_zistepu * spancountminus1
		fadd    fp7,fp2                 ;zi += fp7
		fmove.s #65536,fp7
		fdiv    fp2,fp7                 ;z = (float)0x10000 / zi;
		fmove   fp7,fp6
		fmul    fp0,fp6                 ;fp6 = sdivz * z
		fmove.l fp6,d4                  ;convert to integer
		add.l   a5,d4                   ;snext = d4 + sadjust
		cmp.l   a4,d4                   ;if (snext > bbextents)
		bgt.b   .down5
		cmp.l   #16,d4                  ;if (snext < 16)
		bge.b   .keep5
.up5
		moveq   #16,d4                  ;snext = 16
		bra.b   .keep5
.down5
		move.l  a4,d4                   ;snext = bbextents
.keep5
		fmul    fp1,fp7                 ;fp7 = tdivz * z
		fmove.l fp7,d5                  ;convert to integer
		add.l   a3,d5                   ;tnext = d5 + tadjust
		cmp.l   a2,d5                   ;if (tnext > bbextentt)
		bgt.b   .down6
		cmp.l   #16,d5                  ;if (tnext < 16)
		bge.b   .keep6
.up6
		moveq   #16,d5                  ;tnext = 16
		bra.b   .keep6
.down6
		move.l  a2,d5                   ;tnext = bbextentt
.keep6
		move.l  d4,.saved4(sp)          ;save snext
		move.l  d5,.saved5(sp)          ;save tnext
		sub.l   d6,d4                   ;d4 = snext - s
		sub.l   d7,d5                   ;d5 = tnext - t
		IFEQ    QDIV
		tst.l   d2
		beq.w   .mainloop
		divs.l  d2,d4
		divs.l  d2,d5
		ELSEIF
		cmp     #5,d2                   ;(spancount-1) < 5?
		blt.b   .special                ;yes -> special case
		cmp     #8,d2
		beq.b   .spec_8
.qdiv
		IFNE    NICE_DIV
		lsl.l   #2,d4
		lsl.l   #2,d5
		lea     ReciprocTable,a6
		move    0(a6,d2.w*2),d0
		move.l  d4,d3
		mulu    d0,d3
		clr     d3
		swap    d3
		swap    d4
		muls    d0,d4
		add.l   d3,d4
		move.l  d5,d3
		mulu    d0,d3
		clr     d3
		swap    d3
		swap    d5
		muls    d0,d5
		add.l   d3,d5
		bra.b   .mainloop
		ELSEIF
		asr.l   #7,d4                   ;d4 >> 7
		asr.l   #7,d5                   ;d5 >> 7
		lea     ReciprocTable,a6        ;a6 -> reciprocal table
		move    0(a6,d2.w*2),d0         ;d0 = (1/(spancount-1))<<16
		muls    d0,d4                   ;d4 = d4 / (spancount-1)
		asr.l   #7,d4                   ;sstep = d4 >> 7
		muls    d0,d5                   ;d5 = d5 / (spancount-1)
		asr.l   #7,d5                   ;tstep = d5 >> 7
		bra.b   .mainloop
		ENDC
.special
		cmp     #1,d2                   ;switch (spancount-1)
		ble.b   .mainloop               ;0,1 -> no scaling needed
		cmp     #3,d2                   ;3 -> standard qdiv
		beq.b   .qdiv
		blt.b   .spec_2
		asr.l   #2,d4                   ;4 -> scale by shifting right
		asr.l   #2,d5
		bra.b   .mainloop
.spec_8
		asr.l   #3,d4                   ;8 -> scale by shifting right
		asr.l   #3,d5
		bra.b   .mainloop
.spec_2
		asr.l   #1,d4                   ;2 -> scale by shifting right
		asr.l   #1,d5
		ENDC

******  Main drawing loop. Here lies the speed.
******  Very optimized (removed multiplication from inner loop)

******  d2 : spancount
******  d4 : sstep
******  d5 : tstep
******  d6 : s
******  d7 : t
******  a0 : pdest
******  a1 : pbase

*                        do
*                        {
*                                btemp = *(pbase + (s >> 16) + (t >> 16) * cachewidth);
*                                if (*pz <= (izi >> 16))
*                                {
*                                    *pdest = mainTransTable[(btemp<<8) + (*pdest)];
*                                }
*                                izi += izistep;
*                                pdest++;
*                                pz++;
*                                s += sstep;
*                                t += tstep;
*                        } while (--spancount > 0);

.mainloop
		;move.l  d1,-(sp)
		movem.l d1/a2-a5,-(sp)
		lea     .PixTable,a6            ;a6 -> Functable
		move.l  _cachewidth,d3          ;read cachewidth
		move.l  0(a6,d2.w*4),a6         ;get pointer to function
		swap    d7
		swap    d4
		move.l  d7,d1
		swap    d5
		muls    d3,d7                   ;d7 = t integer part * cachewidth
		move    d5,d2
		clr     d1                      ;d1 = t fractional part
		muls    d3,d2                   ;d2 = tstep integer part * cachewidth
		move    d4,d0                   ;d0 = sstep integer part
		clr     d5                      ;d5 = tstep fractional part
		clr     d4                      ;d4 = sstep fractional part
		swap    d6                      ;d6 = s swapped
; translucent
		move.l  d3,a2                   ;a2 = cachewidth
		move.l  _mainTransTable,a3
		move.l  .pz(sp),a4              ;a4 = pz
		move.l  d4,a5                   ;a5 = sstep fractional part
; translucent
		jmp     (a6)
.Pix16
		;lea     0(a1,d6.w),a6
		;move.b  0(a6,d7.l),(a0)+
; translucent
		move.l  .izi(sp),d3
		swap    d3                      ;d3 = izi >> 16
		cmp     (a4),d3                 ;if (*pz <= (izi >> 16))
		blt.b   .Pix16_next
		lea     0(a1,d6.w),a6
		clr.l   d3
		move.b  (a0),d3                 ;d3 = *pdest
		clr.l   d4
		move.b  0(a6,d7.l),d4           ;btemp = *(pbase + (s >> 16) + (t >> 16) * cachewidth)
		lsl.l   #8,d4
		add.l   d4,d3                   ;(btemp<<8) + (*pdest)
		move.b  0(a3,d3.l),(a0)
.Pix16_next
		addq.l  #1,a0
		move.l  .izistep(sp),d3
		add.l   d3,.izi(sp)             ;izi += izistep
; translucent
		add.l   a5,d6                   ;increment s fractional part
		addx.w  d0,d6                   ;increment s integer part
		add.l   d2,d7                   ;increment t integer part
		add.l   d5,d1                   ;increment t fractional part
		bcc.b   .Pix15                   ;check if carry
		add.l   a2,d7                   ;add cachewidth to t
.Pix15
		;lea     0(a1,d6.w),a6           ;and so long...
		;move.b  0(a6,d7.l),(a0)+
; translucent
		move.l  .izi(sp),d3
		swap    d3
		cmp     (a4),d3
		blt.b   .Pix15_next
		lea     0(a1,d6.w),a6
		clr.l   d3
		move.b  (a0),d3
		clr.l   d4
		move.b  0(a6,d7.l),d4 
		lsl.l   #8,d4
		add.l   d4,d3
		move.b  0(a3,d3.l),(a0)
.Pix15_next
		addq.l  #1,a0
		move.l  .izistep(sp),d3
		add.l   d3,.izi(sp)
; translucent
		add.l   a5,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix14
		add.l   a2,d7
.Pix14
		;lea     0(a1,d6.w),a6
		;move.b  0(a6,d7.l),(a0)+
; translucent
		move.l  .izi(sp),d3
		swap    d3
		cmp     (a4),d3
		blt.b   .Pix14_next
		lea     0(a1,d6.w),a6
		clr.l   d3
		move.b  (a0),d3
		clr.l   d4
		move.b  0(a6,d7.l),d4 
		lsl.l   #8,d4
		add.l   d4,d3
		move.b  0(a3,d3.l),(a0)
.Pix14_next
		addq.l  #1,a0
		move.l  .izistep(sp),d3
		add.l   d3,.izi(sp)
; translucent
		add.l   a5,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix13
		add.l   a2,d7
.Pix13
		;lea     0(a1,d6.w),a6
		;move.b  0(a6,d7.l),(a0)+
; translucent
		move.l  .izi(sp),d3
		swap    d3
		cmp     (a4),d3
		blt.b   .Pix13_next
		lea     0(a1,d6.w),a6
		clr.l   d3
		move.b  (a0),d3
		clr.l   d4
		move.b  0(a6,d7.l),d4 
		lsl.l   #8,d4
		add.l   d4,d3
		move.b  0(a3,d3.l),(a0)
.Pix13_next
		addq.l  #1,a0
		move.l  .izistep(sp),d3
		add.l   d3,.izi(sp)
; translucent
		add.l   a5,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix12
		add.l   a2,d7
.Pix12
		;lea     0(a1,d6.w),a6
		;move.b  0(a6,d7.l),(a0)+
; translucent
		move.l  .izi(sp),d3
		swap    d3
		cmp     (a4),d3
		blt.b   .Pix12_next
		lea     0(a1,d6.w),a6
		clr.l   d3
		move.b  (a0),d3
		clr.l   d4
		move.b  0(a6,d7.l),d4 
		lsl.l   #8,d4
		add.l   d4,d3
		move.b  0(a3,d3.l),(a0)
.Pix12_next
		addq.l  #1,a0
		move.l  .izistep(sp),d3
		add.l   d3,.izi(sp)
; translucent
		add.l   a5,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix11
		add.l   a2,d7
.Pix11
		;lea     0(a1,d6.w),a6
		;move.b  0(a6,d7.l),(a0)+
; translucent
		move.l  .izi(sp),d3
		swap    d3
		cmp     (a4),d3
		blt.b   .Pix11_next
		lea     0(a1,d6.w),a6
		clr.l   d3
		move.b  (a0),d3
		clr.l   d4
		move.b  0(a6,d7.l),d4 
		lsl.l   #8,d4
		add.l   d4,d3
		move.b  0(a3,d3.l),(a0)
.Pix11_next
		addq.l  #1,a0
		move.l  .izistep(sp),d3
		add.l   d3,.izi(sp)
; translucent
		add.l   a5,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix10
		add.l   a2,d7
.Pix10
		;lea     0(a1,d6.w),a6
		;move.b  0(a6,d7.l),(a0)+
; translucent
		move.l  .izi(sp),d3
		swap    d3
		cmp     (a4),d3
		blt.b   .Pix10_next
		lea     0(a1,d6.w),a6
		clr.l   d3
		move.b  (a0),d3
		clr.l   d4
		move.b  0(a6,d7.l),d4 
		lsl.l   #8,d4
		add.l   d4,d3
		move.b  0(a3,d3.l),(a0)
.Pix10_next
		addq.l  #1,a0
		move.l  .izistep(sp),d3
		add.l   d3,.izi(sp)
; translucent
		add.l   a5,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix9
		add.l   a2,d7
.Pix9
		;lea     0(a1,d6.w),a6
		;move.b  0(a6,d7.l),(a0)+
; translucent
		move.l  .izi(sp),d3
		swap    d3
		cmp     (a4),d3
		blt.b   .Pix9_next
		lea     0(a1,d6.w),a6
		clr.l   d3
		move.b  (a0),d3
		clr.l   d4
		move.b  0(a6,d7.l),d4 
		lsl.l   #8,d4
		add.l   d4,d3
		move.b  0(a3,d3.l),(a0)
.Pix9_next
		addq.l  #1,a0
		move.l  .izistep(sp),d3
		add.l   d3,.izi(sp)
; translucent
		add.l   a5,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix8
		add.l   a2,d7
.Pix8
		;lea     0(a1,d6.w),a6
		;move.b  0(a6,d7.l),(a0)+
; translucent
		move.l  .izi(sp),d3
		swap    d3
		cmp     (a4),d3
		blt.b   .Pix8_next
		lea     0(a1,d6.w),a6
		clr.l   d3
		move.b  (a0),d3
		clr.l   d4
		move.b  0(a6,d7.l),d4 
		lsl.l   #8,d4
		add.l   d4,d3
		move.b  0(a3,d3.l),(a0)
.Pix8_next
		addq.l  #1,a0
		move.l  .izistep(sp),d3
		add.l   d3,.izi(sp)
; translucent
		add.l   a5,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix7
		add.l   a2,d7
.Pix7
		;lea     0(a1,d6.w),a6
		;move.b  0(a6,d7.l),(a0)+
; translucent
		move.l  .izi(sp),d3
		swap    d3
		cmp     (a4),d3
		blt.b   .Pix7_next
		lea     0(a1,d6.w),a6
		clr.l   d3
		move.b  (a0),d3
		clr.l   d4
		move.b  0(a6,d7.l),d4 
		lsl.l   #8,d4
		add.l   d4,d3
		move.b  0(a3,d3.l),(a0)
.Pix7_next
		addq.l  #1,a0
		move.l  .izistep(sp),d3
		add.l   d3,.izi(sp)
; translucent
		add.l   a5,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix6
		add.l   a2,d7
.Pix6
		;lea     0(a1,d6.w),a6
		;move.b  0(a6,d7.l),(a0)+
; translucent
		move.l  .izi(sp),d3
		swap    d3
		cmp     (a4),d3
		blt.b   .Pix6_next
		lea     0(a1,d6.w),a6
		clr.l   d3
		move.b  (a0),d3
		clr.l   d4
		move.b  0(a6,d7.l),d4 
		lsl.l   #8,d4
		add.l   d4,d3
		move.b  0(a3,d3.l),(a0)
.Pix6_next
		addq.l  #1,a0
		move.l  .izistep(sp),d3
		add.l   d3,.izi(sp)
; translucent
		add.l   a5,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix5
		add.l   a2,d7
.Pix5
		;lea     0(a1,d6.w),a6
		;move.b  0(a6,d7.l),(a0)+
; translucent
		move.l  .izi(sp),d3
		swap    d3
		cmp     (a4),d3
		blt.b   .Pix5_next
		lea     0(a1,d6.w),a6
		clr.l   d3
		move.b  (a0),d3
		clr.l   d4
		move.b  0(a6,d7.l),d4 
		lsl.l   #8,d4
		add.l   d4,d3
		move.b  0(a3,d3.l),(a0)
.Pix5_next
		addq.l  #1,a0
		move.l  .izistep(sp),d3
		add.l   d3,.izi(sp)
; translucent
		add.l   a5,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix4
		add.l   a2,d7
.Pix4
		;lea     0(a1,d6.w),a6
		;move.b  0(a6,d7.l),(a0)+
; translucent
		move.l  .izi(sp),d3
		swap    d3
		cmp     (a4),d3
		blt.b   .Pix4_next
		lea     0(a1,d6.w),a6
		clr.l   d3
		move.b  (a0),d3
		clr.l   d4
		move.b  0(a6,d7.l),d4 
		lsl.l   #8,d4
		add.l   d4,d3
		move.b  0(a3,d3.l),(a0)
.Pix4_next
		addq.l  #1,a0
		move.l  .izistep(sp),d3
		add.l   d3,.izi(sp)
; translucent
		add.l   a5,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix3
		add.l   a2,d7
.Pix3
		;lea     0(a1,d6.w),a6
		;move.b  0(a6,d7.l),(a0)+
; translucent
		move.l  .izi(sp),d3
		swap    d3
		cmp     (a4),d3
		blt.b   .Pix3_next
		lea     0(a1,d6.w),a6
		clr.l   d3
		move.b  (a0),d3
		clr.l   d4
		move.b  0(a6,d7.l),d4 
		lsl.l   #8,d4
		add.l   d4,d3
		move.b  0(a3,d3.l),(a0)
.Pix3_next
		addq.l  #1,a0
		move.l  .izistep(sp),d3
		add.l   d3,.izi(sp)
; translucent
		add.l   a5,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix2
		add.l   a2,d7
.Pix2
		;lea     0(a1,d6.w),a6
		;move.b  0(a6,d7.l),(a0)+
; translucent
		move.l  .izi(sp),d3
		swap    d3
		cmp     (a4),d3
		blt.b   .Pix2_next
		lea     0(a1,d6.w),a6
		clr.l   d3
		move.b  (a0),d3
		clr.l   d4
		move.b  0(a6,d7.l),d4 
		lsl.l   #8,d4
		add.l   d4,d3
		move.b  0(a3,d3.l),(a0)
.Pix2_next
		addq.l  #1,a0
		move.l  .izistep(sp),d3
		add.l   d3,.izi(sp)
; translucent
		add.l   a5,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix1
		add.l   a2,d7
.Pix1
		;lea     0(a1,d6.w),a6
		;move.b  0(a6,d7.l),(a0)+
; translucent
		move.l  .izi(sp),d3
		swap    d3
		cmp     (a4),d3
		blt.b   .Pix1_next
		lea     0(a1,d6.w),a6
		clr.l   d3
		move.b  (a0),d3
		clr.l   d4
		move.b  0(a6,d7.l),d4 
		lsl.l   #8,d4
		add.l   d4,d3
		move.b  0(a3,d3.l),(a0)
.Pix1_next
		addq.l  #1,a0
		move.l  .izistep(sp),d3
		add.l   d3,.izi(sp)
; translucent
		add.l   a5,d6
		addx.w  d0,d6
		add.l   d2,d7
		add.l   d5,d1
		bcc.b   .Pix0
		add.l   a2,d7
.Pix0
		movem.l (sp)+,d1/a2-a5
		;move.l  (sp)+,d1

******  loop terminations


		move.l   .saved5(sp),d7         ;t = tnext
		move.l   .saved4(sp),d6         ;s = snext

		tst.l   d1                      ;while (count > 0)
		bgt.w   .loop2

		move.l  .savea6(sp),a6          ;while ((pspan = pspan->next) != NULL)
		move.l  SPAN_PNEXT(a6),a6
		tst.l   a6
		bne.w   .loop
		add.l   #.fpuregs,sp
		fmovem.x        (sp)+,fp2-fp7
		movem.l (sp)+,d2-d7/a2-a6
		rts

.PixTable
		dc.l    .Pix1
		dc.l    .Pix2
		dc.l    .Pix3
		dc.l    .Pix4
		dc.l    .Pix5
		dc.l    .Pix6
		dc.l    .Pix7
		dc.l    .Pix8
		dc.l    .Pix9
		dc.l    .Pix10
		dc.l    .Pix11
		dc.l    .Pix12
		dc.l    .Pix13
		dc.l    .Pix14
		dc.l    .Pix15
		dc.l    .Pix16









******************************************************************************
*
*       void D_DrawZSpans (espan_t *pspan)
*
*       standard z-scan drawing function
*
******************************************************************************

		cnop    0,4
_D_DrawZSpans


******  Prologue. Global variables are put into registers or onto the stack

*****   stackframe

		rsreset
.fpuregs        rs.x    5
.intregs        rs.l    7
		rs.l    1
.pspan          rs.l    1

		movem.l d2-d7/a2,-(sp)
		fmovem.x        fp3-fp7,-(sp)
		move.l  .pspan(sp),a2
		move.l  _d_pzbuffer,a0
		move.l  _d_zwidth,d7
		fmove.s _d_ziorigin,fp5
		fmove.s _d_zistepv,fp6
		fmove.s _d_zistepu,fp7
		fmove.s #32768*65536,fp0

*        izistep = (int)(d_zistepu * 0x8000 * 0x10000);

		fmove   fp7,fp1                 ;fp1 = d_zistepu
		fmul    fp0,fp1                 ;multiply by $8000*$10000
		fmove.l fp1,d4                  ;izistep = d4
		moveq   #16,d6

*                pdest = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u;
*
*                count = pspan->count;
*
*        // calculate the initial 1/z
*                du = (float)pspan->u;
*                dv = (float)pspan->v;
*
*                zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
*        // we count on FP exceptions being turned off to avoid range problems
*                izi = (int)(zi * 0x8000 * 0x10000);


.loop
		move.l  (a2)+,d0
		fmove   fp7,fp4
		fmul.l  d0,fp4                  ;fp4 = du * d_zistepu
		move.l  (a2)+,d1
		fmove   fp6,fp3
		fmul.l  d1,fp3                  ;fp3 = dv * d_zistepv
		move.l  (a2)+,d2
		fadd    fp3,fp4
		muls    d7,d1                   ;d1 = pspan->v * d_zwidth
		fadd    fp5,fp4                 ;fp4 = d_ziorigin + fp3 + fp4
		add.l   d0,d1                   ;d1 = d1 + pspan->u
		lea     0(a0,d1.l*2),a1         ;pdest = d_pzbuffer + d1
		fmul    fp0,fp4                 ;izi = zi * $8000 * $10000
		fmove.l fp4,d3                  ;convert to integer

*                if ((long)pdest & 0x02)
*                {
*                        *pdest++ = (short)(izi >> 16);
*                        izi += izistep;
*                        count--;
*                }

		move.l  a1,d0                   ;if ((long)pdest & 0x02)
		and.l   #2,d0
		beq.b   .cont
		swap    d3
		move    d3,(a1)+                ;*pdest++ = (short)(izi>>16)
		swap    d3
		add.l   d4,d3                   ;izi += izistep;
		subq    #1,d2                   ;count--
.cont

*                if ((doublecount = count >> 1) > 0)
*                {
*                        do
*                        {
*                                ltemp = izi >> 16;
*                                izi += izistep;
*                                ltemp |= izi & 0xFFFF0000;
*                                izi += izistep;
*                                *(int *)pdest = ltemp;
*                                pdest += 2;
*                        } while (--doublecount > 0);
*                }

		move.l  d2,d0                   ;if ((doublecount=count>>1)>0)
		asr.l   #1,d0
		ble.b   .cont2
		subq    #1,d0
.loop2
		move.l  d3,d5
		lsr.l   d6,d5                   ;temp = izi >> 16
		add.l   d4,d3                   ;izi += izistep
		move.l  d3,d1
		and.l   #$ffff0000,d1
		or.l    d1,d5                   ;ltemp |= izi & 0xFFFF0000
		add.l   d4,d3                   ;izi += izistep
		move.l  d5,(a1)+                ;*(int *)pdest = ltemp
		dbra    d0,.loop2               ;while (--doublecount > 0)
.cont2

*                if (count & 1)
*                        *pdest = (short)(izi >> 16);

		and.l   #$1,d2                  ;if (count & 1)
		beq.b   .cont3
		swap    d3
		move    d3,(a1)+                ;*pdest = (short)(izi >> 16)
.cont3

*        } while ((pspan = pspan->pnext) != NULL);

		move.l  (a2)+,a2
		tst.l   a2
		bne.w   .loop
		fmovem.x        (sp)+,fp3-fp7
		movem.l (sp)+,d2-d7/a2
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
		dc.w    0
		dc.w    16384/9
		dc.w    16384/10
		dc.w    16384/11
		dc.w    16384/12
		dc.w    16384/13
		dc.w    16384/14
		dc.w    16384/15
