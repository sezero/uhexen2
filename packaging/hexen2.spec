
%{?el2:%define _without_freedesktop 1}
%{?rh7:%define _without_freedesktop 1}

%{?el2:%define _without_gtk2 1}
%{?rh7:%define _without_gtk2 1}

%define desktop_vendor	uhexen2

Name:		hexen2
License:	GPL
Group:		Amusements/Games
URL:		http://uhexen2.sourceforge.net/
Version:	1.2.4a
Release:	1
Summary:	Hexen II
Source:		hexen2source-HoT-%{version}.tgz
Source1:	loki_patch-2005.tgz
Source2:	hexenworld-pakfiles-0.15.tgz
Source3:	gamedata-all-1.12h.tgz
BuildRoot:	%{_tmppath}/%{name}-%{version}-build
BuildRequires:	SDL-devel XFree86-devel XFree86-libs Mesa glut-devel
BuildRequires:	gtk+-devel libstdc++-devel
BuildRequires:	nasm >= 0.98
%{!?_without_freedesktop:BuildRequires: desktop-file-utils}
%{!?_without_gtk2:BuildRequires: gtk2-devel}
Requires:	hexen2-launcher

%description
Hexen II is a class based shooter game by Raven Software from 1997.
This is the Linux port of the GPL'ed source code released by Raven.
This package contains the binaries that will run the original Hexen2
game in software or OpenGL mode.

%package -n hexen2-missionpack
Group:		Amusements/Games
Summary:	Hexen II Mission Pack: Portal of Praevus
Requires:	hexen2 hexen2-launcher

%description -n hexen2-missionpack
Hexen II is a class based shooter game by Raven Software from 1997.
This is the Linux port of the GPL'ed source code released by Raven.
This package contains the binaries that will run the official Mission
Pack: Portal of Praevus in software or OpenGL mode.

%package -n hexenworld
Group:		Amusements/Games
Summary:	HexenWorld Client and Server
Requires:	hexen2 hexen2-launcher

%description -n hexenworld
Hexen II is a class based shooter game by Raven Software from 1997.
This is the Linux port of the GPL'ed source code released by Raven.
This package contains the binaries that are required to run a
HexenWorld server or run HexenWorld Client in software or OpenGL
mode.

%package -n hexen2-launcher
Group:		Amusements/Games
Summary:	Hexen II game launcher

%description -n hexen2-launcher
Hexen II is a class based shooter game by Raven Software from 1997.
This is the Linux port of the GPL'ed source code released by Raven.
This package contains the Hexen 2 Game Launcher that provides a gui
for launching different versions of the game.

%prep
%setup -q -n hexen2source-HoT-%{version} -a1 -a2 -a3
# remove the pre-compiled binary
rm -f loki_patch

%build
# Build the main game binaries
cd hexen2
make -f Makefile.packaging h2_dynamic
make clean
make -f Makefile.packaging mp_dynamic
make clean
# Mission Pack binaries
make -f Makefile.packaging glh2_dynamic
make clean
make -f Makefile.packaging glmp_dynamic
make clean
# HexenWorld binaries
cd ../hexenworld/Client
make -f Makefile.packaging hw_dynamic
make clean
make -f Makefile.packaging glhw_dynamic
make clean
cd ../Server
make
make clean
# Launcher binaries
cd ../../launcher
%if %{!?_without_gtk2:1}0
# Build for GTK2
make
%else
# Build for GTK1
make GTK1=yes
cp h2launcher.gtk1 h2launcher
%endif
make clean
cd ..
# Build game-update patcher loki_patch
cd loki_patch-2005
patch -p1 < xdelta-1.1.3-aclocal.patch
patch -p1 < xdelta-1.1.3-freegen.patch
patch -p1 < xdelta-1.1.3-gcc4.patch
patch -p1 < loki_setupdb-0.diff
patch -p1 < loki_patch-1.diff
patch -p1 < loki_patch-2.diff
cd xdelta-1.1.3
sh configure
make
cd ../loki_setupdb
sh autogen.sh
sh configure
make
cd ../loki_patch
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
%{__install} -D -m755 hexenworld/Client/hwcl %{buildroot}/%{_prefix}/games/%{name}/hwcl
%{__install} -D -m755 hexenworld/Client/glhwcl %{buildroot}/%{_prefix}/games/%{name}/glhwcl
%{__install} -D -m755 launcher/h2launcher %{buildroot}/%{_prefix}/games/%{name}/h2launcher
# Make a symlink of the game-launcher
%{__mkdir_p} %{buildroot}/%{_bindir}
ln -s %{_prefix}/games/hexen2/h2launcher %{buildroot}/%{_bindir}/hexen2

