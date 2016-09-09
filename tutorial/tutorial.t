# $Source: /local/lib/cvs/qt/tutorial/tutorial.t,v $
#
#!
#! This is a custom template for creating an examples/Makefile.
#!
#$ StdInit();

####### Tools

TMAKE	=	tmake

####### Directories

SUBDIRS =	#$ ExpandList("SUBDIRS");

####### Targets

all: #$ $text = $is_unix ? '$(SUBDIRS)' : 'targets';

showdirs:
	@echo $(SUBDIRS)

#${
    if ( $is_unix ) {
	$text = "\$(SUBDIRS): FORCE\n\tcd \$@; \$(MAKE)";
    } else {
	$text = "targets:\n";
	@t = split(/\s+/,$project{"SUBDIRS"});
	foreach $d ( @t ) {
	    $text = $text . "\tcd $d\n\t\$(MAKE) -f $d.mak\n\tcd ..\n";
	}
	chop $text;
    }
#$}

galore:
#${
    if ( $is_unix ) {
	$text = "\t" . 'for a in */* ; do [ -f $$a -a -x $$a ] && ( cd `dirname $$a` ; ../$$a & ) ; done';
    } else {
	@t = split(/\s+/,$project{"SUBDIRS"});
	foreach $d ( @t ) {
	    $text = $text . "\tcd $d\n\t$d.exe\n\tcd ..\n";
	}
	chop $text;
    }
#$}

tmake:
#${
    if ( $is_unix ) {
	$text = "\t" . 'for i in $(SUBDIRS); do ( cd $$i ; $(TMAKE) $$i.pro -o Makefile ) ; done';
    } else {
	@t = split(/\s+/,$project{"SUBDIRS"});
	foreach $d ( @t ) {
	    $text = $text . "\tcd $d\n\t\$(TMAKE) $d.pro -o $d.mak\n\tcd ..\n";
	}
	chop $text;
    }
#$}

clean:
#${
    if ( $is_unix ) {
	$text = "\t" . 'for i in $(SUBDIRS); do ( cd $$i ; $(MAKE) clean ) ; done';
    } else {
	@t = split(/\s+/,$project{"SUBDIRS"});
	foreach $d ( @t ) {
	    $text = $text . "\tcd $d\n\t\$(MAKE) -f $d.mak clean\n\tcd ..\n";
	}
    }
#$}

Makefile: tutorial.t tutorial.pro
	$(TMAKE) tutorial.pro -o Makefile

#$ $text = "FORCE:" if $is_unix;
