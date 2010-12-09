The asm files we use are *.asm in nasm syntax, and actually are
in the game trees.  The *.s files in the asm_id directory are
from iD's quake source and they have comments in them. The masm
files in the asm_raven directory are from Raven's hexen2 source.
Unfortunately, not all hexen2 asm code are identical to their
quake1 counterparts. In addition, there are additional hexen2
specific ones (translucency).  The original q1 asm have their C
equivalents, but the hexen2 specific ones didn't, so the code was
more difficult to port.  As of Hammer of Thyrion 1.4.2, support
for non-intel cpus was added to the software renderer. There are
still a few rough edges, but it is a working solution.  The asm
files here are for reference.

