;
; d_partb.asm
; x86 assembly-language 8-bpp particle-drawing code.
;
; this file uses NASM syntax.
; $Id: d_partb.asm,v 1.6 2008-03-14 12:01:17 sezero Exp $
;

%idefine offset

; externs from C code
 extern d_pzbuffer
 extern d_zrowbytes
 extern d_viewbuffer
 extern d_scantable
 extern r_origin
 extern r_ppn
 extern r_pup
 extern r_pright
 extern ycenter
 extern xcenter
 extern d_vrectbottom_particle
 extern d_vrectright_particle
 extern d_vrecty
 extern d_vrectx
 extern d_pix_shift
 extern d_pix_min
 extern d_pix_max
 extern d_y_aspect_shift
 extern screenwidth
 extern transTable

; externs from ASM-only code
 extern float_point5
 extern izistep
 extern izi
 extern float_1
 extern float_particle_z_clip
 extern float_minus_1
 extern float_0
 extern DP_Count
 extern DP_u
 extern DP_v
 extern DP_32768
 extern DP_Color
 extern DP_Pix
 extern DP_EntryTable


SEGMENT .text

 ALIGN 4

;;;;;;;;;;;;;;;;;;;;;;;;
; D_DrawParticle1x1b
;;;;;;;;;;;;;;;;;;;;;;;;

 global D_DrawParticle1x1b
D_DrawParticle1x1b: 
 push ebp
 push edi
 push ebx
 mov edi, dword [12+4+esp]
 fld  dword [r_origin]
 fsubr  dword [0+edi]
 fld  dword [0+4+edi]
 fsub  dword [r_origin+4]
 fld  dword [0+8+edi]
 fsub  dword [r_origin+8]
 fxch st2
 fld  dword [r_ppn]
 fmul st0,st1
 fld  dword [r_ppn+4]
 fmul st0,st3
 fld  dword [r_ppn+8]
 fmul st0,st5
 fxch st2
 faddp st1,st0
 faddp st1,st0
 fld st0
 fdivr  dword [float_1]
 fxch st1
 fcomp  dword [float_particle_z_clip]
 fxch st3
 fld  dword [r_pup]
 fmul st0,st2
 fld  dword [r_pup+4]
 fnstsw ax
 test ah,1
 jnz near LPop6AndDone
 fmul st0,st4
 fld  dword [r_pup+8]
 fmul st0,st3
 fxch st2
 faddp st1,st0
 faddp st1,st0
 fxch st3
 fmul  dword [r_pright+4]
 fxch st2
 fmul  dword [r_pright]
 fxch st1
 fmul  dword [r_pright+8]
 fxch st2
 faddp st1,st0
 faddp st1,st0
 fxch st1
 fmul st0,st2
 fxch st1
 fmul st0,st2
 fxch st1
 fsubr  dword [ycenter]
 fxch st1
 fadd  dword [xcenter]
 fxch st1
 fadd  dword [float_point5]
 fxch st1
 fadd  dword [float_point5]
 fxch st2
 fmul  dword [DP_32768]
 fxch st2
 fistp  dword [DP_u]
 fistp  dword [DP_v]
 mov eax, dword [DP_u]
 mov edx, dword [DP_v]
 mov ebx, dword [d_vrectbottom_particle]
 mov ecx, dword [d_vrectright_particle]
 cmp edx,ebx
 jg near LPop1AndDone
 cmp eax,ecx
 jg near LPop1AndDone
 mov ebx, dword [d_vrecty]
 mov ecx, dword [d_vrectx]
 cmp edx,ebx
 jl near LPop1AndDone
 cmp eax,ecx
 jl near LPop1AndDone
 fld  dword [12+edi]
 fistp  dword [DP_Color]
 mov ebx, dword [d_viewbuffer]
 add ebx,eax
 mov edi, dword [d_scantable+edx*4]
 imul edx, dword [d_zrowbytes]
 lea edx, [edx+eax*2]
 mov eax, dword [d_pzbuffer]
 fistp  dword [izi]
 add edi,ebx
 add edx,eax
 mov eax, dword [izi]
 mov ecx, dword [d_pix_shift]
 shr eax,cl
 mov ebp, dword [izi]
 mov ebx, dword [d_pix_min]
 mov ecx, dword [d_pix_max]
 cmp eax,ebx
 jnl LTestPixMax
 mov eax,ebx
 jmp LTestDone
LTestPixMax:
 cmp eax,ecx
 jng LTestDone
 mov eax,ecx
LTestDone:
 mov cx, word [DP_Color]  ; get color
 mov ebx, dword [d_y_aspect_shift]
 test ebx,ebx
 jnz LDefault
 cmp eax,4
 ja LDefault


;;;;;;;;;;;;;;;;;;;;;;;;
; globals DP_?x?b
;;;;;;;;;;;;;;;;;;;;;;;;

 global DP_1x1b
DP_1x1b:
 cmp  word [edx],bp
 jg near LDone
; mov  word [edx],bp
 test ch,ch
 jnz Trans
 mov  byte [edi],cl
 jmp LDone
Trans:
 and ecx, 0ffh
 mov ch, cl
 mov eax,  dword [transTable]
 mov cl,  byte [edi]
 mov cl,  byte [eax+ecx]
 mov  byte [edi], cl
 jmp LDone


LDefault:
 test ch,ch
 jnz LTDefault

 mov ebx,eax
 mov  dword [DP_Pix],eax
 push cx
 mov cl, byte [d_y_aspect_shift]
 shl ebx,cl
 pop cx

LGenRowLoop:
 mov eax, dword [DP_Pix]
LGenColLoop:
 cmp  word [-2+edx+eax*2],bp
 jg LGSkip
 mov  word [-2+edx+eax*2],bp
 mov  byte [-1+edi+eax*1],cl
LGSkip:
 dec eax
 jnz LGenColLoop
 add edx, dword [d_zrowbytes]
 add edi, dword [screenwidth]
 dec ebx
 jnz LGenRowLoop
 jmp LDone

LTDefault:
 push esi
 and ecx, 0ffh
 mov ch, cl
 mov esi,  dword [transTable]

 mov ebx,eax
 mov  dword [DP_Pix],eax
 mov cl, byte [d_y_aspect_shift]
 shl ebx,cl
 
LTGenRowLoop:
 mov eax, dword [DP_Pix]
LTGenColLoop:
 cmp  word [-2+edx+eax*2],bp
 jg LTGSkip
 mov  word [-2+edx+eax*2],bp

 mov cl,  byte [-1+edi+eax*1]
 mov cl,  byte [esi+ecx]
 mov  byte [-1+edi+eax*1],cl

LTGSkip:
 dec eax
 jnz LTGenColLoop
 add edx, dword [d_zrowbytes]
 add edi, dword [screenwidth]
 dec ebx
 jnz LTGenRowLoop

 pop esi

LDone:
 pop ebx
 pop edi
 pop ebp
 ret
LPop6AndDone:
 fstp st0
 fstp st0
 fstp st0
 fstp st0
 fstp st0
LPop1AndDone:
 fstp st0
 jmp LDone

