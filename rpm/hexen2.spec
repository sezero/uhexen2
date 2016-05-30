# RPM spec file for RedHat and Fedora.
# $Id$

# build options :
# --without alsa: build without alsa audio support
# --without midi: build without a midi driver support
#		 (defunct: there is no linux midi "driver" yet,
#		  midi playback is by timidity for now.)
# --without timidity: build without timidity music streaming support
# --without wavmusic: build without wav music streaming support
# --without mp3: build without mp3 music streaming support
# --with mpg123: build mp3 music streaming using libmpg123 instead of libmad
# --without ogg: build without ogg/vorbis music streaming support
# --with flac: build with flac music streaming support
# --with opus: build with opus music streaming support
# --with mikmod: build with mikmod (tracker) music streaming support
# --with umx: build with unreal umx music streaming support
# --without asm: do not use x86 assembly even on an intel cpu

%ifnarch %{ix86}
%define _without_asm 1
%endif

# default build options
%{!?_without_asm:%define asm_buildopt USE_X86_ASM=yes}
%{!?_without_alsa:%define alsa_buildopt USE_ALSA=yes}
%{!?_without_midi:%define midi_buildopt USE_MIDI=yes}
%{!?_without_timidity:%define timidity_buildopt USE_CODEC_TIMIDITY=yes}
%{!?_without_wavmusic:%define wavmusic_buildopt USE_CODEC_WAVE=yes}
%{!?_with_mpg123:%define mp3_libraryopt MP3LIB=mad}
%{!?_without_mp3:%define mp3_buildopt USE_CODEC_MP3=yes}
%{!?_without_ogg:%define ogg_buildopt USE_CODEC_VORBIS=yes}
%{!?_with_flac:%define flac_buildopt USE_CODEC_FLAC=no}
%{!?_with_opus:%define opus_buildopt USE_CODEC_OPUS=no}
%{!?_with_mikmod:%define mikmod_buildopt USE_CODEC_MIKMOD=no}
%{!?_with_umx:%define umx_buildopt USE_CODEC_UMX=no}
# build option overrides
%{?_without_asm:%define asm_buildopt USE_X86_ASM=no}
%{?_without_alsa:%define alsa_buildopt USE_ALSA=no}
%{?_without_midi:%define midi_buildopt USE_MIDI=no}
%{?_without_timidity:%define timidity_buildopt USE_CODEC_TIMIDITY=no}
%{?_without_wavmusic:%define wavmusic_buildopt USE_CODEC_WAVE=no}
%{?_with_mpg123:%define mp3_libraryopt MP3LIB=mpg123}
%{?_without_mp3:%define mp3_buildopt USE_CODEC_MP3=no}
%{?_without_ogg:%define ogg_buildopt USE_CODEC_VORBIS=no}
%{?_with_flac:%define flac_buildopt USE_CODEC_FLAC=yes}
%{?_with_opus:%define opus_buildopt USE_CODEC_OPUS=yes}
%{?_with_mikmod:%define mikmod_buildopt USE_CODEC_MIKMOD=yes}
%{?_with_umx:%define umx_buildopt USE_CODEC_UMX=yes}
# all build options passed to makefile
%define engine_buildopt	%{asm_buildopt} %{alsa_buildopt} %{midi_buildopt} %{timidity_buildopt} %{wavmusic_buildopt} %{mp3_buildopt} %{mp3_libraryopt} %{ogg_buildopt} %{opus_buildopt} %{flac_buildopt} %{mikmod_buildopt} %{umx_buildopt}

%define gamecode_ver	1.29a

