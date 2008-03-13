;
; sys_ia32.asm
; x86 assembly-language misc system routines.
;
; this file uses NASM syntax.
; $Id: sys_ia32.asm,v 1.3 2008-03-13 22:02:30 sezero Exp $
;

%idefine offset
 extern snd_scaletable
 extern paintbuffer
 extern snd_linear_count
 extern snd_p
 extern snd_vol
 extern snd_out
 extern vright
 extern vup
 extern vpn
 extern BOPS_Error
SEGMENT .data
 ALIGN 4
fpenv:
 dd 0, 0, 0, 0, 0, 0, 0, 0

SEGMENT .text
 global MaskExceptions
MaskExceptions:
 fnstenv  [fpenv]
 or  dword [fpenv],03Fh
 fldenv   [fpenv]
 ret

SEGMENT .data
 ALIGN 4
 global ceil_cw, single_cw, full_cw, cw, pushed_cw
ceil_cw dd 0
single_cw dd 0
full_cw dd 0
cw dd 0
pushed_cw dd 0

SEGMENT .text
 global Sys_LowFPPrecision
Sys_LowFPPrecision:
 fldcw  word [single_cw]
 ret
 global Sys_HighFPPrecision
Sys_HighFPPrecision:
 fldcw  word [full_cw]
 ret
 global Sys_PushFPCW_SetHigh
Sys_PushFPCW_SetHigh:
 fnstcw  word [pushed_cw]
 fldcw  word [full_cw]
 ret
 global Sys_PopFPCW
Sys_PopFPCW:
 fldcw  word [pushed_cw]
 ret
 global Sys_SetFPCW
Sys_SetFPCW:
 fnstcw  word [cw]
 mov eax, dword [cw]
 and ah,0F0h
 or ah,003h
 mov  dword [full_cw],eax
 and ah,0F0h
 or ah,00Ch
 mov  dword [single_cw],eax
 and ah,0F0h
 or ah,008h
 mov  dword [ceil_cw],eax
 ret

