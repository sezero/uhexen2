; Quake for AMIGA
; byte swap assembler implementations by Frank Wille <frank@phoenix.owl.de>

	XDEF _ShortSwap
	XDEF _LongSwap
	XDEF _FloatSwap

; vasm extensions
	machine 68020
	fpu     1

; short _ShortSwap (short l)
; swap word (LE->BE)
;
	cnop    0,4
_ShortSwap

	rsreset
	rs.l    1
	rs.w	1
.data   rs.w    1

	move   .data(sp),d0
	ror     #8,d0
	rts

; int _LongSwap (int l)
; swap longword (LE->BE)
;
	cnop    0,4
_LongSwap

	rsreset
	rs.l    1
.data   rs.l    1

	move.l .data(sp),d0
	ror     #8,d0
	swap    d0
	ror     #8,d0
	rts

; float _FloatSwap (float f)
; swap float (LE->BE)
;
	cnop    0,4
_FloatSwap

	rsreset
	rs.l    1
.data   rs.s    1

	move.l .data(sp),d0
	ror     #8,d0
	swap    d0
	ror     #8,d0
	fmove.s d0,fp0
	rts
