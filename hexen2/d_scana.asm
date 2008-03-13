;
; d_scana.asm
; x86 assembly-language turbulent texture mapping code
;
; this file uses NASM syntax.
; $Id: d_scana.asm,v 1.3 2008-03-13 22:02:30 sezero Exp $
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
 extern mainTransTable
 extern scanList
SEGMENT .data

SEGMENT .text
 ALIGN 4
 global D_DrawTurbulent8Span
D_DrawTurbulent8Span:
 push ebp
 push esi
 push edi
 push ebx
 mov esi, dword [r_turb_s]
 mov ecx, dword [r_turb_t]
 mov edi, dword [r_turb_pdest]
 mov ebx, dword [r_turb_spancount]
Llp:
 mov eax,ecx
 mov edx,esi
 sar eax,16
 mov ebp, dword [r_turb_turb]
 sar edx,16
 and eax,128-1
 and edx,128-1
 mov eax, dword [ebp+eax*4]
 mov edx, dword [ebp+edx*4]
 add eax,esi
 sar eax,16
 add edx,ecx
 sar edx,16
 and eax,64-1
 and edx,64-1
 shl edx,6
 mov ebp, dword [r_turb_pbase]
 add edx,eax
 inc edi
 add esi, dword [r_turb_sstep]
 add ecx, dword [r_turb_tstep]
 mov dl, byte [ebp+edx*1]
 dec ebx
 mov  byte [-1+edi],dl
 jnz Llp
 mov  dword [r_turb_pdest],edi
 pop ebx
 pop edi
 pop esi
 pop ebp
 ret


 ALIGN 4
 global D_DrawTurbulent8TSpan
D_DrawTurbulent8TSpan:
 push ebp
 push esi
 push edi
 push ebx
 mov esi, dword [r_turb_s]
 mov ecx, dword [r_turb_t]
 mov edi, dword [r_turb_pdest]
 mov ebx, dword [r_turb_spancount]
LlpT:
 mov eax,ecx
 mov edx,esi
 sar eax,16
 mov ebp, dword [r_turb_turb]
 sar edx,16
 and eax,128-1
 and edx,128-1
 mov eax, dword [ebp+eax*4]
 mov edx, dword [ebp+edx*4]
 add eax,esi
 sar eax,16
 add edx,ecx
 sar edx,16
 and eax,64-1
 and edx,64-1
 shl edx,6
 mov ebp, dword [r_turb_pbase]
 add edx,eax
 cmp  byte [scanList + ebx - 1], 1
 jnz skip1
 inc edi
 mov dh, byte [ebp+edx*1]
 add esi, dword [r_turb_sstep]
 mov dl, byte [-1+edi]
 add ecx, dword [r_turb_tstep]
 mov dl, byte [12345678h + edx]
TranPatch1:

 dec ebx
 mov  byte [-1+edi],dl
 ;mov  byte [-1+edi],255
 jnz LlpT
skip2:
 mov  dword [r_turb_pdest],edi
 pop ebx
 pop edi
 pop esi
 pop ebp
 ret

skip1:
 inc edi
 dec ebx
 jnz LlpT
 jmp skip2


 ALIGN 4
 global D_DrawTurbulent8TQuickSpan
D_DrawTurbulent8TQuickSpan:
 push ebp
 push esi
 push edi
 push ebx
 mov esi, dword [r_turb_s]
 mov ecx, dword [r_turb_t]
 mov edi, dword [r_turb_pdest]
 mov ebx, dword [r_turb_spancount]
LlpTQ:
 mov eax,ecx
 mov edx,esi
 sar eax,16
 mov ebp, dword [r_turb_turb]
 sar edx,16
 and eax,128-1
 and edx,128-1
 mov eax, dword [ebp+eax*4]
 mov edx, dword [ebp+edx*4]
 add eax,esi
 sar eax,16
 add edx,ecx
 sar edx,16
 and eax,64-1
 and edx,64-1
 shl edx,6
 mov ebp, dword [r_turb_pbase]
 add edx,eax
 ;cmp  byte [scanList + ebx - 1], 1
 ;jnz skip1
 inc edi
 mov dh, byte [ebp+edx*1]
 add esi, dword [r_turb_sstep]
 mov dl, byte [-1+edi]
 add ecx, dword [r_turb_tstep]
 mov dl, byte [12345678h + edx]
TranPatch2:

 dec ebx
 mov  byte [-1+edi],dl
 jnz LlpTQ

 mov  dword [r_turb_pdest],edi
 pop ebx
 pop edi
 pop esi
 pop ebp
 ret

 global D_DrawTurbulent8TSpanEnd
D_DrawTurbulent8TSpanEnd:


SEGMENT .data
 ALIGN 4
LPatchTable:
 dd TranPatch1-4
 dd TranPatch2-4

SEGMENT .text
 ALIGN 4
 global R_TranPatch7
R_TranPatch7:
 push ebx
 mov eax, dword [mainTransTable]
 mov ebx,offset LPatchTable
 mov ecx,2
LPatchLoop:
 mov edx, dword [ebx]
 add ebx,4
 mov  dword [edx],eax
 dec ecx
 jnz LPatchLoop
 pop ebx
 ret

