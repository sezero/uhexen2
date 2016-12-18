SDL - OS/2 Developer's package
------------------------------


1. General info
---------------


This package contains the latest SDL binaries and headers which can be used
to create or port an application to OS/2, which uses SDL.

There are the following subdirectories currently:

- SDL: (libSDL 1.2.10)
  Contains the base SDL itself.
- SDLMixer: (SDL_Mixer 1.2.6)
  Contains the SDL_Mixer library, but lacks the MP3 decoding capability. All
  other formats (supported by original SDL_Mixer) are supported.
- SDLImage: (SDL_Image 1.2.5)
  Contains the SDL_Image library. Supports every image format that is supported
  by the original SDL_Image library.
- SDLGfx: (SDL_gfx 2.0.13)
  Contains the SDL_Image library. Supports every image format that is supported
  by the original SDL_Image library.
- SDLNet: (SDL_Net 1.2.6)
  Contains the SDL_Net library. Not yet fully tested.
- SDLTTF: (SDL_ttf 2.0.7)
  Contains the SDL_TTF library.


Please note that:

SDL uses the Scitech SNAP API to have video support, so the SDL apps will 
require Scitech SNAP or Scitech Display Doctor in order to run.



3. What is missing?
-------------------

The following things are missing from this SDL implementation:
- MMX, SSE and 3DNOW! optimized video blitters?
- HW Video surfaces
- OpenGL support



4. Special Keys / Full-Screen support
-------------------------------------

There are two special hot-keys implemented:
- Alt+Home switches between fullscreen and windowed mode
- Alt+End simulates closing the window (can be used as a Panic key)
Only the LEFT Alt key will work.



5. Joysticks on SDL/2
---------------------

The Joystick detection only works for standard joysticks (2 buttons, 2 axes
and the like). Therefore, if you use a non-standard joystick, you should
specify its features in the SDL_OS2_JOYSTICK environment variable in a batch
file or CONFIG.SYS, so SDL applications can provide full capability to your
device. The syntax is:

SET SDL_OS2_JOYSTICK=[JOYSTICK_NAME] [AXES] [BUTTONS] [HATS] [BALLS]

So, it you have a Gravis GamePad with 4 axes, 2 buttons, 2 hats and 0 balls,
the line should be:

SET SDL_OS2_JOYSTICK=Gravis_GamePad 4 2 2 0

If you want to add spaces in your joystick name, just surround it with
quotes or double-quotes:

SET SDL_OS2_JOYSTICK='Gravis GamePad' 4 2 2 0

or

SET SDL_OS2_JOYSTICK="Gravis GamePad" 4 2 2 0

   Notive However that Balls and Hats are not supported under OS/2, and the
value will be ignored... but it is wise to define these correctly because 
in the future those can be supported.
   Also the number of buttons is limited to 2 when using two joysticks,
4 when using one joystick with 4 axes, 6 when using a joystick with 3 axes
and 8 when using a joystick with 2 axes. Notice however these are limitations 
of the Joystick Port hardware, not OS/2.



6. Proportional windows
-----------------------

For some SDL applications it can be handy to have proportional windows, so
the windows will keep their aspect ratio when resized.
This can be achieved in two ways:

- Before starting the given SDL application, set the
  SDL_USE_PROPORTIONAL_WINDOW environment variable to something, e.g.:

  SET SDL_USE_PROPORTIONAL_WINDOW=1
  dosbox.exe

- If you have a HOME environment variable set, then SDL will look for a file
  in there called ".sdl.proportionals". If that file contains the name of the
  currently running SDL executable, then that process will have proportional
  windows automatically.

  Please note that this file is created automatically with default values
  at the first run.



7. Audio in SDL applications
----------------------------

Audio effects are one of the most important features in games. Creating audio
effects in sync with the game and without hickups and pauses in the audio are
very important things.

