
%{?el2:%define _without_freedesktop 1}
%{?rh7:%define _without_freedesktop 1}

%{?el2:%define _without_gtk2 1}
%{?rh7:%define _without_gtk2 1}

%define desktop_vendor	uhexen2

Name:		hexen2
License:	GPL
Group:		Amusements/Games
URL:		http://uhexen2.sourceforge.net/
Version:	1.4.0
Release:	1
Summary:	Hexen II
Source:		hexen2source-%{version}.tgz
Source1:	loki_patch-src.tgz
Source2:	hexenworld-pakfiles-0.15.tgz
BuildRoot:	%{_tmppath}/%{name}-%{version}-build
BuildRequires:	nasm >= 0.98
BuildRequires:	SDL-devel >= 1.2.4
BuildRequires:	SDL_mixer-devel >= 1.2.4
BuildRequires:	XFree86-devel XFree86-libs
BuildRequires:	gtk+-devel libstdc++-devel
%{!?_without_freedesktop:BuildRequires: desktop-file-utils}
%{!?_without_gtk2:BuildRequires: gtk2-devel}
Requires:	SDL >= 1.2.4
Requires:	SDL_mixer >= 1.2.4

%description
Hexen II is a class based shooter game by Raven Software from 1997.
This is the Linux port of the GPL'ed source code released by Raven.
This package contains the binaries that will run the original Hexen2
game in software or OpenGL mode.
Also included here is the Hexen 2 Game Launcher that provides a gui
for launching different versions of the game.

%package -n hexen2-missionpack
Group:		Amusements/Games
Summary:	Hexen II Mission Pack: Portal of Praevus
Requires:	hexen2

%description -n hexen2-missionpack
Hexen II is a class based shooter game by Raven Software from 1997.
This is the Linux port of the GPL'ed source code released by Raven.
This package contains the binaries that will run the official Mission
Pack: Portal of Praevus in software or OpenGL mode.

%package -n hexenworld
Group:		Amusements/Games
Summary:	HexenWorld Client and Server
Requires:	SDL >= 1.2.4
Requires:	SDL_mixer >= 1.2.4
Requires:	hexen2

%description -n hexenworld
Hexen II is a class based shooter game by Raven Software from 1997.
This is the Linux port of the GPL'ed source code released by Raven.
This package contains the binaries that are required to run a
HexenWorld server or run HexenWorld Client in software or OpenGL
mode.

%prep
%setup -q -n hexen2source-%{version} -a1 -a2
patch -p1 < 00_Patches/update_script_no_os_check.patch

%build
# Build the main game binaries
make -C hexen2 h2
make -C hexen2 clean
make -C hexen2 glh2
make -C hexen2 clean
# Mission Pack binaries
make -C hexen2 mp
make -C hexen2 clean
make -C hexen2 glmp
make -C hexen2 clean
# HexenWorld binaries
make -C hexenworld/Server
make -C hexenworld/Master
make -C hexenworld/Client hw
make -C hexenworld/Client clean
make -C hexenworld/Client glhw
# Launcher binaries
%if %{!?_without_gtk2:1}0
# Build for GTK2
make -C launcher
%else
# Build for GTK1.2
make -C launcher GTK1=yes
cp launcher/h2launcher.gtk1 launcher/h2launcher
%endif
# Build the hcode compilers
make -C utils/hcc_old
make -C utils/hcc
# Build the game-code
utils/hcc_old/hcc -src gamecode/hc/h2
utils/hcc_old/hcc -src gamecode/hc/h2 -name progs2.src
utils/bin/hcc -src gamecode/hc/portals -oi -on
utils/bin/hcc -src gamecode/hc/hw -oi -on
#utils/bin/hcc -src gamecode/hc/siege -oi -on

# Build game-update patcher loki_patch
cd loki_patch-src
cd loki_setupdb
sh autogen.sh
sh configure
make
cd ../loki_patch
tar xvfz libs/xdelta-1.1.3-patched.tar.gz
cd xdelta-1.1.3
sh configure
make
cd ..
sh autogen.sh
sh configure
make
cd ../..
# Done building

