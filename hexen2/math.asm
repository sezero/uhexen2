;
; math.asm
; x86 assembly-language math routines.
;
; this file uses NASM syntax.
; $Id: math.asm,v 1.6 2008-03-14 09:08:13 sezero Exp $
;

%idefine offset

 extern vright
 extern vup
 extern vpn
 extern BOPS_Error

SEGMENT .data
 ALIGN 4

Ljmptab dd Lcase0, Lcase1, Lcase2, Lcase3
 dd Lcase4, Lcase5, Lcase6, Lcase7


SEGMENT .text

 global Invert24To16
Invert24To16:
 mov ecx, dword [4+esp]
 mov edx,0100h
 cmp ecx,edx
 jle LOutOfRange
 sub eax,eax
 div ecx
 ret
LOutOfRange:
 mov eax,0FFFFFFFFh
 ret


 ALIGN 2

 global TransformVector
TransformVector:
 mov eax, dword [4+esp]
 mov edx, dword [8+esp]
 fld  dword [eax]
 fmul  dword [vright]
 fld  dword [eax]
 fmul  dword [vup]
 fld  dword [eax]
 fmul  dword [vpn]
 fld  dword [4+eax]
 fmul  dword [vright+4]
 fld  dword [4+eax]
 fmul  dword [vup+4]
 fld  dword [4+eax]
 fmul  dword [vpn+4]
 fxch st2
 faddp st5,st0
 faddp st3,st0
 faddp st1,st0
 fld  dword [8+eax]
 fmul  dword [vright+8]
 fld  dword [8+eax]
 fmul  dword [vup+8]
 fld  dword [8+eax]
 fmul  dword [vpn+8]
 fxch st2
 faddp st5,st0
 faddp st3,st0
 faddp st1,st0
 fstp  dword [8+edx]
 fstp  dword [4+edx]
 fstp  dword [edx]
 ret


 ALIGN 2

 global BoxOnPlaneSide
BoxOnPlaneSide:
 push ebx
 mov edx, dword [4+12+esp]
 mov ecx, dword [4+4+esp]
 xor eax,eax
 mov ebx, dword [4+8+esp]
 mov al, byte [17+edx]
 cmp al,8
 jge near Lerror
 fld  dword [0+edx]
 fld st0
 jmp dword[Ljmptab+eax*4]
Lcase0:
 fmul  dword [ebx]
 fld  dword [0+4+edx]
 fxch st2
 fmul  dword [ecx]
 fxch st2
 fld st0
 fmul  dword [4+ebx]
 fld  dword [0+8+edx]
 fxch st2
 fmul  dword [4+ecx]
 fxch st2
 fld st0
 fmul  dword [8+ebx]
 fxch st5
 faddp st3,st0
 fmul  dword [8+ecx]
 fxch st1
 faddp st3,st0
 fxch st3
 faddp st2,st0
 jmp LSetSides
Lcase1:
 fmul  dword [ecx]
 fld  dword [0+4+edx]
 fxch st2
 fmul  dword [ebx]
 fxch st2
 fld st0
 fmul  dword [4+ebx]
 fld  dword [0+8+edx]
 fxch st2
 fmul  dword [4+ecx]
 fxch st2
 fld st0
 fmul  dword [8+ebx]
 fxch st5
 faddp st3,st0
 fmul  dword [8+ecx]
 fxch st1
 faddp st3,st0
 fxch st3
 faddp st2,st0
 jmp LSetSides
Lcase2:
 fmul  dword [ebx]
 fld  dword [0+4+edx]
 fxch st2
 fmul  dword [ecx]
 fxch st2
 fld st0
 fmul  dword [4+ecx]
 fld  dword [0+8+edx]
 fxch st2
 fmul  dword [4+ebx]
 fxch st2
 fld st0
 fmul  dword [8+ebx]
 fxch st5
 faddp st3,st0
 fmul  dword [8+ecx]
 fxch st1
 faddp st3,st0
 fxch st3
 faddp st2,st0
 jmp LSetSides
Lcase3:
 fmul  dword [ecx]
 fld  dword [0+4+edx]
 fxch st2
 fmul  dword [ebx]
 fxch st2
 fld st0
 fmul  dword [4+ecx]
 fld  dword [0+8+edx]
 fxch st2
 fmul  dword [4+ebx]
 fxch st2
 fld st0
 fmul  dword [8+ebx]
 fxch st5
 faddp st3,st0
 fmul  dword [8+ecx]
 fxch st1
 faddp st3,st0
 fxch st3
 faddp st2,st0
 jmp LSetSides
Lcase4:
 fmul  dword [ebx]
 fld  dword [0+4+edx]
 fxch st2
 fmul  dword [ecx]
 fxch st2
 fld st0
 fmul  dword [4+ebx]
 fld  dword [0+8+edx]
 fxch st2
 fmul  dword [4+ecx]
 fxch st2
 fld st0
 fmul  dword [8+ecx]
 fxch st5
 faddp st3,st0
 fmul  dword [8+ebx]
 fxch st1
 faddp st3,st0
 fxch st3
 faddp st2,st0
 jmp LSetSides
Lcase5:
 fmul  dword [ecx]
 fld  dword [0+4+edx]
 fxch st2
 fmul  dword [ebx]
 fxch st2
 fld st0
 fmul  dword [4+ebx]
 fld  dword [0+8+edx]
 fxch st2
 fmul  dword [4+ecx]
 fxch st2
 fld st0
 fmul  dword [8+ecx]
 fxch st5
 faddp st3,st0
 fmul  dword [8+ebx]
 fxch st1
 faddp st3,st0
 fxch st3
 faddp st2,st0
 jmp LSetSides
Lcase6:
 fmul  dword [ebx]
 fld  dword [0+4+edx]
 fxch st2
 fmul  dword [ecx]
 fxch st2
 fld st0
 fmul  dword [4+ecx]
 fld  dword [0+8+edx]
 fxch st2
 fmul  dword [4+ebx]
 fxch st2
 fld st0
 fmul  dword [8+ecx]
 fxch st5
 faddp st3,st0
 fmul  dword [8+ebx]
 fxch st1
 faddp st3,st0
 fxch st3
 faddp st2,st0
 jmp LSetSides
Lcase7:
 fmul  dword [ecx]
 fld  dword [0+4+edx]
 fxch st2
 fmul  dword [ebx]
 fxch st2
 fld st0
 fmul  dword [4+ecx]
 fld  dword [0+8+edx]
 fxch st2
 fmul  dword [4+ebx]
 fxch st2
 fld st0
 fmul  dword [8+ecx]
 fxch st5
 faddp st3,st0
 fmul  dword [8+ebx]
 fxch st1
 faddp st3,st0
 fxch st3
 faddp st2,st0
LSetSides:
 faddp st2,st0
 fcomp  dword [12+edx]
 xor ecx,ecx
 fnstsw ax
 fcomp  dword [12+edx]
 and ah,1
 xor ah,1
 add cl,ah
 fnstsw ax
 and ah,1
 add ah,ah
 add cl,ah
 pop ebx
 mov eax,ecx
 ret
Lerror:
 call BOPS_Error			; call near BOPS_Error

