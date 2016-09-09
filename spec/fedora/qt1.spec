%define _qt1_prefix /opt/qt1
%define _qt1_libdir %{_qt1_prefix}/%{_lib}
%define _qt1_bindir %{_qt1_prefix}/bin
%define _qt1_includedir %{_qt1_prefix}/include

Name: qt1
Summary: Shared library for the Qt GUI toolkit
Version: 1.45
Release: 1%{?dist}
Source: http://download.qt.io/archive/qt/1/qt-%{version}.tar.gz
Patch0: qt-1.44-enablegif.patch
Patch1: qt-1.44-xor.patch
Patch2: qt-1.45-qfont.patch
Patch3: qt-1.45-fixsoname.patch
URL: http://www.qt.io
License: distributable
Group: System Environment/Libraries
BuildRequires: libXext-devel
BuildRequires: libxcb-devel
BuildRequires: libX11-devel

%package devel
Summary: Development files and documentation for the Qt GUI toolkit
Group: Development/Libraries

%description
Qt is a GUI software toolkit. Qt simplifies the task of writing and
maintaining GUI (graphical user interface) applications for X Windows.

Qt is written in C++ and is fully object-oriented. It has everything
you need to create professional GUI applications. And it enables you
to create them quickly.

Qt is a multi-platform toolkit. When developing software with Qt, you
can run it on the X Window System (Unix/X11) or Microsoft Windows NT
and Windows 95/98. Simply recompile your source code on the platform
you want.

This package contains the shared library needed to run Qt applications,
as well as the README files for Qt.

%description devel
Contains the files necessary to develop applications using Qt: header
files, the Qt meta object compiler, man pages, HTML documentation and
example programs.  See http://www.troll.no for more information about
Qt, or file:%{_qt1_libdir}/qt/html/index.html for Qt documentation in HTML.

%prep
%setup -q -n qt-%{version}
%patch0 -p1 -b .enablegif
%patch1 -p1 -b .xor
%patch2 -p1 -b .qfont
%patch3 -p1 -b .fixsoname
rm src/kernel/qt_gif.h.enablegif

%build
QTDIR=`/bin/pwd` export QTDIR
make linux-g++-shared
make

%install
mkdir -p %{buildroot}%{_qt1_bindir}
mkdir -p %{buildroot}%{_qt1_libdir}
mkdir -p %{buildroot}/usr/man
install -s -m 755 bin/moc %{buildroot}%{_qt1_bindir}/moc
cp lib/libqt.so.%{version} %{buildroot}%{_qt1_libdir}
ln -sf libqt.so.%{version} %{buildroot}%{_qt1_libdir}/libqt.so.1
ln -sf libqt.so.1 %{buildroot}%{_qt1_libdir}/libqt.so
mkdir -p %{buildroot}%{_qt1_libdir}/qt %{buildroot}%{_qt1_includedir}/qt
mkdir -p %{buildroot}%{_qt1_libdir}/qt/html %{buildroot}%{_qt1_libdir}/qt/tutorial
mkdir -p %{buildroot}%{_qt1_libdir}/qt/examples
cp -fR html %{buildroot}%{_qt1_libdir}/qt
strip tutorial/*/* || :
strip examples/*/* || :
cp -fR tutorial %{buildroot}%{_qt1_libdir}/qt
cp -fR examples %{buildroot}%{_qt1_libdir}/qt
cp -fR include/. %{buildroot}%{_qt1_includedir}/qt
for a in %{buildroot}%{_qt1_libdir}/qt/*/*/Makefile ; do
  sed 's-^SYSCONF_MOC.*-SYSCONF_MOC		= %{_qt1_bindir}/moc-' < $a > ${a}.2
  mv -v ${a}.2 $a
done
rm %{buildroot}%{_qt1_libdir}/qt/*/*/*.o
chmod -R a+r %{buildroot}%{_qt1_libdir}/libqt.so* %{buildroot}%{_qt1_libdir}/qt

%post -p /sbin/ldconfig

%clean
rm -rf %{buildroot}

%postun
/sbin/ldconfig

%files
%defattr(-,root,root,-)
%doc ANNOUNCE LICENSE README.QT FAQ PORTING README changes-1.40 changes-1.41 changes-1.42 changes-1.43 changes-1.44
%{_qt1_libdir}/libqt.so.%{version}
%{_qt1_libdir}/libqt.so.1
%{_qt1_libdir}/libqt.so

%files devel
%{_qt1_bindir}/moc
%{_qt1_libdir}/qt/
%{_qt1_includedir}/qt/

%changelog
* Sun Sep 04 2016 Helio Chissini de Castro <helio@kde.org> - 1.44-1
- Let's build the anniversary

* Sat Apr 17 1999 Preston Brown <pbrown@redhat.com>
- static library supplied in dev package.

* Wed Apr 07 1999 Preston Brown <pbrown@redhat.com>
- turn on internal GIF reading support

* Tue Apr 06 1999 Preston Brown <pbrown@redhat.com>
- strip binaries

* Mon Mar 15 1999 Preston Brown <pbrown@redhat.com>
- upgrade to qt 1.44.

* Wed Feb 24 1999 Preston Brown <pbrown@redhat.com>
- Injected new description and group.

* Tue Jan 19 1999 Preston Brown <pbrown@redhat.com>
- moved includes to include dir /qt

* Mon Jan 04 1999 Preston Brown <pbrown@redhat.com>
- made setup phase silent.

* Fri Dec 04 1998 Preston Brown <pbrown@redhat.com>
- upgraded to qt 1.42, released today.

* Tue Dec 01 1998 Preston Brown <pbrown@redhat.com>
- took Arnt's RPM and made some minor changes for Red Hat.