%install
%{__rm} -rf %{buildroot}
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/docs
%{__install} -D -m755 hexen2/hexen2 %{buildroot}/%{_prefix}/games/%{name}/hexen2
%{__install} -D -m755 hexen2/h2mp %{buildroot}/%{_prefix}/games/%{name}/h2mp
%{__install} -D -m755 hexen2/glhexen2 %{buildroot}/%{_prefix}/games/%{name}/glhexen2
%{__install} -D -m755 hexen2/glh2mp %{buildroot}/%{_prefix}/games/%{name}/glh2mp
%{__install} -D -m755 hexenworld/Server/hwsv %{buildroot}/%{_prefix}/games/%{name}/hwsv
%{__install} -D -m755 hexenworld/Master/hwmaster %{buildroot}/%{_prefix}/games/%{name}/hwmaster
%{__install} -D -m755 hexenworld/Client/hwcl %{buildroot}/%{_prefix}/games/%{name}/hwcl
%{__install} -D -m755 hexenworld/Client/glhwcl %{buildroot}/%{_prefix}/games/%{name}/glhwcl
%{__install} -D -m755 launcher/h2launcher %{buildroot}/%{_prefix}/games/%{name}/h2launcher
# Make a symlink of the game-launcher
%{__mkdir_p} %{buildroot}/%{_bindir}
ln -s %{_prefix}/games/hexen2/h2launcher %{buildroot}/%{_bindir}/hexen2

# Install the docs
%{__install} -D -m644 docs/README %{buildroot}/%{_prefix}/games/%{name}/docs/README
%{__install} -D -m644 docs/COPYING %{buildroot}/%{_prefix}/games/%{name}/docs/COPYING
%{__install} -D -m644 docs/BUGS %{buildroot}/%{_prefix}/games/%{name}/docs/BUGS
%{__install} -D -m644 docs/TODO %{buildroot}/%{_prefix}/games/%{name}/docs/TODO
%{__install} -D -m644 docs/ABOUT %{buildroot}/%{_prefix}/games/%{name}/docs/ABOUT
%{__install} -D -m644 docs/Features %{buildroot}/%{_prefix}/games/%{name}/docs/Features
%{__install} -D -m644 docs/CHANGES %{buildroot}/%{_prefix}/games/%{name}/docs/CHANGES
%{__install} -D -m644 docs/README.3dfx %{buildroot}/%{_prefix}/games/%{name}/docs/README.3dfx
%{__install} -D -m644 docs/README.launcher %{buildroot}/%{_prefix}/games/%{name}/docs/README.launcher
%{__install} -D -m644 docs/README.hwcl %{buildroot}/%{_prefix}/games/%{name}/docs/README.hwcl
%{__install} -D -m644 docs/README.hwsv %{buildroot}/%{_prefix}/games/%{name}/docs/README.hwsv
%{__install} -D -m644 docs/README.hwmaster %{buildroot}/%{_prefix}/games/%{name}/docs/README.hwmaster
%{__install} -D -m644 docs/ReleaseNotes-1.2.3 %{buildroot}/%{_prefix}/games/%{name}/docs/ReleaseNotes-1.2.3
%{__install} -D -m644 docs/ReleaseNotes-1.2.4a %{buildroot}/%{_prefix}/games/%{name}/docs/ReleaseNotes-1.2.4a
%{__install} -D -m644 docs/ReleaseNotes-1.3.0 %{buildroot}/%{_prefix}/games/%{name}/docs/ReleaseNotes-1.3.0
%{__install} -D -m644 docs/ReleaseNotes-%{version} %{buildroot}/%{_prefix}/games/%{name}/docs/ReleaseNotes-%{version}