Name:		hexen2
License:	GPLv2
Group:		Amusements/Games
Version:	1.5.8
Release:	1
Summary:	Hexen II: Hammer of Thyrion
URL:		http://uhexen2.sourceforge.net/
Source:		http://download.sourceforge.net/uhexen2/hexen2source-%{version}.tgz
#Source1:	http://download.sourceforge.net/uhexen2/gamedata-src-%{gamecode_ver}.tgz
Source1:	http://download.sourceforge.net/uhexen2/hexen2source-gamecode-%{version}.tgz
Source2:	http://download.sourceforge.net/uhexen2/hexenworld-pakfiles-0.15.tgz
BuildRoot:	%{_tmppath}/%{name}-%{version}-build
BuildRequires:	SDL-devel >= 1.2.4
%{!?_without_mp3:BuildRequires:  %{!?_with_mpg123:libmad-devel}%{?_with_mpg123:libmpg123-devel >= 1.12.0}}
%{!?_without_ogg:BuildRequires:  libogg-devel libvorbis-devel}
%{?_with_flac:BuildRequires:  flac-devel}
%{?_with_opus:BuildRequires:  opus-devel opusfile-devel}
%{?_with_mikmod:BuildRequires:  libmikmod-devel}
%{!?_without_asm:BuildRequires:  nasm >= 0.98.38}
Obsoletes:	hexen2-missionpack
Requires:	SDL >= 1.2.4
# timidity++-patches requirement is non-fatal
#%{!?_without_timidity:Requires: timidity++-patches}
# these will be picked by rpm already
#%{?_with_flac:Requires: flac}
#%{?_with_opus:Requires: opus opusfile}
#%{!?_without_mp3:Requires: %{!?_with_mpg123:libmad}%{?_with_mpg123:libmpg123 >= 1.12.0}}
#%{!?_without_ogg:Requires: libvorbis}
#%{?_with_mikmod:Requires: libmikmod}

%description
Hexen II is a class based shooter game by Raven Software from 1997.
Hammer of Thyrion is a port of the GPL'ed Hexen II source code.
This package contains binaries that will run both the original game
and the Portal of Praevus mission pack, and a dedicated server.

%package -n hexenworld
Group:		Amusements/Games
Summary:	HexenWorld Client and Server
Requires:	SDL >= 1.2.4
Requires:	%{name} >= %{version}

%description -n hexenworld
Hexen II is a class based shooter game by Raven Software from 1997.
HexenWorld is an extension of Hexen II with internet play enhancements.
Hammer of Thyrion is a port of the GPL'ed Hexen II source code.
This package contains the files which are required to run a HexenWorld
server or client, and a master server application.

%prep
%setup -q -n hexen2source-%{version} -a1 -a2

%build
# Build the main game binaries
%{__make} -C engine/hexen2 %{engine_buildopt} h2
# use localclean instead of clean to avoid building timidity every time
%{__make} -s -C engine/hexen2 localclean
%{__make} -C engine/hexen2 %{engine_buildopt} glh2
%{__make} -s -C engine/hexen2 localclean
# Build the dedicated server
%{__make} -C engine/hexen2/server
# HexenWorld binaries
%{__make} -C engine/hexenworld/server
%{__make} -C engine/hexenworld/client %{engine_buildopt} hw
%{__make} -s -C engine/hexenworld/client localclean
%{__make} -C engine/hexenworld/client %{engine_buildopt} glhw
# HexenWorld master server
%{__make} -C hw_utils/hwmaster

# Build h2patch
%{__make} -C h2patch

# Build the hcode compiler
%{__make} -C utils/hcc
# Build the game-code
utils/hcc/hcc -src gamecode-%{gamecode_ver}/hc/h2 -os
utils/hcc/hcc -src gamecode-%{gamecode_ver}/hc/h2 -os -name progs2.src
utils/hcc/hcc -src gamecode-%{gamecode_ver}/hc/portals -os -oi -on
utils/hcc/hcc -src gamecode-%{gamecode_ver}/hc/hw -os -oi -on
#utils/hcc/hcc -src gamecode-%{gamecode_ver}/hc/siege -os -oi -on

# Done building

%install
%{__rm} -rf %{buildroot}
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/docs
%{__install} -D -m755 engine/hexen2/glhexen2 %{buildroot}/%{_prefix}/games/%{name}/glhexen2
%{__install} -D -m755 engine/hexen2/hexen2 %{buildroot}/%{_prefix}/games/%{name}/hexen2
%{__install} -D -m755 engine/hexen2/server/h2ded %{buildroot}/%{_prefix}/games/%{name}/h2ded
%{__install} -D -m755 engine/hexenworld/client/hwcl %{buildroot}/%{_prefix}/games/%{name}/hwcl
%{__install} -D -m755 engine/hexenworld/client/glhwcl %{buildroot}/%{_prefix}/games/%{name}/glhwcl
%{__install} -D -m755 engine/hexenworld/server/hwsv %{buildroot}/%{_prefix}/games/%{name}/hwsv
%{__install} -D -m755 hw_utils/hwmaster/hwmaster %{buildroot}/%{_prefix}/games/%{name}/hwmaster
%{__install} -D -m755 h2patch/h2patch %{buildroot}/%{_prefix}/games/%{name}/h2patch

