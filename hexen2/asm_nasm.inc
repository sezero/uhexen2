;
; asm_nasm.inc
; $Id: asm_nasm.inc,v 1.1 2008-03-16 14:32:15 sezero Exp $
;
; common header for NASM format x86-assembly sources
;

;
; underscore prefix handling:
; unless we receive _NO_PREFIX, we shall prefix C-shared
; symbols with '_'. for ELF targets, or whenever necessary,
; you must specifically add -D_NO_PREFIX to your NASMFLAGS.
;
%ifndef _NO_PREFIX
	%macro _sym_prefix 1
	%define %1   _ %+ %1
	%endmacro
%endif

;
; other definitions / macros
;
%idefine offset

