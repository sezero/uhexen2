#include "launcher_defs.h"

/* [with or without OpenGL][with or without MP]
   table [1][0] is with openGL and without MP
   See the name tables below.		*/
int table[2][2] = {  {3,2}, {1,0}  };

char *h2_binary_names[]={

	"glh2mp",	/* OpenGL and Mis. Pack */	/* 0 */
	"glhexen2",	/* GL and no MP		*/	/* 1 */
	"h2mp",		/* software and MP	*/	/* 2 */
	"hexen2"	/* software and no MP	*/	/* 3 */
};

char *hw_binary_names[]={

	"hwcl",		/* Software	*/	/* 0 */
	"glhwcl"	/* OpenGL	*/	/* 1 */
};

/* [resolution]
   -width values only. corresponding -height is in the game binary */
char *resolution_args[]={

	"320",
	"400",
	"512",
	"640",
	"800",
	"1024",
	"1280"
};