# Install the run script and make symlinks to it
%{__install} -D -m755 scripts/hexen2-run.sh %{buildroot}/%{_bindir}/hexen2-run.sh
%{__ln_s} hexen2-run.sh %{buildroot}/%{_bindir}/glhexen2
%{__ln_s} hexen2-run.sh %{buildroot}/%{_bindir}/hexen2
%{__ln_s} hexen2-run.sh %{buildroot}/%{_bindir}/h2ded
%{__ln_s} hexen2-run.sh %{buildroot}/%{_bindir}/glhwcl
%{__ln_s} hexen2-run.sh %{buildroot}/%{_bindir}/hwcl
%{__ln_s} hexen2-run.sh %{buildroot}/%{_bindir}/hwsv

# Install the cd-rip scripts
%{__install} -D -m755 scripts/cdrip_hexen2.sh %{buildroot}/%{_prefix}/games/%{name}/cdrip_hexen2.sh
%{__install} -D -m755 scripts/cdrip_hexen2_xplosiv.sh %{buildroot}/%{_prefix}/games/%{name}/cdrip_hexen2_xplosiv.sh
%{__install} -D -m755 scripts/cdrip_hexen2_matroxm3d.sh %{buildroot}/%{_prefix}/games/%{name}/cdrip_hexen2_matroxm3d.sh
%{__install} -D -m755 scripts/cdrip_missionpack.sh %{buildroot}/%{_prefix}/games/%{name}/cdrip_missionpack.sh

# Install the docs
%{__install} -D -m644 docs/README %{buildroot}/%{_prefix}/games/%{name}/docs/README
%{__install} -D -m644 docs/COPYING %{buildroot}/%{_prefix}/games/%{name}/docs/COPYING
%{__install} -D -m644 docs/BUGS %{buildroot}/%{_prefix}/games/%{name}/docs/BUGS
%{__install} -D -m644 docs/TODO %{buildroot}/%{_prefix}/games/%{name}/docs/TODO
%{__install} -D -m644 docs/ABOUT %{buildroot}/%{_prefix}/games/%{name}/docs/ABOUT
%{__install} -D -m644 docs/AUTHORS %{buildroot}/%{_prefix}/games/%{name}/docs/AUTHORS
%{__install} -D -m644 docs/Features %{buildroot}/%{_prefix}/games/%{name}/docs/Features
%{__install} -D -m644 docs/CHANGES %{buildroot}/%{_prefix}/games/%{name}/docs/CHANGES
%{__install} -D -m644 docs/CHANGES.old %{buildroot}/%{_prefix}/games/%{name}/docs/CHANGES.old
%{__install} -D -m644 docs/README.music %{buildroot}/%{_prefix}/games/%{name}/docs/README.music
%{__install} -D -m644 docs/README.3dfx %{buildroot}/%{_prefix}/games/%{name}/docs/README.3dfx
%{__install} -D -m644 docs/README.hwcl %{buildroot}/%{_prefix}/games/%{name}/docs/README.hwcl
%{__install} -D -m644 docs/README.hwsv %{buildroot}/%{_prefix}/games/%{name}/docs/README.hwsv
%{__install} -D -m644 docs/README.hwmaster %{buildroot}/%{_prefix}/games/%{name}/docs/README.hwmaster
%{__install} -D -m644 docs/SrcNotes.txt %{buildroot}/%{_prefix}/games/%{name}/docs/SrcNotes.txt
%{__install} -D -m644 docs/ReleaseNotes %{buildroot}/%{_prefix}/games/%{name}/docs/ReleaseNotes
%{__install} -D -m644 docs/ReleaseNotes.old %{buildroot}/%{_prefix}/games/%{name}/docs/ReleaseNotes.old

