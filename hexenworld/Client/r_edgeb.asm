;
; r_edgeb.asm
; x86 assembly-language edge-processing code.
;
; this file uses NASM syntax.
; $Id: r_edgeb.asm,v 1.5 2008-03-13 22:02:32 sezero Exp $
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
Ltemp dd 0
float_1_div_0100000h dd 035800000h
float_point_999 dd 0.999
float_1_point_001 dd 1.001

SEGMENT .text
 ALIGN 4
 global R_EdgeCodeStartT
R_EdgeCodeStartT:
TrailingEdge:
 mov eax, dword [20+esi]
 dec eax
 jnz LInverted
 mov  dword [20+esi],eax
 mov ecx, dword [40+esi]
 mov edx, dword [12345678h]
LPatch0:
 mov eax, dword [r_bmodelactive]
 sub eax,ecx
 cmp edx,esi
 mov  dword [r_bmodelactive],eax
 jnz LNoEmit
 mov eax, dword [0+ebx]
 shr eax,20
 mov edx, dword [16+esi]
 mov ecx, dword [0+esi]
 cmp eax,edx
 jle LNoEmit2

;rj
 bt  dword [24+esi],7   ; surf->flags & SURF_TRANSLUCENT
 jnc LNoEmit2

 mov  dword [16+ecx],eax
 sub eax,edx
 mov  dword [0+ebp],edx
 mov  dword [8+ebp],eax
 mov eax, dword [current_iv]
 mov  dword [4+ebp],eax
 mov eax, dword [8+esi]
 mov  dword [12+ebp],eax
 mov  dword [8+esi],ebp
 add ebp,16
 mov edx, dword [0+esi]
 mov esi, dword [4+esi]
 mov  dword [0+esi],edx
 mov  dword [4+edx],esi
 ret
LNoEmit2:
 mov  dword [16+ecx],eax
 mov edx, dword [0+esi]
 mov esi, dword [4+esi]
 mov  dword [0+esi],edx
 mov  dword [4+edx],esi
 ret
LNoEmit:
 mov edx, dword [0+esi]
 mov esi, dword [4+esi]
 mov  dword [0+esi],edx
 mov  dword [4+edx],esi
 ret
LInverted:
 mov  dword [20+esi],eax
 ret

Lgs_trailing:
 push offset dword Lgs_nextedge
 jmp TrailingEdge
 global R_GenerateTSpans
R_GenerateTSpans:
 push ebp
 push edi
 push esi
 push ebx
 mov eax, dword [surfaces]
 mov edx, dword [edge_head_u_shift20]
 add eax,64
 mov ebp, dword [span_p]
 mov  dword [r_bmodelactive],0
 mov  dword [0+eax],eax
 mov  dword [4+eax],eax
 mov  dword [16+eax],edx
 mov ebx, dword [edge_head+12]
 cmp ebx,offset edge_tail
 jz near Lgs_lastspan
Lgs_edgeloop:
 mov edi, dword [16+ebx]
 mov eax, dword [surfaces]
 mov esi,edi
 and edi,0FFFF0000h
 and esi,0FFFFh
 jz Lgs_leading
 shl esi,6
 add esi,eax
 test edi,edi
 jz Lgs_trailing
 call TrailingEdge			; call near TrailingEdge
 mov eax, dword [surfaces]
Lgs_leading:
 shr edi,16-6
 mov eax, dword [surfaces]
 add edi,eax
 mov esi, dword [12345678h]
LPatch2:
 mov edx, dword [20+edi]
 mov eax, dword [40+edi]
 test eax,eax
 jnz Lbmodel_leading
 test edx,edx
 jnz near Lxl_done
 inc edx
 mov eax, dword [12+edi]
 mov  dword [20+edi],edx
 mov ecx, dword [12+esi]
 cmp eax,ecx
 jl near Lnewtop
Lsortloopnb:
 mov esi, dword [0+esi]
 mov ecx, dword [12+esi]
 cmp eax,ecx
 jge Lsortloopnb
 jmp LInsertAndExit
 ALIGN 4
Lbmodel_leading:
 test edx,edx
 jnz near Lxl_done
 mov ecx, dword [r_bmodelactive]
 inc edx
 inc ecx
 mov  dword [20+edi],edx
 mov  dword [r_bmodelactive],ecx
 mov eax, dword [12+edi]
 mov ecx, dword [12+esi]
 cmp eax,ecx
 jl near Lnewtop
 jz near Lzcheck_for_newtop
