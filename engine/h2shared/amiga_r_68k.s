; 680x0 optimised Quake render routines by John Selck.

	; r_alias.c:

		XDEF	_R_AliasTransformAndProjectFinalVerts

	; r_draw.c:

		XDEF	_R_ClipEdge

	; r_misc.c:

		XDEF	_TransformVector
		XDEF	@TransformVector

	; r_surf.c:

		XDEF	_R_DrawSurfaceBlock16

	; external defs:

		XREF	_vright
		XREF	_vup
		XREF	_vpn

		XREF	_modelorg
		XREF	_xscale
		XREF	_yscale
		XREF	_xcenter
		XREF	_ycenter
		XREF	_r_refdef
		XREF	_r_framecount
		XREF	_edge_p
		XREF	_surface_p
		XREF	_surfaces
		XREF	_newedges
		XREF	_removeedges

		XREF	_cacheoffset
		XREF	_r_pedge
		XREF	_r_leftclipped
		XREF	_r_rightclipped
		XREF	_r_nearzionly
		XREF	_r_leftenter
		XREF	_r_leftexit
		XREF	_r_rightenter
		XREF	_r_rightexit
		XREF	_r_emitted
		XREF	_r_nearzi
		XREF	_r_u1
		XREF	_r_v1
		XREF	_r_lzi1
		XREF	_r_ceilv1
		XREF	_r_lastvertvalid

		XREF	_aliasxcenter
		XREF	_aliasycenter
		XREF	_r_avertexnormals

		;XREF	_tmatrix	; $00
		;XREF	_viewmatrix	; $30
		;XREF	_ptriangles	; $60
		;XREF	_r_affinetridesc	; $64
		;XREF	_acolormap	; $88
		XREF	_r_apverts	; $8c
		;XREF	_pmdl		; $90
		XREF	_r_plightvec	; $94
		XREF	_r_ambientlight	; $a0
		XREF	_r_shadelight	; $a4
		;XREF	_paliashdr	; $a8
		;XREF	_pfinalverts	; $ac
		;XREF	_pauxverts	; $b0
		;XREF	_ziscale	; $b4
		;XREF	_pmodel		; $b8
		;XREF	_alias_forward	; $bc
		;XREF	_alias_right	; $c8
		;XREF	_alias_up	; $d4
		;XREF	_pskindesc	; $e0
		;XREF	_r_amodels_drawn	; $e4
		;XREF	_a_skinwidth	; $e8
		;XREF	_r_anumverts	; $ec
		XREF	_aliastransform	; $f0	-> $120

__farbss	equ	_aliastransform-$f0

		SECTION	CODE,CODE

; void R_AliasTransformAndProjectFinalVerts (finalvert_t *fv, stvert_t *pstverts)

;	point transformation of entities

_R_AliasTransformAndProjectFinalVerts:
		move.l	8(sp),a1
		move.l	4(sp),a0
		sub.w	#$4c,sp
		movem.l	d6-d7/a2-a3/a5,-(sp)
		fmovem.x	fp2-fp7,-(sp)

		move.l	a0,a5
		moveq	#0,d7

		lea	$64(a7),a3

		move.l	__farbss+$118,(a3)+
		move.l	__farbss+$114,(a3)+
		move.l	__farbss+$110,(a3)+
		move.l	__farbss+$11c,(a3)+

		move.l	__farbss+$f8,(a3)+
		move.l	__farbss+$f4,(a3)+
		move.l	__farbss+$f0,(a3)+
		move.l	__farbss+$fc,(a3)+
		move.l	_aliasxcenter,(a3)+

		move.l	__farbss+$108,(a3)+
		move.l	__farbss+$104,(a3)+
		move.l	__farbss+$100,(a3)+
		move.l	__farbss+$10c,(a3)+
		move.l	_aliasycenter,(a3)+

		move.l	_r_plightvec+8,(a3)+
		move.l	_r_plightvec+4,(a3)+
		move.l	_r_plightvec+0,(a3)

		move.l	_r_apverts,a2
		bra.w	.atin