# Install the gamedata
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/data1/
%{__install} -D -m644 gamecode-%{gamecode_ver}/hc/h2/progs.dat %{buildroot}/%{_prefix}/games/%{name}/data1/progs.dat
%{__install} -D -m644 gamecode-%{gamecode_ver}/hc/h2/progs2.dat %{buildroot}/%{_prefix}/games/%{name}/data1/progs2.dat
%{__install} -D -m644 gamecode-%{gamecode_ver}/res/h2/hexen.rc %{buildroot}/%{_prefix}/games/%{name}/data1/hexen.rc
%{__install} -D -m644 gamecode-%{gamecode_ver}/res/h2/strings.txt %{buildroot}/%{_prefix}/games/%{name}/data1/strings.txt
%{__install} -D -m644 gamecode-%{gamecode_ver}/res/h2/default.cfg %{buildroot}/%{_prefix}/games/%{name}/data1/default.cfg
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/portals/
%{__install} -D -m644 gamecode-%{gamecode_ver}/hc/portals/progs.dat %{buildroot}/%{_prefix}/games/%{name}/portals/progs.dat
%{__install} -D -m644 gamecode-%{gamecode_ver}/res/portals/hexen.rc %{buildroot}/%{_prefix}/games/%{name}/portals/hexen.rc
%{__install} -D -m644 gamecode-%{gamecode_ver}/res/portals/strings.txt %{buildroot}/%{_prefix}/games/%{name}/portals/strings.txt
%{__install} -D -m644 gamecode-%{gamecode_ver}/res/portals/infolist.txt %{buildroot}/%{_prefix}/games/%{name}/portals/infolist.txt
%{__install} -D -m644 gamecode-%{gamecode_ver}/res/portals/maplist.txt %{buildroot}/%{_prefix}/games/%{name}/portals/maplist.txt
%{__install} -D -m644 gamecode-%{gamecode_ver}/res/portals/puzzles.txt %{buildroot}/%{_prefix}/games/%{name}/portals/puzzles.txt
%{__install} -D -m644 gamecode-%{gamecode_ver}/res/portals/default.cfg %{buildroot}/%{_prefix}/games/%{name}/portals/default.cfg
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/hw/
%{__install} -D -m644 gamecode-%{gamecode_ver}/hc/hw/hwprogs.dat %{buildroot}/%{_prefix}/games/%{name}/hw/hwprogs.dat
%{__install} -D -m644 gamecode-%{gamecode_ver}/res/hw/mapcycle.cfg %{buildroot}/%{_prefix}/games/%{name}/hw/mapcycle.cfg
%{__install} -D -m644 gamecode-%{gamecode_ver}/res/hw/server.cfg %{buildroot}/%{_prefix}/games/%{name}/hw/server.cfg
%{__install} -D -m644 gamecode-%{gamecode_ver}/res/hw/strings.txt %{buildroot}/%{_prefix}/games/%{name}/hw/strings.txt
%{__install} -D -m644 gamecode-%{gamecode_ver}/res/hw/default.cfg %{buildroot}/%{_prefix}/games/%{name}/hw/default.cfg
%{__install} -D -m644 hw/pak4.pak %{buildroot}/%{_prefix}/games/%{name}/hw/pak4.pak

