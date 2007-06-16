;
; surf8.asm
; $Id: surf8.asm,v 1.2 2007-06-16 09:21:56 sezero Exp $
; x86 assembly-language 8 bpp surface block drawing code.
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
sb_v dd 0

SEGMENT .text
 ALIGN 4
 global R_Surf8Start
R_Surf8Start:
 ALIGN 4
 global R_DrawSurfaceBlock8_mip0
R_DrawSurfaceBlock8_mip0:
 push ebp
 push edi
 push esi
 push ebx
 mov ebx, dword [r_lightptr]
 mov eax, dword [r_numvblocks]
 mov  dword [sb_v],eax
 mov edi, dword [prowdestbase]
 mov esi, dword [pbasesource]
Lv_loop_mip0:
 mov eax, dword [ebx]
 mov edx, dword [4+ebx]
 mov ebp,eax
 mov ecx, dword [r_lightwidth]
 mov  dword [lightright],edx
 sub ebp,edx
 and ebp,0FFFFFh
 lea ebx, [ebx+ecx*4]
 mov  dword [r_lightptr],ebx
 mov ecx, dword [4+ebx]
 mov ebx, dword [ebx]
 sub ebx,eax
 sub ecx,edx
 sar ecx,4
 or ebp,0F0000000h
 sar ebx,4
 mov  dword [lightrightstep],ecx
 sub ebx,ecx
 and ebx,0FFFFFh
 or ebx,0F0000000h
 sub ecx,ecx
 mov  dword [lightdeltastep],ebx
 sub ebx,ebx
Lblockloop8_mip0:
 mov  dword [lightdelta],ebp
 mov cl, byte [14+esi]
 sar ebp,4
 mov bh,dh
 mov bl, byte [15+esi]
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch0:
 mov bl, byte [13+esi]
 mov al, byte [12345678h+ecx]
