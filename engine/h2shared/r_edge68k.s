**
** Quake for AMIGA
** r_edge.c assembler implementations by Frank Wille <frank@phoenix.owl.de>
** Adapted for Hexen II by Szilard Biro
**

;		INCLUDE	"quakedef68k.i"
SURF_NEXT	equ	0
SURF_PREV	equ	4
SURF_SPANS	equ	8
SURF_KEY	equ	12
SURF_LAST_U	equ	16
SURF_SPANSTATE	equ	20
SURF_FLAGS	equ	24
SURF_DATA	equ	28
SURF_ENTITY	equ	32
SURF_NEARZI	equ	36
SURF_INSUBMODEL	equ	40
SURF_D_ZIORIGIN	equ	44
SURF_D_ZISTEPU	equ	48
SURF_D_ZISTEPV	equ	52
SURF_SIZEOF_EXP	equ	6
SURF_SIZEOF	equ	64
EDGE_U	equ	0
EDGE_U_STEP	equ	4
EDGE_PREV	equ	8
EDGE_NEXT	equ	12
EDGE_SURFS	equ	16
EDGE_NEXTREMOVE	equ	20
EDGE_NEARZI	equ	24
EDGE_OWNER	equ	28
EDGE_SIZEOF_EXP	equ	5
EDGE_SIZEOF	equ	32

		;XREF    _r_bmodelactive
		XREF    _surfaces
		XREF    _span_p
		XREF    _current_iv
		XREF    _fv
		XREF    _edge_head
		XREF    _edge_tail
		XREF    _edge_aftertail
		XREF    _edge_head_u_shift20
		;XREF    _R_CleanupSpan
		XREF    _FoundTrans
		XREF    _edge_tail_u_shift20

		XDEF    _R_RemoveEdges
		XDEF    _R_InsertNewEdges
		XDEF    _R_StepActiveU
		XDEF    _R_GenerateSpans
		XDEF    _R_GenerateTSpans

		fpu

******************************************************************************
*
*       void _R_InsertNewEdges (edge_t *edgestoadd, edge_t *edgelist)
*
******************************************************************************

		cnop    0,4
_R_InsertNewEdges

*****   stackframe

		rsreset
.intregs        rs.l    1
		rs.l    1
.edgestoadd     rs.l    1
.edgelist       rs.l    1


*        do
*        {
*                next_edge = edgestoadd->next;
*edgesearch:
*                if (edgelist->u >= edgestoadd->u)
*                        goto addedge;
*                edgelist=edgelist->next;
*                if (edgelist->u >= edgestoadd->u)
*                        goto addedge;
*                edgelist=edgelist->next;
*                if (edgelist->u >= edgestoadd->u)
*                        goto addedge;
*                edgelist=edgelist->next;
*                if (edgelist->u >= edgestoadd->u)
*                        goto addedge;
*                edgelist=edgelist->next;
*                goto edgesearch;
*
*        // insert edgestoadd before edgelist
*addedge:
*                edgestoadd->next = edgelist;
*                edgestoadd->prev = edgelist->prev;
*                edgelist->prev->next = edgestoadd;
*                edgelist->prev = edgestoadd;
*        } while ((edgestoadd = next_edge) != NULL);

		move.l  a2,-(sp)
		move.l  .edgestoadd(sp),a0
		move.l  .edgelist(sp),a1
		move.l  EDGE_U(a0),d1
.loop
		cmp.l   EDGE_U(a1),d1           ;if (edgelist->u >= edgestoadd->u)
		ble.b   .addedge                ;goto addedge
		move.l  EDGE_NEXT(a1),a1        ;edgelist=edgelist->next
		cmp.l   EDGE_U(a1),d1
		ble.b   .addedge
		move.l  EDGE_NEXT(a1),a1
		cmp.l   EDGE_U(a1),d1
		ble.b   .addedge
		move.l  EDGE_NEXT(a1),a1
		cmp.l   EDGE_U(a1),d1
		ble.b   .addedge
		move.l  EDGE_NEXT(a1),a1
		bra.b   .loop
.addedge
		move.l  EDGE_NEXT(a0),d0        ;next_edge = edgestoadd->next
		move.l  a1,EDGE_NEXT(a0)        ;edgestoadd->next = edgelist
		move.l  EDGE_PREV(a1),a2
		move.l  a2,EDGE_PREV(a0)        ;edgestoadd->prev = edgelist->prev
		move.l  a0,EDGE_NEXT(a2)        ;edgelist->prev->next = edgestoadd
		move.l  a0,EDGE_PREV(a1)        ;edgelist->prev = edgestoadd
		tst.l   d0                      ;while ((edgestoadd = next_edge) != NULL)
		beq.b   .end
		move.l  d0,a0
		move.l  EDGE_U(a0),d1
		bra.b   .loop
.end
		move.l  (sp)+,a2
		rts






******************************************************************************
*
*       void _R_RemoveEdges (edge_t *pedge)
*
******************************************************************************

		cnop    0,4
_R_RemoveEdges

*****   stackframe

		rsreset
.intregs        rs.l    1
		rs.l    1
.pedge          rs.l    1


*        do
*        {
*                pedge->next->prev = pedge->prev;
*                pedge->prev->next = pedge->next;
*        } while ((pedge = pedge->nextremove) != NULL);

		move.l  a2,-(sp)
		move.l  .pedge(sp),a0
.loop
		move.l  EDGE_NEXT(a0),a1
		move.l  EDGE_PREV(a0),a2
		move.l  a2,EDGE_PREV(a1)        ;pedge->next->prev = pedge->prev
		move.l  a1,EDGE_NEXT(a2)        ;pedge->prev->next = pedge->next
		move.l  EDGE_NEXTREMOVE(a0),a0  ;while ((pedge = pedge->nextremove) != NULL)
		tst.l   a0
		bne.b   .loop
		move.l  (sp)+,a2
		rts