However there are multithreaded SDL applications that have tight loops as their
main logic loop. This kills performance in OS/2, and takes too much CPU from
other threads in the same process, for example from the thread to create the 
sound effects.

For this reason, the OS/2 port of SDL can be instructed to run the audio thread
in high priority, which makes sure that there will be enough time for the 
processing of the audio data.

At default, SDL/2 runs the audio thread at ForegroundServer+0 priority. Well 
written and well behaving SDL applications should work well in this mode.
For other applications, you can tell SDL/2 to run the audio thread at 
TimeCritical priority by setting an env.variable before starting the SDL app:

    SET SDL_USE_TIMECRITICAL_AUDIO=1

Please note that this is a bit risky, because if the SDL application runs a
tight infinite loop in this thread, this will make the whole system 
unresponsive, so use it with care, and only for applications that need it!



8. Next steps...
----------------

Things to do:
- Implement missing stuffs (look for 'TODO' string in source code!)
- Finish video driver (the 'wincommon' can be a good example for missing
  things like application icon and so on...)
- Enable MMX/SSE/SSE2 acceleration functions
- Rewrite CDROM support using DOS Ioctl for better support.



9. Contacts
-----------

   You can contact the developers for bugs:

   Area					Developer		email
   General (Audio/Video/System)		Doodle			doodle@scenergy.dfmk.hu
   CDROM and Joystick			Caetano			daniel@caetano.eng.br

   Notice however that SDL/2 is 'in development' stage so ... if you want to help,
please, be our guest and contact us!



10. Changelog of the OS/2 port
------------------------------

Version 1.2.10 - 2008-08-04  - Doodle
 - Fixed the mouse double clicking bug (one mouse press event was not generated in case
   of mouse button double click)
   
Version 1.2.10 - 2008-01-21  - Doodle
 - Replaced DIVE backend with VMAN backend, for better performance for Panorama users.
   It also has the advantage of having fullscreen support.
 - Made a workaround for the game Wesnoth, so SDL_GetTicks() can now be called without
   SDL itself being initialized first.
 - Fixed a bug with unicode keyboard handling.

Version 1.2.10 - 2007-06-18  - Doodle
 - Added support for DIVE. This is being used if SNAP is not found or cannot
   be used for some reason. Please note that SDL in DIVE mode does not support
   fullscreen mode. Also note that DIVE is a bit slower than SNAP.
 - New environment variable: SDL_DISABLE_FSLIB. Set it to something if you
   want to use the DIVE mode even if you have SNAP installed. 
   e.g: set SDL_DISABLE_FSLIB=1
 - Minimizing the SDL window does not report resize to the SDL app from now on.
   (This fixes rearranged windows problem of Pixel)
 - Fixed SetVideoMode() call not to free the old surface.
   (This fixes the crash of the OS/2 port of MPlayer)

Version 1.2.10 - 2007-01-21  - Doodle
 - Fixed a bug of reporting available fullscreen modes which prevented the OS/2
   port of MPlayer to work properly.
 - Updated FSLib.dll to latest version, fixing a bug which happened when the 
   SDL app wanted to use the same resolution for fullscreen mode that is used
   for Desktop mode.
 - Lowered the priority of PM thread from TIMECRITICAL to FOREGROUNDSERVER
 - Small fix in DART callback

Version 1.2.10 - 2006-06-02  - Doodle
 - Small change in SDL_thread.h to better support GCC on OS/2
 - Fixed testapp makefile
 - Fixed SDLNET.DLL

Version 1.2.10 - 2006-05-17  - Doodle
 - Small modifications for v1.2.10 release
 - Changed DLL name to include version info (currently SDL12.dll)

Version 1.2 - 2006-05-01  - Doodle
 - Modified makefile system to have only one makefile
 - Included FSLib headers, DLL and LIB file

Version 1.2 - 2006-02-26  - Doodle
 - Updated the official SDL version with the OS/2 specific changes.
 - Added support for real unicode keycode conversion.

