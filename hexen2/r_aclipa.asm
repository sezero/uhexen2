;
; r_aclipa.asm
; x86 assembly-language Alias model transform and project code.
;
; this file uses NASM syntax.
; $Id: r_aclipa.asm,v 1.5 2008-03-16 14:30:46 sezero Exp $
;

%include "asm_nasm.inc"

; underscore prefix handling
; for C-shared symbols:
%ifmacro _sym_prefix
; C-shared externs:
 _sym_prefix r_refdef
; C-shared globals:
 _sym_prefix R_Alias_clip_bottom
 _sym_prefix R_Alias_clip_top
 _sym_prefix R_Alias_clip_right
 _sym_prefix R_Alias_clip_left
%endif	; _sym_prefix

; externs from C code
 extern r_refdef

; externs from ASM-only code
 extern float_point5


SEGMENT .data

Ltemp0 dd 0
Ltemp1 dd 0

SEGMENT .text

;;;;;;;;;;;;;;;;;;;;;;;;
; R_Alias_clip_bottom
;;;;;;;;;;;;;;;;;;;;;;;;

 global R_Alias_clip_bottom
R_Alias_clip_bottom:
 push esi
 push edi
 mov esi, dword [8+4+esp]
 mov edi, dword [8+8+esp]
 mov eax, dword [r_refdef+52]
LDoForwardOrBackward:
 mov edx, dword [0+4+esi]
 mov ecx, dword [0+4+edi]
 cmp edx,ecx
 jl LDoForward
 mov ecx, dword [0+4+esi]
 mov edx, dword [0+4+edi]
 mov edi, dword [8+4+esp]
 mov esi, dword [8+8+esp]
LDoForward:
 sub ecx,edx
 sub eax,edx
 mov  dword [Ltemp1],ecx
 mov  dword [Ltemp0],eax
 fild  dword [Ltemp1]
 fild  dword [Ltemp0]
 mov edx, dword [8+12+esp]
 mov eax,2
 fdivrp st1,st0
LDo3Forward:
 fild  dword [0+0+esi]
 fild  dword [0+0+edi]
 fild  dword [0+4+esi]
 fild  dword [0+4+edi]
 fild  dword [0+8+esi]
 fild  dword [0+8+edi]
 fxch st5
 fsub st4,st0
 fxch st3
 fsub st2,st0
 fxch st1
 fsub st5,st0
 fxch st6
 fmul st4,st0
 add edi,12
 fmul st2,st0
 add esi,12
 add edx,12
 fmul st5,st0
 fxch st3
 faddp st4,st0
 faddp st1,st0
 fxch st4
 faddp st3,st0
 fxch st1
 fadd  dword [float_point5]
 fxch st3
 fadd  dword [float_point5]
 fxch st2
 fadd  dword [float_point5]
 fxch st3
 fistp  dword [0+0-12+edx]
 fxch st1
 fistp  dword [0+4-12+edx]
 fxch st1
 fistp  dword [0+8-12+edx]
 dec eax
 jnz LDo3Forward
 fstp st0
 pop edi
 pop esi
 ret


;;;;;;;;;;;;;;;;;;;;;;;;
; R_Alias_clip_top
;;;;;;;;;;;;;;;;;;;;;;;;

 global R_Alias_clip_top
R_Alias_clip_top:
 push esi
 push edi
 mov esi, dword [8+4+esp]
 mov edi, dword [8+8+esp]
 mov eax, dword [r_refdef+20+4]
 jmp LDoForwardOrBackward


;;;;;;;;;;;;;;;;;;;;;;;;
; R_Alias_clip_right
;;;;;;;;;;;;;;;;;;;;;;;;

 global R_Alias_clip_right
R_Alias_clip_right:
 push esi
 push edi
 mov esi, dword [8+4+esp]
 mov edi, dword [8+8+esp]
 mov eax, dword [r_refdef+48]
LRightLeftEntry:
 mov edx, dword [0+4+esi]
 mov ecx, dword [0+4+edi]
 cmp edx,ecx
 mov edx, dword [0+0+esi]
 mov ecx, dword [0+0+edi]
 jl LDoForward2
 mov ecx, dword [0+0+esi]
 mov edx, dword [0+0+edi]
 mov edi, dword [8+4+esp]
 mov esi, dword [8+8+esp]
LDoForward2:
 jmp LDoForward


;;;;;;;;;;;;;;;;;;;;;;;;
; R_Alias_clip_left
;;;;;;;;;;;;;;;;;;;;;;;;

 global R_Alias_clip_left
R_Alias_clip_left:
 push esi
 push edi
 mov esi, dword [8+4+esp]
 mov edi, dword [8+8+esp]
 mov eax, dword [r_refdef+20+0]
 jmp LRightLeftEntry

