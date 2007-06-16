;
; d_varsa.asm
; $Id: d_varsa.asm,v 1.2 2007-06-16 09:21:56 sezero Exp $
; rasterization driver global variables
;

%idefine offset
; extern d_zistepu
; extern d_pzbuffer
; extern d_zistepv
; extern d_zrowbytes
; extern d_ziorigin
 extern r_turb_s
 extern r_turb_t
 extern r_turb_pdest
 extern r_turb_spancount
 extern r_turb_turb
 extern r_turb_pbase
 extern r_turb_sstep
 extern r_turb_tstep
 extern r_bmodelactive
; extern d_sdivzstepu
; extern d_tdivzstepu
; extern d_sdivzstepv
; extern d_tdivzstepv
; extern d_sdivzorigin
; extern d_tdivzorigin
; extern sadjust
; extern tadjust
; extern bbextents
; extern bbextentt
; extern cacheblock
; extern d_viewbuffer
; extern cachewidth
; extern d_pzbuffer
; extern d_zrowbytes
; extern d_zwidth
 extern d_scantable
 extern r_lightptr
 extern r_numvblocks
 extern prowdestbase
 extern pbasesource
 extern r_lightwidth
 extern lightright
 extern lightrightstep
 extern lightdeltastep
 extern lightdelta
 extern lightright
 extern lightdelta
 extern sourcetstep
 extern surfrowbytes
 extern lightrightstep
 extern lightdeltastep
 extern r_sourcemax
 extern r_stepback
 extern colormap
 extern blocksize
 extern sourcesstep
 extern lightleft
 extern blockdivshift
 extern blockdivmask
 extern lightleftstep
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
 extern vright
 extern vup
 extern vpn
 extern BOPS_Error
 extern snd_scaletable
 extern paintbuffer
 extern snd_linear_count
 extern snd_p
 extern snd_vol
 extern snd_out
 extern r_leftclipped
 extern r_leftenter
 extern r_rightclipped
 extern r_rightenter
 extern modelorg
 extern xscale
 extern r_refdef
 extern yscale
 extern r_leftexit
 extern r_rightexit
 extern r_lastvertvalid
 extern cacheoffset
 extern newedges
 extern removeedges
 extern r_pedge
 extern r_framecount
 extern r_u1
 extern r_emitted
 extern edge_p
 extern surface_p
 extern surfaces
 extern r_lzi1
 extern r_v1
 extern r_ceilv1
 extern r_nearzi
 extern r_nearzionly
 extern edge_aftertail
 extern edge_tail
 extern current_iv
 extern edge_head_u_shift20
 extern span_p
 extern edge_head
 extern fv
 extern edge_tail_u_shift20
 extern r_apverts
 extern r_anumverts
 extern aliastransform
 extern r_avertexnormals
 extern r_plightvec
 extern r_ambientlight
 extern r_shadelight
 extern aliasxcenter
 extern aliasycenter
 extern a_sstepxfrac
 extern r_affinetridesc
 extern acolormap
 extern d_pcolormap
 extern r_affinetridesc
 extern d_sfrac
 extern d_ptex
 extern d_pedgespanpackage
 extern d_tfrac
 extern d_light
 extern d_zi
 extern d_pdest
 extern d_pz
 extern d_aspancount
 extern erroradjustup
 extern errorterm
 extern d_xdenom
 extern r_p0
 extern r_p1
 extern r_p2
 extern a_tstepxfrac
 extern r_sstepx
 extern r_tstepx
 extern a_ststepxwhole
 extern zspantable
 extern skintable
 extern r_zistepx
 extern erroradjustdown
 extern d_countextrastep
 extern ubasestep
 extern a_ststepxwhole
 extern a_tstepxfrac
 extern r_lstepx
 extern a_spans
 extern erroradjustdown
 extern d_pdestextrastep
 extern d_pzextrastep
 extern d_sfracextrastep
 extern d_ptexextrastep
 extern d_countextrastep
 extern d_tfracextrastep
 extern d_lightextrastep
 extern d_ziextrastep
 extern d_pdestbasestep
 extern d_pzbasestep
 extern d_sfracbasestep
 extern d_ptexbasestep
 extern ubasestep
 extern d_tfracbasestep
 extern d_lightbasestep
 extern d_zibasestep
 extern zspantable
 extern r_lstepy
 extern r_sstepy
 extern r_tstepy
 extern r_zistepy
 extern D_PolysetSetEdgeTable
 extern D_RasterizeAliasPolySmooth
 extern float_point5
 extern Float2ToThe31nd
; extern izistep
; extern izi
 extern FloatMinus2ToThe31nd
 extern float_1
 extern float_particle_z_clip
 extern float_minus_1
 extern float_0
 extern fp_16
 extern fp_64k
 extern fp_1m
 extern fp_1m_minus_1
 extern fp_8
; extern entryvec_table
; extern advancetable
; extern sstep
; extern tstep
; extern pspantemp
; extern counttemp
; extern jumptemp
; extern reciprocal_table
; extern DP_Count
; extern DP_u
; extern DP_v
; extern DP_32768
; extern DP_Color
; extern DP_Pix
; extern DP_EntryTable
; extern DP_EntryTransTable
; extern pbase
; extern s
; extern t
; extern sfracf
; extern tfracf
; extern snext
; extern tnext
; extern spancountminus1
; extern zi16stepu
; extern sdivz16stepu
; extern tdivz16stepu
; extern zi8stepu
; extern sdivz8stepu
; extern tdivz8stepu
; extern reciprocal_table_16
; extern entryvec_table_16
 extern ceil_cw
 extern single_cw
 extern fp_64kx64k
; extern pz
; extern spr8entryvec_table
; extern spr8Tentryvec_table
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


 END