# Install ent fixes handling map quirks
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/data1/maps/
%{__install} -D -m644 gamecode-%{gamecode_ver}/mapfixes/data1/maps/README.txt %{buildroot}/%{_prefix}/games/%{name}/data1/maps/README.txt
%{__install} -D -m644 gamecode-%{gamecode_ver}/mapfixes/data1/maps/cath.ent %{buildroot}/%{_prefix}/games/%{name}/data1/maps/cath.ent
%{__install} -D -m644 gamecode-%{gamecode_ver}/mapfixes/data1/maps/cath.txt %{buildroot}/%{_prefix}/games/%{name}/data1/maps/cath.txt
%{__install} -D -m644 gamecode-%{gamecode_ver}/mapfixes/data1/maps/demo2.ent %{buildroot}/%{_prefix}/games/%{name}/data1/maps/demo2.ent
%{__install} -D -m644 gamecode-%{gamecode_ver}/mapfixes/data1/maps/demo2.txt %{buildroot}/%{_prefix}/games/%{name}/data1/maps/demo2.txt
%{__install} -D -m644 gamecode-%{gamecode_ver}/mapfixes/data1/maps/egypt4.ent %{buildroot}/%{_prefix}/games/%{name}/data1/maps/egypt4.ent
%{__install} -D -m644 gamecode-%{gamecode_ver}/mapfixes/data1/maps/egypt4.txt %{buildroot}/%{_prefix}/games/%{name}/data1/maps/egypt4.txt
%{__install} -D -m644 gamecode-%{gamecode_ver}/mapfixes/data1/maps/egypt5.ent %{buildroot}/%{_prefix}/games/%{name}/data1/maps/egypt5.ent
%{__install} -D -m644 gamecode-%{gamecode_ver}/mapfixes/data1/maps/egypt5.txt %{buildroot}/%{_prefix}/games/%{name}/data1/maps/egypt5.txt
%{__install} -D -m644 gamecode-%{gamecode_ver}/mapfixes/data1/maps/romeric5.ent %{buildroot}/%{_prefix}/games/%{name}/data1/maps/romeric5.ent
%{__install} -D -m644 gamecode-%{gamecode_ver}/mapfixes/data1/maps/romeric5.txt %{buildroot}/%{_prefix}/games/%{name}/data1/maps/romeric5.txt
%{__install} -D -m644 gamecode-%{gamecode_ver}/mapfixes/data1/maps/tower.ent %{buildroot}/%{_prefix}/games/%{name}/data1/maps/tower.ent
%{__install} -D -m644 gamecode-%{gamecode_ver}/mapfixes/data1/maps/tower.txt %{buildroot}/%{_prefix}/games/%{name}/data1/maps/tower.txt
%{__install} -D -m644 gamecode-%{gamecode_ver}/mapfixes/portals/maps/README.txt %{buildroot}/%{_prefix}/games/%{name}/portals/maps/README.txt
%{__install} -D -m644 gamecode-%{gamecode_ver}/mapfixes/portals/maps/tibet2.ent %{buildroot}/%{_prefix}/games/%{name}/portals/maps/tibet2.ent
%{__install} -D -m644 gamecode-%{gamecode_ver}/mapfixes/portals/maps/tibet2.txt %{buildroot}/%{_prefix}/games/%{name}/portals/maps/tibet2.txt
%{__install} -D -m644 gamecode-%{gamecode_ver}/mapfixes/portals/maps/tibet9.ent %{buildroot}/%{_prefix}/games/%{name}/portals/maps/tibet9.ent
%{__install} -D -m644 gamecode-%{gamecode_ver}/mapfixes/portals/maps/tibet9.txt %{buildroot}/%{_prefix}/games/%{name}/portals/maps/tibet9.txt

# Install the pak deltas
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/patchdat/
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/patchdat/data1
%{__install} -D -m644 gamecode-%{gamecode_ver}/patch111/patchdat/data1/data1pk0.xd3 %{buildroot}/%{_prefix}/games/%{name}/patchdat/data1/data1pk0.xd3
%{__install} -D -m644 gamecode-%{gamecode_ver}/patch111/patchdat/data1/data1pk1.xd3 %{buildroot}/%{_prefix}/games/%{name}/patchdat/data1/data1pk1.xd3
%{__install} -D -m644 gamecode-%{gamecode_ver}/patch111/patchdat.txt %{buildroot}/%{_prefix}/games/%{name}/patchdat.txt

