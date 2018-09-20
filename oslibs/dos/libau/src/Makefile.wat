.c.obj:	.AUTODEPEND
	wcc386 -bt=DOS -zp=1 -zq -zm -zl -zls -zld -ox -3r -fp3 -fpi87 -ecd -dSDR -fo=$^@ $<

OBJ = pcibios.obj dpmi_c.obj mdma.obj au.obj tim.obj sc_inthd.obj ac97_def.obj sc_cmi.obj sc_e1371.obj sc_ich.obj sc_sbl24.obj sc_sbliv.obj sc_sbxfi.obj sc_via82.obj

libau.lib: $(OBJ)
	wlib -q -b -n -c -pa -s -t -zld -ii -io $@ $(OBJ)

clean: .SYMBOLIC
	rm -f *.obj
distclean: clean .SYMBOLIC
	rm -f *.lib *.err
