# -*- makefile -*-
#
# Main Makefile for building the Qt library, examples and tutorial.
# Read PORTING for instructions how to port Qt to a new platform.


all: moc src tutorial examples
	@echo
	@echo "The Qt library is now built in ./lib"
	@echo "The Qt examples are built in the directories in ./examples"
	@echo "The Qt tutorials are built in the directories in ./tutorial"
	@echo
	@echo "Enjoy!   - the Troll Tech team"
	@echo

moc: variables FORCE
	cd src/moc; $(MAKE)
	cp src/moc/moc bin/moc

src: moc variables FORCE
	cd $@; $(MAKE)

tutorial examples: src FORCE
	cd $@; $(MAKE)

clean:
	-rm variables
	cd src/moc; $(MAKE) clean
	cd src; $(MAKE) clean
	cd tutorial; $(MAKE) clean
	cd examples; $(MAKE) clean

depend:
	cd src; $(MAKE) depend
	cd tutorial; $(MAKE) depend
	cd examples; $(MAKE) depend

config: variables
	-rm variables
	$(MAKE)

variables: Makefile
	@echo
	@echo These targets are available:
	@echo
	@ls configs
	@echo
	@echo Make any of them to configure Qt for building.
	@echo
	@echo 'Please note that the GIF image reading functionality in Qt is disabled'
	@echo 'by default. If you want to enable it, see the file src/kernel/qt_gif.h'
	@echo
	@echo 'To configure Qt for building, select a target from the list above that'
	@echo 'matches your system and desired configuration, and give the command'
	@echo '   make <your-selected-target>'
	@echo
	@echo 'This make process will now abort with an error message; this is normal'
	@echo 'Simply proceed as described.'
	@echo
	@exit 1


# individual targets

%: configs/%
	./propagate configs/$*

aix-g++-static: Makefile
	./propagate configs/aix-g++-static

aix-g++-static-debug: Makefile
	./propagate configs/aix-g++-static-debug

aix-xlc-shared: Makefile
	./propagate configs/aix-xlc-shared

aix-xlc-shared-debug: Makefile
	./propagate configs/aix-xlc-shared-debug

aix-xlc-static: Makefile
	./propagate configs/aix-xlc-static

aix-xlc-static-debug: Makefile
	./propagate configs/aix-xlc-static-debug

bsdi-g++-shared: Makefile
	./propagate configs/bsdi-g++-shared

bsdi-g++-shared-debug: Makefile
	./propagate configs/bsdi-g++-shared-debug

bsdi-g++-static: Makefile
	./propagate configs/bsdi-g++-static

bsdi-g++-static-debug: Makefile
	./propagate configs/bsdi-g++-static-debug

dgux-g++-shared: Makefile
	./propagate configs/dgux-g++-shared

dgux-g++-shared-debug: Makefile
	./propagate configs/dgux-g++-shared-debug

dgux-g++-static: Makefile
	./propagate configs/dgux-g++-static

dgux-g++-static-debug: Makefile
	./propagate configs/dgux-g++-static-debug

freebsd-g++-shared: Makefile
	./propagate configs/freebsd-g++-shared

freebsd-g++-shared-debug: Makefile
	./propagate configs/freebsd-g++-shared-debug

freebsd-g++-static: Makefile
	./propagate configs/freebsd-g++-static

freebsd-g++-static-debug: Makefile
	./propagate configs/freebsd-g++-static-debug

gnu-g++-shared: Makefile
	./propagate configs/gnu-g++-shared

gnu-g++-shared-debug: Makefile
	./propagate configs/gnu-g++-shared-debug

gnu-g++-static: Makefile
	./propagate configs/gnu-g++-static

gnu-g++-static-debug: Makefile
	./propagate configs/gnu-g++-static-debug

hpux-acc-shared: Makefile
	./propagate configs/hpux-acc-shared

hpux-acc-shared-debug: Makefile
	./propagate configs/hpux-acc-shared-debug

hpux-acc-static: Makefile
	./propagate configs/hpux-acc-static

hpux-acc-static-debug: Makefile
	./propagate configs/hpux-acc-static-debug

hpux-cc-shared: Makefile
	./propagate configs/hpux-cc-shared

hpux-cc-shared-debug: Makefile
	./propagate configs/hpux-cc-shared-debug

hpux-cc-static: Makefile
	./propagate configs/hpux-cc-static

hpux-cc-static-debug: Makefile
	./propagate configs/hpux-cc-static-debug

hpux-g++-shared: Makefile
	./propagate configs/hpux-g++-shared

hpux-g++-shared-debug: Makefile
	./propagate configs/hpux-g++-shared-debug

hpux-g++-static: Makefile
	./propagate configs/hpux-g++-static

hpux-g++-static-debug: Makefile
	./propagate configs/hpux-g++-static-debug

irix-64-shared: Makefile
	./propagate configs/irix-64-shared

irix-64-shared-debug: Makefile
	./propagate configs/irix-64-shared-debug

irix-64-static: Makefile
	./propagate configs/irix-64-static

irix-64-static-debug: Makefile
	./propagate configs/irix-64-static-debug

irix-dcc-shared: Makefile
	./propagate configs/irix-dcc-shared

irix-dcc-shared-debug: Makefile
	./propagate configs/irix-dcc-shared-debug