******************************************************************************
*
*       void _R_StepActiveU (edge_t *pedge)
*
******************************************************************************

		cnop    0,4
_R_StepActiveU

*****   stackframe

		rsreset
.intregs        rs.l    5
		rs.l    1
.pedge          rs.l    1


*nextedge:
*                pedge->u += pedge->u_step;
*                if (pedge->u < pedge->prev->u)
*                        goto pushback;
*                pedge = pedge->next;
*
*                pedge->u += pedge->u_step;
*                if (pedge->u < pedge->prev->u)
*                        goto pushback;
*                pedge = pedge->next;
*
*                pedge->u += pedge->u_step;
*                if (pedge->u < pedge->prev->u)
*                        goto pushback;
*                pedge = pedge->next;
*
*                pedge->u += pedge->u_step;
*                if (pedge->u < pedge->prev->u)
*                        goto pushback;
*                pedge = pedge->next;
*
*                goto nextedge;

		movem.l a2-a6,-(sp)
		move.l  .pedge(sp),a0
		move.l  EDGE_PREV(a0),a1
		move.l  EDGE_U(a1),d1
		lea     _edge_aftertail,a2
		lea     _edge_tail,a3
.loop
		move.l  EDGE_U(a0),d0
		add.l   EDGE_U_STEP(a0),d0
		move.l  d0,EDGE_U(a0)           ;pedge->u += pedge->u_step
		cmp.l   d1,d0                   ;if (pedge->u < pedge->prev->u)
		blt.b   .pushback               ;goto pushback
		move.l  d0,d1
		move.l  EDGE_NEXT(a0),a0        ;pedge = pedge->next
		move.l  EDGE_U(a0),d0
		add.l   EDGE_U_STEP(a0),d0
		move.l  d0,EDGE_U(a0)
		cmp.l   d1,d0
		blt.b   .pushback
		move.l  d0,d1
		move.l  EDGE_NEXT(a0),a0
		move.l  EDGE_U(a0),d0
		add.l   EDGE_U_STEP(a0),d0
		move.l  d0,EDGE_U(a0)
		cmp.l   d1,d0
		blt.b   .pushback
		move.l  d0,d1
		move.l  EDGE_NEXT(a0),a0
		move.l  EDGE_U(a0),d0
		add.l   EDGE_U_STEP(a0),d0
		move.l  d0,EDGE_U(a0)
		cmp.l   d1,d0
		blt.b   .pushback
		move.l  d0,d1
		move.l  EDGE_NEXT(a0),a0
		bra.b   .loop

*                if (pedge == &edge_aftertail)
*                        return;
*
*        // push it back to keep it sorted
*                pnext_edge = pedge->next;
*
*        // pull the edge out of the edge list
*                pedge->next->prev = pedge->prev;
*                pedge->prev->next = pedge->next;
*
*        // find out where the edge goes in the edge list
*                pwedge = pedge->prev->prev;
*
*                while (pwedge->u > pedge->u)
*                {
*                        pwedge = pwedge->prev;
*                }
*
*        // put the edge back into the edge list
*                pedge->next = pwedge->next;
*                pedge->prev = pwedge;
*                pedge->next->prev = pedge;
*                pwedge->next = pedge;
*
*                pedge = pnext_edge;
*                if (pedge == &edge_tail)
*                        return;

.pushback
		cmp.l   a0,a2                   ;if (pedge == &edge_aftertail)
		beq.w   .end                    ;return
		move.l  EDGE_NEXT(a0),a4        ;pnext_edge = pedge->next
		move.l  EDGE_PREV(a0),a5
		move.l  a5,EDGE_PREV(a4)        ;pedge->next->prev = pedge->prev
		move.l  a4,EDGE_NEXT(a5)        ;pedge->prev->next = pedge->next
		move.l  EDGE_PREV(a5),a5        ;pwedge = pedge->prev->prev
.loop2
		cmp.l   EDGE_U(a5),d0           ;while (pwedge->u > pedge->u)
		bgt.b   .cont
		move.l  EDGE_PREV(a5),a5        ;pwedge = pwedge->prev
		bra.b   .loop2
.cont
		move.l  EDGE_NEXT(a5),a1
		move.l  a1,EDGE_NEXT(a0)        ;pedge->next = pwedge->next
		move.l  a5,EDGE_PREV(a0)        ;pedge->prev = pwedge
		move.l  a0,EDGE_PREV(a1)        ;pedge->next->prev = pedge
		move.l  a0,EDGE_NEXT(a5)        ;pwedge->next = pedge
		move.l  a4,a0                   ;pedge = pnext_edge
		cmp.l   a3,a0                   ;if (pedge == &edge_tail)
		bne.b   .loop
.end
		movem.l (sp)+,a2-a6
		rts








******************************************************************************
*
*       void _R_GenerateSpans (void)
*
*       R_TrailingEdge, R_LeadingEdge and R_CleanupSpan are inlined
*
*       notes:
*       Increment and Decrement of _r_bmodelactive removed, because it's
*       obsolete here
*
******************************************************************************

		cnop    0,4
_R_GenerateSpans

******  prologue

		movem.l d2-d7/a2-a6,-(sp)
		fmovem.x        fp2-fp7,-(sp)

*        r_bmodelactive = 0;
*        FoundTrans = 0;

		;clr.l   _r_bmodelactive
		clr.l   _FoundTrans