# Install the docs
%{__install} -D -m644 docs/README %{buildroot}/%{_prefix}/games/%{name}/docs/README
%{__install} -D -m644 docs/BUGS %{buildroot}/%{_prefix}/games/%{name}/docs/BUGS
%{__install} -D -m644 docs/ABOUT %{buildroot}/%{_prefix}/games/%{name}/docs/ABOUT
%{__install} -D -m644 docs/CHANGES %{buildroot}/%{_prefix}/games/%{name}/docs/CHANGES
%{__install} -D -m644 docs/README.launcher %{buildroot}/%{_prefix}/games/%{name}/docs/README.launcher

# Install the gamedata
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/data1/
%{__install} -D -m644 data1/progs.dat %{buildroot}/%{_prefix}/games/%{name}/data1/progs.dat
%{__install} -D -m644 data1/progs2.dat %{buildroot}/%{_prefix}/games/%{name}/data1/progs2.dat
%{__install} -D -m644 data1/hexen.rc %{buildroot}/%{_prefix}/games/%{name}/data1/hexen.rc
%{__install} -D -m644 data1/strings.txt %{buildroot}/%{_prefix}/games/%{name}/data1/strings.txt
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/portals/
%{__install} -D -m644 portals/progs.dat %{buildroot}/%{_prefix}/games/%{name}/portals/progs.dat
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/hw/
%{__install} -D -m644 hw/hwprogs.dat %{buildroot}/%{_prefix}/games/%{name}/hw/hwprogs.dat
%{__install} -D -m644 hw/pak4.pak %{buildroot}/%{_prefix}/games/%{name}/hw/pak4.pak
%{__install} -D -m644 hw/strings.txt %{buildroot}/%{_prefix}/games/%{name}/hw/strings.txt

# Install the Hexen2 and H2MP xdelta updates
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/patchdata/
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/patchdata/data1
%{__install} -D -m644 update_h2 %{buildroot}/%{_prefix}/games/%{name}/update_h2
%{__install} -D -m644 patchdata/data1/pak0.pak.103_111 %{buildroot}/%{_prefix}/games/%{name}/patchdata/data1/pak0.pak.103_111
%{__install} -D -m644 patchdata/data1/pak1.pak.103_111 %{buildroot}/%{_prefix}/games/%{name}/patchdata/data1/pak1.pak.103_111
%{__install} -D -m644 h2_103_111.dat %{buildroot}/%{_prefix}/games/%{name}/h2_103_111.dat

# Install the update-patcher binaries
%{__install} -D -m755 loki_patch-2005/loki_patch/loki_patch %{buildroot}/%{_prefix}/games/%{name}/loki_patch

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
%{_prefix}/games/%{name}/patchdata/data1/pak0.pak.103_111
%{_prefix}/games/%{name}/patchdata/data1/pak1.pak.103_111
%{_prefix}/games/%{name}/h2_103_111.dat
%{_prefix}/games/%{name}/data1/progs.dat
%{_prefix}/games/%{name}/data1/progs2.dat
%{_prefix}/games/%{name}/data1/hexen.rc
%{_prefix}/games/%{name}/data1/strings.txt

%files -n hexen2-missionpack
%defattr(-,root,root)
%{_prefix}/games/%{name}/h2mp
%{_prefix}/games/%{name}/glh2mp
%{_prefix}/games/%{name}/portals/progs.dat

%files -n hexenworld
%defattr(-,root,root)
%{_prefix}/games/%{name}/hwsv
%{_prefix}/games/%{name}/hwcl
%{_prefix}/games/%{name}/glhwcl
%{_prefix}/games/%{name}/hw/hwprogs.dat
%{_prefix}/games/%{name}/hw/pak4.pak
%{_prefix}/games/%{name}/hw/strings.txt

%files -n hexen2-launcher
%defattr(-,root,root)
%{_bindir}/hexen2
%{_datadir}/pixmaps/%{name}.png
%{_prefix}/games/%{name}/h2launcher
%{_prefix}/games/%{name}/docs/README
%{_prefix}/games/%{name}/docs/BUGS
%{_prefix}/games/%{name}/docs/ABOUT
%{_prefix}/games/%{name}/docs/CHANGES
%{_prefix}/games/%{name}/docs/README.launcher
%{!?_without_freedesktop:%{_datadir}/applications/%{desktop_vendor}-%{name}.desktop}
%{?_without_freedesktop:%{_sysconfdir}/X11/applnk/Games/%{name}.desktop}

%changelog
* Thu Feb 25 2005 O.Sezer <sezero@users.sourceforge.net> 1.2.4-1
- First sketchy spec file for RedHat and Fedora Core
