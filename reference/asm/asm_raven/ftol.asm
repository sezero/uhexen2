
;**************************************************************************
;**
;** FTOL.ASM
;**
;**************************************************************************

.386
.MODEL FLAT
OPTION OLDSTRUCTS

; MACROS ------------------------------------------------------------------

; TYPES -------------------------------------------------------------------

.DATA

ftolData		dq	?

; CODE --------------------------------------------------------------------

.CODE

;==========================================================================
;
; ftol
;
; Replacement for Visual C++ ftol.  Doesn't muck with the fpu control word
; or any of that other time consuming stuff.
;==========================================================================

PUBLIC __ftol
__ftol:
	
	fistp QWORD PTR [ftolData]
	mov eax, DWORD PTR [ftolData]	
	mov edx, DWORD PTR [ftolData+4]

	ret

END
