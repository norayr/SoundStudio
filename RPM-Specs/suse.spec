%define version 1.0.3

Summary:	A Tcl/Tk sound file editor.
Name:		SoundStudio
Version:	%{version}
Release:	3
Group:		Applications/Multimedia
Copyright:	GPL
Source:		http://download.sourceforge.net/studio/SoundStudio-%{version}.tar.gz
URL:		http://sourceforge.net/projects/studio/
Vendor:		Nick Bailey <Nick@bailey-family.org.uk>
Packager:	Volker Kuhlmann <VolkerKuhlmann@gmx.de>
Distribution:	SuSE Linux 7.0 (i386) contrib
BuildRoot:	%{_tmppath}/%{name}-root
Requires:	tcl >= 7.4, tk >= 4.0, sox >= 11.4, /bin/more

%description
Sound Studio provides a graphical user interface for manipulating
digitized sound.  With Sound Studio, you can record, play and edit sound
files in various formats, including Microsoft's .wav, Sun's .au, and
Creative Lab's .voc formats. You can adjust recording levels and output
volumes using your sound card's built-in mixer.  Sound Studio also
provides you with information about the sample and allows conversion
into other formats, sampling rates, etc.

%prep
%setup -q

%build
export MYFILES=/usr/lib/${RPM_PACKAGE_NAME}
./build default

%install
rm -rf ${RPM_BUILD_ROOT}

#We don't need these for SuSE [VK]:
#mkdir -p ${RPM_BUILD_ROOT}/etc/X11/wmconfig
#mkdir -p ${RPM_BUILD_ROOT}/usr/share/icons/
#mkdir -p ${RPM_BUILD_ROOT}/usr/share/icons/mini
install -m755 -d ${RPM_BUILD_ROOT}/usr/lib/${RPM_PACKAGE_NAME}
install -m755 -d ${RPM_BUILD_ROOT}/usr/bin
install -m555 studio reset_dsp ${RPM_BUILD_ROOT}/usr/bin
install -m555 maxmin studio_mixer fader ${RPM_BUILD_ROOT}/usr/lib/${RPM_PACKAGE_NAME}
%ifarch i386
install -m555 studio_tool ${RPM_BUILD_ROOT}/usr/lib/${RPM_PACKAGE_NAME}
%endif
install -m444 *.tk *.ico ${RPM_BUILD_ROOT}/usr/lib/${RPM_PACKAGE_NAME}
install -m444 help1.hlp rubber.wav v12.au ${RPM_BUILD_ROOT}/usr/lib/${RPM_PACKAGE_NAME}
#We don't need these for SuSE [VK]:
#install -m644 -o root -g root studio-mini.xpm ${RPM_BUILD_ROOT}/usr/share/icons/mini/studio.xpm
#install -m644 -o root -g root studio.xpm ${RPM_BUILD_ROOT}/usr/share/icons/studio.xpm
install -m755 -d ${RPM_BUILD_ROOT}/etc/X11/applnk/Multimedia

cat > $RPM_BUILD_ROOT/etc/X11/applnk/Multimedia/SoundStudio.desktop <<EOF
[Desktop Entry]
Name=SoundStudio
Type=Application
Description=A Tcl/Tk sound file editor.
Exec=studio
EOF

#strip -s $RPM_BUILD_ROOT/usr/bin/* || :


%clean
rm -rf ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root)
%doc COPYING README studio.lsm DOCS/*
/usr/bin/studio
/usr/bin/reset_dsp
/usr/lib/SoundStudio
%config /etc/X11/applnk/Multimedia/SoundStudio.desktop
#We don't need these for SuSE [VK]:
#/usr/share/icons/studio.xpm
#/usr/share/icons/mini/studio.xpm

%changelog
* Mon Nov 20 2000 Nick Bailey <Nick@bailey-family.org.uk>
- Added my email to the "Author" field.

* Mon Oct 30 2000 Volker Kuhlmann <VolkerKuhlmann@gmx.de>
- For SuSE 7.0.

* Sun Feb 06 2000 Volker Kuhlmann <v.kuhlmann@elec.canterbury.ac.nz>
- For SuSE 6.3. Not adjusted (yet) for any window managers.

* Tue Oct 26 1999 Tim Powers <timp@redhat.com>
- using spec file downloaded from
  http://www.elec-eng.leeds.ac.uk/staff/een6njb/Software/Studio/screens.html
- strip binaries
- added %post and %postun sections
- added %ifarch in the %install section so that sparc and alpha would build
  (they don't have studio_tool)