%{__install} -D -m644 engine/resource/hexen2.png %{buildroot}/%{_prefix}/games/%{name}/hexen2.png

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-,root,root)
%{_bindir}/hexen2-run.sh
%{_bindir}/glhexen2
%{_bindir}/hexen2
%{_bindir}/h2ded
%{_prefix}/games/%{name}/hexen2
%{_prefix}/games/%{name}/glhexen2
%{_prefix}/games/%{name}/h2ded
%{_prefix}/games/%{name}/h2patch
%{_prefix}/games/%{name}/patchdat/data1/data1pk0.xd3
%{_prefix}/games/%{name}/patchdat/data1/data1pk1.xd3
%{_prefix}/games/%{name}/patchdat.txt
%{_prefix}/games/%{name}/data1/progs.dat
%{_prefix}/games/%{name}/data1/progs2.dat
%{_prefix}/games/%{name}/data1/hexen.rc
%{_prefix}/games/%{name}/data1/strings.txt
%{_prefix}/games/%{name}/data1/default.cfg
%{_prefix}/games/%{name}/data1/maps/README.txt
%{_prefix}/games/%{name}/data1/maps/cath.ent
%{_prefix}/games/%{name}/data1/maps/cath.txt
%{_prefix}/games/%{name}/data1/maps/demo2.ent
%{_prefix}/games/%{name}/data1/maps/demo2.txt
%{_prefix}/games/%{name}/data1/maps/egypt4.ent
%{_prefix}/games/%{name}/data1/maps/egypt4.txt
%{_prefix}/games/%{name}/data1/maps/egypt5.ent
%{_prefix}/games/%{name}/data1/maps/egypt5.txt
%{_prefix}/games/%{name}/data1/maps/romeric5.ent
%{_prefix}/games/%{name}/data1/maps/romeric5.txt
%{_prefix}/games/%{name}/data1/maps/tower.ent
%{_prefix}/games/%{name}/data1/maps/tower.txt
%{_prefix}/games/%{name}/portals/progs.dat
%{_prefix}/games/%{name}/portals/hexen.rc
%{_prefix}/games/%{name}/portals/strings.txt
%{_prefix}/games/%{name}/portals/puzzles.txt
%{_prefix}/games/%{name}/portals/infolist.txt
%{_prefix}/games/%{name}/portals/maplist.txt
%{_prefix}/games/%{name}/portals/default.cfg
%{_prefix}/games/%{name}/portals/maps/README.txt
%{_prefix}/games/%{name}/portals/maps/tibet2.ent
%{_prefix}/games/%{name}/portals/maps/tibet2.txt
%{_prefix}/games/%{name}/portals/maps/tibet9.ent
%{_prefix}/games/%{name}/portals/maps/tibet9.txt
%{_prefix}/games/%{name}/cdrip_hexen2.sh
%{_prefix}/games/%{name}/cdrip_hexen2_xplosiv.sh
%{_prefix}/games/%{name}/cdrip_hexen2_matroxm3d.sh
%{_prefix}/games/%{name}/cdrip_missionpack.sh
%{_prefix}/games/%{name}/docs/README
%{_prefix}/games/%{name}/docs/COPYING
%{_prefix}/games/%{name}/docs/BUGS
%{_prefix}/games/%{name}/docs/ABOUT
%{_prefix}/games/%{name}/docs/AUTHORS
%{_prefix}/games/%{name}/docs/Features
%{_prefix}/games/%{name}/docs/CHANGES
%{_prefix}/games/%{name}/docs/CHANGES.old
%{_prefix}/games/%{name}/docs/README.music
%{_prefix}/games/%{name}/docs/README.3dfx
%{_prefix}/games/%{name}/docs/TODO
%{_prefix}/games/%{name}/docs/SrcNotes.txt
%{_prefix}/games/%{name}/docs/ReleaseNotes
%{_prefix}/games/%{name}/docs/ReleaseNotes.old
%{_prefix}/games/%{name}/hexen2.png

%files -n hexenworld
%defattr(-,root,root)
%{_bindir}/glhwcl
%{_bindir}/hwcl
%{_bindir}/hwsv
%{_prefix}/games/%{name}/hwsv
%{_prefix}/games/%{name}/hwmaster
%{_prefix}/games/%{name}/hwcl
%{_prefix}/games/%{name}/glhwcl
%{_prefix}/games/%{name}/hw/hwprogs.dat
%{_prefix}/games/%{name}/hw/mapcycle.cfg
%{_prefix}/games/%{name}/hw/server.cfg
%{_prefix}/games/%{name}/hw/pak4.pak
%{_prefix}/games/%{name}/hw/strings.txt
%{_prefix}/games/%{name}/hw/default.cfg
%{_prefix}/games/%{name}/docs/README.hwcl
%{_prefix}/games/%{name}/docs/README.hwsv
%{_prefix}/games/%{name}/docs/README.hwmaster

