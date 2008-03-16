;
; r_varsa.asm
; global refresh variables
;
; this file uses NASM syntax.
; $Id: r_varsa.asm,v 1.5 2008-03-16 14:30:46 sezero Exp $
;

%include "asm_nasm.inc"

; underscore prefix handling
; for C-shared symbols:
%ifmacro _sym_prefix
; C-shared externs:
; C-shared globals:
 _sym_prefix r_bmodelactive
%endif	; _sym_prefix

SEGMENT .data

 global float_1, float_particle_z_clip, float_point5
 global float_minus_1, float_0
float_0 dd 0.0
float_1 dd 1.0
float_minus_1 dd -1.0
float_particle_z_clip dd 8.0
float_point5 dd 0.5

 global fp_16, fp_64k, fp_1m, fp_64kx64k
 global fp_1m_minus_1
 global fp_8
fp_1m dd 1048576.0
fp_1m_minus_1 dd 1048575.0
fp_64k dd 65536.0
fp_8 dd 8.0
fp_16 dd 16.0
fp_64kx64k dd 04f000000h

 global FloatZero, Float2ToThe31nd, FloatMinus2ToThe31nd
FloatZero dd 0
Float2ToThe31nd dd 04f000000h
FloatMinus2ToThe31nd dd 0cf000000h

 global r_bmodelactive
r_bmodelactive dd 0

