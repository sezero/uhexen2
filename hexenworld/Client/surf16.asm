;
; surf16.asm
; $Id: surf16.asm,v 1.2 2007-06-16 09:21:56 sezero Exp $
; x86 assembly-language 16 bpp surface block drawing code.
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
k dd 0
loopentry dd 0
 ALIGN 4
blockjumptable16:
 dd LEnter2_16
 dd LEnter4_16
 dd 0, LEnter8_16
 dd 0, 0, 0, LEnter16_16

SEGMENT .text
 ALIGN 4
 global R_Surf16Start
R_Surf16Start:
 ALIGN 4
 global R_DrawSurfaceBlock16
R_DrawSurfaceBlock16:
 push ebp
 push edi
 push esi
 push ebx
 mov eax, dword [blocksize]
 mov edi, dword [prowdestbase]
 mov esi, dword [pbasesource]
 mov ebx, dword [sourcesstep]
 mov ecx, dword [blockjumptable16-4+eax*2]
 mov  dword [k],eax
 mov  dword [loopentry],ecx
 mov edx, dword [lightleft]
 mov ebp, dword [lightright]
Lblockloop16:
 sub ebp,edx
 mov cl, byte [blockdivshift]
 sar ebp,cl
 jns Lp1_16
 test ebp, dword [blockdivmask]
 jz Lp1_16
 inc ebp
Lp1_16:
 sub eax,eax
 sub ecx,ecx
 jmp dword[loopentry]
 ALIGN 4
LEnter16_16:
 mov al, byte [esi]
 mov cl, byte [esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi, [esi+ebx*2]
 mov ax, word [12345678h+eax*2]
LBPatch0:
 add edx,ebp
 mov  word [edi],ax
 mov cx, word [12345678h+ecx*2]
LBPatch1:
 mov  word [2+edi],cx
 add edi,04h
 mov al, byte [esi]
 mov cl, byte [esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi, [esi+ebx*2]
 mov ax, word [12345678h+eax*2]
LBPatch2:
 add edx,ebp
 mov  word [edi],ax
 mov cx, word [12345678h+ecx*2]
LBPatch3:
 mov  word [2+edi],cx
 add edi,04h
 mov al, byte [esi]
 mov cl, byte [esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi, [esi+ebx*2]
 mov ax, word [12345678h+eax*2]
LBPatch4:
 add edx,ebp
 mov  word [edi],ax
 mov cx, word [12345678h+ecx*2]
LBPatch5:
 mov  word [2+edi],cx
 add edi,04h
 mov al, byte [esi]
 mov cl, byte [esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi, [esi+ebx*2]
 mov ax, word [12345678h+eax*2]
LBPatch6:
 add edx,ebp
 mov  word [edi],ax
 mov cx, word [12345678h+ecx*2]
LBPatch7:
 mov  word [2+edi],cx
 add edi,04h
LEnter8_16:
 mov al, byte [esi]
 mov cl, byte [esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi, [esi+ebx*2]
 mov ax, word [12345678h+eax*2]
LBPatch8:
 add edx,ebp
 mov  word [edi],ax
 mov cx, word [12345678h+ecx*2]
LBPatch9:
 mov  word [2+edi],cx
 add edi,04h
 mov al, byte [esi]
 mov cl, byte [esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi, [esi+ebx*2]
 mov ax, word [12345678h+eax*2]
LBPatch10:
 add edx,ebp
 mov  word [edi],ax
 mov cx, word [12345678h+ecx*2]
LBPatch11:
 mov  word [2+edi],cx
 add edi,04h
LEnter4_16:
 mov al, byte [esi]
 mov cl, byte [esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi, [esi+ebx*2]
 mov ax, word [12345678h+eax*2]
LBPatch12:
 add edx,ebp
 mov  word [edi],ax
 mov cx, word [12345678h+ecx*2]
LBPatch13:
 mov  word [2+edi],cx
 add edi,04h
LEnter2_16:
 mov al, byte [esi]
 mov cl, byte [esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi, [esi+ebx*2]
 mov ax, word [12345678h+eax*2]
LBPatch14:
 add edx,ebp
 mov  word [edi],ax
 mov cx, word [12345678h+ecx*2]
LBPatch15:
 mov  word [2+edi],cx
 add edi,04h
 mov esi, dword [pbasesource]
 mov edx, dword [lightleft]
 mov ebp, dword [lightright]
 mov eax, dword [sourcetstep]
 mov ecx, dword [lightrightstep]
 mov edi, dword [prowdestbase]
 add esi,eax
 add ebp,ecx
 mov eax, dword [lightleftstep]
 mov ecx, dword [surfrowbytes]
 add edx,eax
 add edi,ecx
 mov  dword [pbasesource],esi
 mov  dword [lightright],ebp
 mov eax, dword [k]
 mov  dword [lightleft],edx
 dec eax
 mov  dword [prowdestbase],edi
 mov  dword [k],eax
 jnz near Lblockloop16
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 global R_Surf16End
R_Surf16End:

SEGMENT .data
 ALIGN 4
LPatchTable16:
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

SEGMENT .text
 ALIGN 4
 global R_Surf16Patch
R_Surf16Patch:
 push ebx
 mov eax, dword [colormap]
 mov ebx,offset LPatchTable16
 mov ecx,16
LPatchLoop16:
 mov edx, dword [ebx]
 add ebx,4
 mov  dword [edx],eax
 dec ecx
 jnz LPatchLoop16
 pop ebx
 ret

 END
