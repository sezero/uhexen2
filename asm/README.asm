
These are the *.s x86 asm files.

The asm files we use are *.asm in nasm syntax, and are actually
in the game trees.  The *.s here have comments in them, and all
seem to be from original quake1.  Unfortunately, not all hexen2
asm code are identical their quake1 counterparts, and there are
additional hexen2 specific ones, as well..  The original q1 asm
have their C equivalents in some way, but the hexen II specific
ones don't, so the code is more difficult to port.

From Dan Olson:  " The code isn't compilable on non-intel until
all of the asm is taken out. Don't worry about the id386 define
*yet*, and even after all of the assembly is replaced, you may
still need it defined for non-x86  compiles.  The eventual goal
should probably be to get rid of all x86 specific stuff. "
