
%{?el2:%define _without_freedesktop 1}
%{?rh7:%define _without_freedesktop 1}

%{?el2:%define _without_gtk2 1}
%{?rh7:%define _without_gtk2 1}

%define desktop_vendor	uhexen2

Name:		hexen2
License:	GPL
Group:		Amusements/Games
URL:		http://uhexen2.sourceforge.net/
Version:	1.2.3
Release:	1
Summary:	Hexen II
Source:		HoT-%{version}.src.tgz
Source1:	loki_patch-2004.tar.gz
Source2:	xdelta-1.1.3.tar.gz
Source3:	hexenworld-data-0.15.tgz
Source4:	game-updates-1.12d.tgz
Source5:	h2-icons.tar.gz
Source6:	game-updates-1.12d.readme
Patch1:		xdelta-1.1.3-freegen.patch
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

%package -n hexen2-mp
Group:		Amusements/Games
Summary:	Hexen II Mission Pack: Portal of Praevus
Requires:	hexen2-launcher

%description -n hexen2-mp
Hexen II is a class based shooter game by Raven Software from 1997.
This is the Linux port of the GPL'ed source code released by Raven.
This package contains the binaries that will run the official Mission
Pack: Portal of Praevus in software or OpenGL mode.

%package -n hexenworld
Group:		Amusements/Games
Summary:	HexenWorld Client and Server
Requires:	hexen2-launcher

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

%package -n hexen2-data
Group:		Amusements/Games
Summary:	Hexen II game-data updates

%description -n hexen2-data
Hexen II is a class based shooter game by Raven Software from 1997.
This package contains version 1.12d updates to the game data.

%package -n hexenworld-data
Group:		Amusements/Games
Summary:	HexenWorld game-data

%description -n hexenworld-data
Hexen II is a class based shooter game by Raven Software from 1997.
This package contains version 0.15 of HexenWorld game-data.

%prep
%setup -q -n HoT-%{version} -a1 -a2 -a3 -a4 -a5
%patch1 -p0
cp %{S:6} .
# remove the pre-compiled binary
rm -f loki_patch
# prepare loki_patch
cd loki_patch-2004
tar xvfz loki_setupdb-20041226.tar.gz
tar xvfz loki_patch-20041226.tar.gz
patch -p1 < loki_setupdb-0.diff
patch -p0 < loki_patch-1.diff
cd ..

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
make GTK2=yes
cp h2launcher.gtk2 h2launcher
%else
# Build for GTK1
make
cp h2launcher.gtk1 h2launcher
%endif
make clean
cd ..
# Build game-update patcher loki_patch
cd loki_patch-2004/loki_setupdb
sh autogen.sh
sh configure
make
cd ../loki_patch
sh autogen.sh
sh configure
make
cd ../..
# Build game-update patcher xdelta113
cd xdelta-1.1.3
%configure  --disable-shared --enable-static
libtoolize --force
make %{?_smp_mflags} CFLAGS="$RPM_OPT_FLAGS"
# Done building
cd ..

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

# Install the HexenWorld beta-0.15 data
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/hw/
%{__install} -D -m644 hw/hwprogs.dat %{buildroot}/%{_prefix}/games/%{name}/hw/hwprogs.dat
%{__install} -D -m644 hw/pak4.pak %{buildroot}/%{_prefix}/games/%{name}/hw/pak4.pak
%{__install} -D -m644 hw/strings.txt %{buildroot}/%{_prefix}/games/%{name}/hw/strings.txt