LBPatch1:
 mov cl, byte [12+esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch2:
 mov bl, byte [11+esi]
 mov al, byte [12345678h+ecx]
LBPatch3:
 mov cl, byte [10+esi]
 mov  dword [12+edi],eax
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch4:
 mov bl, byte [9+esi]
 mov al, byte [12345678h+ecx]
LBPatch5:
 mov cl, byte [8+esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch6:
 mov bl, byte [7+esi]
 mov al, byte [12345678h+ecx]
LBPatch7:
 mov cl, byte [6+esi]
 mov  dword [8+edi],eax
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch8:
 mov bl, byte [5+esi]
 mov al, byte [12345678h+ecx]
LBPatch9:
 mov cl, byte [4+esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch10:
 mov bl, byte [3+esi]
 mov al, byte [12345678h+ecx]
LBPatch11:
 mov cl, byte [2+esi]
 mov  dword [4+edi],eax
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch12:
 mov bl, byte [1+esi]
 mov al, byte [12345678h+ecx]
LBPatch13:
 mov cl, byte [esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 mov ah, byte [12345678h+ebx]
LBPatch14:
 mov edx, dword [lightright]
 mov al, byte [12345678h+ecx]
LBPatch15:
 mov ebp, dword [lightdelta]
 mov  dword [edi],eax
 add esi, dword [sourcetstep]
 add edi, dword [surfrowbytes]
 add edx, dword [lightrightstep]
 add ebp, dword [lightdeltastep]
 mov  dword [lightright],edx
 jc near Lblockloop8_mip0
 cmp esi, dword [r_sourcemax]
 jb LSkip_mip0
 sub esi, dword [r_stepback]
LSkip_mip0:
 mov ebx, dword [r_lightptr]
 dec  dword [sb_v]
 jnz near Lv_loop_mip0
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 ALIGN 4
 global R_DrawSurfaceBlock8_mip1
R_DrawSurfaceBlock8_mip1:
 push ebp
 push edi
 push esi
 push ebx
 mov ebx, dword [r_lightptr]
 mov eax, dword [r_numvblocks]
 mov  dword [sb_v],eax
 mov edi, dword [prowdestbase]
 mov esi, dword [pbasesource]
Lv_loop_mip1:
 mov eax, dword [ebx]
 mov edx, dword [4+ebx]
 mov ebp,eax
 mov ecx, dword [r_lightwidth]
 mov  dword [lightright],edx
 sub ebp,edx
 and ebp,0FFFFFh
 lea ebx, [ebx+ecx*4]
 mov  dword [r_lightptr],ebx
 mov ecx, dword [4+ebx]
 mov ebx, dword [ebx]
 sub ebx,eax
 sub ecx,edx
 sar ecx,3
 or ebp,070000000h
 sar ebx,3
 mov  dword [lightrightstep],ecx
 sub ebx,ecx
 and ebx,0FFFFFh
 or ebx,0F0000000h
 sub ecx,ecx
 mov  dword [lightdeltastep],ebx
 sub ebx,ebx
Lblockloop8_mip1:
 mov  dword [lightdelta],ebp
 mov cl, byte [6+esi]
 sar ebp,3
 mov bh,dh
 mov bl, byte [7+esi]
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch22:
 mov bl, byte [5+esi]
 mov al, byte [12345678h+ecx]
LBPatch23:
 mov cl, byte [4+esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch24:
 mov bl, byte [3+esi]
 mov al, byte [12345678h+ecx]
LBPatch25:
 mov cl, byte [2+esi]
 mov  dword [4+edi],eax
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch26:
 mov bl, byte [1+esi]
 mov al, byte [12345678h+ecx]
LBPatch27:
 mov cl, byte [esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 mov ah, byte [12345678h+ebx]
LBPatch28:
 mov edx, dword [lightright]
 mov al, byte [12345678h+ecx]
LBPatch29:
 mov ebp, dword [lightdelta]
 mov  dword [edi],eax
 mov eax, dword [sourcetstep]
 add esi,eax
 mov eax, dword [surfrowbytes]
 add edi,eax
 mov eax, dword [lightrightstep]
 add edx,eax
 mov eax, dword [lightdeltastep]
 add ebp,eax
 mov  dword [lightright],edx
 jc near Lblockloop8_mip1
 cmp esi, dword [r_sourcemax]
 jb LSkip_mip1
 sub esi, dword [r_stepback]
LSkip_mip1:
 mov ebx, dword [r_lightptr]
 dec  dword [sb_v]
 jnz near Lv_loop_mip1
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 ALIGN 4
 global R_DrawSurfaceBlock8_mip2
R_DrawSurfaceBlock8_mip2:
 push ebp
 push edi
 push esi
 push ebx
 mov ebx, dword [r_lightptr]
 mov eax, dword [r_numvblocks]
 mov  dword [sb_v],eax
 mov edi, dword [prowdestbase]
 mov esi, dword [pbasesource]
Lv_loop_mip2:
 mov eax, dword [ebx]
 mov edx, dword [4+ebx]
 mov ebp,eax
 mov ecx, dword [r_lightwidth]
 mov  dword [lightright],edx
 sub ebp,edx
 and ebp,0FFFFFh
 lea ebx, [ebx+ecx*4]
 mov  dword [r_lightptr],ebx
 mov ecx, dword [4+ebx]
 mov ebx, dword [ebx]
 sub ebx,eax
 sub ecx,edx
 sar ecx,2
 or ebp,030000000h
 sar ebx,2
 mov  dword [lightrightstep],ecx
 sub ebx,ecx
 and ebx,0FFFFFh
 or ebx,0F0000000h
 sub ecx,ecx
 mov  dword [lightdeltastep],ebx
 sub ebx,ebx
Lblockloop8_mip2:
 mov  dword [lightdelta],ebp
 mov cl, byte [2+esi]
 sar ebp,2
 mov bh,dh
 mov bl, byte [3+esi]
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch18:
 mov bl, byte [1+esi]
 mov al, byte [12345678h+ecx]
LBPatch19:
 mov cl, byte [esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 mov ah, byte [12345678h+ebx]
LBPatch20:
 mov edx, dword [lightright]
 mov al, byte [12345678h+ecx]
LBPatch21:
 mov ebp, dword [lightdelta]
 mov  dword [edi],eax
 mov eax, dword [sourcetstep]
 add esi,eax
 mov eax, dword [surfrowbytes]
 add edi,eax
 mov eax, dword [lightrightstep]
 add edx,eax
 mov eax, dword [lightdeltastep]
 add ebp,eax
 mov  dword [lightright],edx
 jc Lblockloop8_mip2
 cmp esi, dword [r_sourcemax]
 jb LSkip_mip2
 sub esi, dword [r_stepback]
LSkip_mip2:
 mov ebx, dword [r_lightptr]
 dec  dword [sb_v]
 jnz near Lv_loop_mip2
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 ALIGN 4
 global R_DrawSurfaceBlock8_mip3
R_DrawSurfaceBlock8_mip3:
 push ebp
 push edi
 push esi
 push ebx
 mov ebx, dword [r_lightptr]
 mov eax, dword [r_numvblocks]
 mov  dword [sb_v],eax
 mov edi, dword [prowdestbase]
 mov esi, dword [pbasesource]
Lv_loop_mip3:
 mov eax, dword [ebx]
 mov edx, dword [4+ebx]
 mov ebp,eax
 mov ecx, dword [r_lightwidth]
 mov  dword [lightright],edx
 sub ebp,edx
 and ebp,0FFFFFh
 lea ebx, [ebx+ecx*4]
 mov  dword [lightdelta],ebp
 mov  dword [r_lightptr],ebx
 mov ecx, dword [4+ebx]
 mov ebx, dword [ebx]
 sub ebx,eax
 sub ecx,edx
 sar ecx,1
 sar ebx,1
 mov  dword [lightrightstep],ecx
 sub ebx,ecx
 and ebx,0FFFFFh
 sar ebp,1
 or ebx,0F0000000h
 mov  dword [lightdeltastep],ebx
 sub ebx,ebx
 mov bl, byte [1+esi]
 sub ecx,ecx
 mov bh,dh
 mov cl, byte [esi]
 add edx,ebp
 mov ch,dh
 mov al, byte [12345678h+ebx]
LBPatch16:
 mov edx, dword [lightright]
 mov  byte [1+edi],al
 mov al, byte [12345678h+ecx]
LBPatch17:
 mov  byte [edi],al
 mov eax, dword [sourcetstep]
 add esi,eax
 mov eax, dword [surfrowbytes]
 add edi,eax
 mov eax, dword [lightdeltastep]
 mov ebp, dword [lightdelta]
 mov cl, byte [esi]
 add ebp,eax
 mov eax, dword [lightrightstep]
 sar ebp,1
 add edx,eax
 mov bh,dh
 mov bl, byte [1+esi]
 add edx,ebp
 mov ch,dh
 mov al, byte [12345678h+ebx]
LBPatch30:
 mov edx, dword [sourcetstep]
 mov  byte [1+edi],al
 mov al, byte [12345678h+ecx]
LBPatch31:
 mov  byte [edi],al
 mov ebp, dword [surfrowbytes]
 add esi,edx
 add edi,ebp
 cmp esi, dword [r_sourcemax]
 jb LSkip_mip3
 sub esi, dword [r_stepback]
LSkip_mip3:
 mov ebx, dword [r_lightptr]
 dec  dword [sb_v]
 jnz near Lv_loop_mip3
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 global R_Surf8End
R_Surf8End:

SEGMENT .data
 ALIGN 4
LPatchTable8:
 dd LBPatch0-4
 dd LBPatch1-4
 dd LBPatch2-4
 dd LBPatch3-4
 dd LBPatch4-4
 dd LBPatch5-4
 dd LBPatch6-4
 dd LBPatch7-4
 dd LBPatch8-4
 dd LBPatch9-4
 dd LBPatch10-4
 dd LBPatch11-4
 dd LBPatch12-4
 dd LBPatch13-4
 dd LBPatch14-4
 dd LBPatch15-4
 dd LBPatch16-4
 dd LBPatch17-4
 dd LBPatch18-4
 dd LBPatch19-4
 dd LBPatch20-4
 dd LBPatch21-4
 dd LBPatch22-4
 dd LBPatch23-4
 dd LBPatch24-4
 dd LBPatch25-4
 dd LBPatch26-4
 dd LBPatch27-4
 dd LBPatch28-4
 dd LBPatch29-4
 dd LBPatch30-4
 dd LBPatch31-4

SEGMENT .text
 ALIGN 4
 global R_Surf8Patch
R_Surf8Patch:
 push ebx
 mov eax, dword [colormap]
 mov ebx,offset LPatchTable8
 mov ecx,32
LPatchLoop8:
 mov edx, dword [ebx]
 add ebx,4
 mov  dword [edx],eax
 dec ecx
 jnz LPatchLoop8
 pop ebx
 ret

 END