# Install the gamedata
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/data1/
%{__install} -D -m644 gamecode/hc/h2/progs.dat %{buildroot}/%{_prefix}/games/%{name}/data1/progs.dat
%{__install} -D -m644 gamecode/hc/h2/progs2.dat %{buildroot}/%{_prefix}/games/%{name}/data1/progs2.dat
%{__install} -D -m644 gamecode/txt/h2/hexen.rc %{buildroot}/%{_prefix}/games/%{name}/data1/hexen.rc
%{__install} -D -m644 gamecode/txt/h2/strings.txt %{buildroot}/%{_prefix}/games/%{name}/data1/strings.txt
%{__install} -D -m644 gamecode/txt/h2/default.cfg %{buildroot}/%{_prefix}/games/%{name}/data1/default.cfg
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/portals/
%{__install} -D -m644 gamecode/hc/portals/progs.dat %{buildroot}/%{_prefix}/games/%{name}/portals/progs.dat
%{__install} -D -m644 gamecode/txt/portals/hexen.rc %{buildroot}/%{_prefix}/games/%{name}/portals/hexen.rc
%{__install} -D -m644 gamecode/txt/portals/strings.txt %{buildroot}/%{_prefix}/games/%{name}/portals/strings.txt
%{__install} -D -m644 gamecode/txt/portals/infolist.txt %{buildroot}/%{_prefix}/games/%{name}/portals/infolist.txt
%{__install} -D -m644 gamecode/txt/portals/puzzles.txt %{buildroot}/%{_prefix}/games/%{name}/portals/puzzles.txt
%{__install} -D -m644 gamecode/txt/portals/default.cfg %{buildroot}/%{_prefix}/games/%{name}/portals/default.cfg
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/hw/
%{__install} -D -m644 gamecode/hc/hw/hwprogs.dat %{buildroot}/%{_prefix}/games/%{name}/hw/hwprogs.dat
%{__install} -D -m644 gamecode/txt/hw/strings.txt %{buildroot}/%{_prefix}/games/%{name}/hw/strings.txt
%{__install} -D -m644 gamecode/txt/hw/default.cfg %{buildroot}/%{_prefix}/games/%{name}/hw/default.cfg
%{__install} -D -m644 hw/pak4.pak %{buildroot}/%{_prefix}/games/%{name}/hw/pak4.pak

# Install the Hexen2 and H2MP xdelta updates
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/patchdata/
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/patchdata/data1
%{__install} -D -m755 gamecode/pak_v111/update_h2 %{buildroot}/%{_prefix}/games/%{name}/update_h2
%{__install} -D -m644 gamecode/pak_v111/patchdata/data1/data1pak0.xd %{buildroot}/%{_prefix}/games/%{name}/patchdata/data1/data1pak0.xd
%{__install} -D -m644 gamecode/pak_v111/patchdata/data1/data1pak1.xd %{buildroot}/%{_prefix}/games/%{name}/patchdata/data1/data1pak1.xd
%{__install} -D -m644 gamecode/pak_v111/h2_103_111.dat %{buildroot}/%{_prefix}/games/%{name}/h2_103_111.dat

# Install the update-patcher binaries
%{__install} -D -m755 loki_patch-src/loki_patch/loki_patch %{buildroot}/%{_prefix}/games/%{name}/loki_patch

# Install the menu icon
%{__mkdir_p} %{buildroot}/%{_datadir}/pixmaps
%{__install} -D -m644 hexen2/icons/h2_32x32x4.png %{buildroot}/%{_datadir}/pixmaps/%{name}.png

# Install menu entry
%{__cat} > %{name}.desktop << EOF
[Desktop Entry]
Name=Hexen 2
Comment=Hexen II
Exec=hexen2
Icon=hexen2.png
Terminal=false
Type=Application
Encoding=UTF-8
Categories=Application;Game;
EOF

%if %{!?_without_freedesktop:1}0
%{__mkdir_p} %{buildroot}%{_datadir}/applications
desktop-file-install \
	--vendor %{desktop_vendor} \
	--dir %{buildroot}%{_datadir}/applications \
	%{name}.desktop
%else
%{__install} -D -m 0644 %{name}.desktop \
	%{buildroot}%{_sysconfdir}/X11/applnk/Games/%{name}.desktop