.atl0
		moveq	#0,d0
		move.b	(a2),d0
		fmove.w	d0,fp7
		move.b	1(a2),d0
		fmove.w	d0,fp6
		move.b	2(a2),d0
		fmove.w	d0,fp5

		lea	$64(a7),a3

		fmove.s	(a3)+,fp1
		fmul.x	fp5,fp1
	move.b	3(a2),d0
		fmove.s	(a3)+,fp2
		fmul.x	fp6,fp2
	mulu.w	#12,d0
		fmove.s	(a3)+,fp3
		fmul.x	fp7,fp3
	lea	_r_avertexnormals,a0
		fadd.s	(a3)+,fp3
		fadd.x	fp1,fp3
		fadd.x	fp2,fp3
		fmove.s	#1,fp0
		fdiv.x	fp3,fp0
	move.l	4(a1),8(a5)
	move.l	8(a1),12(a5)
	move.l	(a1),$18(a5)
		fmove.l	fp0,$14(a5)

		fmove.s	(a3)+,fp1
		fmul.x	fp5,fp1
	add.l	d0,a0
		fmove.s	(a3)+,fp2
		fmul.x	fp6,fp2
	move.l	_r_ambientlight,d6
		fmove.s	(a3)+,fp3
		fmul.x	fp7,fp3
	addq.l	#1,d7
		fadd.s	(a3)+,fp3
		fadd.x	fp1,fp3
		fadd.x	fp2,fp3
		fmul.x	fp0,fp3
	addq.l	#4,a2
		fadd.s	(a3)+,fp3
		fmove.l	fp3,(a5)

		fmove.s	(a3)+,fp1
		fmul.x	fp5,fp1
	add.w	#12,a1
		fmove.s	(a3)+,fp2
		fmul.x	fp6,fp2
		fmove.s	(a3)+,fp3
		fmul.x	fp7,fp3
		fadd.s	(a3)+,fp3
		fadd.x	fp1,fp3
		fadd.x	fp2,fp3
		fmul.x	fp0,fp3
		fadd.s	(a3)+,fp3
		fmove.l	fp3,4(a5)

		fmove.s	8(a0),fp2
		fmul.s	(a3)+,fp2
		fmove.s	4(a0),fp3
		fmul.s	(a3)+,fp3
		fmove.s	(a0),fp0
		fmul.s	(a3),fp0
	add.w	#$20,a5
		fadd.x	fp2,fp0
		fadd.x	fp3,fp0

		ftst.x	fp0
		fboge.w	.s0

		fmul.s	_r_shadelight,fp0
		fmove.l	fp0,d0
		add.l	d0,d6
		bge.b	.s0
		moveq	#0,d6
.s0
		move.l	d6,-$10(a5)
.atin
		cmp.l	_r_anumverts,d7
		blt.w	.atl0

		fmovem.x	(sp)+,fp2-fp7
		movem.l	(sp)+,d6-d7/a2-a3/a5
		add.w	#$4c,sp
		rts

; void R_ClipEdge (mvertex_t *pv0, mvertex_t *pv1, clipplane_t *clip);

;	clipping

		cnop	0,4
_R_ClipEdge:
		movem.l	a2-a3,-(sp)
		move.l	20(sp),a2
		move.l	16(sp),a1
		move.l	12(sp),a0
		fmovem.x	fp2-fp4,-(sp)

		bsr.b	R_ClipEdge_inner

		fmovem.x	(sp)+,fp2-fp4
		movem.l	(sp)+,a2-a3
		rts

		cnop	0,4
R_ClipEdge_inner:
		sub.w	#12,sp

		move.l	a2,d0
		beq.w	.emit
.next
		fmove.s	(a0),fp0
		fmul.s	(a2),fp0
		fmove.s	4(a0),fp1
		fmul.s	4(a2),fp1
		fmove.s	8(a0),fp2
		fmul.s	8(a2),fp2
		fadd.x	fp1,fp0
		fadd.x	fp2,fp0
		fsub.s	12(a2),fp0

		fmove.s	(a1),fp1
		fmul.s	(a2),fp1
		fmove.s	4(a1),fp2
		fmul.s	4(a2),fp2
		fmove.s	8(a1),fp3
		fmul.s	8(a2),fp3
		fadd.x	fp2,fp1
		fadd.x	fp3,fp1
		fsub.s	12(a2),fp1

		ftst.x	fp0
		fbolt.w	.enter
		ftst.x	fp1
		fbuge.w	.loop

		move.l	#$7fffffff,_cacheoffset

		fmove.x	fp0,fp2
		fsub.x	fp1,fp2
		fdiv.x	fp2,fp0

		fmove.s	(a0),fp1
		fmove.s	(a1),fp2
		fsub.x	fp1,fp2
		fmul.x	fp0,fp2
		fadd.x	fp1,fp2
		fmove.s	fp2,(sp)

		fmove.s	4(a0),fp1
		fmove.s	4(a1),fp2
		fsub.x	fp1,fp2
		fmul.x	fp0,fp2
		fadd.x	fp1,fp2
		fmove.s	fp2,4(sp)

		fmove.s	8(a0),fp1
		fmove.s	8(a1),fp2
		fsub.x	fp1,fp2
		fmul.x	fp0,fp2
		fadd.x	fp1,fp2
		fmove.s	fp2,8(sp)

		tst.b	$14(a2)
		beq.b	.ces0

		moveq	#1,d0
		move.l	d0,_r_leftclipped

		lea	_r_leftexit,a3
		bra.b	.s1