%changelog
* Mon May 30 2016 O.Sezer <sezero@users.sourceforge.net> 1.5.8-1
- Bump version to 1.5.8.
- Bump gamecode version to 1.29a

* Thu Jan 21 2016 O.Sezer <sezero@users.sourceforge.net> 1.5.7-1
- Removed gtk launcher, added a shell script to run different
  versions of the game, instead.
- Use hcc's new -os switch when building the hcode
- Added --with flac|mikmod|umx build options
- Bump version to 1.5.7.
- Bump gamecode version to 1.29

* Fri Mar 08 2013 O.Sezer <sezero@users.sourceforge.net> 1.5.6-1
- Add --with opus build option
- Bump version to 1.5.6.
- Bump gamecode version to 1.28

* Tue Sep 11 2012 O.Sezer <sezero@users.sourceforge.net> 1.5.5-1
- Adjusted gamedata resource installation after txt->res rename
- Adjusted h2ded build and installation after moved Makefile.sv
- Bump version to 1.5.5
- Bump gamecode version to 1.27

* Tue Jun 19 2012 O.Sezer <sezero@users.sourceforge.net> 1.5.4-1
- Changed --without gtk2 option to --with gtk1.
- Removed the beta/prerelease versioning stuff.
- Bump gamecode version to 1.26.
- Updated documents installation after the ReleaseNotes* changes.
- Bump version to 1.5.4

* Sat Apr 07 2012 O.Sezer <sezero@users.sourceforge.net> 1.5.3-1
- Bump version to 1.5.3

* Wed Feb 15 2012 O.Sezer <sezero@users.sourceforge.net> 1.5.2-1
- Added --with gtk3 build option for the launcher

* Sat Jan 14 2012 O.Sezer <sezero@users.sourceforge.net>
- gamecode version 1.25

* Sat Dec 17 2011 O.Sezer <sezero@users.sourceforge.net>
- Install the AUTHORS file

* Fri Dec 02 2011 O.Sezer <sezero@users.sourceforge.net>
- Bumped version to 1.5.2

* Mon Nov 21 2011 O.Sezer <sezero@users.sourceforge.net> 1.5.1-1
- Install the cd-rip scripts for the less common game media, too.

* Tue Oct 18 2011 O.Sezer <sezero@users.sourceforge.net>
- Fixed some typoes
- Install the CHANGES.old document.

* Fri Oct 14 2011 O.Sezer <sezero@users.sourceforge.net>
- Bumped version to 1.5.1
- Adjusted for build system changes

* Thu Sep 15 2011 O.Sezer <sezero@users.sourceforge.net> 1.5.0-1
- 1.5.0-final.

* Fri Aug 05 2011 O.Sezer <sezero@users.sourceforge.net>
- docs/ReleaseNotes-1.4.4 is no more.

* Mon Jun 20 2011 O.Sezer <sezero@users.sourceforge.net>
- Install the cd-rip scripts.

* Sun Jun 19 2011 O.Sezer <sezero@users.sourceforge.net>
- Install modified entities for the cathedral map to handle the map's
  quirks.

* Sun Jun 05 2011 O.Sezer <sezero@users.sourceforge.net>
- Build the main game progs using the new hcc tool.

* Wed Jun 01 2011 O.Sezer <sezero@users.sourceforge.net>
- Update spec file after the xdelta3/h2patch changes.

* Fri May 20 2011 O.Sezer <sezero@users.sourceforge.net>
- Install fixed entities for the tower map to handle the map's quirks.

* Wed May 04 2011 O.Sezer <sezero@users.sourceforge.net>
- Install demo2 and egypt4 entity fixes for handling map quirks.

* Tue Mar 08 2011 O.Sezer <sezero@users.sourceforge.net>
- Fix license tag as GPLv2.
- Add missing missing pack map entity fixes to the packaged files list.

* Thu Mar 03 2011 O.Sezer <sezero@users.sourceforge.net>
- Nasm version 0.98 can not be supported anymore due to its inability to
  handle -I arguments. Bumped the minimum required version to 0.98.38.

* Sun Feb 27 2011 O.Sezer <sezero@users.sourceforge.net>
- Add support for building against libmpg123 instead of libmad.

