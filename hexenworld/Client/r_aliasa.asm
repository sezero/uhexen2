;
; r_aliasa.asm
; x86 assembly-language Alias model transform and project code.
;
; this file uses NASM syntax.
; $Id: r_aliasa.asm,v 1.4 2008-03-15 09:01:08 sezero Exp $
;

%idefine offset

; externs from C code
 extern r_apverts
 extern r_anumverts
 extern aliastransform
 extern r_avertexnormals
 extern r_plightvec
 extern r_ambientlight
 extern r_shadelight
 extern aliasxcenter
 extern aliasycenter

; externs from ASM-only code


SEGMENT .data

Lfloat_1 dd 1.0
Ltemp dd 0
Lcoords dd 0, 0, 0


SEGMENT .text

;;;;;;;;;;;;;;;;;;;;;;;;
; R_AliasTransformAndProjectFinalVerts
;;;;;;;;;;;;;;;;;;;;;;;;

 global R_AliasTransformAndProjectFinalVerts
R_AliasTransformAndProjectFinalVerts:
 push ebp
 push edi
 push esi
 mov esi,dword[r_apverts]
 mov ebp,dword[12+8+esp]
 mov edi,dword[12+4+esp]
 mov ecx,dword[r_anumverts]
 sub edx,edx
Lloop:
 mov dl,byte[esi]
 mov byte[Lcoords],dl
 fild dword[Lcoords]
 mov dl,byte[1+esi]
 mov byte[Lcoords+4],dl
 fild dword[Lcoords+4]
 mov dl,byte[2+esi]
 mov byte[Lcoords+8],dl
 fild dword[Lcoords+8]
 fld st2
 fmul dword[aliastransform+32]
 fld st2
 fmul dword[aliastransform+36]
 fxch st1
 fadd dword[aliastransform+44]
 fld st2
 fmul dword[aliastransform+40]
 fxch st1
 faddp st2,st0
 mov dl,byte[3+esi]
; mov eax,dword[4+ebp]	;load .s
; mov dword[0+8+edi],eax	;store .s
 faddp st1,st0
; mov eax,dword[8+ebp]	;.t
; mov dword[0+12+edi],eax	;.t
 fdivr dword[Lfloat_1]
; mov eax,dword[0+ebp]	; .onseam
; mov dword[24+edi],eax
 
 mov eax,dword[32+edi]
 mov eax,dword[12+ebp]
 mov eax,dword[4+esi]
 lea eax,[edx+edx*2]
 fxch st3
 fld dword[r_avertexnormals+eax*4]
 fmul dword[r_plightvec]
 fld dword[r_avertexnormals+4+eax*4]
 fmul dword[r_plightvec+4]
 fld dword[r_avertexnormals+8+eax*4]
 fmul dword[r_plightvec+8]
 fxch st1
 faddp st2,st0
 fld st2
 fmul dword[aliastransform+0]
 fxch st2
 faddp st1,st0
 fst dword[Ltemp]
 mov eax,dword[r_ambientlight]
 mov dl,byte[Ltemp+3]
 test dl,080h
 jz near Lsavelight
 fmul dword[r_shadelight]
 fistp dword[Ltemp]
 add eax,dword[Ltemp]
 jns Lp1
 sub eax,eax
Lp1:
 fxch st1
 fmul dword[aliastransform+16]
 fxch st3
 fld st0
 fmul dword[aliastransform+4]
 fxch st1
 mov dword[0+16+edi],eax
 fmul dword[aliastransform+20]
 fxch st2
 fadd dword[aliastransform+12]
 fxch st4
 fadd dword[aliastransform+28]
 fxch st3
 fld st0
 fmul dword[aliastransform+8]
 fxch st1
 fmul dword[aliastransform+24]
 fxch st5
 faddp st2,st0
 fxch st3
 faddp st2,st0
 add esi,4
 faddp st2,st0
 faddp st2,st0
 add ebp,12
 fmul st0,st2
 fxch st1
 fmul st0,st2
 fxch st1
 fadd dword[aliasxcenter]
 fxch st1
 fadd dword[aliasycenter]
 fxch st2
 fistp dword[0+20+edi]
 fistp dword[0+0+edi]
 fistp dword[0+4+edi]
 add edi,32
 dec ecx
 jnz near Lloop
 pop esi
 pop edi
 pop ebp
 ret
Lsavelight:
 fstp st0
 jmp Lp1