.ces0
		tst.b	$15(a2)
		beq.b	.s2

		moveq	#1,d0
		move.l	d0,_r_rightclipped

		lea	_r_rightexit,a3
.s1
		move.l	sp,a1
		move.l	(a1)+,(a3)+
		move.l	(a1)+,(a3)+
		move.l	(a1),(a3)
.s2
		move.l	sp,a1
		move.l	16(a2),a2
		bsr.w	R_ClipEdge_inner
		bra.w	.end
.enter
		ftst.x	fp1
		fboge.w	.s3
		tst.l	_r_leftclipped
		bne.w	.end

		move.l	_r_framecount,d0
		or.l	#$80000000,d0
		move.l	d0,_cacheoffset
		bra.w	.end
.s3
		clr.l	_r_lastvertvalid
		move.l	#$7fffffff,_cacheoffset

		fmove.x	fp0,fp2
		fsub.x	fp1,fp2
		fdiv.x	fp2,fp0

		fmove.s	(a0),fp1
		fmove.s	(a1),fp2
		fsub.x	fp1,fp2
		fmul.x	fp0,fp2
		fadd.x	fp1,fp2
		fmove.s	fp2,(sp)

		fmove.s	4(a0),fp1
		fmove.s	4(a1),fp2
		fsub.x	fp1,fp2
		fmul.x	fp0,fp2
		fadd.x	fp1,fp2
		fmove.s	fp2,4(sp)

		fmove.s	8(a0),fp1
		fmove.s	8(a1),fp2
		fsub.x	fp1,fp2
		fmul.x	fp0,fp2
		fadd.x	fp1,fp2
		fmove.s	fp2,8(sp)

		tst.b	$14(a2)
		beq.b	.s4

		moveq	#1,d0
		move.l	d0,_r_leftclipped

		lea	_r_leftenter,a3
		bra.b	.s5
.s4
		tst.b	$15(a2)
		beq.b	.s6

		moveq	#1,d0
		move.l	d0,_r_rightclipped

		lea	_r_rightenter,a3
.s5
		move.l	sp,a0
		move.l	(a0)+,(a3)+
		move.l	(a0)+,(a3)+
		move.l	(a0),(a3)
.s6
		move.l	sp,a0
		move.l	16(a2),a2
		bsr.w	R_ClipEdge_inner
		bra.b	.end
.loop
		move.l	16(a2),a2
		move.l	a2,d0
		bne.w	.next
.emit
		add.w	#12,sp
		bra.b	@R_EmitEdge
.end
		add.w	#12,sp
		rts

; void R_EmitEdge (mvertex_t *pv0, mvertex_t *pv1)

	; u0 = FP2
	; v0 = FP3
	; lzi0 = FP4

		cnop	0,4
@R_EmitEdge:
		movem.l	d4-d5/d7/a5,-(sp)

		move.l	a1,a2

		tst.l	_r_lastvertvalid
		beq.b	.notval
		fmove.s	_r_u1,fp2
		fmove.s	_r_v1,fp3
		fmove.s	_r_lzi1,fp4
		move.l	_r_ceilv1,d7
		bra.w	.valid
.notval
		lea	local(pc),a3
		fmove.s	(a0)+,fp0
		fsub.s	_modelorg,fp0
		fmove.s	fp0,(a3)+
		fmove.s	(a0)+,fp0
		fsub.s	_modelorg+4,fp0
		fmove.s	fp0,(a3)+
		fmove.s	(a0),fp0
		fsub.s	_modelorg+8,fp0
		fmove.s	fp0,(a3)

		lea	local(pc),a0
		lea	transformed(pc),a1
		bsr	@TransformVector

		fmove.s	transformed+8(pc),fp1
		fcmp.s	#1.000000e-02,fp1
		fboge.w	.res0
		fmove.s	#1.000000e-02,fp1
