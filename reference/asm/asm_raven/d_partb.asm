.386P
 .model FLAT
 externdef _d_zistepu:dword
 externdef _d_pzbuffer:dword
 externdef _d_zistepv:dword
 externdef _d_zrowbytes:dword
 externdef _d_ziorigin:dword
 externdef _r_turb_s:dword
 externdef _r_turb_t:dword
 externdef _r_turb_pdest:dword
 externdef _r_turb_spancount:dword
 externdef _r_turb_turb:dword
 externdef _r_turb_pbase:dword
 externdef _r_turb_sstep:dword
 externdef _r_turb_tstep:dword
 externdef _r_bmodelactive:dword
 externdef _d_sdivzstepu:dword
 externdef _d_tdivzstepu:dword
 externdef _d_sdivzstepv:dword
 externdef _d_tdivzstepv:dword
 externdef _d_sdivzorigin:dword
 externdef _d_tdivzorigin:dword
 externdef _sadjust:dword
 externdef _tadjust:dword
 externdef _bbextents:dword
 externdef _bbextentt:dword
 externdef _cacheblock:dword
 externdef _d_viewbuffer:dword
 externdef _cachewidth:dword
 externdef _d_pzbuffer:dword
 externdef _d_zrowbytes:dword
 externdef _d_zwidth:dword
 externdef _d_scantable:dword
 externdef _r_lightptr:dword
 externdef _r_numvblocks:dword
 externdef _prowdestbase:dword
 externdef _pbasesource:dword
 externdef _r_lightwidth:dword
 externdef _lightright:dword
 externdef _lightrightstep:dword
 externdef _lightdeltastep:dword
 externdef _lightdelta:dword
 externdef _lightright:dword
 externdef _lightdelta:dword
 externdef _sourcetstep:dword
 externdef _surfrowbytes:dword
 externdef _lightrightstep:dword
 externdef _lightdeltastep:dword
 externdef _r_sourcemax:dword
 externdef _r_stepback:dword
 externdef _colormap:dword
 externdef _blocksize:dword
 externdef _sourcesstep:dword
 externdef _lightleft:dword
 externdef _blockdivshift:dword
 externdef _blockdivmask:dword
 externdef _lightleftstep:dword
 externdef _r_origin:dword
 externdef _r_ppn:dword
 externdef _r_pup:dword
 externdef _r_pright:dword
 externdef _ycenter:dword
 externdef _xcenter:dword
 externdef _d_vrectbottom_particle:dword
 externdef _d_vrectright_particle:dword
 externdef _d_vrecty:dword
 externdef _d_vrectx:dword
 externdef _d_pix_shift:dword
 externdef _d_pix_min:dword
 externdef _d_pix_max:dword
 externdef _d_y_aspect_shift:dword
 externdef _screenwidth:dword
 externdef _vright:dword
 externdef _vup:dword
 externdef _vpn:dword
 externdef _BOPS_Error:dword
 externdef _snd_scaletable:dword
 externdef _paintbuffer:dword
 externdef _snd_linear_count:dword
 externdef _snd_p:dword
 externdef _snd_vol:dword
 externdef _snd_out:dword
 externdef _r_leftclipped:dword
 externdef _r_leftenter:dword
 externdef _r_rightclipped:dword
 externdef _r_rightenter:dword
 externdef _modelorg:dword
 externdef _xscale:dword
 externdef _r_refdef:dword
 externdef _yscale:dword
 externdef _r_leftexit:dword
 externdef _r_rightexit:dword
 externdef _r_lastvertvalid:dword
 externdef _cacheoffset:dword
 externdef _newedges:dword
 externdef _removeedges:dword
 externdef _r_pedge:dword
 externdef _r_framecount:dword
 externdef _r_u1:dword
 externdef _r_emitted:dword
 externdef _edge_p:dword
 externdef _surface_p:dword
 externdef _surfaces:dword
 externdef _r_lzi1:dword
 externdef _r_v1:dword
 externdef _r_ceilv1:dword
 externdef _r_nearzi:dword
 externdef _r_nearzionly:dword
 externdef _edge_aftertail:dword
 externdef _edge_tail:dword
 externdef _current_iv:dword
 externdef _edge_head_u_shift20:dword
 externdef _span_p:dword
 externdef _edge_head:dword
 externdef _fv:dword
 externdef _edge_tail_u_shift20:dword
 externdef _r_apverts:dword
 externdef _r_anumverts:dword
 externdef _aliastransform:dword
 externdef _r_avertexnormals:dword
 externdef _r_plightvec:dword
 externdef _r_ambientlight:dword
 externdef _r_shadelight:dword
 externdef _aliasxcenter:dword
 externdef _aliasycenter:dword
 externdef _a_sstepxfrac:dword
 externdef _r_affinetridesc:dword
 externdef _acolormap:dword
 externdef _d_pcolormap:dword
 externdef _r_affinetridesc:dword
 externdef _d_sfrac:dword
 externdef _d_ptex:dword
 externdef _d_pedgespanpackage:dword
 externdef _d_tfrac:dword
 externdef _d_light:dword
 externdef _d_zi:dword
 externdef _d_pdest:dword
 externdef _d_pz:dword
 externdef _d_aspancount:dword
 externdef _erroradjustup:dword
 externdef _errorterm:dword
 externdef _d_xdenom:dword
 externdef _r_p0:dword
 externdef _r_p1:dword
 externdef _r_p2:dword
 externdef _a_tstepxfrac:dword
 externdef _r_sstepx:dword
 externdef _r_tstepx:dword
 externdef _a_ststepxwhole:dword
 externdef _zspantable:dword
 externdef _skintable:dword
 externdef _r_zistepx:dword
 externdef _erroradjustdown:dword
 externdef _d_countextrastep:dword
 externdef _ubasestep:dword
 externdef _a_ststepxwhole:dword
 externdef _a_tstepxfrac:dword
 externdef _r_lstepx:dword
 externdef _a_spans:dword
 externdef _erroradjustdown:dword
 externdef _d_pdestextrastep:dword
 externdef _d_pzextrastep:dword
 externdef _d_sfracextrastep:dword
 externdef _d_ptexextrastep:dword
 externdef _d_countextrastep:dword
 externdef _d_tfracextrastep:dword
 externdef _d_lightextrastep:dword
 externdef _d_ziextrastep:dword
 externdef _d_pdestbasestep:dword
 externdef _d_pzbasestep:dword
 externdef _d_sfracbasestep:dword
 externdef _d_ptexbasestep:dword
 externdef _ubasestep:dword
 externdef _d_tfracbasestep:dword
 externdef _d_lightbasestep:dword
 externdef _d_zibasestep:dword
 externdef _zspantable:dword
 externdef _r_lstepy:dword
 externdef _r_sstepy:dword
 externdef _r_tstepy:dword
 externdef _r_zistepy:dword
 externdef _D_PolysetSetEdgeTable:dword
 externdef _D_RasterizeAliasPolySmooth:dword
 externdef float_point5:dword
 externdef Float2ToThe31nd:dword
 externdef izistep:dword
 externdef izi:dword
 externdef FloatMinus2ToThe31nd:dword
 externdef float_1:dword
 externdef float_particle_z_clip:dword
 externdef float_minus_1:dword
 externdef float_0:dword
 externdef fp_16:dword
 externdef fp_64k:dword
 externdef fp_1m:dword
 externdef fp_1m_minus_1:dword
 externdef fp_8:dword
 externdef entryvec_table:dword
 externdef advancetable:dword
 externdef sstep:dword
 externdef tstep:dword
 externdef pspantemp:dword
 externdef counttemp:dword
 externdef jumptemp:dword
 externdef reciprocal_table:dword
 externdef DP_Count:dword
 externdef DP_u:dword
 externdef DP_v:dword
 externdef DP_32768:dword
 externdef DP_Color:dword
 externdef DP_Pix:dword
 externdef DP_EntryTable:dword
 externdef pbase:dword
 externdef s:dword
 externdef t:dword
 externdef sfracf:dword
 externdef tfracf:dword
 externdef snext:dword
 externdef tnext:dword
 externdef spancountminus1:dword
 externdef zi16stepu:dword
 externdef sdivz16stepu:dword
 externdef tdivz16stepu:dword
 externdef zi8stepu:dword
 externdef sdivz8stepu:dword
 externdef tdivz8stepu:dword
 externdef reciprocal_table_16:dword
 externdef entryvec_table_16:dword
 externdef ceil_cw:dword
 externdef single_cw:dword
 externdef fp_64kx64k:dword
 externdef pz:dword
 externdef spr8entryvec_table:dword
 externdef _transTable:dword
_TEXT SEGMENT
 align 4
 public _D_DrawParticle1x1b
_D_DrawParticle1x1b: 
 push ebp
 push edi
 push ebx
 mov edi,ds:dword ptr[12+4+esp]
 fld ds:dword ptr[_r_origin]
 fsubr ds:dword ptr[0+edi]
 fld ds:dword ptr[0+4+edi]
 fsub ds:dword ptr[_r_origin+4]
 fld ds:dword ptr[0+8+edi]
 fsub ds:dword ptr[_r_origin+8]
 fxch st(2)
 fld ds:dword ptr[_r_ppn]
 fmul st(0),st(1)
 fld ds:dword ptr[_r_ppn+4]
 fmul st(0),st(3)
 fld ds:dword ptr[_r_ppn+8]
 fmul st(0),st(5)
 fxch st(2)
 faddp st(1),st(0)
 faddp st(1),st(0)
 fld st(0)
 fdivr ds:dword ptr[float_1]
 fxch st(1)
 fcomp ds:dword ptr[float_particle_z_clip]
 fxch st(3)
 fld ds:dword ptr[_r_pup]
 fmul st(0),st(2)
 fld ds:dword ptr[_r_pup+4]
 fnstsw ax
 test ah,1
 jnz LPop6AndDone
 fmul st(0),st(4)
 fld ds:dword ptr[_r_pup+8]
 fmul st(0),st(3)
 fxch st(2)
 faddp st(1),st(0)
 faddp st(1),st(0)
 fxch st(3)
 fmul ds:dword ptr[_r_pright+4]
 fxch st(2)
 fmul ds:dword ptr[_r_pright]
 fxch st(1)
 fmul ds:dword ptr[_r_pright+8]
 fxch st(2)
 faddp st(1),st(0)
 faddp st(1),st(0)
 fxch st(1)
 fmul st(0),st(2)
 fxch st(1)
 fmul st(0),st(2)
 fxch st(1)
 fsubr ds:dword ptr[_ycenter]
 fxch st(1)
 fadd ds:dword ptr[_xcenter]
 fxch st(1)
 fadd ds:dword ptr[float_point5]
 fxch st(1)
 fadd ds:dword ptr[float_point5]
 fxch st(2)
 fmul ds:dword ptr[DP_32768]
 fxch st(2)
 fistp ds:dword ptr[DP_u]
 fistp ds:dword ptr[DP_v]
 mov eax,ds:dword ptr[DP_u]
 mov edx,ds:dword ptr[DP_v]
 mov ebx,ds:dword ptr[_d_vrectbottom_particle]
 mov ecx,ds:dword ptr[_d_vrectright_particle]
 cmp edx,ebx
 jg LPop1AndDone
 cmp eax,ecx
 jg LPop1AndDone
 mov ebx,ds:dword ptr[_d_vrecty]
 mov ecx,ds:dword ptr[_d_vrectx]
 cmp edx,ebx
 jl LPop1AndDone
 cmp eax,ecx
 jl LPop1AndDone
 fld ds:dword ptr[12+edi]
 fistp ds:dword ptr[DP_Color]
 mov ebx,ds:dword ptr[_d_viewbuffer]
 add ebx,eax
 mov edi,ds:dword ptr[_d_scantable+edx*4]
 imul edx,ds:dword ptr[_d_zrowbytes]
 lea edx,ds:dword ptr[edx+eax*2]
 mov eax,ds:dword ptr[_d_pzbuffer]
 fistp ds:dword ptr[izi]
 add edi,ebx
 add edx,eax
 mov eax,ds:dword ptr[izi]
 mov ecx,ds:dword ptr[_d_pix_shift]
 shr eax,cl
 mov ebp,ds:dword ptr[izi]
 mov ebx,ds:dword ptr[_d_pix_min]
 mov ecx,ds:dword ptr[_d_pix_max]
 cmp eax,ebx
 jnl LTestPixMax
 mov eax,ebx
 jmp LTestDone
LTestPixMax:
 cmp eax,ecx
 jng LTestDone
 mov eax,ecx
LTestDone:
 mov cx,ds:word ptr[DP_Color]  ; get color
 mov ebx,ds:dword ptr[_d_y_aspect_shift]
 test ebx,ebx
 jnz LDefault
 cmp eax,4
 ja LDefault
 public DP_1x1b
DP_1x1b:
 cmp ds:word ptr[edx],bp
 jg LDone
; mov ds:word ptr[edx],bp
 test ch,ch
 jnz Trans
 mov ds:byte ptr[edi],cl
 jmp LDone
Trans:
 and ecx, 0ffh
 mov ch, cl
 mov eax, ds:dword ptr[_transTable]
 mov cl, ds:byte ptr[edi]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[edi], cl
 jmp LDone


LDefault:
 test ch,ch
 jnz LTDefault

 mov ebx,eax
 mov ds:dword ptr[DP_Pix],eax
 push cx
 mov cl,ds:byte ptr[_d_y_aspect_shift]
 shl ebx,cl
 pop cx

LGenRowLoop:
 mov eax,ds:dword ptr[DP_Pix]
LGenColLoop:
 cmp ds:word ptr[-2+edx+eax*2],bp
 jg LGSkip
 mov ds:word ptr[-2+edx+eax*2],bp
 mov ds:byte ptr[-1+edi+eax*1],cl