*// clear active surfaces to just the background surface
*        surfaces[1].next = surfaces[1].prev = &surfaces[1];
*        surfaces[1].last_u = edge_head_u_shift20;

		move.l  _current_iv,d2          ;d2 = current_iv 
		move.l  _surfaces,a3            ;a3 = surfaces
		lea     1*SURF_SIZEOF(a3),a4    ;a4 = &surfaces[1]
		move.l  a4,SURF_NEXT(a4)        ;surfaces[1].next = a4
		move.l  a4,SURF_PREV(a4)        ;surfaces[1].prev = a4
		move.l  _edge_head_u_shift20,SURF_LAST_U(a4) ;surfaces[1].last_u = edge_head_u_shift20

*        for (edge = edge_head.next ; edge != &edge_tail; edge = edge->next)

		move.l  _edge_head+EDGE_NEXT,a5 ;a5 = edge->next
		lea     _edge_tail,a6           ;a6 = &edge_tail
		move.l  _span_p,a4 
		bra.w   .try
.loop
		move.l  EDGE_U(a5),d4
		move.l  d4,d7                   ;d7 = edge->u
		moveq   #20,d0
		asr.l   d0,d4                   ;d4 = edge->u >> 20

*        if (edge->surfs[0])

		move.l  EDGE_SURFS(a5),d1       ;d1 = edge->surfs[0]
		move.l  d1,d0                   ;if (edge->surfs[0])
		swap    d0
		ext.l   d0
		beq.b   .cont
		asl.l   #SURF_SIZEOF_EXP,d0
		lea     0(a3,d0.l),a0           ;a0 = &surfaces[edge->surfs[0]];

******  R_TrailingEdge (inlined)

*        if (surf->flags & SURF_TRANSLUCENT)
*        {
*                FoundTrans = 1;
*                return;
*        }
*
*        if (--surf->spanstate == 0)
*        {
*                if (surf->insubmodel)
*                        r_bmodelactive--;
*
*                if (surf == surfaces[1].next)
*                {
*                // emit a span (current top going away)
*                        iu = edge->u >> 20;
*                        if (iu > surf->last_u)
*                        {
*                                span = span_p++;
*                                span->u = surf->last_u;
*                                span->count = iu - span->u;
*                                span->v = current_iv;
*                                span->pnext = surf->spans;
*                                surf->spans = span;
*                        }
*
*                // set last_u on the surface below
*                        surf->next->last_u = iu;
*                }
*
*                surf->prev->next = surf->next;
*                surf->next->prev = surf->prev;
*        }

; translucent
		btst.b  #7,SURF_FLAGS+3(a0)      ;if (surf->flags & SURF_TRANSLUCENT)
		beq.b   .te_notfound
		move.l  #1,_FoundTrans
		bra.b   .cont
.te_notfound
; translucent
		subq.l  #1,SURF_SPANSTATE(a0)   ;if (--surf->spanstate) == 0
		bne.b   .cont
		move.l  SURF_NEXT(a0),a1
		cmp.l   1*SURF_SIZEOF+SURF_NEXT(a3),a0 ;if (surf==surfaces[1].next)
		bne.b   .te_cont2
		move.l  d4,d0                   ;iu = edge->u >> 20
		move.l  SURF_LAST_U(a0),d5
		move.l  d0,SURF_LAST_U(a1)      ;surf->next->last_u = iu
		sub.l   d5,d0                   ;if (iu > surf->last_u)
		ble.b   .te_cont2
		move.l  SURF_SPANS(a0),d3
		move.l  a4,SURF_SPANS(a0)       ;surf->spans = span
		move.l  d5,(a4)+                ;span->u = surf->last_u
		move.l  d2,(a4)+                ;span->v = current_iv
		move.l  d0,(a4)+                ;span->count = iu - span->u
		move.l  d3,(a4)+                ;span->pnext = surf->spans
.te_cont2
		move.l  SURF_PREV(a0),a2
		move.l  a1,SURF_NEXT(a2)        ;surf->prev->next = surf->next
		move.l  a2,SURF_PREV(a1)        ;surf->next->prev = surf->prev

******  end of R_TrailingEdge

.cont

******  R_LeadingEdge (inlined)