.res0
		fmove.s	#1,fp4
		fdiv.x	fp1,fp4

		fmove.s	_xscale,fp2
		fmul.x	fp4,fp2
		fmul.s	transformed(pc),fp2
		fadd.s	_xcenter,fp2

		fcmp.s	_r_refdef+$44,fp2
		fboge.w	.res1
		fmove.s	_r_refdef+$44,fp2
.res1
		fcmp.s	_r_refdef+$54,fp2
		fbole.w	.res2
		fmove.s	_r_refdef+$54,fp2
.res2

		fmove.s	_yscale,fp3
		fmul.x	fp4,fp3
		fmul.s	transformed+4(pc),fp3
		fneg.x	fp3
		fadd.s	_ycenter,fp3

		fcmp.s	_r_refdef+$48,fp3
		fboge.w	.res3
		fmove.s	_r_refdef+$48,fp3
.res3
		fcmp.s	_r_refdef+$58,fp3
		fbole.w	.res4
		fmove.s	_r_refdef+$58,fp3
.res4
		fmove.l	fp3,d7
		fcmp.l	d7,fp3
		fbule.w	.rc0
		addq.l	#1,d7
.rc0
.valid
		lea	local(pc),a3
		fmove.s	(a2)+,fp0
		fsub.s	_modelorg,fp0
		fmove.s	fp0,(a3)+
		fmove.s	(a2)+,fp0
		fsub.s	_modelorg+4,fp0
		fmove.s	fp0,(a3)+
		fmove.s	(a2),fp0
		fsub.s	_modelorg+8,fp0
		fmove.s	fp0,(a3)

		lea	local(pc),a0
		lea	transformed(pc),a1
		bsr	@TransformVector

		fmove.s	transformed+8(pc),fp1
		fcmp.s	#1.000000e-02,fp1
		fboge.w	.res5
		fmove.s	#1.000000e-02,fp1
.res5
		fmove.s	#1,fp0
		fdiv.x	fp1,fp0
		fmove.s	fp0,_r_lzi1

		fmove.s	_xscale,fp0
		fmul.s	_r_lzi1,fp0
		fmul.s	transformed(pc),fp0
		fadd.s	_xcenter,fp0

		fcmp.s	_r_refdef+$44,fp0
		fboge.w	.res6
		fmove.s	_r_refdef+$44,fp0
.res6
		fcmp.s	_r_refdef+$54,fp0
		fbole.w	.res7
		fmove.s	_r_refdef+$54,fp0
.res7
		fmove.s	fp0,_r_u1

		fmove.s	_yscale,fp0
		fmul.s	_r_lzi1,fp0
		fmul.s	transformed+4(pc),fp0
		fneg.x	fp0
		fadd.s	_ycenter,fp0

		fcmp.s	_r_refdef+$48,fp0
		fboge.w	.res8
		fmove.s	_r_refdef+$48,fp0
.res8
		fcmp.s	_r_refdef+$58,fp0
		fbole.w	.res9
		fmove.s	_r_refdef+$58,fp0
.res9
		fmove.s	fp0,_r_v1

		fmove.s	_r_lzi1,fp1
		fcmp.x	fp4,fp1
		fbole.w	.res10
		fmove.x	fp1,fp4
.res10
		fmove.x	fp4,fp1
		fcmp.s	_r_nearzi,fp1
		fbole.w	.res11
		fmove.s	fp1,_r_nearzi
.res11
		tst.l	_r_nearzionly
		bne.w	.endemit

		move.l	#1,_r_emitted

		fmove.l	fp0,d0
		fcmp.l	d0,fp0
		fbule.w	.rc1
		addq.l	#1,d0
.rc1
		move.l	d0,_r_ceilv1

		cmp.l	d7,d0
		bne.b	.nocreate
		cmp.l	#$7fffffff,_cacheoffset
		beq.w	.endemit

		move.l	_r_framecount,d0
		or.l	#$80000000,d0
		move.l	d0,_cacheoffset
		bra.w	.endemit
