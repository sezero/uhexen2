;
; snd_mixa.asm
; x86 assembly-language sound code
;
; this file uses NASM syntax.
; $Id: snd_mixa.asm,v 1.4 2008-03-14 09:08:13 sezero Exp $
;

%idefine offset

 extern snd_scaletable
 extern paintbuffer
 extern snd_linear_count
 extern snd_p
 extern snd_vol
 extern snd_out

SEGMENT .text

 global SND_PaintChannelFrom8
SND_PaintChannelFrom8:
 push esi
 push edi
 push ebx
 push ebp
 mov ebx, dword [4+16+esp]
 mov esi, dword [8+16+esp]
 mov eax, dword [4+ebx]
 mov edx, dword [8+ebx]
 cmp eax,255
 jna LLeftSet
 mov eax,255
LLeftSet:
 cmp edx,255
 jna LRightSet
 mov edx,255
LRightSet:
 and eax,0F8h
 add esi,20
 and edx,0F8h
 mov edi, dword [16+ebx]
 mov ecx, dword [12+16+esp]
 add esi,edi
 shl eax,7
 add edi,ecx
 shl edx,7
 mov  dword [16+ebx],edi
 add eax,offset snd_scaletable
 add edx,offset snd_scaletable
 sub ebx,ebx
 mov bl, byte [-1+esi+ecx*1]
 test ecx,1
 jz LMix8Loop
 mov edi, dword [eax+ebx*4]
 mov ebp, dword [edx+ebx*4]
 add edi, dword [paintbuffer+0-8+ecx*8]
 add ebp, dword [paintbuffer+4-8+ecx*8]
 mov  dword [paintbuffer+0-8+ecx*8],edi
 mov  dword [paintbuffer+4-8+ecx*8],ebp
 mov bl, byte [-2+esi+ecx*1]
 dec ecx
 jz LDone
LMix8Loop:
 mov edi, dword [eax+ebx*4]
 mov ebp, dword [edx+ebx*4]
 add edi, dword [paintbuffer+0-8+ecx*8]
 add ebp, dword [paintbuffer+4-8+ecx*8]
 mov bl, byte [-2+esi+ecx*1]
 mov  dword [paintbuffer+0-8+ecx*8],edi
 mov  dword [paintbuffer+4-8+ecx*8],ebp
 mov edi, dword [eax+ebx*4]
 mov ebp, dword [edx+ebx*4]
 mov bl, byte [-3+esi+ecx*1]
 add edi, dword [paintbuffer+0-8*2+ecx*8]
 add ebp, dword [paintbuffer+4-8*2+ecx*8]
 mov  dword [paintbuffer+0-8*2+ecx*8],edi
 mov  dword [paintbuffer+4-8*2+ecx*8],ebp
 sub ecx,2
 jnz LMix8Loop
LDone:
 pop ebp
 pop ebx
 pop edi
 pop esi
 ret

 global Snd_WriteLinearBlastStereo16
Snd_WriteLinearBlastStereo16:
 push esi
 push edi
 push ebx
 mov ecx, dword [snd_linear_count]
 mov ebx, dword [snd_p]
 mov esi, dword [snd_vol]
 mov edi, dword [snd_out]
LWLBLoopTop:
 mov eax, dword [-8+ebx+ecx*4]
 imul eax,esi
 sar eax,8
 cmp eax,07FFFh
 jg LClampHigh
 cmp eax,0FFFF8000h
 jnl LClampDone
 mov eax,0FFFF8000h
 jmp LClampDone
LClampHigh:
 mov eax,07FFFh
LClampDone:
 mov edx, dword [-4+ebx+ecx*4]
 imul edx,esi
 sar edx,8
 cmp edx,07FFFh
 jg LClampHigh2
 cmp edx,0FFFF8000h
 jnl LClampDone2
 mov edx,0FFFF8000h
 jmp LClampDone2
LClampHigh2:
 mov edx,07FFFh
LClampDone2:
 shl edx,16
 and eax,0FFFFh
 or edx,eax
 mov  dword [-4+edi+ecx*2],edx
 sub ecx,2
 jnz LWLBLoopTop
 pop ebx
 pop edi
 pop esi
 ret

