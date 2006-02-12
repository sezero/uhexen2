================================================================================================
This is version 1.0 of JsColour 
--------------------------------------------------------
JsColour is a modification of MHColour (based on source for version 0.5). It is a light 
colouring utility that adds colored lighting information into .BSP file or generates .LIT 
file. Colour is based on texture type of textures that are close to light source.
  
Now you can define colours for texture based coloring in a external file instead having to 
define it source code and recompile it. To specify external definition file use -external 
switch with name of file as parameter.

JsColour still contain definition for default texture sets in source code. In case you 
don't want to use them you can prevent it by using -nodefault switch on command line.  
Definition from external file dont replace built-in definition for the same texture. If 
you want to modify settings for default textures use -nodefault switch and define all 
textures you want to use in coloring process externaly. I have exported default settings 
for in-build textures into file default.def .
     

New command line switches
-------------------------
-external FILENAME
Turns on usage of external light colour definition file. External definition file name (FILENAME) 
must be specified.  

-nodefault
Suppres usage of built-in light color definition for default texture sets. Usefull if you want
to use light coloring for only limited subset of textures. Those that should be used then should 
be defined in external definition file.   
			
Format of external light colour definition file
-----------------------------------------------
It is prety simple. Each entry is on a new line. Entry starts with texture name (just a string 
without qoutes) and is folowed by colour triplet eg. red, green and blue component (iteger number 
in range of ( 0 - 255 ). They are separated by space. Entry ends with newline char (cr,lf).

texture_name red_val green_val blue_val	<newline>	
[1-64 chars] [0-255] [0-255]   [0-255] 

Example
-------
*lava 	  255  10  10
*water 	   64 128 128
*slime 	   10 255  10
*teleport  30  30  30

File default.def contains in-built values in external form.
--------------------------------------------------------

Legal : JsColour modifies MHColour which falls under GPL so it goes under GPL too naturaly. Read
gnu.txt for more informations.
================================================================================================
This is version 0.5 of MHColour - use at your own peril!
--------------------------------------------------------

(...although I must admit it won't really cause you any problems...)

MHColour is *not* a replacement light utility.  It is intended to be used on any existing
BSP as a method of getting coloured light in.  In it's current incarnation, it's pretty
accurate, if somewhat slow.

In it's default mode, it will generate a (fake) BSP 30 map for you.

You can also use it to generate a LIT file compatible with the DarkPlaces engine.  This
will cause it to lose some accuracy in places since it must fiddle with the light offsets
of the LIT file a little in order to avoid mutilating the BSP.

BSP 30 is recommended as the most reliable method.


Usage
-----
MHColour was derived from Tyrlite 0.8, so the usage is mostly similar.  The -colored switch
has been removed because it makes no sense in this context.

MHColour currently only supports the standard Quake, Rogue and Hipnotic texture sets.  I
have received reports of it crashing on TF maps, but have been too busy with the new
engine to look into this in detail.

New command line switches
-------------------------
-force
While running, MHColour will display loads of interesting information on how effective the
colouring process is gonna be.  At any time it decides that the process will be ineffective
you will get an option to abandon ship.  Use -force to override this and always continue,
but don't say I didn't warn you if you don't like what you see.

-lit
Generate a LIT file.  Bear in mind the warning above.  The loss of accuracy will only
occur in one or two places in one or two maps, so it's not really that big an issue.


Boring legal stuff
------------------
MHColour was derived from Tyrlite 0.8 which was GPL'ed - consequently, the GPL also covers
MHColour, even if *not explicitly stated* in the program files.  Any evil, bad or nasty
people out there who like making money out of stuff that others have given away for free
can go fuck off right now.