.nocreate
		fmove.s	_r_u1,fp0
		fsub.x	fp2,fp0
		fmove.s	_r_v1,fp1
		fsub.x	fp3,fp1
		fdiv.x	fp1,fp0

		move.l	_edge_p,a5
		add.l	#$20,_edge_p

		fmove.s	fp4,$18(a5)
		move.l	_r_pedge,$1c(a5)

		move.l	_surface_p,d1
		sub.l	_surfaces,d1
		asr.l	#6,d1

		cmp.l	d0,d7
		bgt.b	.notrail

		move.l	d7,d5

		fmove.l	d5,fp1
		fsub.x	fp3,fp1
		fmul.x	fp0,fp1
	move.l	_r_ceilv1,d4
	move.w	d1,$10(a5)
	clr.w	$12(a5)
		fadd.x	fp2,fp1

		bra.b	.nolead
.notrail
		move.l	_r_ceilv1,d5

		fmove.l	d5,fp1
		fsub.s	_r_v1,fp1
		fmul.x	fp0,fp1
	move.l	d7,d4
	clr.w	$10(a5)
	move.w	d1,$12(a5)
		fadd.s	_r_u1,fp1
.nolead
		fmul.s	#1048576,fp1
	lea	_newedges,a0
	subq.l	#1,d4
	lea	0(a0,d5.l*4),a1
	move.l	(a1),a0
		fadd.s	#1048575,fp1
		fmove.l	fp1,d0

		fmul.s	#1048576,fp0
		cmp.l	_r_refdef+$4c,d0
		bge.b	.rer0
		move.l	_r_refdef+$4c,d0
.rer0
		cmp.l	_r_refdef+$50,d0
		ble.b	.rer1
		move.l	_r_refdef+$50,d0
.rer1
		move.l	d0,(a5)
		fmove.l	fp0,4(a5)

		tst.w	$10(a5)
		beq.b	.rer2
		addq.l	#1,d0
.rer2
		move.l	d0,d7

		move.l	a0,d0
		beq.b	.re43
		move.l	(a0),d1
		cmp.l	d7,d1
		blt.b	.re44
.re43:
		move.l	d0,12(a5)
		move.l	a5,(a1)
		bra.b	.re49
.re44:
		move.l	d0,a3
		dc.w	$0c40
.re45:
		move.l	a2,a3
		move.l	12(a3),a2
		move.l	a2,d0
		beq.b	.re48
		cmp.l	(a2),d7
		bge.b	.re45
.re48:
		move.l	a2,12(a5)
		move.l	a5,12(a3)
.re49:
		lea	_removeedges,a0
		lea	0(a0,d4.l*4),a1
		move.l	(a1),$14(a5)
		move.l	a5,(a1)
.endemit
		movem.l	(sp)+,d4-d5/d7/a5
		rts

		cnop	0,8
local:		dc.l	0,0,0
transformed:	dc.l	0,0,0

; void TransformVector (vec3_t in, vec3_t out);

;	general vector transformation

_TransformVector:
	move.l	8(sp),a1
	move.l	4(sp),a0
@TransformVector:
	move.l	a2,-(sp)

	lea	_vright,a2
		fmove.s	(a2)+,fp0
		fmul.s	(a0)+,fp0
		fmove.s	(a2)+,fp1
		fmul.s	(a0)+,fp1
		fadd.x	fp1,fp0
		fmove.s	(a2),fp1
		fmul.s	(a0),fp1
	subq.l	#8,a0
	lea	_vup,a2
		fadd.x	fp1,fp0
		fmove.s	fp0,(a1)+

		fmove.s	(a2)+,fp0
		fmul.s	(a0)+,fp0
		fmove.s	(a2)+,fp1
		fmul.s	(a0)+,fp1
		fadd.x	fp1,fp0
		fmove.s	(a2),fp1
		fmul.s	(a0),fp1
	subq.l	#8,a0
	lea	_vpn,a2
		fadd.x	fp1,fp0
		fmove.s	fp0,(a1)+

		fmove.s	(a2)+,fp0
		fmul.s	(a0)+,fp0
		fmove.s	(a2)+,fp1
		fmul.s	(a0)+,fp1
		fadd.x	fp1,fp0
		fmove.s	(a2),fp1
		fmul.s	(a0),fp1
	move.l	(sp)+,a2
		fadd.x	fp1,fp0
		fmove.s	fp0,(a1)
	rts

; void R_DrawSurfaceBlock16 (void)

;	dummy

_R_DrawSurfaceBlock16:	; not needed in quake 1
		rts


		END
