;
; d_varsa.asm
; rasterization driver global variables
;
; this file uses NASM syntax.
; $Id: d_varsa.asm,v 1.5 2008-03-16 14:30:46 sezero Exp $
;

%include "asm_nasm.inc"

; underscore prefix handling
; for C-shared symbols:
%ifmacro _sym_prefix
; C-shared externs:
; C-shared globals:
 _sym_prefix d_sdivzstepu
 _sym_prefix d_tdivzstepu
 _sym_prefix d_zistepu
 _sym_prefix d_sdivzstepv
 _sym_prefix d_tdivzstepv
 _sym_prefix d_zistepv
 _sym_prefix d_sdivzorigin
 _sym_prefix d_tdivzorigin
 _sym_prefix d_ziorigin
 _sym_prefix sadjust
 _sym_prefix tadjust
 _sym_prefix bbextents
 _sym_prefix bbextentt
 _sym_prefix cacheblock
 _sym_prefix d_viewbuffer
 _sym_prefix cachewidth
 _sym_prefix d_pzbuffer
 _sym_prefix d_zrowbytes
 _sym_prefix d_zwidth
%endif	; _sym_prefix

SEGMENT .data

 ALIGN 4

 global d_sdivzstepu
 global d_tdivzstepu
 global d_zistepu
 global d_sdivzstepv
 global d_tdivzstepv
 global d_zistepv
 global d_sdivzorigin
 global d_tdivzorigin
 global d_ziorigin
d_sdivzstepu dd 0
d_tdivzstepu dd 0
d_zistepu dd 0
d_sdivzstepv dd 0
d_tdivzstepv dd 0
d_zistepv dd 0
d_sdivzorigin dd 0
d_tdivzorigin dd 0
d_ziorigin dd 0

 global sadjust
 global tadjust
 global bbextents
 global bbextentt
sadjust dd 0
tadjust dd 0
bbextents dd 0
bbextentt dd 0

 global cacheblock
 global d_viewbuffer
 global cachewidth
 global d_pzbuffer
 global d_zrowbytes
 global d_zwidth
cacheblock dd 0
cachewidth dd 0
d_viewbuffer dd 0
d_pzbuffer dd 0
d_zrowbytes dd 0
d_zwidth dd 0

;
; ASM-only variables
;
 global izi
izi dd 0

 global pbase, s, t, sfracf, tfracf, snext, tnext
 global spancountminus1, zi16stepu, sdivz16stepu, tdivz16stepu
 global zi8stepu, sdivz8stepu, tdivz8stepu, pz
s dd 0
t dd 0
snext dd 0
tnext dd 0
sfracf dd 0
tfracf dd 0
pbase dd 0
zi8stepu dd 0
sdivz8stepu dd 0
tdivz8stepu dd 0
zi16stepu dd 0
sdivz16stepu dd 0
tdivz16stepu dd 0
spancountminus1 dd 0
pz dd 0

 global izistep
izistep dd 0

 global reciprocal_table_16, entryvec_table_16, entryvec_table_16T
reciprocal_table_16 dd 040000000h, 02aaaaaaah, 020000000h
 dd 019999999h, 015555555h, 012492492h
 dd 010000000h, 0e38e38eh, 0ccccccch, 0ba2e8bah
 dd 0aaaaaaah, 09d89d89h, 09249249h, 08888888h

 extern Entry2_16
 extern Entry3_16
 extern Entry4_16
 extern Entry5_16
 extern Entry6_16
 extern Entry7_16
 extern Entry8_16
 extern Entry9_16
 extern Entry10_16
 extern Entry11_16
 extern Entry12_16
 extern Entry13_16
 extern Entry14_16
 extern Entry15_16
 extern Entry16_16