%endif

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_prefix}/games/%{name}/hexen2
%{_prefix}/games/%{name}/glhexen2
%{_prefix}/games/%{name}/loki_patch
%{_prefix}/games/%{name}/update_h2
%{_prefix}/games/%{name}/patchdata/data1/data1pak0.xd
%{_prefix}/games/%{name}/patchdata/data1/data1pak1.xd
%{_prefix}/games/%{name}/h2_103_111.dat
%{_prefix}/games/%{name}/data1/progs.dat
%{_prefix}/games/%{name}/data1/progs2.dat
%{_prefix}/games/%{name}/data1/hexen.rc
%{_prefix}/games/%{name}/data1/strings.txt
%{_prefix}/games/%{name}/data1/default.cfg
%{_bindir}/hexen2
%{_datadir}/pixmaps/%{name}.png
%{_prefix}/games/%{name}/h2launcher
%{_prefix}/games/%{name}/docs/README
%{_prefix}/games/%{name}/docs/COPYING
%{_prefix}/games/%{name}/docs/BUGS
%{_prefix}/games/%{name}/docs/ABOUT
%{_prefix}/games/%{name}/docs/Features
%{_prefix}/games/%{name}/docs/CHANGES
%{_prefix}/games/%{name}/docs/README.launcher
%{_prefix}/games/%{name}/docs/README.3dfx
%{_prefix}/games/%{name}/docs/TODO
%{_prefix}/games/%{name}/docs/ReleaseNotes-1.2.3
%{_prefix}/games/%{name}/docs/ReleaseNotes-1.2.4a
%{_prefix}/games/%{name}/docs/ReleaseNotes-1.3.0
%{_prefix}/games/%{name}/docs/ReleaseNotes-%{version}
%{!?_without_freedesktop:%{_datadir}/applications/%{desktop_vendor}-%{name}.desktop}
%{?_without_freedesktop:%{_sysconfdir}/X11/applnk/Games/%{name}.desktop}

%files -n hexen2-missionpack
%defattr(-,root,root)
%{_prefix}/games/%{name}/h2mp
%{_prefix}/games/%{name}/glh2mp
%{_prefix}/games/%{name}/portals/progs.dat
%{_prefix}/games/%{name}/portals/hexen.rc
%{_prefix}/games/%{name}/portals/strings.txt
%{_prefix}/games/%{name}/portals/puzzles.txt
%{_prefix}/games/%{name}/portals/infolist.txt
%{_prefix}/games/%{name}/portals/default.cfg

%files -n hexenworld
%defattr(-,root,root)
%{_prefix}/games/%{name}/hwsv
%{_prefix}/games/%{name}/hwmaster
%{_prefix}/games/%{name}/hwcl
%{_prefix}/games/%{name}/glhwcl
%{_prefix}/games/%{name}/hw/hwprogs.dat
%{_prefix}/games/%{name}/hw/pak4.pak
%{_prefix}/games/%{name}/hw/strings.txt
%{_prefix}/games/%{name}/hw/default.cfg
%{_prefix}/games/%{name}/docs/README.hwcl
%{_prefix}/games/%{name}/docs/README.hwsv
%{_prefix}/games/%{name}/docs/README.hwmaster

%changelog
* Mon Mar 26 2006 O.Sezer <sezero@users.sourceforge.net> 1.4.0-4
- Moved hexenworld related documentation to the hexenworld package
  lib3dfxgamma is no longer needed. not packaging it.

* Thu Mar 02 2006 O.Sezer <sezero@users.sourceforge.net> 1.4.0-3
- Added Features to the packaged documentation

* Wed Mar 01 2006 O.Sezer <sezero@users.sourceforge.net> 1.4.0-2
- Updated after the utilities reorganization

* Sun Feb 12 2006 O.Sezer <sezero@users.sourceforge.net> 1.4.0-1
- Updated for 1.4.0

* Thu Aug 29 2005 O.Sezer <sezero@users.sourceforge.net> 1.3.0-2
- Patch: We need to remove OS checks from the update_h2 script

* Thu Aug 21 2005 O.Sezer <sezero@users.sourceforge.net> 1.3.0-1
- First sketchy spec file for RedHat and Fedora Core