* Tue Jan 04 2011 O.Sezer <sezero@users.sourceforge.net>
- Install tibet2/tibet9 ent fixes for handling map quirks.

* Wed Dec 29 2010 O.Sezer <sezero@users.sourceforge.net>
- Install ent fixes handling map quirks.

* Sun Dec 19 2010 O.Sezer <sezero@users.sourceforge.net>
- Added new build options after the music playback changes.
- Dropped SDL_mixer dependency which is not used anymore.
- Added README.music among the installed documents.
- Reworked the style we use for passing the build options to the makefiles.
- Fixed icon path.

* Fri Dec 17 2010 O.Sezer <sezero@users.sourceforge.net>
- Moved xdelta under the libs directory.

* Fri Apr 04 2008 O.Sezer <sezero@users.sourceforge.net> 1.4.3-1
- 1.4.3-final.

* Tue Feb 05 2008 O.Sezer <sezero@users.sourceforge.net>
- incremented the gamecode version number to 1.19a

* Wed Oct 03 2007 O.Sezer <sezero@users.sourceforge.net> 1.4.2-1
- 1.4.2-final.

* Mon Aug 13 2007 O.Sezer <sezero@users.sourceforge.net>
- removed the .gtk1 suffix from launcher gtk-1.2 builds

* Fri Jun 15 2007 O.Sezer <sezero@users.sourceforge.net>
- The software renderer clients can now be compiled on non-intel.

* Tue Apr 10 2007 O.Sezer <sezero@users.sourceforge.net>
- xdelta now builds without autotools.

* Tue Mar 20 2007 O.Sezer <sezero@users.sourceforge.net>
- xdelta version is 1.1.4: rename the binary properly.

* Sun Mar 18 2007 O.Sezer <sezero@users.sourceforge.net>
- gamecode version changed to 1.17.

* Mon Feb 05 2007 O.Sezer <sezero@users.sourceforge.net>
- xdelta is now included in the source tarball.

* Fri Dec 01 2006 O.Sezer <sezero@users.sourceforge.net> 1.4.1-2
- Version 1.4.1-rev1 :
  - Updated to gamedata-1.16a
  - Updated to xdelta-1.1.3b
  - Updated the URLs

* Wed Oct 18 2006 O.Sezer <sezero@users.sourceforge.net> 1.4.1-1
- Merged the hexen2 and mission pack packages.
- Added build option --without midi.
- Added build option --without alsa.
- Added build option --without asm.
- Disabled x86 assembly on non-intel cpus.
- Do not build or package the software renderer versions when not
  using x86 assembly until we fix them properly.
- Version 1.4.1-final.

* Mon Aug 14 2006 O.Sezer <sezero@users.sourceforge.net>
- Added the dedicated server to the packaged binaries.
  Preparing for a future 1.4.1 release.

* Tue Apr 18 2006 O.Sezer <sezero@users.sourceforge.net> 1.4.0-7
- More packaging tidy-ups for 1.4.0-final

* Sun Apr 16 2006 O.Sezer <sezero@users.sourceforge.net>
- Back to xdelta: removed loki_patch. All of its fancy bloat can
  be done in a shell script, which is more customizable.

* Tue Apr 04 2006 O.Sezer <sezero@users.sourceforge.net>
- Since 1.4.0-rc2 no mission pack specific binaries are needed.

* Sun Mar 26 2006 O.Sezer <sezero@users.sourceforge.net>
- Moved hexenworld related documentation to the hexenworld package
  lib3dfxgamma is no longer needed. not packaging it.

* Thu Mar 02 2006 O.Sezer <sezero@users.sourceforge.net>
- Added Features to the packaged documentation

* Wed Mar 01 2006 O.Sezer <sezero@users.sourceforge.net>
- Updated after the utilities reorganization

* Sun Feb 12 2006 O.Sezer <sezero@users.sourceforge.net>
- Updated for a future 1.4.0

* Mon Aug 29 2005 O.Sezer <sezero@users.sourceforge.net> 1.3.0-2
- Patch: We need to remove OS checks from the update_h2 script

* Sun Aug 21 2005 O.Sezer <sezero@users.sourceforge.net> 1.3.0-1
- First sketchy spec file for RedHat and Fedora Core