entryvec_table_16 dd 0, Entry2_16, Entry3_16, Entry4_16
 dd Entry5_16, Entry6_16, Entry7_16, Entry8_16
 dd Entry9_16, Entry10_16, Entry11_16, Entry12_16
 dd Entry13_16, Entry14_16, Entry15_16, Entry16_16

 extern Entry2_16T
 extern Entry3_16T
 extern Entry4_16T
 extern Entry5_16T
 extern Entry6_16T
 extern Entry7_16T
 extern Entry8_16T
 extern Entry9_16T
 extern Entry10_16T
 extern Entry11_16T
 extern Entry12_16T
 extern Entry13_16T
 extern Entry14_16T
 extern Entry15_16T
 extern Entry16_16T
entryvec_table_16T dd 0, Entry2_16T, Entry3_16T, Entry4_16T
 dd Entry5_16T, Entry6_16T, Entry7_16T, Entry8_16T
 dd Entry9_16T, Entry10_16T, Entry11_16T, Entry12_16T
 dd Entry13_16T, Entry14_16T, Entry15_16T, Entry16_16T

 global DP_Count, DP_u, DP_v, DP_32768, DP_Color, DP_Pix, DP_EntryTable, DP_EntryTransTable
DP_Count dd 0
DP_u dd 0
DP_v dd 0
DP_32768 dd 32768.0
DP_Color dd 0
DP_Pix dd 0

 extern DP_1x1
 extern DP_2x2
 extern DP_3x3
 extern DP_4x4
DP_EntryTable dd DP_1x1, DP_2x2, DP_3x3, DP_4x4

 extern DP_T1x1
 extern DP_T2x2
 extern DP_T3x3
 extern DP_T4x4
DP_EntryTransTable dd DP_T1x1, DP_T2x2, DP_T3x3, DP_T4x4

 global advancetable, sstep, tstep, pspantemp, counttemp, jumptemp
advancetable dd 0, 0
sstep dd 0
tstep dd 0
pspantemp dd 0
counttemp dd 0
jumptemp dd 0

 global reciprocal_table, entryvec_table
reciprocal_table dd 040000000h, 02aaaaaaah, 020000000h
 dd 019999999h, 015555555h, 012492492h

 extern Entry2_8
 extern Entry3_8
 extern Entry4_8
 extern Entry5_8
 extern Entry6_8
 extern Entry7_8
 extern Entry8_8
entryvec_table dd 0, Entry2_8, Entry3_8, Entry4_8
 dd Entry5_8, Entry6_8, Entry7_8, Entry8_8

 extern Spr8Entry2_8
 extern Spr8Entry3_8
 extern Spr8Entry4_8
 extern Spr8Entry5_8
 extern Spr8Entry6_8
 extern Spr8Entry7_8
 extern Spr8Entry8_8
 global spr8entryvec_table
spr8entryvec_table dd 0, Spr8Entry2_8, Spr8Entry3_8, Spr8Entry4_8
 dd Spr8Entry5_8, Spr8Entry6_8, Spr8Entry7_8, Spr8Entry8_8

 extern Spr8Entry2_8T
 extern Spr8Entry3_8T
 extern Spr8Entry4_8T
 extern Spr8Entry5_8T
 extern Spr8Entry6_8T
 extern Spr8Entry7_8T
 extern Spr8Entry8_8T
 global spr8Tentryvec_table
spr8Tentryvec_table dd 0, Spr8Entry2_8T, Spr8Entry3_8T, Spr8Entry4_8T
 dd Spr8Entry5_8T, Spr8Entry6_8T, Spr8Entry7_8T, Spr8Entry8_8T

 extern Spr8Entry2_8T2
 extern Spr8Entry3_8T2
 extern Spr8Entry4_8T2
 extern Spr8Entry5_8T2
 extern Spr8Entry6_8T2
 extern Spr8Entry7_8T2
 extern Spr8Entry8_8T2
 global spr8T2entryvec_table
spr8T2entryvec_table dd 0, Spr8Entry2_8T2, Spr8Entry3_8T2, Spr8Entry4_8T2
 dd Spr8Entry5_8T2, Spr8Entry6_8T2, Spr8Entry7_8T2, Spr8Entry8_8T2

