all:
	#$ ExpandGlue("MAKEFILES","\$(MAKE) -f ","\n\t\$(MAKE) -f ","");

tmake:
	#${
	    ExpandGlue("MAKEFILES","tmake -nodepend ","\n\ttmake -nodepend ","");
	    $text =~ s/(([a-z]*)\.mak)/$2 -o $1/gi;
	#$}

clean:
	#$ ExpandGlue("MAKEFILES","\$(MAKE) -f "," clean\n\t\$(MAKE) -f "," clean");