Lsortloop:
 mov esi, dword [0+esi]
 mov ecx, dword [12+esi]
 cmp eax,ecx
 jg Lsortloop
 jne near LInsertAndExit
 mov eax, dword [0+ebx]
 sub eax,0FFFFFh
 mov  dword [Ltemp],eax
 fild  dword [Ltemp]
 fmul  dword [float_1_div_0100000h]
 fld st0
 fmul  dword [48+edi]
 fld  dword [fv]
 fmul  dword [52+edi]
 fxch st1
 fadd  dword [44+edi]
 fld  dword [48+esi]
 fmul st0,st3
 fxch st1
 faddp st2,st0
 fld  dword [fv]
 fmul  dword [52+esi]
 fld st2
 fmul  dword [float_point_999]
 fxch st2
 fadd  dword [44+esi]
 faddp st1,st0
 fxch st1
 fcomp st1
 fxch st1
 fmul  dword [float_1_point_001]
 fxch st1
 fnstsw ax
 test ah,001h
 jz Lgotposition_fpop3
 fcomp st1
 fnstsw ax
 test ah,045h
 jz near Lsortloop_fpop2
 fld  dword [48+edi]
 fcomp  dword [48+esi]
 fnstsw ax
 test ah,001h
 jz Lgotposition_fpop2
 fstp st0
 fstp st0
 mov eax, dword [12+edi]
 jmp Lsortloop
Lgotposition_fpop3:
 fstp st0
Lgotposition_fpop2:
 fstp st0
 fstp st0
 jmp LInsertAndExit
Lnewtop_fpop3:
 fstp st0
Lnewtop_fpop2:
 fstp st0
 fstp st0
 mov eax, dword [12+edi]
Lnewtop:
 mov eax, dword [0+ebx]
 mov edx, dword [16+esi]
 shr eax,20
 mov  dword [16+edi],eax
 cmp eax,edx
 jle LInsertAndExit

;rj
 bt  dword [24+esi],7   ; surf->flags & SURF_TRANSLUCENT
 jnc LInsertAndExit

 sub eax,edx
 mov  dword [0+ebp],edx
 mov  dword [8+ebp],eax
 mov eax, dword [current_iv]
 mov  dword [4+ebp],eax
 mov eax, dword [8+esi]
 mov  dword [12+ebp],eax
 mov  dword [8+esi],ebp
 add ebp,16
LInsertAndExit:
 mov  dword [0+edi],esi
 mov eax, dword [4+esi]
 mov  dword [4+edi],eax
 mov  dword [4+esi],edi
 mov  dword [0+eax],edi
Lgs_nextedge:
 mov ebx, dword [12+ebx]
 cmp ebx,offset edge_tail
 jnz near Lgs_edgeloop
Lgs_lastspan:
 mov esi, dword [12345678h]
LPatch3:
 mov eax, dword [edge_tail_u_shift20]
 xor ecx,ecx
 mov edx, dword [16+esi]
 sub eax,edx
 jle Lgs_resetspanstate

;rj
 bt  dword [24+esi],7   ; surf->flags & SURF_TRANSLUCENT
 jnc Lgs_resetspanstate

 mov  dword [0+ebp],edx
 mov  dword [8+ebp],eax
 mov eax, dword [current_iv]
 mov  dword [4+ebp],eax
 mov eax, dword [8+esi]
 mov  dword [12+ebp],eax
 mov  dword [8+esi],ebp
 add ebp,16
Lgs_resetspanstate:
 mov  dword [20+esi],ecx
 mov esi, dword [0+esi]
 cmp esi,012345678h
LPatch4:
 jnz Lgs_resetspanstate
 mov  dword [span_p],ebp
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 ALIGN 4
Lxl_done:
 inc edx
 mov  dword [20+edi],edx
 jmp Lgs_nextedge
 ALIGN 4
Lzcheck_for_newtop:
 mov eax, dword [0+ebx]
 sub eax,0FFFFFh
 mov  dword [Ltemp],eax
 fild  dword [Ltemp]
 fmul  dword [float_1_div_0100000h]
 fld st0
 fmul  dword [48+edi]
 fld  dword [fv]
 fmul  dword [52+edi]
 fxch st1
 fadd  dword [44+edi]
 fld  dword [48+esi]
 fmul st0,st3
 fxch st1
 faddp st2,st0
 fld  dword [fv]
 fmul  dword [52+esi]
 fld st2
 fmul  dword [float_point_999]
 fxch st2
 fadd  dword [44+esi]
 faddp st1,st0
 fxch st1
 fcomp st1
 fxch st1
 fmul  dword [float_1_point_001]
 fxch st1
 fnstsw ax
 test ah,001h
 jz near Lnewtop_fpop3
 fcomp st1
 fnstsw ax
 test ah,045h
 jz Lsortloop_fpop2
 fld  dword [48+edi]
 fcomp  dword [48+esi]
 fnstsw ax
 test ah,001h
 jz near Lnewtop_fpop2
Lsortloop_fpop2:
 fstp st0
 fstp st0
 mov eax, dword [12+edi]
 jmp Lsortloop
 global R_EdgeCodeEndT
R_EdgeCodeEndT:
 ALIGN 4

 global R_SurfacePatchT
R_SurfacePatchT:
 mov eax, dword [surfaces]
 add eax,64
 mov  dword [LPatch4-4],eax
 add eax,0
 mov  dword [LPatch0-4],eax
 mov  dword [LPatch2-4],eax
 mov  dword [LPatch3-4],eax
 ret

