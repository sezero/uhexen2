del *.obj
del *.lib
wmaker -h -f Makefile.wat

wlib -q -b -t au_cards.lib +au.obj
wlib -q -b -t au_cards.lib +ac97_def.obj
wlib -q -b -t au_cards.lib +dpmi_c.obj
wlib -q -b -t au_cards.lib +mdma.obj
wlib -q -b -t au_cards.lib +pcibios.obj
wlib -q -b -t au_cards.lib +sc_cmi.obj
wlib -q -b -t au_cards.lib +sc_e1371.obj
wlib -q -b -t au_cards.lib +sc_ich.obj
wlib -q -b -t au_cards.lib +sc_inthd.obj
wlib -q -b -t au_cards.lib +sc_sbliv.obj
wlib -q -b -t au_cards.lib +sc_sbl24.obj
wlib -q -b -t au_cards.lib +sc_sbxfi.obj
wlib -q -b -t au_cards.lib +sc_via82.obj
wlib -q -b -t au_cards.lib +tim.obj

del *.obj
