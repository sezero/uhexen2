;
; surf16.asm
; x86 assembly-language 16 bpp surface block drawing code.
;
; this file uses NASM syntax.
; $Id: surf16.asm,v 1.4 2008-03-14 18:55:55 sezero Exp $
;

%idefine offset

; externs from C code
 extern prowdestbase
 extern pbasesource
 extern lightright
 extern lightrightstep
 extern lightleft
 extern lightleftstep
 extern lightdeltastep
 extern lightdelta
 extern sourcetstep
 extern surfrowbytes
 extern colormap
 extern blocksize
 extern sourcesstep
 extern blockdivshift
 extern blockdivmask
 extern r_lightptr
 extern r_lightwidth
 extern r_numvblocks
 extern r_sourcemax
 extern r_stepback

; externs from ASM-only code


SEGMENT .data

k dd 0
loopentry dd 0

 ALIGN 4

blockjumptable16:
 dd LEnter2_16
 dd LEnter4_16
 dd 0, LEnter8_16
 dd 0, 0, 0, LEnter16_16

SEGMENT .text

 ALIGN 4

;;;;;;;;;;;;;;;;;;;;;;;;
; R_Surf16Start
;;;;;;;;;;;;;;;;;;;;;;;;
 global R_Surf16Start
R_Surf16Start:

;;;;;;;;;;;;;;;;;;;;;;;;
; R_DrawSurfaceBlock16
;;;;;;;;;;;;;;;;;;;;;;;;

 ALIGN 4

 global R_DrawSurfaceBlock16
R_DrawSurfaceBlock16:
 push ebp
 push edi
 push esi
 push ebx
 mov eax, dword [blocksize]
 mov edi, dword [prowdestbase]
 mov esi, dword [pbasesource]
 mov ebx, dword [sourcesstep]
 mov ecx, dword [blockjumptable16-4+eax*2]
 mov  dword [k],eax
 mov  dword [loopentry],ecx
 mov edx, dword [lightleft]
 mov ebp, dword [lightright]
Lblockloop16:
 sub ebp,edx
 mov cl, byte [blockdivshift]
 sar ebp,cl
 jns Lp1_16
 test ebp, dword [blockdivmask]
 jz Lp1_16
 inc ebp
Lp1_16:
 sub eax,eax
 sub ecx,ecx
 jmp dword[loopentry]
 ALIGN 4
LEnter16_16:
 mov al, byte [esi]
 mov cl, byte [esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi, [esi+ebx*2]
 mov ax, word [12345678h+eax*2]
LBPatch0:
 add edx,ebp
 mov  word [edi],ax
 mov cx, word [12345678h+ecx*2]
LBPatch1:
 mov  word [2+edi],cx
 add edi,04h
 mov al, byte [esi]
 mov cl, byte [esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi, [esi+ebx*2]
 mov ax, word [12345678h+eax*2]
LBPatch2:
 add edx,ebp
 mov  word [edi],ax
 mov cx, word [12345678h+ecx*2]
LBPatch3:
 mov  word [2+edi],cx
 add edi,04h
 mov al, byte [esi]
 mov cl, byte [esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi, [esi+ebx*2]
 mov ax, word [12345678h+eax*2]
LBPatch4:
 add edx,ebp
 mov  word [edi],ax
 mov cx, word [12345678h+ecx*2]
LBPatch5:
 mov  word [2+edi],cx
 add edi,04h
 mov al, byte [esi]
 mov cl, byte [esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi, [esi+ebx*2]
 mov ax, word [12345678h+eax*2]
LBPatch6:
 add edx,ebp
 mov  word [edi],ax
 mov cx, word [12345678h+ecx*2]
LBPatch7:
 mov  word [2+edi],cx
 add edi,04h
LEnter8_16:
 mov al, byte [esi]
 mov cl, byte [esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi, [esi+ebx*2]
 mov ax, word [12345678h+eax*2]
LBPatch8:
 add edx,ebp
 mov  word [edi],ax
 mov cx, word [12345678h+ecx*2]
LBPatch9:
 mov  word [2+edi],cx
 add edi,04h
 mov al, byte [esi]
 mov cl, byte [esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi, [esi+ebx*2]
 mov ax, word [12345678h+eax*2]
LBPatch10:
 add edx,ebp
 mov  word [edi],ax
 mov cx, word [12345678h+ecx*2]
LBPatch11:
 mov  word [2+edi],cx
 add edi,04h
LEnter4_16:
 mov al, byte [esi]
 mov cl, byte [esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi, [esi+ebx*2]
 mov ax, word [12345678h+eax*2]
LBPatch12:
 add edx,ebp
 mov  word [edi],ax
 mov cx, word [12345678h+ecx*2]
LBPatch13:
 mov  word [2+edi],cx
 add edi,04h
LEnter2_16:
 mov al, byte [esi]
 mov cl, byte [esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi, [esi+ebx*2]
 mov ax, word [12345678h+eax*2]
LBPatch14:
 add edx,ebp
 mov  word [edi],ax
 mov cx, word [12345678h+ecx*2]
LBPatch15:
 mov  word [2+edi],cx
 add edi,04h
 mov esi, dword [pbasesource]
 mov edx, dword [lightleft]
 mov ebp, dword [lightright]
 mov eax, dword [sourcetstep]
 mov ecx, dword [lightrightstep]
 mov edi, dword [prowdestbase]
 add esi,eax
 add ebp,ecx
 mov eax, dword [lightleftstep]
 mov ecx, dword [surfrowbytes]
 add edx,eax
 add edi,ecx
 mov  dword [pbasesource],esi
 mov  dword [lightright],ebp
 mov eax, dword [k]
 mov  dword [lightleft],edx
 dec eax
 mov  dword [prowdestbase],edi
 mov  dword [k],eax
 jnz near Lblockloop16
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret

;;;;;;;;;;;;;;;;;;;;;;;;
; R_Surf16End
;;;;;;;;;;;;;;;;;;;;;;;;
 global R_Surf16End
R_Surf16End:

SEGMENT .data

 ALIGN 4

LPatchTable16:
 dd LBPatch0-4
 dd LBPatch1-4
 dd LBPatch2-4
 dd LBPatch3-4
 dd LBPatch4-4
 dd LBPatch5-4
 dd LBPatch6-4
 dd LBPatch7-4
 dd LBPatch8-4
 dd LBPatch9-4
 dd LBPatch10-4
 dd LBPatch11-4
 dd LBPatch12-4
 dd LBPatch13-4
 dd LBPatch14-4
 dd LBPatch15-4

SEGMENT .text

 ALIGN 4

;;;;;;;;;;;;;;;;;;;;;;;;
; R_Surf16Patch
;;;;;;;;;;;;;;;;;;;;;;;;

 global R_Surf16Patch
R_Surf16Patch:
 push ebx
 mov eax, dword [colormap]
 mov ebx,offset LPatchTable16
 mov ecx,16
LPatchLoop16:
 mov edx, dword [ebx]
 add ebx,4
 mov  dword [edx],eax
 dec ecx
 jnz LPatchLoop16
 pop ebx
 ret