irix-dcc-static: Makefile
	./propagate configs/irix-dcc-static

irix-dcc-static-debug: Makefile
	./propagate configs/irix-dcc-static-debug

irix-g++-shared: Makefile
	./propagate configs/irix-g++-shared

irix-g++-shared-debug: Makefile
	./propagate configs/irix-g++-shared-debug

irix-g++-static: Makefile
	./propagate configs/irix-g++-static

irix-g++-static-debug: Makefile
	./propagate configs/irix-g++-static-debug

irix-n32-shared: Makefile
	./propagate configs/irix-n32-shared

irix-n32-shared-debug: Makefile
	./propagate configs/irix-n32-shared-debug

irix-n32-static: Makefile
	./propagate configs/irix-n32-static

irix-n32-static-debug: Makefile
	./propagate configs/irix-n32-static-debug

irix-o32-shared: Makefile
	./propagate configs/irix-o32-shared

irix-o32-shared-debug: Makefile
	./propagate configs/irix-o32-shared-debug

irix-o32-static: Makefile
	./propagate configs/irix-o32-static

irix-o32-static-debug: Makefile
	./propagate configs/irix-o32-static-debug

linux-g++-shared: Makefile
	./propagate configs/linux-g++-shared

linux-g++-shared-debug: Makefile
	./propagate configs/linux-g++-shared-debug

linux-g++-static: Makefile
	./propagate configs/linux-g++-static

linux-g++-static-debug: Makefile
	./propagate configs/linux-g++-static-debug

netbsd-g++-shared: Makefile
	./propagate configs/netbsd-g++-shared

netbsd-g++-shared-debug: Makefile
	./propagate configs/netbsd-g++-shared-debug

netbsd-g++-static: Makefile
	./propagate configs/netbsd-g++-static

netbsd-g++-static-debug: Makefile
	./propagate configs/netbsd-g++-static-debug

openbsd-g++-shared: Makefile
	./propagate configs/openbsd-g++-shared

openbsd-g++-shared-debug: Makefile
	./propagate configs/openbsd-g++-shared-debug

openbsd-g++-static: Makefile
	./propagate configs/openbsd-g++-static

openbsd-g++-static-debug: Makefile
	./propagate configs/openbsd-g++-static-debug

osf1-cxx-shared: Makefile
	./propagate configs/osf1-cxx-shared

osf1-cxx-shared-debug: Makefile
	./propagate configs/osf1-cxx-shared-debug

osf1-cxx-static: Makefile
	./propagate configs/osf1-cxx-static

osf1-cxx-static-debug: Makefile
	./propagate configs/osf1-cxx-static-debug

osf1-g++-shared: Makefile
	./propagate configs/osf1-g++-shared

osf1-g++-shared-debug: Makefile
	./propagate configs/osf1-g++-shared-debug

osf1-g++-static: Makefile
	./propagate configs/osf1-g++-static

osf1-g++-static-debug: Makefile
	./propagate configs/osf1-g++-static-debug

qnx-g++-shared: Makefile
	./propagate configs/qnx-g++-shared

qnx-g++-shared-debug: Makefile
	./propagate configs/qnx-g++-shared-debug

qnx-g++-static: Makefile
	./propagate configs/qnx-g++-static

qnx-g++-static-debug: Makefile
	./propagate configs/qnx-g++-static-debug

sco-g++-shared: Makefile
	./propagate configs/sco-g++-shared

sco-g++-shared-debug: Makefile
	./propagate configs/sco-g++-shared-debug

sco-g++-static: Makefile
	./propagate configs/sco-g++-static

sco-g++-static-debug: Makefile
	./propagate configs/sco-g++-static-debug

solaris-cc-shared: Makefile
	./propagate configs/solaris-cc-shared

solaris-cc-shared-debug: Makefile
	./propagate configs/solaris-cc-shared-debug

solaris-cc-static: Makefile
	./propagate configs/solaris-cc-static

solaris-cc-static-debug: Makefile
	./propagate configs/solaris-cc-static-debug

solaris-g++-shared: Makefile
	./propagate configs/solaris-g++-shared

solaris-g++-shared-debug: Makefile
	./propagate configs/solaris-g++-shared-debug

solaris-g++-static: Makefile
	./propagate configs/solaris-g++-static

solaris-g++-static-debug: Makefile
	./propagate configs/solaris-g++-static-debug

sunos-g++-shared: Makefile
	./propagate configs/sunos-g++-shared

sunos-g++-shared-debug: Makefile
	./propagate configs/sunos-g++-shared-debug

sunos-g++-static: Makefile
	./propagate configs/sunos-g++-static

sunos-g++-static-debug: Makefile
	./propagate configs/sunos-g++-static-debug

ultrix-g++-static: Makefile
	./propagate configs/ultrix-g++-static

ultrix-g++-static-debug: Makefile
	./propagate configs/ultrix-g++-static-debug

unixware-g++-shared: Makefile
	./propagate configs/unixware-g++-shared

unixware-g++-shared-debug: Makefile
	./propagate configs/unixware-g++-shared-debug

unixware-g++-static: Makefile
	./propagate configs/unixware-g++-static

unixware-g++-static-debug: Makefile
	./propagate configs/unixware-g++-static-debug


dep: depend

FORCE:


