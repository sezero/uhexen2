 .386P
 .model FLAT
 externdef _snd_scaletable:dword
 externdef _paintbuffer:dword
 externdef _snd_linear_count:dword
 externdef _snd_p:dword
 externdef _snd_vol:dword
 externdef _snd_out:dword
 externdef _vright:dword
 externdef _vup:dword
 externdef _vpn:dword
 externdef _BOPS_Error:dword
_DATA SEGMENT
Ltemp dd 0
_DATA ENDS
_TEXT SEGMENT
 align 4
 public _SV_HullPointContents
_SV_HullPointContents:
 push edi
 mov eax,ds:dword ptr[8+4+esp]
 test eax,eax
 js Lhquickout
 push ebx
 mov ebx,ds:dword ptr[4+8+esp]
 push ebp
 mov edx,ds:dword ptr[12+12+esp]
 mov edi,ds:dword ptr[0+ebx]
 mov ebp,ds:dword ptr[4+ebx]
 sub ebx,ebx
 push esi
Lhloop:
 mov ecx,ds:dword ptr[0+edi+eax*8]
 mov eax,ds:dword ptr[4+edi+eax*8]
 mov esi,eax
 ror eax,16
 lea ecx,ds:dword ptr[ecx+ecx*4]
 mov bl,ds:byte ptr[16+ebp+ecx*4]
 cmp bl,3
 jb Lnodot
 fld ds:dword ptr[0+ebp+ecx*4]
 fmul ds:dword ptr[0+edx]
 fld ds:dword ptr[0+4+ebp+ecx*4]
 fmul ds:dword ptr[4+edx]
 fld ds:dword ptr[0+8+ebp+ecx*4]
 fmul ds:dword ptr[8+edx]
 fxch st(1)
 faddp st(2),st(0)
 faddp st(1),st(0)
 fsub ds:dword ptr[12+ebp+ecx*4]
 jmp Lsub
Lnodot:
 fld ds:dword ptr[12+ebp+ecx*4]
 fsubr ds:dword ptr[edx+ebx*4]
Lsub:
 sar eax,16
 sar esi,16
 fstp ds:dword ptr[Ltemp]
 mov ecx,ds:dword ptr[Ltemp]
 sar ecx,31
 and esi,ecx
 xor ecx,0FFFFFFFFh
 and eax,ecx
 or eax,esi
 jns Lhloop
Lhdone:
 pop esi
 pop ebp
 pop ebx
Lhquickout:
 pop edi
 ret
_TEXT ENDS
 END
