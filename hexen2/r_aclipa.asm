;
; r_aclipa.asm
; $Id: r_aclipa.asm,v 1.2 2007-06-16 09:21:51 sezero Exp $
; x86 assembly-language Alias model transform and project code.
;

%idefine offset
 extern d_zistepu
 extern d_pzbuffer
 extern d_zistepv
 extern d_zrowbytes
 extern d_ziorigin
 extern r_turb_s
 extern r_turb_t
 extern r_turb_pdest
 extern r_turb_spancount
 extern r_turb_turb
 extern r_turb_pbase
 extern r_turb_sstep
 extern r_turb_tstep
 extern r_bmodelactive
 extern d_sdivzstepu
 extern d_tdivzstepu
 extern d_sdivzstepv
 extern d_tdivzstepv
 extern d_sdivzorigin
 extern d_tdivzorigin
 extern sadjust
 extern tadjust
 extern bbextents
 extern bbextentt
 extern cacheblock
 extern d_viewbuffer
 extern cachewidth
 extern d_pzbuffer
 extern d_zrowbytes
 extern d_zwidth
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
 extern izistep
 extern izi
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
 extern entryvec_table
 extern advancetable
 extern sstep
 extern tstep
 extern pspantemp
 extern counttemp
 extern jumptemp
 extern reciprocal_table
 extern DP_Count
 extern DP_u
 extern DP_v
 extern DP_32768
 extern DP_Color
 extern DP_Pix
 extern DP_EntryTable
 extern pbase
 extern s
 extern t
 extern sfracf
 extern tfracf
 extern snext
 extern tnext
 extern spancountminus1
 extern zi16stepu
 extern sdivz16stepu
 extern tdivz16stepu
 extern zi8stepu
 extern sdivz8stepu
 extern tdivz8stepu
 extern reciprocal_table_16
 extern entryvec_table_16
 extern ceil_cw
 extern single_cw
 extern fp_64kx64k
 extern pz
 extern spr8entryvec_table
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
Ltemp0 dd 0
Ltemp1 dd 0

SEGMENT .text
 global R_Alias_clip_bottom
R_Alias_clip_bottom:
 push esi
 push edi
 mov esi, dword [8+4+esp]
 mov edi, dword [8+8+esp]
 mov eax, dword [r_refdef+52]
LDoForwardOrBackward:
 mov edx, dword [0+4+esi]
 mov ecx, dword [0+4+edi]
 cmp edx,ecx
 jl LDoForward
 mov ecx, dword [0+4+esi]
 mov edx, dword [0+4+edi]
 mov edi, dword [8+4+esp]
 mov esi, dword [8+8+esp]
LDoForward:
 sub ecx,edx
 sub eax,edx
 mov  dword [Ltemp1],ecx
 mov  dword [Ltemp0],eax
 fild  dword [Ltemp1]
 fild  dword [Ltemp0]
 mov edx, dword [8+12+esp]
 mov eax,2
 fdivrp st1,st0
LDo3Forward:
 fild  dword [0+0+esi]
 fild  dword [0+0+edi]
 fild  dword [0+4+esi]
 fild  dword [0+4+edi]
 fild  dword [0+8+esi]
 fild  dword [0+8+edi]
 fxch st5
 fsub st4,st0
 fxch st3
 fsub st2,st0
 fxch st1
 fsub st5,st0
 fxch st6
 fmul st4,st0
 add edi,12
 fmul st2,st0
 add esi,12
 add edx,12
 fmul st5,st0
 fxch st3
 faddp st4,st0
 faddp st1,st0
 fxch st4
 faddp st3,st0
 fxch st1
 fadd  dword [float_point5]
 fxch st3
 fadd  dword [float_point5]
 fxch st2
 fadd  dword [float_point5]
 fxch st3
 fistp  dword [0+0-12+edx]
 fxch st1
 fistp  dword [0+4-12+edx]
 fxch st1
 fistp  dword [0+8-12+edx]
 dec eax
 jnz LDo3Forward
 fstp st0
 pop edi
 pop esi
 ret
 global R_Alias_clip_top
R_Alias_clip_top:
 push esi
 push edi
 mov esi, dword [8+4+esp]
 mov edi, dword [8+8+esp]
 mov eax, dword [r_refdef+20+4]
 jmp LDoForwardOrBackward
 global R_Alias_clip_right
R_Alias_clip_right:
 push esi
 push edi
 mov esi, dword [8+4+esp]
 mov edi, dword [8+8+esp]
 mov eax, dword [r_refdef+48]
LRightLeftEntry:
 mov edx, dword [0+4+esi]
 mov ecx, dword [0+4+edi]
 cmp edx,ecx
 mov edx, dword [0+0+esi]
 mov ecx, dword [0+0+edi]
 jl LDoForward2
 mov ecx, dword [0+0+esi]
 mov edx, dword [0+0+edi]
 mov edi, dword [8+4+esp]
 mov esi, dword [8+8+esp]
LDoForward2:
 jmp LDoForward
 global R_Alias_clip_left
R_Alias_clip_left:
 push esi
 push edi
 mov esi, dword [8+4+esp]
 mov edi, dword [8+8+esp]
 mov eax, dword [r_refdef+20+0]
 jmp LRightLeftEntry

 END