*        if (edge->surfs[1])
*        {
*        // it's adding a new surface in, so find the correct place
*                surf = &surfaces[edge->surfs[1]];
*
		ext.l   d1                      ;if (edge->surfs[1])
		beq.w   .next
		asl.l   #SURF_SIZEOF_EXP,d1
		lea     0(a3,d1.l),a1           ;surf = &surfaces[edge->surfs[1]]
*        if (surf->flags & SURF_TRANSLUCENT) 
*        {
*                FoundTrans = 1;
*                return;
*        }

; translucent
		btst.b  #7,SURF_FLAGS+3(a1)      ;if (surf->flags & SURF_TRANSLUCENT)
		beq.b   .le_notfound
		move.l  #1,_FoundTrans
		bra.w   .next
.le_notfound
; translucent
*
*        // don't start a span if this is an inverted span, with the end
*        // edge preceding the start edge (that is, we've already seen the
*        // end edge)
*                if (++surf->spanstate == 1)
*                {
*                        if (surf->insubmodel)
*                                r_bmodelactive++;
*
*                        surf2 = surfaces[1].next;
*
*                        if (surf->key < surf2->key)
*                                goto newtop;
*                // if it's two surfaces on the same plane, the one that's already
*                // active is in front, so keep going unless it's a bmodel
*                        if (surf->insubmodel && (surf->key == surf2->key))
*                        {


		move.l  SURF_SPANSTATE(a1),d0   ;if (++surf->spanstate == 1)
		beq.b   .le_zero
		addq.l  #1,SURF_SPANSTATE(a1)
		bra.w   .next
.le_zero
		addq.l  #1,d0
		move.l  d0,SURF_SPANSTATE(a1)
		move.l  SURF_INSUBMODEL(a1),d5  ;if (surf->insubmodel)
		move.l  1*SURF_SIZEOF+SURF_NEXT(a3),a0  ;surf2 = surfaces[1].next
		move.l  SURF_KEY(a1),d6
		moveq   #0,d3
		cmp.l   SURF_KEY(a0),d6         ;if (surf->key < surf2->key)
		blt.b   .le_newtop              ;goto newtop
		bgt.b   .le_search
		tst.b   d5
		beq.b   .le_search

		moveq   #-1,d3
		move.l  d7,d0
		sub.l   #$fffff,d0              ;edge->u - 0xFFFFF
		fmove.l d0,fp0                  ;(float)(edge->u - 0xFFFFF)
		fmul.s  #(1.0/(16*65536)),fp0   ;fu = fp0 * (1 / $100000)
		fmove.s _fv,fp1                 ;fv
		fmove.s SURF_D_ZIORIGIN(a1),fp2
		fmove.s SURF_D_ZISTEPV(a1),fp3
		fmul    fp1,fp3                 ;fp1 = fv * surf->d_zistepv
		fadd    fp3,fp2
		fmove.s SURF_D_ZISTEPU(a1),fp3
		fmove   fp3,fp4                 ;fp4 = surf->d_zistepu
		fmul    fp0,fp3                 ;fp3 = fu * surf->d_zistepu
		fadd    fp3,fp2                 ;newzi = d_ziorigin + fp1 + fp3
		fmove   fp2,fp3
		fmul.s  #0.999,fp2               ;newzibottom = newzi * 0.999
		fmul.s  #1.001,fp3               ;newzitop = newzi * 1.001


*                        // must be two bmodels in the same leaf; sort on 1/z
*                                fu = (float)(edge->u - 0xFFFFF) * (1.0 / 0x100000);
*                                newzi = surf->d_ziorigin + fv*surf->d_zistepv +
*                                                fu*surf->d_zistepu;
*                                newzibottom = newzi * 0.999;
*
*                                testzi = surf2->d_ziorigin + fv*surf2->d_zistepv +
*                                                fu*surf2->d_zistepu;
*
*                                if (newzibottom >= testzi)
*                                {
*                                        goto newtop;
*                                }
*
*                                newzitop = newzi * 1.001;
*                                if (newzitop >= testzi)
*                                {
*                                        if (surf->d_zistepu >= surf2->d_zistepu)
*                                        {
*                                                goto newtop;
*                                        }
*                                }

		fmove.s SURF_D_ZIORIGIN(a0),fp5
		fmove.s SURF_D_ZISTEPV(a0),fp6
		fmul    fp1,fp6                 ;fp1 = fv * surf2->d_zistepv
		fadd    fp6,fp5
		fmove.s SURF_D_ZISTEPU(a0),fp6
		fmove   fp6,fp7
		fmul    fp0,fp6                 ;fp3 = fu * surf2->d_zistepu
		fadd    fp6,fp5                 ;testzi = d_ziorigin + fp1 + fp3
		fcmp    fp5,fp2                 ;if (newzibottom >= testzi)
		fbge.w  .le_newtop              ;goto newtop
		fcmp    fp5,fp3                 ;if (newzitop >= testzi)
		fblt.w  .le_search
		fcmp    fp7,fp4                 ;if (surf->d_zistepu >= surf2->d_zistepu)
		fbge.w  .le_newtop              ;goto newtop

*                        do
*                        {
*                                surf2 = surf2->next;
*                        } while (surf->key > surf2->key);
*


.le_search
		move.l  SURF_NEXT(a0),a0        ;surf2 = surf2->next
		cmp.l   SURF_KEY(a0),d6         ;while (surf->key > surf2->key)
		bgt.b   .le_search

*                        if (surf->key == surf2->key)
*                        {
*                        // if it's two surfaces on the same plane, the one that's already
*                        // active is in front, so keep going unless it's a bmodel
*                                if (!surf->insubmodel)
*                                        goto continue_search;

		bne.b   .le_gotposition
		tst.b   d5
		beq.b   .le_search
		tst     d3
		bne.b   .le_precalc_done
		moveq   #-1,d3
		move.l  d7,d0
		sub.l   #$fffff,d0              ;edge->u - 0xFFFFF
		fmove.l d0,fp0                  ;(float)(edge->u - 0xFFFFF)
		fmul.s  #(1.0/(16*65536)),fp0   ;fu = fp0 * (1 / $100000)
		fmove.s _fv,fp1                 ;fv
		fmove.s SURF_D_ZIORIGIN(a1),fp2
		fmove.s SURF_D_ZISTEPV(a1),fp3
		fmul    fp1,fp3                 ;fp1 = fv * surf->d_zistepv
		fadd    fp3,fp2
		fmove.s SURF_D_ZISTEPU(a1),fp3
		fmove   fp3,fp4                 ;fp4 = surf->d_zistepu
		fmul    fp0,fp3                 ;fp3 = fu * surf->d_zistepu
		fadd    fp3,fp2                 ;newzi = d_ziorigin + fp1 + fp3
		fmove   fp2,fp3
		fmul.s  #0.999,fp2               ;newzibottom = newzi * 0.999
		fmul.s  #1.001,fp3               ;newzitop = newzi * 1.001
.le_precalc_done

*                        // must be two bmodels in the same leaf; sort on 1/z
*                                fu = (float)(edge->u - 0xFFFFF) * (1.0 / 0x100000);
*                                newzi = surf->d_ziorigin + fv*surf->d_zistepv +
*                                                fu*surf->d_zistepu;
*                                newzibottom = newzi * 0.999;
*
*                                testzi = surf2->d_ziorigin + fv*surf2->d_zistepv +
*                                                fu*surf2->d_zistepu;
*
*                                if (newzibottom >= testzi)
*                                {
*                                        goto gotposition;
*                                }
*
*                                newzitop = newzi * 1.001;
*                                if (newzitop >= testzi)
*                                {
*                                        if (surf->d_zistepu >= surf2->d_zistepu)
*                                        {
*                                                goto gotposition;
*                                        }
*                                }
*
*                                goto continue_search;
*                        }
*
*                        goto gotposition;

		fmove.s SURF_D_ZIORIGIN(a0),fp5
		fmove.s SURF_D_ZISTEPV(a0),fp6
		fmul    fp1,fp6                 ;fp1 = fv * surf2->d_zistepv
		fadd    fp6,fp5
		fmove.s SURF_D_ZISTEPU(a0),fp6
		fmove   fp6,fp7
		fmul    fp0,fp6                 ;fp3 = fu * surf2->d_zistepu
		fadd    fp6,fp5                 ;testzi = d_ziorigin + fp1 + fp3
		fcmp    fp5,fp2                 ;if (newzibottom >= testzi)
		fbge.w  .le_gotposition         ;goto gotposition
		fcmp    fp5,fp3                 ;if (newzitop >= testzi)
		fblt.w  .le_search
		fcmp    fp7,fp4                 ;if (surf->d_zistepu >= surf2->d_zistepu)
		fbge.w  .le_gotposition         ;goto gotposition
		bra.b   .le_search

*                // emit a span (obscures current top)
*                        iu = edge->u >> 20;
*
*                        if (iu > surf2->last_u && !(surf2->flags & SURF_TRANSLUCENT))
*                        {
*                                span = span_p++;
*                                span->u = surf2->last_u;
*                                span->count = iu - span->u;
*                                span->v = current_iv;
*                                span->pnext = surf2->spans;
*                                surf2->spans = span;
*                        }
*
*                        // set last_u on the new span
*                        surf->last_u = iu;

.le_newtop
		move.l  SURF_LAST_U(a0),d5
		move.l  d4,SURF_LAST_U(a1)      ;surf->last_u = iu
		sub.l   d5,d4                   ;if (iu > surf2->last_u)
		ble.b   .le_cont2
; translucent
		btst.b  #7,SURF_FLAGS+3(a0)      ;if (!(surf->flags & SURF_TRANSLUCENT))
		bne.b   .le_cont2
; translucent
		move.l  SURF_SPANS(a0),d3
		move.l  a4,SURF_SPANS(a0)       ;surf2->spans = span
		move.l  d5,(a4)+                ;span->u = surf2->last_u
		move.l  d2,(a4)+                ;span->v = current_iv
		move.l  d4,(a4)+                ;span->count = iu - span->u
		move.l  d3,(a4)+                ;span->pnext = surf2->spans
.le_cont2

*                // insert before surf2
*                        surf->next = surf2;
*                        surf->prev = surf2->prev;
*                        surf2->prev->next = surf;
*                        surf2->prev = surf;

.le_gotposition
		move.l  SURF_PREV(a0),a2
		move.l  a2,SURF_PREV(a1)        ;surf->prev = surf2->prev
		move.l  a0,SURF_NEXT(a1)        ;surf->next = surf2
		move.l  a1,SURF_NEXT(a2)        ;surf2->prev->next = surf
		move.l  a1,SURF_PREV(a0)        ;surf2->prev = surf

******  end of R_LeadingEdge

.next
		move.l  EDGE_NEXT(a5),a5
.try
		cmp.l   a5,a6                   ;if (edge != &edge_tail)
		bne.b   .loop

******  R_CleanupSpan (inlined)

*        // now that we've reached the right edge of the screen, we're done with any
*        // unfinished surfaces, so emit a span for whatever's on top
*                surf = surfaces[1].next;
*                iu = edge_tail_u_shift20;
*                if (iu > surf->last_u && !(surf->flags & SURF_TRANSLUCENT))
*                {
*                        span = span_p++;
*                        span->u = surf->last_u;
*                        span->count = iu - span->u;
*                        span->v = current_iv;
*                        span->pnext = surf->spans;
*                        surf->spans = span;
*                }

		move.l  1*SURF_SIZEOF+SURF_NEXT(a3),a0  ;surf = surfaces[1].next
		move.l  _edge_tail_u_shift20,d4 ;iu = edge_tail_u_shift20

		move.l  SURF_LAST_U(a0),d5
		sub.l   d5,d4                   ;if (iu > surf2->last_u)
		ble.b   .cs_cont
; translucent
		btst.b  #7,SURF_FLAGS+3(a0)      ;if (!(surf->flags & SURF_TRANSLUCENT))
		bne.b   .cs_cont
; translucent
		move.l  SURF_SPANS(a0),d3
		move.l  a4,SURF_SPANS(a0)       ;surf->spans = span
		move.l  d5,(a4)+                ;span->u = surf->last_u
		move.l  d2,(a4)+                ;span->v = current_iv
		move.l  d4,(a4)+                ;span->count = iu - span->u
		move.l  d3,(a4)+                ;span->pnext = surf->spans
.cs_cont

*        // reset spanstate for all surfaces in the surface stack
*                do
*                {
*                        surf->spanstate = 0;
*                        surf = surf->next;
*                } while (surf != &surfaces[1]);

		lea     1*SURF_SIZEOF(a3),a1  ;a1 = &surfaces[1]
.cs_loop
		clr.l   SURF_SPANSTATE(a0)      ;surf->spanstate = 0
		move.l  SURF_NEXT(a0),a0        ;surf = surf->next
		cmp.l   a0,a1                   ;while (surf != &surfaces[1])
		bne.b   .cs_loop

******  end of R_CleanupSpan

		move.l  a4,_span_p
		;jsr     _R_CleanupSpan

		fmovem.x        (sp)+,fp2-fp7
		movem.l (sp)+,d2-d7/a2-a6
		rts



******************************************************************************
*
*       void _R_GenerateTSpans (void)
*
*       R_TrailingEdgeT, R_LeadingEdgeT and R_CleanupSpanT are inlined
*
*       notes:
*       Increment and Decrement of _r_bmodelactive removed, because it's
*       obsolete here
*
******************************************************************************

		cnop    0,4
_R_GenerateTSpans

******  prologue

		movem.l d2-d7/a2-a6,-(sp)
		fmovem.x        fp2-fp7,-(sp)

*        r_bmodelactive = 0;

		;clr.l   _r_bmodelactive

*// clear active surfaces to just the background surface
*        surfaces[1].next = surfaces[1].prev = &surfaces[1];
*        surfaces[1].last_u = edge_head_u_shift20;

		move.l  _current_iv,d2          ;d2 = current_iv 
		move.l  _surfaces,a3            ;a3 = surfaces
		lea     1*SURF_SIZEOF(a3),a4    ;a4 = &surfaces[1]
		move.l  a4,SURF_NEXT(a4)        ;surfaces[1].next = a4
		move.l  a4,SURF_PREV(a4)        ;surfaces[1].prev = a4
		move.l  _edge_head_u_shift20,SURF_LAST_U(a4) ;surfaces[1].last_u = edge_head_u_shift20

*        for (edge = edge_head.next ; edge != &edge_tail; edge = edge->next)

		move.l  _edge_head+EDGE_NEXT,a5 ;a5 = edge->next
		lea     _edge_tail,a6           ;a6 = &edge_tail
		move.l  _span_p,a4 
		bra.w   .try
.loop
		move.l  EDGE_U(a5),d4
		move.l  d4,d7                   ;d7 = edge->u
		moveq   #20,d0
		asr.l   d0,d4                   ;d4 = edge->u >> 20

*        if (edge->surfs[0])

		move.l  EDGE_SURFS(a5),d1       ;d1 = edge->surfs[0]
		move.l  d1,d0                   ;if (edge->surfs[0])
		swap    d0
		ext.l   d0
		beq.b   .cont
		asl.l   #SURF_SIZEOF_EXP,d0
		lea     0(a3,d0.l),a0           ;a0 = &surfaces[edge->surfs[0]];

******  R_TrailingEdgeT (inlined)

*        if (--surf->spanstate == 0)
*        {
*                if (surf->insubmodel)
*                        r_bmodelactive--;
*
*                if (surf == surfaces[1].next)
*                {
*                // emit a span (current top going away)
*                        iu = edge->u >> 20;
*                        if (iu > surf->last_u && (surf->flags & SURF_TRANSLUCENT))
*                        {
*                                span = span_p++;
*                                span->u = surf->last_u;
*                                span->count = iu - span->u;
*                                span->v = current_iv;
*                                span->pnext = surf->spans;
*                                surf->spans = span;
*                        }
*
*                // set last_u on the surface below
*                        surf->next->last_u = iu;
*                }
*
*                surf->prev->next = surf->next;
*                surf->next->prev = surf->prev;
*        }

		subq.l  #1,SURF_SPANSTATE(a0)   ;if (--surf->spanstate) == 0
		bne.b   .cont
		move.l  SURF_NEXT(a0),a1
		cmp.l   1*SURF_SIZEOF+SURF_NEXT(a3),a0 ;if (surf==surfaces[1].next)
		bne.b   .te_cont2
		move.l  d4,d0                   ;iu = edge->u >> 20
		move.l  SURF_LAST_U(a0),d5
		move.l  d0,SURF_LAST_U(a1)      ;surf->next->last_u = iu
		sub.l   d5,d0                   ;if (iu > surf->last_u)
		ble.b   .te_cont2
; translucent
		btst.b  #7,SURF_FLAGS+3(a0)     ;if (surf->flags & SURF_TRANSLUCENT)
		beq.b   .te_cont2
; translucent
		move.l  SURF_SPANS(a0),d3
		move.l  a4,SURF_SPANS(a0)       ;surf->spans = span
		move.l  d5,(a4)+                ;span->u = surf->last_u
		move.l  d2,(a4)+                ;span->v = current_iv
		move.l  d0,(a4)+                ;span->count = iu - span->u
		move.l  d3,(a4)+                ;span->pnext = surf->spans
.te_cont2
		move.l  SURF_PREV(a0),a2
		move.l  a1,SURF_NEXT(a2)        ;surf->prev->next = surf->next
		move.l  a2,SURF_PREV(a1)        ;surf->next->prev = surf->prev

******  end of R_TrailingEdgeT

.cont

******  R_LeadingEdgeT (inlined)

*        if (edge->surfs[1])
*        {
*        // it's adding a new surface in, so find the correct place
*                surf = &surfaces[edge->surfs[1]];
*
		ext.l   d1                      ;if (edge->surfs[1])
		beq.w   .next
		asl.l   #SURF_SIZEOF_EXP,d1
		lea     0(a3,d1.l),a1           ;surf = &surfaces[edge->surfs[1]]
*
*        // don't start a span if this is an inverted span, with the end
*        // edge preceding the start edge (that is, we've already seen the
*        // end edge)
*                if (++surf->spanstate == 1)
*                {
*                        if (surf->insubmodel)
*                                r_bmodelactive++;
*
*                        surf2 = surfaces[1].next;
*
*                        if (surf->key < surf2->key)
*                                goto newtop;
*                // if it's two surfaces on the same plane, the one that's already
*                // active is in front, so keep going unless it's a bmodel
*                        if (surf->insubmodel && (surf->key == surf2->key))
*                        {


		move.l  SURF_SPANSTATE(a1),d0   ;if (++surf->spanstate == 1)
		beq.b   .le_zero
		addq.l  #1,SURF_SPANSTATE(a1)
		bra.w   .next
.le_zero
		addq.l  #1,d0
		move.l  d0,SURF_SPANSTATE(a1)
		move.l  SURF_INSUBMODEL(a1),d5  ;if (surf->insubmodel)
		move.l  1*SURF_SIZEOF+SURF_NEXT(a3),a0  ;surf2 = surfaces[1].next
		move.l  SURF_KEY(a1),d6
		moveq   #0,d3
		cmp.l   SURF_KEY(a0),d6         ;if (surf->key < surf2->key)
		blt.b   .le_newtop              ;goto newtop
		bgt.b   .le_search
		tst.b   d5
		beq.b   .le_search

		moveq   #-1,d3
		move.l  d7,d0
		sub.l   #$fffff,d0              ;edge->u - 0xFFFFF
		fmove.l d0,fp0                  ;(float)(edge->u - 0xFFFFF)
		fmul.s  #(1.0/(16*65536)),fp0   ;fu = fp0 * (1 / $100000)
		fmove.s _fv,fp1                 ;fv
		fmove.s SURF_D_ZIORIGIN(a1),fp2
		fmove.s SURF_D_ZISTEPV(a1),fp3
		fmul    fp1,fp3                 ;fp1 = fv * surf->d_zistepv
		fadd    fp3,fp2
		fmove.s SURF_D_ZISTEPU(a1),fp3
		fmove   fp3,fp4                 ;fp4 = surf->d_zistepu
		fmul    fp0,fp3                 ;fp3 = fu * surf->d_zistepu
		fadd    fp3,fp2                 ;newzi = d_ziorigin + fp1 + fp3
		fmove   fp2,fp3
		fmul.s  #0.999,fp2               ;newzibottom = newzi * 0.999
		fmul.s  #1.001,fp3               ;newzitop = newzi * 1.001


*                        // must be two bmodels in the same leaf; sort on 1/z
*                                fu = (float)(edge->u - 0xFFFFF) * (1.0 / 0x100000);
*                                newzi = surf->d_ziorigin + fv*surf->d_zistepv +
*                                                fu*surf->d_zistepu;
*                                newzibottom = newzi * 0.999;
*
*                                testzi = surf2->d_ziorigin + fv*surf2->d_zistepv +
*                                                fu*surf2->d_zistepu;
*
*                                if (newzibottom >= testzi)
*                                {
*                                        goto newtop;
*                                }
*
*                                newzitop = newzi * 1.001;
*                                if (newzitop >= testzi)
*                                {
*                                        if (surf->d_zistepu >= surf2->d_zistepu)
*                                        {
*                                                goto newtop;
*                                        }
*                                }

		fmove.s SURF_D_ZIORIGIN(a0),fp5
		fmove.s SURF_D_ZISTEPV(a0),fp6
		fmul    fp1,fp6                 ;fp1 = fv * surf2->d_zistepv
		fadd    fp6,fp5
		fmove.s SURF_D_ZISTEPU(a0),fp6
		fmove   fp6,fp7
		fmul    fp0,fp6                 ;fp3 = fu * surf2->d_zistepu
		fadd    fp6,fp5                 ;testzi = d_ziorigin + fp1 + fp3
		fcmp    fp5,fp2                 ;if (newzibottom >= testzi)
		fbge.w  .le_newtop              ;goto newtop
		fcmp    fp5,fp3                 ;if (newzitop >= testzi)
		fblt.w  .le_search
		fcmp    fp7,fp4                 ;if (surf->d_zistepu >= surf2->d_zistepu)
		fbge.w  .le_newtop              ;goto newtop

*                        do
*                        {
*                                surf2 = surf2->next;
*                        } while (surf->key > surf2->key);
*


.le_search
		move.l  SURF_NEXT(a0),a0        ;surf2 = surf2->next
		cmp.l   SURF_KEY(a0),d6         ;while (surf->key > surf2->key)
		bgt.b   .le_search

*                        if (surf->key == surf2->key)
*                        {
*                        // if it's two surfaces on the same plane, the one that's already
*                        // active is in front, so keep going unless it's a bmodel
*                                if (!surf->insubmodel)
*                                        goto continue_search;

		bne.b   .le_gotposition
		tst.b   d5
		beq.b   .le_search
		tst     d3
		bne.b   .le_precalc_done
		moveq   #-1,d3
		move.l  d7,d0
		sub.l   #$fffff,d0              ;edge->u - 0xFFFFF
		fmove.l d0,fp0                  ;(float)(edge->u - 0xFFFFF)
		fmul.s  #(1.0/(16*65536)),fp0   ;fu = fp0 * (1 / $100000)
		fmove.s _fv,fp1                 ;fv
		fmove.s SURF_D_ZIORIGIN(a1),fp2
		fmove.s SURF_D_ZISTEPV(a1),fp3
		fmul    fp1,fp3                 ;fp1 = fv * surf->d_zistepv
		fadd    fp3,fp2
		fmove.s SURF_D_ZISTEPU(a1),fp3
		fmove   fp3,fp4                 ;fp4 = surf->d_zistepu
		fmul    fp0,fp3                 ;fp3 = fu * surf->d_zistepu
		fadd    fp3,fp2                 ;newzi = d_ziorigin + fp1 + fp3
		fmove   fp2,fp3
		fmul.s  #0.999,fp2               ;newzibottom = newzi * 0.999
		fmul.s  #1.001,fp3               ;newzitop = newzi * 1.001
.le_precalc_done

*                        // must be two bmodels in the same leaf; sort on 1/z
*                                fu = (float)(edge->u - 0xFFFFF) * (1.0 / 0x100000);
*                                newzi = surf->d_ziorigin + fv*surf->d_zistepv +
*                                                fu*surf->d_zistepu;
*                                newzibottom = newzi * 0.999;
*
*                                testzi = surf2->d_ziorigin + fv*surf2->d_zistepv +
*                                                fu*surf2->d_zistepu;
*
*                                if (newzibottom >= testzi)
*                                {
*                                        goto gotposition;
*                                }
*
*                                newzitop = newzi * 1.001;
*                                if (newzitop >= testzi)
*                                {
*                                        if (surf->d_zistepu >= surf2->d_zistepu)
*                                        {
*                                                goto gotposition;
*                                        }
*                                }
*
*                                goto continue_search;
*                        }
*
*                        goto gotposition;

		fmove.s SURF_D_ZIORIGIN(a0),fp5
		fmove.s SURF_D_ZISTEPV(a0),fp6
		fmul    fp1,fp6                 ;fp1 = fv * surf2->d_zistepv
		fadd    fp6,fp5
		fmove.s SURF_D_ZISTEPU(a0),fp6
		fmove   fp6,fp7
		fmul    fp0,fp6                 ;fp3 = fu * surf2->d_zistepu
		fadd    fp6,fp5                 ;testzi = d_ziorigin + fp1 + fp3
		fcmp    fp5,fp2                 ;if (newzibottom >= testzi)
		fbge.w  .le_gotposition         ;goto gotposition
		fcmp    fp5,fp3                 ;if (newzitop >= testzi)
		fblt.w  .le_search
		fcmp    fp7,fp4                 ;if (surf->d_zistepu >= surf2->d_zistepu)
		fbge.w  .le_gotposition         ;goto gotposition
		bra.b   .le_search

*                // emit a span (obscures current top)
*                        iu = edge->u >> 20;
*
*                        if (iu > surf2->last_u && (surf2->flags & SURF_TRANSLUCENT))
*                        {
*                                span = span_p++;
*                                span->u = surf2->last_u;
*                                span->count = iu - span->u;
*                                span->v = current_iv;
*                                span->pnext = surf2->spans;
*                                surf2->spans = span;
*                        }
*
*                        // set last_u on the new span
*                        surf->last_u = iu;

.le_newtop
		move.l  SURF_LAST_U(a0),d5
		move.l  d4,SURF_LAST_U(a1)      ;surf->last_u = iu
		sub.l   d5,d4                   ;if (iu > surf2->last_u)
		ble.b   .le_cont2
; translucent
		btst.b  #7,SURF_FLAGS+3(a0)      ;if (surf->flags & SURF_TRANSLUCENT)
		beq.b   .le_cont2
; translucent
		move.l  SURF_SPANS(a0),d3
		move.l  a4,SURF_SPANS(a0)       ;surf2->spans = span
		move.l  d5,(a4)+                ;span->u = surf2->last_u
		move.l  d2,(a4)+                ;span->v = current_iv
		move.l  d4,(a4)+                ;span->count = iu - span->u
		move.l  d3,(a4)+                ;span->pnext = surf2->spans
.le_cont2

*                // insert before surf2
*                        surf->next = surf2;
*                        surf->prev = surf2->prev;
*                        surf2->prev->next = surf;
*                        surf2->prev = surf;

.le_gotposition
		move.l  SURF_PREV(a0),a2
		move.l  a2,SURF_PREV(a1)        ;surf->prev = surf2->prev
		move.l  a0,SURF_NEXT(a1)        ;surf->next = surf2
		move.l  a1,SURF_NEXT(a2)        ;surf2->prev->next = surf
		move.l  a1,SURF_PREV(a0)        ;surf2->prev = surf

******  end of R_LeadingEdgeT

.next
		move.l  EDGE_NEXT(a5),a5
.try
		cmp.l   a5,a6                   ;if (edge != &edge_tail)
		bne.b   .loop

******  R_CleanupSpanT (inlined)

*        // now that we've reached the right edge of the screen, we're done with any
*        // unfinished surfaces, so emit a span for whatever's on top
*                surf = surfaces[1].next;
*                iu = edge_tail_u_shift20;
*                if (iu > surf->last_u && !(surf->flags & SURF_TRANSLUCENT))
*                {
*                        span = span_p++;
*                        span->u = surf->last_u;
*                        span->count = iu - span->u;
*                        span->v = current_iv;
*                        span->pnext = surf->spans;
*                        surf->spans = span;
*                }

		move.l  1*SURF_SIZEOF+SURF_NEXT(a3),a0  ;surf = surfaces[1].next
		move.l  _edge_tail_u_shift20,d4 ;iu = edge_tail_u_shift20

		move.l  SURF_LAST_U(a0),d5
		sub.l   d5,d4                   ;if (iu > surf2->last_u)
		ble.b   .cs_cont
; translucent
		btst.b  #7,SURF_FLAGS+3(a0)     ;if (surf->flags & SURF_TRANSLUCENT)
		beq.b   .cs_cont
; translucent
		move.l  SURF_SPANS(a0),d3
		move.l  a4,SURF_SPANS(a0)       ;surf->spans = span
		move.l  d5,(a4)+                ;span->u = surf->last_u
		move.l  d2,(a4)+                ;span->v = current_iv
		move.l  d4,(a4)+                ;span->count = iu - span->u
		move.l  d3,(a4)+                ;span->pnext = surf->spans
.cs_cont

*        // reset spanstate for all surfaces in the surface stack
*                do
*                {
*                        surf->spanstate = 0;
*                        surf = surf->next;
*                } while (surf != &surfaces[1]);

		lea     1*SURF_SIZEOF(a3),a1  ;a1 = &surfaces[1]
.cs_loop
		clr.l   SURF_SPANSTATE(a0)      ;surf->spanstate = 0
		move.l  SURF_NEXT(a0),a0        ;surf = surf->next
		cmp.l   a0,a1                   ;while (surf != &surfaces[1])
		bne.b   .cs_loop

******  end of R_CleanupSpanT

		move.l  a4,_span_p
		;jsr     _R_CleanupSpan

		fmovem.x        (sp)+,fp2-fp7
		movem.l (sp)+,d2-d7/a2-a6
		rts
