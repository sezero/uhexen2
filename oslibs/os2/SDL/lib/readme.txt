Simple DirectMedia Layer for OS/2 & eComStation


This archive contains:

  SDL v. 1.2.15
  SDL_ttf v. 2.0.10
  SDL_image v. 1.2.12
  SDL_mixer v. 1.2.12
  SDL_net v. 1.2.8
  SDL_sound v. 1.0.3

OpenGL not supported by this port.

"Original" port SDL/2 by Doodle fastkeys implemented:
- Alt+Home switches between fullscreen and windowed mode.
- Alt+End simulates closing the window (can be used as a Panic key).
(Alt is left Alt key, not AltGr)

This port's features:
- Video drivers DIVE or VMAN can be selected for different applications.
- AltGr+Enter fastkey switches between fullscreen and windowed mode.
- mp3 format support (SDL_mixer, SDL_sound)

See sdl.ini for detailed information.

You may significantly increase video output speed with OS4 kernel and patched
files vman.dll and dive.dll or with latest versions of ACPI support and video
driver Panorama.

Latest versions of OS4 kernel:
  http://gus.biysk.ru/os4/

Patched files vman.dll and dive.dll:
  http://gus.biysk.ru/os4/test/pached_dll/PATCHED_DLL.RAR


Installing
----------

eComStation

1. Make a Backup copy of
     SDL12.DLL 
     SDLIMAGE.DLL
     SDLMIXER.DLL
     SDLNET.DLL
     SDLTTF.DLL
   located in D:\ecs\dll (where D: is disk on which installed eComStation).

2. Stop all programs running with SDL

3. Copy sdl12.dll, sdlimage.dll, sdlmixer.dll, sdlnet.dll, sdlttf.dll and
   sdlsound.dll from .\dll to D:\ecs\dll

OS/2

Copy sdl12.dll, sdlimage.dll, sdlmixer.dll, sdlnet.dll, sdlttf.dll and
sdlsound.dll from .\dll to any directory on your LIBPATH.  If you have a
previous version installed, close all SDL applications before replacing the old
copy. Also make sure that any other old versions of DLLs are removed from your
system.

You can safely ignore all other files from this package unless you plan to
develop SDL programs using this library.


Andrey Vasilkin, 2016
digi@os2.snc.ru