# Install the Hexen2 and H2MP xdelta updates
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/patchdata/
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/patchdata/data1
%{__mkdir_p} %{buildroot}/%{_prefix}/games/%{name}/patchdata/portals
%{__install} -D -m644 game-updates-1.12d.readme %{buildroot}/%{_prefix}/games/%{name}/game-updates-1.12d.readme
%{__install} -D -m644 update112d %{buildroot}/%{_prefix}/games/%{name}/update112d
%{__install} -D -m644 patchdata/data1/pak0.pak.103_111 %{buildroot}/%{_prefix}/games/%{name}/patchdata/data1/pak0.pak.103_111
%{__install} -D -m644 patchdata/data1/pak1.pak.103_111 %{buildroot}/%{_prefix}/games/%{name}/patchdata/data1/pak1.pak.103_111
%{__install} -D -m644 patchdata/data1/progs.dat.103_111 %{buildroot}/%{_prefix}/games/%{name}/patchdata/data1/progs.dat.103_111
%{__install} -D -m644 patchdata/data1/progs2.dat.103_111 %{buildroot}/%{_prefix}/games/%{name}/patchdata/data1/progs2.dat.103_111
%{__install} -D -m644 patchdata/data1/progs.dat.111_112b %{buildroot}/%{_prefix}/games/%{name}/patchdata/data1/progs.dat.111_112b
%{__install} -D -m644 patchdata/data1/progs2.dat.111_112b %{buildroot}/%{_prefix}/games/%{name}/patchdata/data1/progs2.dat.111_112b
%{__install} -D -m644 patchdata/data1/progs2.dat.112b_112c %{buildroot}/%{_prefix}/games/%{name}/patchdata/data1/progs2.dat.112b_112c
%{__install} -D -m644 patchdata/data1/progs.dat.112c_112d %{buildroot}/%{_prefix}/games/%{name}/patchdata/data1/progs.dat.112c_112d
%{__install} -D -m644 patchdata/data1/progs2.dat.112c_112d %{buildroot}/%{_prefix}/games/%{name}/patchdata/data1/progs2.dat.112c_112d
%{__install} -D -m644 patchdata/portals/progs.dat.112_112a %{buildroot}/%{_prefix}/games/%{name}/patchdata/portals/progs.dat.112_112a
%{__install} -D -m644 patchdata/portals/progs.dat.112a_112d %{buildroot}/%{_prefix}/games/%{name}/patchdata/portals/progs.dat.112a_112d
%{__install} -D -m644 h2_103_111.dat %{buildroot}/%{_prefix}/games/%{name}/h2_103_111.dat
%{__install} -D -m644 h2_111_112b.dat %{buildroot}/%{_prefix}/games/%{name}/h2_111_112b.dat
%{__install} -D -m644 h2_112b_112c.dat %{buildroot}/%{_prefix}/games/%{name}/h2_112b_112c.dat
%{__install} -D -m644 h2_112c_112d.dat %{buildroot}/%{_prefix}/games/%{name}/h2_112c_112d.dat
%{__install} -D -m644 mp_112_112a.dat %{buildroot}/%{_prefix}/games/%{name}/mp_112_112a.dat
%{__install} -D -m644 mp_112a_112d.dat %{buildroot}/%{_prefix}/games/%{name}/mp_112a_112d.dat

# Install the update-patcher binaries
%{__install} -D -m755 loki_patch-2004/loki_patch/loki_patch %{buildroot}/%{_prefix}/games/%{name}/loki_patch
%{__install} -D -m755 xdelta-1.1.3/xdelta %{buildroot}/%{_prefix}/games/%{name}/xdelta113

# Install the menu icon
%{__mkdir_p} %{buildroot}/%{_datadir}/pixmaps
%{__install} -D -m644 h2icons/h2_32x32x4.png %{buildroot}/%{_datadir}/pixmaps/%{name}.png

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

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%defattr(-,root,root)
%{_prefix}/games/%{name}/hexen2
%{_prefix}/games/%{name}/glhexen2

%files -n hexen2-mp
%defattr(-,root,root)
%{_prefix}/games/%{name}/h2mp
%{_prefix}/games/%{name}/glh2mp

%files -n hexenworld
%defattr(-,root,root)
%{_prefix}/games/%{name}/hwsv
%{_prefix}/games/%{name}/hwcl
%{_prefix}/games/%{name}/glhwcl

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

%files -n hexenworld-data
%defattr(-,root,root)
%{_prefix}/games/%{name}/hw/hwprogs.dat
%{_prefix}/games/%{name}/hw/pak4.pak
%{_prefix}/games/%{name}/hw/strings.txt

%files -n hexen2-data
%defattr(-,root,root)
%{_prefix}/games/%{name}/loki_patch
%{_prefix}/games/%{name}/xdelta113
%{_prefix}/games/%{name}/game-updates-1.12d.readme
%{_prefix}/games/%{name}/update112d
%{_prefix}/games/%{name}/patchdata/data1/pak0.pak.103_111
%{_prefix}/games/%{name}/patchdata/data1/pak1.pak.103_111
%{_prefix}/games/%{name}/patchdata/data1/progs.dat.103_111
%{_prefix}/games/%{name}/patchdata/data1/progs2.dat.103_111
%{_prefix}/games/%{name}/patchdata/data1/progs.dat.111_112b
%{_prefix}/games/%{name}/patchdata/data1/progs2.dat.111_112b
%{_prefix}/games/%{name}/patchdata/data1/progs2.dat.112b_112c
%{_prefix}/games/%{name}/patchdata/data1/progs.dat.112c_112d
%{_prefix}/games/%{name}/patchdata/data1/progs2.dat.112c_112d
%{_prefix}/games/%{name}/patchdata/portals/progs.dat.112_112a
%{_prefix}/games/%{name}/patchdata/portals/progs.dat.112a_112d
%{_prefix}/games/%{name}/h2_103_111.dat
%{_prefix}/games/%{name}/h2_111_112b.dat
%{_prefix}/games/%{name}/h2_112b_112c.dat
%{_prefix}/games/%{name}/h2_112c_112d.dat
%{_prefix}/games/%{name}/mp_112_112a.dat
%{_prefix}/games/%{name}/mp_112a_112d.dat

%changelog
* Thu Jan 6 2005 O.Sezer <sezero@users.sourceforge.net> 1.2.3-1
- First sketchy spec file for RedHat and Fedora Core

