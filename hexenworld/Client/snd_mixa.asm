;
; snd_mixa.asm
; x86 assembly-language sound code
;
; this file uses NASM syntax.
; $Id: snd_mixa.asm,v 1.3 2008-03-13 22:02:32 sezero Exp $
;

%idefine offset
 extern d_zistepu
 extern d_zistepv
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
 extern r_sstepx
 extern r_tstepx
 extern skintable
 extern r_zistepx
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

