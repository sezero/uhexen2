; d_varsa.asm -- for MASM.
; rasterization driver global variables
;
; Copyright (C) 1996-1997  Id Software, Inc.
; Copyright (C) 1997-1998  Raven Software Corp.
;
; This program is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 2
; of the License, or (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
;
; See the GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to:
; Free Software Foundation, Inc.
; 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
;

 .386P
 .model FLAT

_DATA SEGMENT
 align 4
 public _d_sdivzstepu
 public _d_tdivzstepu
 public _d_zistepu
 public _d_sdivzstepv
 public _d_tdivzstepv
 public _d_zistepv
 public _d_sdivzorigin
 public _d_tdivzorigin
 public _d_ziorigin
_d_sdivzstepu dd 0
_d_tdivzstepu dd 0
_d_zistepu dd 0
_d_sdivzstepv dd 0
_d_tdivzstepv dd 0
_d_zistepv dd 0
_d_sdivzorigin dd 0
_d_tdivzorigin dd 0
_d_ziorigin dd 0
 public _sadjust
 public _tadjust
 public _bbextents
 public _bbextentt
_sadjust dd 0
_tadjust dd 0
_bbextents dd 0
_bbextentt dd 0
 public _cacheblock
 public _d_viewbuffer
 public _cachewidth
 public _d_pzbuffer
 public _d_zrowbytes
 public _d_zwidth
_cacheblock dd 0
_cachewidth dd 0
_d_viewbuffer dd 0
_d_pzbuffer dd 0
_d_zrowbytes dd 0
_d_zwidth dd 0
 public izi
izi dd 0
 public pbase, s, t, sfracf, tfracf, snext, tnext
 public spancountminus1, zi16stepu, sdivz16stepu, tdivz16stepu
 public zi8stepu, sdivz8stepu, tdivz8stepu, pz
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
 public izistep
izistep dd 0
 public reciprocal_table_16, entryvec_table_16, entryvec_table_16T
reciprocal_table_16 dd 040000000h, 02aaaaaaah, 020000000h
 dd 019999999h, 015555555h, 012492492h
 dd 010000000h, 0e38e38eh, 0ccccccch, 0ba2e8bah
 dd 0aaaaaaah, 09d89d89h, 09249249h, 08888888h
 externdef Entry2_16:dword
 externdef Entry3_16:dword
 externdef Entry4_16:dword
 externdef Entry5_16:dword
 externdef Entry6_16:dword
 externdef Entry7_16:dword
 externdef Entry8_16:dword
 externdef Entry9_16:dword
 externdef Entry10_16:dword
 externdef Entry11_16:dword
 externdef Entry12_16:dword
 externdef Entry13_16:dword
 externdef Entry14_16:dword
 externdef Entry15_16:dword
 externdef Entry16_16:dword
entryvec_table_16 dd 0, Entry2_16, Entry3_16, Entry4_16
 dd Entry5_16, Entry6_16, Entry7_16, Entry8_16
 dd Entry9_16, Entry10_16, Entry11_16, Entry12_16
 dd Entry13_16, Entry14_16, Entry15_16, Entry16_16

 externdef Entry2_16T:dword
 externdef Entry3_16T:dword
 externdef Entry4_16T:dword
 externdef Entry5_16T:dword
 externdef Entry6_16T:dword
 externdef Entry7_16T:dword
 externdef Entry8_16T:dword
 externdef Entry9_16T:dword
 externdef Entry10_16T:dword
 externdef Entry11_16T:dword
 externdef Entry12_16T:dword
 externdef Entry13_16T:dword
 externdef Entry14_16T:dword
 externdef Entry15_16T:dword
 externdef Entry16_16T:dword
entryvec_table_16T dd 0, Entry2_16T, Entry3_16T, Entry4_16T
 dd Entry5_16T, Entry6_16T, Entry7_16T, Entry8_16T
 dd Entry9_16T, Entry10_16T, Entry11_16T, Entry12_16T
 dd Entry13_16T, Entry14_16T, Entry15_16T, Entry16_16T

 public DP_Count, DP_u, DP_v, DP_32768, DP_Color, DP_Pix, DP_EntryTable, DP_EntryTransTable
DP_Count dd 0
DP_u dd 0
DP_v dd 0
DP_32768 dd 32768.0
DP_Color dd 0
DP_Pix dd 0
 externdef DP_1x1:dword
 externdef DP_2x2:dword
 externdef DP_3x3:dword
 externdef DP_4x4:dword
DP_EntryTable dd DP_1x1, DP_2x2, DP_3x3, DP_4x4
 externdef DP_T1x1:dword
 externdef DP_T2x2:dword
 externdef DP_T3x3:dword
 externdef DP_T4x4:dword
DP_EntryTransTable dd DP_T1x1, DP_T2x2, DP_T3x3, DP_T4x4
 public advancetable, sstep, tstep, pspantemp, counttemp, jumptemp
advancetable dd 0, 0
sstep dd 0
tstep dd 0
pspantemp dd 0
counttemp dd 0
jumptemp dd 0
 public reciprocal_table, entryvec_table
reciprocal_table dd 040000000h, 02aaaaaaah, 020000000h
 dd 019999999h, 015555555h, 012492492h
 externdef Entry2_8:dword
 externdef Entry3_8:dword
 externdef Entry4_8:dword
 externdef Entry5_8:dword
 externdef Entry6_8:dword
 externdef Entry7_8:dword
 externdef Entry8_8:dword
entryvec_table dd 0, Entry2_8, Entry3_8, Entry4_8
 dd Entry5_8, Entry6_8, Entry7_8, Entry8_8
 externdef Spr8Entry2_8:dword
 externdef Spr8Entry3_8:dword
 externdef Spr8Entry4_8:dword
 externdef Spr8Entry5_8:dword
 externdef Spr8Entry6_8:dword
 externdef Spr8Entry7_8:dword
 externdef Spr8Entry8_8:dword
 public spr8entryvec_table
spr8entryvec_table dd 0, Spr8Entry2_8, Spr8Entry3_8, Spr8Entry4_8
 dd Spr8Entry5_8, Spr8Entry6_8, Spr8Entry7_8, Spr8Entry8_8

 externdef Spr8Entry2_8T:dword
 externdef Spr8Entry3_8T:dword
 externdef Spr8Entry4_8T:dword
 externdef Spr8Entry5_8T:dword
 externdef Spr8Entry6_8T:dword
 externdef Spr8Entry7_8T:dword
 externdef Spr8Entry8_8T:dword
 public spr8Tentryvec_table
spr8Tentryvec_table dd 0, Spr8Entry2_8T, Spr8Entry3_8T, Spr8Entry4_8T
 dd Spr8Entry5_8T, Spr8Entry6_8T, Spr8Entry7_8T, Spr8Entry8_8T

 externdef Spr8Entry2_8T2:dword
 externdef Spr8Entry3_8T2:dword
 externdef Spr8Entry4_8T2:dword
 externdef Spr8Entry5_8T2:dword
 externdef Spr8Entry6_8T2:dword
 externdef Spr8Entry7_8T2:dword
 externdef Spr8Entry8_8T2:dword
 public spr8T2entryvec_table
spr8T2entryvec_table dd 0, Spr8Entry2_8T2, Spr8Entry3_8T2, Spr8Entry4_8T2
 dd Spr8Entry5_8T2, Spr8Entry6_8T2, Spr8Entry7_8T2, Spr8Entry8_8T2

_DATA ENDS
 END