Version 1.2.7 - 2006-01-20  - Doodle
 - Added support for selectively using timecritical priority for
   audio threads by SDL_USE_TIMECRITICAL_AUDIO environment variable.
   (e.g.: 
    SET SDL_USE_TIMECRITICAL_AUDIO=1
    dosbox.exe
   )

Version 1.2.7 - 2005-12-22  - Doodle
 - Added support for proportional SDL windows.
   There are two ways to have proportional (aspect-keeping) windows for
   a given SDL application: Either set the SDL_USE_PROPORTIONAL_WINDOW
   environment variable to something before starting the application
   (e.g.: 
    SET SDL_USE_PROPORTIONAL_WINDOW=1
    dosbox.exe
   )
   or, if you have the HOME environment variable set, then SDL1210.DLL will
   create a file in that directory called .sdl.proportionals, and you can
   put there the name of executable files that will be automatically made
   proportional.

Version 1.2.7 - 2005-10-14  - Doodle
 - Enabled Exception handler code in FSLib to be able to restore original
   desktop video mode in case the application crashes.
 - Added the missing FSLib_Uninitialize() call into SDL.
   (The lack of it did not cause problems, but it's cleaner this way.)
 - Fixed a mouse problem in Fullscreen mode where any mouse click
   re-centered the mouse.

Version 1.2.7 - 2005-10-09  - Doodle
 - Implemented window icon support

Version 1.2.7 - 2005-10-03  - Doodle
 - Reworked semaphore support again
 - Tuned thread priorities

Version 1.2.7 - 2005-10-02  - Doodle
 - Added support for custom mouse pointers
 - Fixed WM_CLOSE processing: give a chance to SDL app to ask user...
 - Added support for MMX-accelerated audio mixers
 - Other small fixes

Version 1.2.7 - 2005-09-12  - Doodle
 - Small fixes for DosBox incorporated into public release
 - Fixed semaphore support (SDL_syssem.c)
 - Fixed FSLib to have good clipping in scaled window mode,
   and to prevent occasional desktop freezes.

Version 1.2.7 - 2004-09-08a - Caetano
	- Improved joystick support (general verifications about hardware).
	- Added support up to 8 buttons in 2 axes joysticks and 6 buttons in 3 axes joysticks.
	- Added support to environment variable SDL_OS2_JOYSTICK to specify a joystick.
	- Improved Joystick test to handle every type of joystick and display only relevant information.
	- Merged with Doodle 2004-09-08
	- Little tid up in README.OS2
	- Added explanation about SDL_OS2_JOYSTICK environment variable on README.OS2

Version 1.2.7 - 2004-09-07 - Caetano
	- Merged with changes in headers for GCC compiling.
	- Added Joystick support using basic IBM GAME$ support, allowing it to work with all joystick drivers since OS/2 2.1.
	- Improved joystick detection (hacked!). OS/2 do not allow real joystick detection, so... 
	- Modified makefile in test to compile "testjoystick". Anyway, it's useless, since it seems to cause a lot of trouble in OS/2 (because os video routines, not Joystick support).
	- Created separated Joystick test program to test only joystick functions.
	- Improved joystick auto-centering.
	- Improved the coordinate correction routine to use two scale factors for each axis.

Version 1.2.7 - 2004-07-05 - Caetano
	- Corrected the time returned by status in CDROM support (it was incorrect)
	- Added the testcdrom.c and corrected the linking directive (it was causing an error)

Version 1.2.7 - 2004-07-02a - Caetano
	- Corrected a little problem in a comment at SDL-1.2.7\test\torturethread.c, line 18 (missing */, nested comment)
	- Added CDROM support to tree (SDL-1.2.7\src\cdrom\os2\SDL_syscdrom.c)
	- Modified makefile (SDL-1.2.7\src\makefiles.wat and SDL-1.2.7\watcom.mif) to build with CDROM support
	- Added the "extra" SDL_types.h forgotten in 2004-07-02 version.

<End-Of-File>
