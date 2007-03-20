The asm files we use are *.asm in nasm syntax, and are actually
in the game trees.  The *.s files in the id_asm directory are
from iD's quake source and they have comments in them. The masm
files in the asm_raven directory are from Raven's hexen2 source.
Unfortunately, not all hexen2 asm code are identical to their
quake1 counterparts. In addition, there are additional hexen2
specific ones..  The original q1 asm have their C equivalents in
some way, but the hexen2 specific ones don't, so the code is more
difficult to port.

From Dan Olson:  " The code isn't compilable on non-intel until
all of the asm is taken out. Don't worry about the id386 define
*yet*, and even after all of the assembly is replaced, you may
still need it defined for non-x86  compiles.  The eventual goal
should probably be to get rid of all x86 specific stuff. "