LGSkip:
 dec eax
 jnz LGenColLoop
 add edx,ds:dword ptr[_d_zrowbytes]
 add edi,ds:dword ptr[_screenwidth]
 dec ebx
 jnz LGenRowLoop
 jmp LDone

LTDefault:
 push esi
 and ecx, 0ffh
 mov ch, cl
 mov esi, ds:dword ptr[_transTable]

 mov ebx,eax
 mov ds:dword ptr[DP_Pix],eax
 mov cl,ds:byte ptr[_d_y_aspect_shift]
 shl ebx,cl
 
LTGenRowLoop:
 mov eax,ds:dword ptr[DP_Pix]
LTGenColLoop:
 cmp ds:word ptr[-2+edx+eax*2],bp
 jg LTGSkip
 mov ds:word ptr[-2+edx+eax*2],bp

 mov cl, ds:byte ptr[-1+edi+eax*1]
 mov cl, ds:byte ptr[esi+ecx]
 mov ds:byte ptr[-1+edi+eax*1],cl

LTGSkip:
 dec eax
 jnz LTGenColLoop
 add edx,ds:dword ptr[_d_zrowbytes]
 add edi,ds:dword ptr[_screenwidth]
 dec ebx
 jnz LTGenRowLoop

 pop esi

LDone:
 pop ebx
 pop edi
 pop ebp
 ret
LPop6AndDone:
 fstp st(0)
 fstp st(0)
 fstp st(0)
 fstp st(0)
 fstp st(0)
LPop1AndDone:
 fstp st(0)
 jmp LDone
_TEXT ENDS
 END
