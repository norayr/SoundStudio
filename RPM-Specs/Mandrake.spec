%define name SoundStudio
%define version 1.0.5
%define release 0mdk_1kc

%ifos solaris2.5.1
    %define myprefix /login/kevinc
    %define menudir %{myprefix}/lib/menu
    %define etcdir %{myprefix}/etc
    %define compext gz
    %define compress gzip
%else
    %define myprefix /usr/local
    %define menudir /usr/lib/menu
    %define etcdir /etc
    %define compext bz2
    %define compress bzip2
%endif

Summary: A Tcl/Tk sound file editor.
Name: %{name}
Version: %{version}
Release: %{release}
Source0: http://download.sourceforge.net/studio/%{name}-%{version}.tar.gz
Copyright: GPL
# This is for Redhat   -> Group: Applications/Multimedia
# This is for Mandrake -> Group: Sound
Group: Sound
URL: http://sourceforge.net/projects/studio/
Requires: tcl >= 7.4, tk >= 4.0, sox >= 11.4, /bin/more

BuildRoot: %{_topdir}/%{name}-buildroot
Prefix: %{myprefix}

%description
Sound Studio provides a graphical user interface for manipulating
digitized sound.  With Sound Studio, you can record, play and edit sound
files in various formats, including Microsoft's .wav, Sun's .au, and
Creative Lab's .voc formats. You can adjust recording levels and output
volumes using your sound card's built-in mixer.  Sound Studio also
provides you with information about the sample and allows conversion
into other formats, sampling rates, etc.

%prep
# %setup -q -n %{name}-%{version}
%setup -q -n %{name}

%build
export MYFILES=%{prefix}/lib/%{name}
export SOX_DIR=%{prefix}/bin
./build default

%install
[ "${RPM_BUILD_ROOT}" != "/" ] && rm -rf ${RPM_BUILD_ROOT}/

mkdir -p ${RPM_BUILD_ROOT}%{etcdir}/X11/wmconfig
mkdir -p ${RPM_BUILD_ROOT}%{prefix}/share/icons/mini
install -m755 -d ${RPM_BUILD_ROOT}%{prefix}/lib/%{name}
install -m755 -d ${RPM_BUILD_ROOT}%{prefix}/bin

cat studio \
 | sed 's@set MYFILES /usr/lib/SoundStudio@set MYFILES %{prefix}/lib/%{name}@' \
 | sed 's@set TEMP_DIR /tmp@set TEMP_DIR /tmp/%{name}@' \
 | sed 's@set SOX_DIR /usr/bin@set SOX_DIR %{prefix}/bin@' \
    > ${RPM_BUILD_ROOT}%{prefix}/bin/studio
chmod 755 ${RPM_BUILD_ROOT}%{prefix}/bin/studio
#| sed 's@set SOX_DIR /usr/bin@set SOX_DIR %{prefix}/lib/SoundStudio/sox/bin@' \

install -m555 reset_dsp ${RPM_BUILD_ROOT}%{prefix}/bin
install -m555 maxmin studio_mixer fader \
	${RPM_BUILD_ROOT}%{prefix}/lib/%{name}
%ifos Linux
install -m555 studio_tool \
	${RPM_BUILD_ROOT}%{prefix}/lib/%{name}
%endif

install -m444 *.tk *.ico ${RPM_BUILD_ROOT}%{prefix}/lib/%{name}
install -m444 help1.hlp v12.au \
	${RPM_BUILD_ROOT}%{prefix}/lib/%{name}
install -m644 SoundStudio.wmconfig \
	${RPM_BUILD_ROOT}%{etcdir}/X11/wmconfig/SoundStudio
install -m644 studio-mini.xpm \
	${RPM_BUILD_ROOT}%{prefix}/share/icons/mini/studio.xpm
install -m644 studio.xpm \
	${RPM_BUILD_ROOT}%{prefix}/share/icons/studio.xpm

strip -s ${RPM_BUILD_ROOT}%{prefix}/bin/* || :

%ifos Linux
# Mandrake specific menu entry
mkdir -p ${RPM_BUILD_ROOT}%{prefix}/lib/menu
cat << EOF > ${RPM_BUILD_ROOT}%{prefix}/lib/menu/%{name}
?package(%{name}):\
needs="X11" \
section="Multimedia/Sound" \
title="SoundStudio" \
command="%{prefix}/bin/studio" \
icon="studio.xpm" \
longtitle="General purpose sound file manipulation tool"
EOF
mkdir -p ${RPM_BUILD_ROOT}%{menudir}
ln -s %{prefix}/lib/menu/%{name} ${RPM_BUILD_ROOT}%{menudir}
%endif

find ${RPM_BUILD_ROOT}%{prefix} -type f -print \
    | sed "s@^${RPM_BUILD_ROOT}@@g" \
    > %{name}-%{version}-filelist

%post
wmconfig --output fvwm2 >> /dev/null 2> /dev/null

%postun
wmconfig --output fvwm2 >> /dev/null 2> /dev/null

%clean
[ "${RPM_BUILD_ROOT}" != "/" ] && rm -rf ${RPM_BUILD_ROOT}/

# %files -f %{name}-%{version}-filelist
%files
%defattr(-,root,root,755)
%doc COPYING README studio.lsm DOCS/*
%attr(644,root,root) %{prefix}/lib/menu/%{name}
%{menudir}/%{name}
%{etcdir}/X11/wmconfig/SoundStudio
%{prefix}/bin/reset_dsp
%{prefix}/bin/studio
%{prefix}/lib/SoundStudio
%{prefix}/share/icons/mini/studio.xpm
%{prefix}/share/icons/studio.xpm

%changelog
* Mon Nov 27 2000 Kevin Cosgrove <kevinc@dOink.COM>
- Updated to 1.0.5.

* Wed Nov 15 2000 Kevin Cosgrove <kevinc@dOink.COM>
- Patched file.tk to work with sox 12.17 official release.
- Updated URLs to specify SourceForge home.

* Thu Jul 13 2000 Kevin Cosgrove <kevinc@dOink.COM>
- studio_tool now installs on Linux, not just i386

* Thu May 18 2000 Kevin Cosgrove <kevinc@dOink.COM>
- Patched file.tk to work with sox 12.17-pre2

* Wed May 10 2000 Kevin Cosgrove <kevinc@dOink.COM>
- Now requires special version of sox 12.16 installed in
  %{prefix}/lib/SoundStudio/sox.  Version 12.17-pre2 breaks
  SoundStudio but has lots of good new filters and a compander.
 
* Fri Apr 21 2000 Kevin Cosgrove <kevinc@dOink.COM>
- No longer requires root privilege to build package.
 
* Tue Oct 26 1999 Tim Powers <timp@redhat.com>
- using spec file downloaded from
  http://www.elec-eng.leeds.ac.uk/staff/een6njb/Software/Studio/screens.html
- strip binaries
- added %post and %postun sections
- added %ifarch in the %install section so that sparc and alpha would build
  (they don't have studio_tool)

