/**********************************************************************
** $Id: qpsprinter.cpp,v 2.29.2.8 1999/01/26 16:15:09 warwick Exp $
**
** Implementation of QPSPrinter class
**
** Created : 941003
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of Qt Free Edition, version 1.45.
**
** See the file LICENSE included in the distribution for the usage
** and distribution terms, or http://www.troll.no/free-license.html.
**
** IMPORTANT NOTE: You may NOT copy this file or any part of it into
** your own programs or libraries.
**
** Please see http://www.troll.no/pricing.html for information about
** Qt Professional Edition, which is this same library but with a
** license which allows creation of commercial/proprietary software.
**
*****************************************************************************/

#include "qpsprinter.h"
#include "qpainter.h"
#include "qpaintdevicedefs.h"
#include "qpaintdevicemetrics.h"
#include "qimage.h"
#include "qdatetime.h"

#include "qstring.h"
#include "qdict.h"
#include "qregexp.h"

#include "qfile.h"
#include "qbuffer.h"
#include "qintdict.h"

#if defined(_OS_WIN32_)
#include <io.h>
#else
#include <unistd.h>
#endif
#include <ctype.h>

// Note: this is comment-stripped and word-wrapped later.
static const char *ps_header[] = {
"/D  {bind def} bind def", // first word MUST be shorter than 78 characters
"/ED {exch def} D",
"/LT {lineto} D",
"/MT {moveto} D",
"/S  {stroke} D",
"/F  {setfont} D",
"/SW {setlinewidth} D",
"/CP {closepath} D",
"/RL {rlineto} D",
"/NP {newpath} D",
"/CM {currentmatrix} D",
"/SM {setmatrix} D",
"/TR {translate} D",
"/SRGB {setrgbcolor} D",
"/SC {aload pop SRGB} D",
"/GS {gsave} D",
"/GR {grestore} D",
"",
"/BSt 0 def",				// brush style
"/LWi 1 def",				// line width
"/PSt 1 def",				// pen style
"/Cx  0 def",				// current x position
"/Cy  0 def",				// current y position
"/WFi false def",			// winding fill
"/OMo false def",			// opaque mode (not transparent)
"",
"/BCol  [ 1 1 1 ] def",			// brush color
"/PCol  [ 0 0 0 ] def",			// pen color
"/BkCol [ 1 1 1 ] def",			// background color
"",
"/nS 0 def",				// number of saved painter states
"",
"/LArr[",					// Pen styles:
"    []		     []",			//   solid line
"    [ 10 3 ]	     [ 3 10 ]",			//   dash line
"    [ 3 3 ]	     [ 3 3 ]",			//   dot line
"    [ 5 3 3 3 ]	     [ 3 5 3 3 ]",	//   dash dot line
"    [ 5 3 3 3 3 3 ]  [ 3 5 3 3 3 3 ]",		//   dash dot dot line
"] def",
"",
"",//
"",// Returns the line pattern (from pen style PSt).
"",//
"",// Argument:
"",//   bool pattern
"",//	true : draw pattern
"",//	false: fill pattern
"",//
"",
"/GPS {",
"  PSt 1 ge PSt 5 le and",			// valid pen pattern?
"    { { LArr PSt 1 sub 2 mul get }",		// draw pattern
"      { LArr PSt 2 mul 1 sub get } ifelse",    // opaque pattern
"    }",
"    { [] } ifelse",				// out of range => solid line
"} D",
"",
"/QS {",				// stroke command
"    PSt 0 ne",				// != NO_PEN
"    { LWi SW",				// set line width
"      GS",
"      PCol SC",			// set pen color
"      true GPS 0 setdash S",		// draw line pattern
"      OMo PSt 1 ne and",		// opaque mode and not solid line?
"      { GR BkCol SC",
"	false GPS dup 0 get setdash S",	// fill in opaque pattern
"      }",
"      { GR } ifelse",
"    } if",
"} D",
"",
"/QF {",				// fill command
"    GS",
"    BSt 2 ge BSt 8 le and",		// dense pattern?
"    { BDArr BSt 2 sub get setgray fill } if",
"    BSt 9 ge BSt 14 le and",		// fill pattern?
"    { BF } if",
"    BSt 1 eq",				// solid brush?
"    { BCol SC WFi { fill } { eofill } ifelse } if",
"    GR",
"} D",
"",
"/PF {",				// polygon fill command
"    GS",
"    BSt 2 ge BSt 8 le and",		// dense pattern?
"    { BDArr BSt 2 sub get setgray WFi { fill } { eofill } ifelse } if",
"    BSt 9 ge BSt 14 le and",		// fill pattern?
"    { BF } if",
"    BSt 1 eq",				// solid brush?
"    { BCol SC WFi { fill } { eofill } ifelse } if",
"    GR",
"} D",
"",
"/BDArr[",				// Brush dense patterns:
"    0.94 0.88 0.63 0.50 0.37 0.12 0.6",
"] def",
"",
"/ArcDict 8 dict def",
"ArcDict begin",
"    /tmp matrix def",
"end",
"",
"/ARC {",				// Generic ARC function [ X Y W H ang1 ang2 ]
"    ArcDict begin",
"    /ang2 ED /ang1 ED /h ED /w ED /y ED /x ED",
"    tmp CM pop",
"    x w 2 div add y h 2 div add TR",
"    1 h w div neg scale",
"    ang2 0 ge",
"    {0 0 w 2 div ang1 ang1 ang2 add arc }",
"    {0 0 w 2 div ang1 ang1 ang2 add arcn} ifelse",
"    tmp SM",
"    end",
"} D",
"",
"",
"/QI {",
"    /savedContext save def",
"    clippath pathbbox",
"    3 index /PageX ED",
"    0 index /PageY ED",
"    3 2 roll",
"    exch",
"    sub neg /PageH ED",
"    sub neg /PageW ED",
"",
"    PageX PageY TR",
"    1 -1 scale",
"    /defM matrix CM def",		// default transformation matrix
"    /Cx  0 def",			// reset current x position
"    /Cy  0 def",			// reset current y position
"    255 255 255 BC",
"    /OMo false def",
"    1 0 0 0 0 PE",
"    0 0 0 0 B",
"} D",
"",
"/QP {",				// show page
"    savedContext restore",
"    showpage",
"} D",
"",
"/P {",					// PDC_DRAWPOINT [x y]
"    NP",
"    MT",
"    0.5 0.5 rmoveto",
"    0  -1 RL",
"    -1	0 RL",
"    0	1 RL",
"    CP",
"    PCol SC",
"    fill",
"} D",
"",
"/M {",					// PDC_MOVETO [x y]
"    /Cy ED /Cx ED",
"} D",
"",
"/L {",					// PDC_LINETO [x y]
"    NP",
"    Cx Cy MT",
"    /Cy ED /Cx ED",
"    Cx Cy LT",
"    QS",
"} D",
"",
"/DL {",				// PDC_DRAWLINE [x0 y0 x1 y1]
"    4 2 roll",
"    NP",
"    MT",
"    LT",
"    QS",
"} D",
"",
"/RDict 5 dict def",
"/R {",					// PDC_DRAWRECT [x y w h]
"    RDict begin",
"    /h ED /w ED /y ED /x ED",
"    NP",
"    x y MT",
"    0 h RL",
"    w 0 RL",
"    0 h neg RL",
"    CP",
"    QF",
"    QS",
"    end",
"} D",
"",
"/ACRDict 5 dict def",
"/ACR {",					// add clip rect
"    ACRDict begin",
"    /h ED /w ED /y ED /x ED",
"    x y MT",
"    0 h RL",
"    w 0 RL",
"    0 h neg RL",
"    CP",
"    end",
"} D",
"",
"/CLSTART {",				// clipping start
"    /clipTmp matrix CM def",		// save current matrix
"    defM SM",				// Page default matrix
"    NP",
"} D",
"",
"/CLEND {",				// clipping end
"    clip",
"    NP",
"    clipTmp SM",			// restore the current matrix
"} D",
"",
"/CLO {",				// clipping off
"    GR",				// restore top of page state
"    GS",				// save it back again
"    defM SM",				// set coordsys (defensive progr.)
"} D",
"",
"/RRDict 11 dict def",
"/RR {",				// PDC_DRAWROUNDRECT [x y w h xr yr]
"    RRDict begin",
"    /yr ED /xr ED /h ED /w ED /y ED /x ED",
"    xr 0 le yr 0 le or",
"    {x y w h R}",		    // Do rect if one of rounding values is less than 0.
"    {xr 100 ge yr 100 ge or",
"	{x y w h E}",			 // Do ellipse if both rounding values are larger than 100
"	{",
"	 /rx xr w mul 200 div def",
"	 /ry yr h mul 200 div def",
"	 /rx2 rx 2 mul def",
"	 /ry2 ry 2 mul def",
"	 NP",
"	 x rx add y MT",
"	 x w add rx2 sub y		 rx2 ry2 90  -90 ARC",
"	 x w add rx2 sub y h add ry2 sub rx2 ry2 0   -90 ARC",
"	 x		 y h add ry2 sub rx2 ry2 270 -90 ARC",
"	 x		 y		 rx2 ry2 180 -90 ARC",
"	 CP",
"	 QF",
"	 QS",
"	} ifelse",
"    } ifelse",
"    end",
"} D",
"",
"",
"/EDict 5 dict def",
"EDict begin",
"/tmp matrix def",
"end",
"/E {",				// PDC_DRAWELLIPSE [x y w h]
"    EDict begin",
"    /h ED /w ED /y ED /x ED",
"    tmp CM pop",
"    x w 2 div add y h 2 div add translate",
"    1 h w div scale",
"    NP",
"    0 0 w 2 div 0 360 arc",
"    tmp SM",
"    QF",
"    QS",
"    end",
"} D",
"",
"",
"/A {",				// PDC_DRAWARC [x y w h ang1 ang2]
"    16 div exch 16 div exch",
"    NP",
"    ARC",
"    QS",
"} D",
"",
"",
"/PieDict 7 dict def",
"/PIE {",				// PDC_DRAWPIE [x y w h ang1 ang2]
"    PieDict begin",
"    /ang2 ED /ang1 ED /h ED /w ED /y ED /x ED",
"    NP",
"    x w 2 div add y h 2 div add MT",
"    x y w h ang1 16 div ang2 16 div ARC",
"    CP",
"    QF",
"    QS",
"    end",
"} D",
"",
"/CH {",				// PDC_DRAWCHORD [x y w h ang1 ang2]
"    16 div exch 16 div exch",
"    NP",
"    ARC",
"    CP",
"    QF",
"    QS",
"} D",
"",
"",
"/BZ {",				// PDC_DRAWQUADBEZIER [3 points]
"    curveto",
"    QS",
"} D",
"",
"",
"/CRGB {",				// Compute RGB [R G B] => R/255 G/255 B/255
"    255 div 3 1 roll",
"    255 div 3 1 roll",
"    255 div 3 1 roll",
"} D",
"",
"",
"/SV {",				// Save painter state
"    BSt LWi PSt Cx Cy WFi OMo BCol PCol BkCol",
"    /nS nS 1 add def",
"    GS",
"} D",
"",
"/RS {",				// Restore painter state
"    nS 0 gt",
"    { GR",
"      /BkCol ED /PCol ED /BCol ED /OMo ED /WFi ED",
"      /Cy ED /Cx ED /PSt ED /LWi ED /BSt ED",
"      /nS nS 1 sub def",
"    } if",
"",
"} D",
"",
"/BC {",				// PDC_SETBKCOLOR [R G B]
"    CRGB",
"    BkCol astore pop",
"} D",
"",
"/B {",					// PDC_SETBRUSH [style R G B]
"    CRGB",
"    BCol astore pop",
"    /BSt ED",
"} D",
"",
"/PE {",				// PDC_SETPEN [style width R G B]
"    CRGB",
"    PCol astore pop",
"    /LWi ED",
"    /PSt ED",
"    LWi 0 eq { 0.3 /LWi ED } if",
"} D",
"",
"/ST {",				// SET TRANSFORM [matrix]
"    defM setmatrix",
"    concat",
"} D",
"",
"/MF {",				// newname encoding fontname
"  findfont dup length dict begin",
"  {",
"    1 index /FID ne",
"    {def}{pop pop}ifelse",
"  } forall",
"  /Encoding ED currentdict end",
"  definefont pop",
"} D",
"",
"/DF {",				// newname pointsize fontmame
"  findfont",
"  /FONTSIZE 3 -1 roll def [ FONTSIZE 0 0 FONTSIZE -1 mul 0 0 ] makefont",
"  def",
"} D",
"",
"",
"",// isn't this important enough to try to avoid the SC?
"",
"/T {",					// PDC_DRAWTEXT [string x y]
"    MT",				// !!!! Uff
"    PCol SC",				// set pen/text color
"    show",
"} D",
"",
"",
"/BFDict 3 dict def",
"/BF {",				// brush fill
"    BSt 9 ge BSt 14 le and",		// valid brush pattern?
"    {",
"     BFDict begin",
"     GS",
"     WFi { clip } { eoclip } ifelse",
"     defM SM",
"     pathbbox",			// left upper right lower
"     3 index 3 index translate",
"     4 2 roll",			// right lower left upper
"     3 2 roll",			// right left upper lower
"     exch",				// left right lower upper
"     sub /h ED",
"     sub /w ED",
"     OMo {",
"	  NP",
"	  0 0 MT",
"	  0 h RL",
"	  w 0 RL",
"	  0 h neg RL",
"	  CP",
"	  BkCol SC",
"	  fill",
"     } if",
"     BCol SC",
"     0.3 SW",
"     BSt 9 eq BSt 11 eq or",		// horiz or cross pattern
"     { 0 4 h",				// draw horiz lines !!! alignment
"       { NP dup 0 exch MT w exch LT S } for",
"     } if",
"     BSt 10 eq BSt 11 eq or",		// vert or cross pattern
"     { 0 4 w",				// draw vert lines !!! alignment
"       { NP dup 0 MT h LT S } for",
"     } if",
"     BSt 12 eq BSt 14 eq or",		// F-diag or diag cross
"     { w h gt",
"       { 0 6 w h add",
"	{ NP dup h MT h sub 0 LT S } for }",
"       { 0 6 w h add",
"	 { NP dup w exch MT w add 0 exch LT S } for } ifelse",
"     } if",
"     BSt 13 eq BSt 14 eq or",		// B-diag or diag cross
"     { w h gt",
"       { 0 6 w h add",
"	 { NP dup 0 MT h sub h LT S } for }",
"       { 0 6 w h add",
"	 { NP dup 0 exch MT w add w exch LT S } for } ifelse",
"     } if",
"     GR",
"     end",
"    } if",
"} D",
"",
"/LArr[",					// Pen styles:
"    []		     []",			//   solid line
"    [ 10 3 ]	     [ 3 10 ]",			//   dash line
"    [ 3 3 ]	     [ 3 3 ]",			//   dot line
"    [ 5 3 3 3 ]	     [ 3 5 3 3 ]",	//   dash dot line
"    [ 5 3 3 3 3 3 ]  [ 3 5 3 3 3 3 ]",		//   dash dot dot line
"] def",
"",
"",//
"",// Returns the line pattern (from pen style PSt).
"",//
"",// Argument:
"",//   bool pattern
"",//	true : draw pattern
"",//	false: fill pattern
"",//
"",
"/GPS {",
"  PSt 1 ge PSt 5 le and",			// valid pen pattern?
"    { { LArr PSt 1 sub 2 mul get }",		// draw pattern
"      { LArr PSt 2 mul 1 sub get } ifelse",    // opaque pattern
"    }",
"    { [] } ifelse",				// out of range => solid line
"} D",
"",
"",
"/colorimage where {",
"  pop",
"  /QCI {", // as colorimage but without the last two arguments
"    false 3 colorimage",
"  } D",
"} {", // the hard way.  based on PD code by John Walker <kelvin@autodesk.com>
"  /QCIDict 25 dict def",
"  /QCI {",
"    QCIDict begin",
"      /Function ED",
"      /Matrix ED",
"      /Bcomp ED",
"      /Height ED",
"      /Width ED",
"      /Bycomp Bcomp 7 add 8 idiv def",
"      Width Height Bcomp Matrix",
"      /Gstr sl length 3 idiv string def",
"      { Function exec",
"	 /Cstr ED",
"	 0 1 Cstr length 3 idiv 1 sub {",
"	   /I ED",
"	   /X I 3 mul def",
"	   Gstr I",
"	   Cstr X       get 0.30 mul",
"	   Cstr X 1 add get 0.59 mul",
"	   Cstr X 2 add get 0.11 mul",
"	   add add cvi",
"	   put",
"	 } for",
"	 Gstr",
"      }",
"      image",
"    end",
"  } D",
"} ifelse",
"/setstrokeadjust where { pop true setstrokeadjust } if",
0};



// the next table is derived from a list provided by Adobe on its web
// server.  the start of the header comment:
//
// Name:          Adobe Glyph List with Unicode Values
// Table version: 1.1
// Date:          24 November 1997
//
// This list relates Unicode values (UVs) to glyph names, and should be used
// only as described in the document "Unicode and Glyph Names." The Unicode
// Standard version 2.0 is used.


static struct {
    Q_UINT16 u;
    const char * g;
} unicodetoglyph[] = {
/*
  $ grep '^[0-9A-F][0-9A-F][0-9A-F][0-9A-F];' < glyphlist.txt | \
    sed -e 's/;/, "/' -e 's/;.*$/" },/' -e 's/^/    { 0x/' | sort
*/
    { 0x0020, "space" },
    { 0x0021, "exclam" },
    { 0x0022, "quotedbl" },
    { 0x0023, "numbersign" },
    { 0x0024, "dollar" },
    { 0x0025, "percent" },
    { 0x0026, "ampersand" },
    { 0x0027, "quotesingle" },
    { 0x0028, "parenleft" },
    { 0x0029, "parenright" },
    { 0x002A, "asterisk" },
    { 0x002B, "plus" },
    { 0x002C, "comma" },
    { 0x002D, "hyphen" },
    { 0x002E, "period" },
    { 0x002F, "slash" },
    { 0x0030, "zero" },
    { 0x0031, "one" },
    { 0x0032, "two" },
    { 0x0033, "three" },
    { 0x0034, "four" },
    { 0x0035, "five" },
    { 0x0036, "six" },
    { 0x0037, "seven" },
    { 0x0038, "eight" },
    { 0x0039, "nine" },
    { 0x003A, "colon" },
    { 0x003B, "semicolon" },
    { 0x003C, "less" },
    { 0x003D, "equal" },
    { 0x003E, "greater" },
    { 0x003F, "question" },
    { 0x0040, "at" },
    { 0x0041, "A" },
    { 0x0042, "B" },
    { 0x0043, "C" },
    { 0x0044, "D" },
    { 0x0045, "E" },
    { 0x0046, "F" },
    { 0x0047, "G" },
    { 0x0048, "H" },
    { 0x0049, "I" },
    { 0x004A, "J" },
    { 0x004B, "K" },
    { 0x004C, "L" },
    { 0x004D, "M" },
    { 0x004E, "N" },
    { 0x004F, "O" },
    { 0x0050, "P" },
    { 0x0051, "Q" },
    { 0x0052, "R" },
    { 0x0053, "S" },
    { 0x0054, "T" },
    { 0x0055, "U" },
    { 0x0056, "V" },
    { 0x0057, "W" },
    { 0x0058, "X" },
    { 0x0059, "Y" },
    { 0x005A, "Z" },
    { 0x005B, "bracketleft" },
    { 0x005C, "backslash" },
    { 0x005D, "bracketright" },
    { 0x005E, "asciicircum" },
    { 0x005F, "underscore" },
    { 0x0060, "grave" },
    { 0x0061, "a" },
    { 0x0062, "b" },
    { 0x0063, "c" },
    { 0x0064, "d" },
    { 0x0065, "e" },
    { 0x0066, "f" },
    { 0x0067, "g" },
    { 0x0068, "h" },
    { 0x0069, "i" },
    { 0x006A, "j" },
    { 0x006B, "k" },
    { 0x006C, "l" },
    { 0x006D, "m" },
    { 0x006E, "n" },
    { 0x006F, "o" },
    { 0x0070, "p" },
    { 0x0071, "q" },
    { 0x0072, "r" },
    { 0x0073, "s" },
    { 0x0074, "t" },
    { 0x0075, "u" },
    { 0x0076, "v" },
    { 0x0077, "w" },
    { 0x0078, "x" },
    { 0x0079, "y" },
    { 0x007A, "z" },
    { 0x007B, "braceleft" },
    { 0x007C, "bar" },
    { 0x007D, "braceright" },
    { 0x007E, "asciitilde" },
    { 0x00A0, "space" },
    { 0x00A1, "exclamdown" },
    { 0x00A2, "cent" },
    { 0x00A3, "sterling" },
    { 0x00A4, "currency" },
    { 0x00A5, "yen" },
    { 0x00A6, "brokenbar" },
    { 0x00A7, "section" },
    { 0x00A8, "dieresis" },
    { 0x00A9, "copyright" },
    { 0x00AA, "ordfeminine" },
    { 0x00AB, "guillemotleft" },
    { 0x00AC, "logicalnot" },
    { 0x00AD, "hyphen" },
    { 0x00AE, "registered" },
    { 0x00AF, "macron" },
    { 0x00B0, "degree" },
    { 0x00B1, "plusminus" },
    { 0x00B2, "twosuperior" },
    { 0x00B3, "threesuperior" },
    { 0x00B4, "acute" },
    { 0x00B5, "mu" },
    { 0x00B6, "paragraph" },
    { 0x00B7, "periodcentered" },
    { 0x00B8, "cedilla" },
    { 0x00B9, "onesuperior" },
    { 0x00BA, "ordmasculine" },
    { 0x00BB, "guillemotright" },
    { 0x00BC, "onequarter" },
    { 0x00BD, "onehalf" },
    { 0x00BE, "threequarters" },
    { 0x00BF, "questiondown" },
    { 0x00C0, "Agrave" },
    { 0x00C1, "Aacute" },
    { 0x00C2, "Acircumflex" },
    { 0x00C3, "Atilde" },
    { 0x00C4, "Adieresis" },
    { 0x00C5, "Aring" },
    { 0x00C6, "AE" },
    { 0x00C7, "Ccedilla" },
    { 0x00C8, "Egrave" },
    { 0x00C9, "Eacute" },
    { 0x00CA, "Ecircumflex" },
    { 0x00CB, "Edieresis" },
    { 0x00CC, "Igrave" },
    { 0x00CD, "Iacute" },
    { 0x00CE, "Icircumflex" },
    { 0x00CF, "Idieresis" },
    { 0x00D0, "Eth" },
    { 0x00D1, "Ntilde" },
    { 0x00D2, "Ograve" },
    { 0x00D3, "Oacute" },
    { 0x00D4, "Ocircumflex" },
    { 0x00D5, "Otilde" },
    { 0x00D6, "Odieresis" },
    { 0x00D7, "multiply" },
    { 0x00D8, "Oslash" },
    { 0x00D9, "Ugrave" },
    { 0x00DA, "Uacute" },
    { 0x00DB, "Ucircumflex" },
    { 0x00DC, "Udieresis" },
    { 0x00DD, "Yacute" },
    { 0x00DE, "Thorn" },
    { 0x00DF, "germandbls" },
    { 0x00E0, "agrave" },
    { 0x00E1, "aacute" },
    { 0x00E2, "acircumflex" },
    { 0x00E3, "atilde" },
    { 0x00E4, "adieresis" },
    { 0x00E5, "aring" },
    { 0x00E6, "ae" },
    { 0x00E7, "ccedilla" },
    { 0x00E8, "egrave" },
    { 0x00E9, "eacute" },
    { 0x00EA, "ecircumflex" },
    { 0x00EB, "edieresis" },
    { 0x00EC, "igrave" },
    { 0x00ED, "iacute" },
    { 0x00EE, "icircumflex" },
    { 0x00EF, "idieresis" },
    { 0x00F0, "eth" },
    { 0x00F1, "ntilde" },
    { 0x00F2, "ograve" },
    { 0x00F3, "oacute" },
    { 0x00F4, "ocircumflex" },
    { 0x00F5, "otilde" },
    { 0x00F6, "odieresis" },
    { 0x00F7, "divide" },
    { 0x00F8, "oslash" },
    { 0x00F9, "ugrave" },
    { 0x00FA, "uacute" },
    { 0x00FB, "ucircumflex" },
    { 0x00FC, "udieresis" },
    { 0x00FD, "yacute" },
    { 0x00FE, "thorn" },
    { 0x00FF, "ydieresis" },
    { 0x0100, "Amacron" },
    { 0x0101, "amacron" },
    { 0x0102, "Abreve" },
    { 0x0103, "abreve" },
    { 0x0104, "Aogonek" },
    { 0x0105, "aogonek" },
    { 0x0106, "Cacute" },
    { 0x0107, "cacute" },
    { 0x0108, "Ccircumflex" },
    { 0x0109, "ccircumflex" },
    { 0x010A, "Cdotaccent" },
    { 0x010B, "cdotaccent" },
    { 0x010C, "Ccaron" },
    { 0x010D, "ccaron" },
    { 0x010E, "Dcaron" },
    { 0x010F, "dcaron" },
    { 0x0110, "Dcroat" },
    { 0x0111, "dcroat" },
    { 0x0112, "Emacron" },
    { 0x0113, "emacron" },
    { 0x0114, "Ebreve" },
    { 0x0115, "ebreve" },
    { 0x0116, "Edotaccent" },
    { 0x0117, "edotaccent" },
    { 0x0118, "Eogonek" },
    { 0x0119, "eogonek" },
    { 0x011A, "Ecaron" },
    { 0x011B, "ecaron" },
    { 0x011C, "Gcircumflex" },
    { 0x011D, "gcircumflex" },
    { 0x011E, "Gbreve" },
    { 0x011F, "gbreve" },
    { 0x0120, "Gdotaccent" },
    { 0x0121, "gdotaccent" },
    { 0x0122, "Gcommaaccent" },
    { 0x0123, "gcommaaccent" },
    { 0x0124, "Hcircumflex" },
    { 0x0125, "hcircumflex" },
    { 0x0126, "Hbar" },
    { 0x0127, "hbar" },
    { 0x0128, "Itilde" },
    { 0x0129, "itilde" },
    { 0x012A, "Imacron" },
    { 0x012B, "imacron" },
    { 0x012C, "Ibreve" },
    { 0x012D, "ibreve" },
    { 0x012E, "Iogonek" },
    { 0x012F, "iogonek" },
    { 0x0130, "Idotaccent" },
    { 0x0131, "dotlessi" },
    { 0x0132, "IJ" },
    { 0x0133, "ij" },
    { 0x0134, "Jcircumflex" },
    { 0x0135, "jcircumflex" },
    { 0x0136, "Kcommaaccent" },
    { 0x0137, "kcommaaccent" },
    { 0x0138, "kgreenlandic" },
    { 0x0139, "Lacute" },
    { 0x013A, "lacute" },
    { 0x013B, "Lcommaaccent" },
    { 0x013C, "lcommaaccent" },
    { 0x013D, "Lcaron" },
    { 0x013E, "lcaron" },
    { 0x013F, "Ldot" },
    { 0x0140, "ldot" },
    { 0x0141, "Lslash" },
    { 0x0142, "lslash" },
    { 0x0143, "Nacute" },
    { 0x0144, "nacute" },
    { 0x0145, "Ncommaaccent" },
    { 0x0146, "ncommaaccent" },
    { 0x0147, "Ncaron" },
    { 0x0148, "ncaron" },
    { 0x0149, "napostrophe" },
    { 0x014A, "Eng" },
    { 0x014B, "eng" },
    { 0x014C, "Omacron" },
    { 0x014D, "omacron" },
    { 0x014E, "Obreve" },
    { 0x014F, "obreve" },
    { 0x0150, "Ohungarumlaut" },
    { 0x0151, "ohungarumlaut" },
    { 0x0152, "OE" },
    { 0x0153, "oe" },
    { 0x0154, "Racute" },
    { 0x0155, "racute" },
    { 0x0156, "Rcommaaccent" },
    { 0x0157, "rcommaaccent" },
    { 0x0158, "Rcaron" },
    { 0x0159, "rcaron" },
    { 0x015A, "Sacute" },
    { 0x015B, "sacute" },
    { 0x015C, "Scircumflex" },
    { 0x015D, "scircumflex" },
    { 0x015E, "Scommaaccent" },
    { 0x015F, "scommaaccent" },
    { 0x0160, "Scaron" },
    { 0x0161, "scaron" },
    { 0x0162, "Tcommaaccent" },
    { 0x0163, "tcommaaccent" },
    { 0x0164, "Tcaron" },
    { 0x0165, "tcaron" },
    { 0x0166, "Tbar" },
    { 0x0167, "tbar" },
    { 0x0168, "Utilde" },
    { 0x0169, "utilde" },
    { 0x016A, "Umacron" },
    { 0x016B, "umacron" },
    { 0x016C, "Ubreve" },
    { 0x016D, "ubreve" },
    { 0x016E, "Uring" },
    { 0x016F, "uring" },
    { 0x0170, "Uhungarumlaut" },
    { 0x0171, "uhungarumlaut" },
    { 0x0172, "Uogonek" },
    { 0x0173, "uogonek" },
    { 0x0174, "Wcircumflex" },
    { 0x0175, "wcircumflex" },
    { 0x0176, "Ycircumflex" },
    { 0x0177, "ycircumflex" },
    { 0x0178, "Ydieresis" },
    { 0x0179, "Zacute" },
    { 0x017A, "zacute" },
    { 0x017B, "Zdotaccent" },
    { 0x017C, "zdotaccent" },
    { 0x017D, "Zcaron" },
    { 0x017E, "zcaron" },
    { 0x017F, "longs" },
    { 0x0192, "florin" },
    { 0x01A0, "Ohorn" },
    { 0x01A1, "ohorn" },
    { 0x01AF, "Uhorn" },
    { 0x01B0, "uhorn" },
    { 0x01E6, "Gcaron" },
    { 0x01E7, "gcaron" },
    { 0x01FA, "Aringacute" },
    { 0x01FB, "aringacute" },
    { 0x01FC, "AEacute" },
    { 0x01FD, "aeacute" },
    { 0x01FE, "Oslashacute" },
    { 0x01FF, "oslashacute" },
    { 0x02BC, "afii57929" },
    { 0x02BD, "afii64937" },
    { 0x02C6, "circumflex" },
    { 0x02C7, "caron" },
    { 0x02C9, "macron" },
    { 0x02D8, "breve" },
    { 0x02D9, "dotaccent" },
    { 0x02DA, "ring" },
    { 0x02DB, "ogonek" },
    { 0x02DC, "tilde" },
    { 0x02DD, "hungarumlaut" },
    { 0x0300, "gravecomb" },
    { 0x0301, "acutecomb" },
    { 0x0303, "tildecomb" },
    { 0x0309, "hookabovecomb" },
    { 0x0323, "dotbelowcomb" },
    { 0x0384, "tonos" },
    { 0x0385, "dieresistonos" },
    { 0x0386, "Alphatonos" },
    { 0x0387, "anoteleia" },
    { 0x0388, "Epsilontonos" },
    { 0x0389, "Etatonos" },
    { 0x038A, "Iotatonos" },
    { 0x038C, "Omicrontonos" },
    { 0x038E, "Upsilontonos" },
    { 0x038F, "Omegatonos" },
    { 0x0390, "iotadieresistonos" },
    { 0x0391, "Alpha" },
    { 0x0392, "Beta" },
    { 0x0393, "Gamma" },
    { 0x0394, "Delta" },
    { 0x0395, "Epsilon" },
    { 0x0396, "Zeta" },
    { 0x0397, "Eta" },
    { 0x0398, "Theta" },
    { 0x0399, "Iota" },
    { 0x039A, "Kappa" },
    { 0x039B, "Lambda" },
    { 0x039C, "Mu" },
    { 0x039D, "Nu" },
    { 0x039E, "Xi" },
    { 0x039F, "Omicron" },
    { 0x03A0, "Pi" },
    { 0x03A1, "Rho" },
    { 0x03A3, "Sigma" },
    { 0x03A4, "Tau" },
    { 0x03A5, "Upsilon" },
    { 0x03A6, "Phi" },
    { 0x03A7, "Chi" },
    { 0x03A8, "Psi" },
    { 0x03A9, "Omega" },
    { 0x03AA, "Iotadieresis" },
    { 0x03AB, "Upsilondieresis" },
    { 0x03AC, "alphatonos" },
    { 0x03AD, "epsilontonos" },
    { 0x03AE, "etatonos" },
    { 0x03AF, "iotatonos" },
    { 0x03B0, "upsilondieresistonos" },
    { 0x03B1, "alpha" },
    { 0x03B2, "beta" },
    { 0x03B3, "gamma" },
    { 0x03B4, "delta" },
    { 0x03B5, "epsilon" },
    { 0x03B6, "zeta" },
    { 0x03B7, "eta" },
    { 0x03B8, "theta" },
    { 0x03B9, "iota" },
    { 0x03BA, "kappa" },
    { 0x03BB, "lambda" },
    { 0x03BC, "mu" },
    { 0x03BD, "nu" },
    { 0x03BE, "xi" },
    { 0x03BF, "omicron" },
    { 0x03C0, "pi" },
    { 0x03C1, "rho" },
    { 0x03C2, "sigma1" },
    { 0x03C3, "sigma" },
    { 0x03C4, "tau" },
    { 0x03C5, "upsilon" },
    { 0x03C6, "phi" },
    { 0x03C7, "chi" },
    { 0x03C8, "psi" },
    { 0x03C9, "omega" },
    { 0x03CA, "iotadieresis" },
    { 0x03CB, "upsilondieresis" },
    { 0x03CC, "omicrontonos" },
    { 0x03CD, "upsilontonos" },
    { 0x03CE, "omegatonos" },
    { 0x03D1, "theta1" },
    { 0x03D2, "Upsilon1" },
    { 0x03D5, "phi1" },
    { 0x03D6, "omega1" },
    { 0x0401, "afii10023" },
    { 0x0402, "afii10051" },
    { 0x0403, "afii10052" },
    { 0x0404, "afii10053" },
    { 0x0405, "afii10054" },
    { 0x0406, "afii10055" },
    { 0x0407, "afii10056" },
    { 0x0408, "afii10057" },
    { 0x0409, "afii10058" },
    { 0x040A, "afii10059" },
    { 0x040B, "afii10060" },
    { 0x040C, "afii10061" },
    { 0x040E, "afii10062" },
    { 0x040F, "afii10145" },
    { 0x0410, "afii10017" },
    { 0x0411, "afii10018" },
    { 0x0412, "afii10019" },
    { 0x0413, "afii10020" },
    { 0x0414, "afii10021" },
    { 0x0415, "afii10022" },
    { 0x0416, "afii10024" },
    { 0x0417, "afii10025" },
    { 0x0418, "afii10026" },
    { 0x0419, "afii10027" },
    { 0x041A, "afii10028" },
    { 0x041B, "afii10029" },
    { 0x041C, "afii10030" },
    { 0x041D, "afii10031" },
    { 0x041E, "afii10032" },
    { 0x041F, "afii10033" },
    { 0x0420, "afii10034" },
    { 0x0421, "afii10035" },
    { 0x0422, "afii10036" },
    { 0x0423, "afii10037" },
    { 0x0424, "afii10038" },
    { 0x0425, "afii10039" },
    { 0x0426, "afii10040" },
    { 0x0427, "afii10041" },
    { 0x0428, "afii10042" },
    { 0x0429, "afii10043" },
    { 0x042A, "afii10044" },
    { 0x042B, "afii10045" },
    { 0x042C, "afii10046" },
    { 0x042D, "afii10047" },
    { 0x042E, "afii10048" },
    { 0x042F, "afii10049" },
    { 0x0430, "afii10065" },
    { 0x0431, "afii10066" },
    { 0x0432, "afii10067" },
    { 0x0433, "afii10068" },
    { 0x0434, "afii10069" },
    { 0x0435, "afii10070" },
    { 0x0436, "afii10072" },
    { 0x0437, "afii10073" },
    { 0x0438, "afii10074" },
    { 0x0439, "afii10075" },
    { 0x043A, "afii10076" },
    { 0x043B, "afii10077" },
    { 0x043C, "afii10078" },
    { 0x043D, "afii10079" },
    { 0x043E, "afii10080" },
    { 0x043F, "afii10081" },
    { 0x0440, "afii10082" },
    { 0x0441, "afii10083" },
    { 0x0442, "afii10084" },
    { 0x0443, "afii10085" },
    { 0x0444, "afii10086" },
    { 0x0445, "afii10087" },
    { 0x0446, "afii10088" },
    { 0x0447, "afii10089" },
    { 0x0448, "afii10090" },
    { 0x0449, "afii10091" },
    { 0x044A, "afii10092" },
    { 0x044B, "afii10093" },
    { 0x044C, "afii10094" },
    { 0x044D, "afii10095" },
    { 0x044E, "afii10096" },
    { 0x044F, "afii10097" },
    { 0x0451, "afii10071" },
    { 0x0452, "afii10099" },
    { 0x0453, "afii10100" },
    { 0x0454, "afii10101" },
    { 0x0455, "afii10102" },
    { 0x0456, "afii10103" },
    { 0x0457, "afii10104" },
    { 0x0458, "afii10105" },
    { 0x0459, "afii10106" },
    { 0x045A, "afii10107" },
    { 0x045B, "afii10108" },
    { 0x045C, "afii10109" },
    { 0x045E, "afii10110" },
    { 0x045F, "afii10193" },
    { 0x0462, "afii10146" },
    { 0x0463, "afii10194" },
    { 0x0472, "afii10147" },
    { 0x0473, "afii10195" },
    { 0x0474, "afii10148" },
    { 0x0475, "afii10196" },
    { 0x0490, "afii10050" },
    { 0x0491, "afii10098" },
    { 0x04D9, "afii10846" },
    { 0x05B0, "afii57799" },
    { 0x05B1, "afii57801" },
    { 0x05B2, "afii57800" },
    { 0x05B3, "afii57802" },
    { 0x05B4, "afii57793" },
    { 0x05B5, "afii57794" },
    { 0x05B6, "afii57795" },
    { 0x05B7, "afii57798" },
    { 0x05B8, "afii57797" },
    { 0x05B9, "afii57806" },
    { 0x05BB, "afii57796" },
    { 0x05BC, "afii57807" },
    { 0x05BD, "afii57839" },
    { 0x05BE, "afii57645" },
    { 0x05BF, "afii57841" },
    { 0x05C0, "afii57842" },
    { 0x05C1, "afii57804" },
    { 0x05C2, "afii57803" },
    { 0x05C3, "afii57658" },
    { 0x05D0, "afii57664" },
    { 0x05D1, "afii57665" },
    { 0x05D2, "afii57666" },
    { 0x05D3, "afii57667" },
    { 0x05D4, "afii57668" },
    { 0x05D5, "afii57669" },
    { 0x05D6, "afii57670" },
    { 0x05D7, "afii57671" },
    { 0x05D8, "afii57672" },
    { 0x05D9, "afii57673" },
    { 0x05DA, "afii57674" },
    { 0x05DB, "afii57675" },
    { 0x05DC, "afii57676" },
    { 0x05DD, "afii57677" },
    { 0x05DE, "afii57678" },
    { 0x05DF, "afii57679" },
    { 0x05E0, "afii57680" },
    { 0x05E1, "afii57681" },
    { 0x05E2, "afii57682" },
    { 0x05E3, "afii57683" },
    { 0x05E4, "afii57684" },
    { 0x05E5, "afii57685" },
    { 0x05E6, "afii57686" },
    { 0x05E7, "afii57687" },
    { 0x05E8, "afii57688" },
    { 0x05E9, "afii57689" },
    { 0x05EA, "afii57690" },
    { 0x05F0, "afii57716" },
    { 0x05F1, "afii57717" },
    { 0x05F2, "afii57718" },
    { 0x060C, "afii57388" },
    { 0x061B, "afii57403" },
    { 0x061F, "afii57407" },
    { 0x0621, "afii57409" },
    { 0x0622, "afii57410" },
    { 0x0623, "afii57411" },
    { 0x0624, "afii57412" },
    { 0x0625, "afii57413" },
    { 0x0626, "afii57414" },
    { 0x0627, "afii57415" },
    { 0x0628, "afii57416" },
    { 0x0629, "afii57417" },
    { 0x062A, "afii57418" },
    { 0x062B, "afii57419" },
    { 0x062C, "afii57420" },
    { 0x062D, "afii57421" },
    { 0x062E, "afii57422" },
    { 0x062F, "afii57423" },
    { 0x0630, "afii57424" },
    { 0x0631, "afii57425" },
    { 0x0632, "afii57426" },
    { 0x0633, "afii57427" },
    { 0x0634, "afii57428" },
    { 0x0635, "afii57429" },
    { 0x0636, "afii57430" },
    { 0x0637, "afii57431" },
    { 0x0638, "afii57432" },
    { 0x0639, "afii57433" },
    { 0x063A, "afii57434" },
    { 0x0640, "afii57440" },
    { 0x0641, "afii57441" },
    { 0x0642, "afii57442" },
    { 0x0643, "afii57443" },
    { 0x0644, "afii57444" },
    { 0x0645, "afii57445" },
    { 0x0646, "afii57446" },
    { 0x0647, "afii57470" },
    { 0x0648, "afii57448" },
    { 0x0649, "afii57449" },
    { 0x064A, "afii57450" },
    { 0x064B, "afii57451" },
    { 0x064C, "afii57452" },
    { 0x064D, "afii57453" },
    { 0x064E, "afii57454" },
    { 0x064F, "afii57455" },
    { 0x0650, "afii57456" },
    { 0x0651, "afii57457" },
    { 0x0652, "afii57458" },
    { 0x0660, "afii57392" },
    { 0x0661, "afii57393" },
    { 0x0662, "afii57394" },
    { 0x0663, "afii57395" },
    { 0x0664, "afii57396" },
    { 0x0665, "afii57397" },
    { 0x0666, "afii57398" },
    { 0x0667, "afii57399" },
    { 0x0668, "afii57400" },
    { 0x0669, "afii57401" },
    { 0x066A, "afii57381" },
    { 0x066D, "afii63167" },
    { 0x0679, "afii57511" },
    { 0x067E, "afii57506" },
    { 0x0686, "afii57507" },
    { 0x0688, "afii57512" },
    { 0x0691, "afii57513" },
    { 0x0698, "afii57508" },
    { 0x06A4, "afii57505" },
    { 0x06AF, "afii57509" },
    { 0x06BA, "afii57514" },
    { 0x06D2, "afii57519" },
    { 0x06D5, "afii57534" },
    { 0x1E80, "Wgrave" },
    { 0x1E81, "wgrave" },
    { 0x1E82, "Wacute" },
    { 0x1E83, "wacute" },
    { 0x1E84, "Wdieresis" },
    { 0x1E85, "wdieresis" },
    { 0x1E9E, "Scedilla" },
    { 0x1E9F, "scedilla" },
    { 0x1EF2, "Ygrave" },
    { 0x1EF3, "ygrave" },
    { 0x200C, "afii61664" },
    { 0x200D, "afii301" },
    { 0x200E, "afii299" },
    { 0x200F, "afii300" },
    { 0x2012, "figuredash" },
    { 0x2013, "endash" },
    { 0x2014, "emdash" },
    { 0x2015, "afii00208" },
    { 0x2017, "underscoredbl" },
    { 0x2018, "quoteleft" },
    { 0x2019, "quoteright" },
    { 0x201A, "quotesinglbase" },
    { 0x201B, "quotereversed" },
    { 0x201C, "quotedblleft" },
    { 0x201D, "quotedblright" },
    { 0x201E, "quotedblbase" },
    { 0x2020, "dagger" },
    { 0x2021, "daggerdbl" },
    { 0x2022, "bullet" },
    { 0x2024, "onedotenleader" },
    { 0x2025, "twodotenleader" },
    { 0x2026, "ellipsis" },
    { 0x202C, "afii61573" },
    { 0x202D, "afii61574" },
    { 0x202E, "afii61575" },
    { 0x2030, "perthousand" },
    { 0x2032, "minute" },
    { 0x2033, "second" },
    { 0x2039, "guilsinglleft" },
    { 0x203A, "guilsinglright" },
    { 0x203C, "exclamdbl" },
    { 0x2044, "fraction" },
    { 0x2070, "zerosuperior" },
    { 0x2074, "foursuperior" },
    { 0x2075, "fivesuperior" },
    { 0x2076, "sixsuperior" },
    { 0x2077, "sevensuperior" },
    { 0x2078, "eightsuperior" },
    { 0x2079, "ninesuperior" },
    { 0x207D, "parenleftsuperior" },
    { 0x207E, "parenrightsuperior" },
    { 0x207F, "nsuperior" },
    { 0x2080, "zeroinferior" },
    { 0x2081, "oneinferior" },
    { 0x2082, "twoinferior" },
    { 0x2083, "threeinferior" },
    { 0x2084, "fourinferior" },
    { 0x2085, "fiveinferior" },
    { 0x2086, "sixinferior" },
    { 0x2087, "seveninferior" },
    { 0x2088, "eightinferior" },
    { 0x2089, "nineinferior" },
    { 0x208D, "parenleftinferior" },
    { 0x208E, "parenrightinferior" },
    { 0x20A1, "colonmonetary" },
    { 0x20A3, "franc" },
    { 0x20A4, "lira" },
    { 0x20A7, "peseta" },
    { 0x20AA, "afii57636" },
    { 0x20AB, "dong" },
    { 0x20AC, "Euro" },
    { 0x2105, "afii61248" },
    { 0x2111, "Ifraktur" },
    { 0x2113, "afii61289" },
    { 0x2116, "afii61352" },
    { 0x2118, "weierstrass" },
    { 0x211C, "Rfraktur" },
    { 0x211E, "prescription" },
    { 0x2122, "trademark" },
    { 0x2126, "Omega" },
    { 0x212E, "estimated" },
    { 0x2135, "aleph" },
    { 0x2153, "onethird" },
    { 0x2154, "twothirds" },
    { 0x215B, "oneeighth" },
    { 0x215C, "threeeighths" },
    { 0x215D, "fiveeighths" },
    { 0x215E, "seveneighths" },
    { 0x2190, "arrowleft" },
    { 0x2191, "arrowup" },
    { 0x2192, "arrowright" },
    { 0x2193, "arrowdown" },
    { 0x2194, "arrowboth" },
    { 0x2195, "arrowupdn" },
    { 0x21A8, "arrowupdnbse" },
    { 0x21B5, "carriagereturn" },
    { 0x21D0, "arrowdblleft" },
    { 0x21D1, "arrowdblup" },
    { 0x21D2, "arrowdblright" },
    { 0x21D3, "arrowdbldown" },
    { 0x21D4, "arrowdblboth" },
    { 0x2200, "universal" },
    { 0x2202, "partialdiff" },
    { 0x2203, "existential" },
    { 0x2205, "emptyset" },
    { 0x2206, "Delta" },
    { 0x2207, "gradient" },
    { 0x2208, "element" },
    { 0x2209, "notelement" },
    { 0x220B, "suchthat" },
    { 0x220F, "product" },
    { 0x2211, "summation" },
    { 0x2212, "minus" },
    { 0x2215, "fraction" },
    { 0x2217, "asteriskmath" },
    { 0x2219, "periodcentered" },
    { 0x221A, "radical" },
    { 0x221D, "proportional" },
    { 0x221E, "infinity" },
    { 0x221F, "orthogonal" },
    { 0x2220, "angle" },
    { 0x2227, "logicaland" },
    { 0x2228, "logicalor" },
    { 0x2229, "intersection" },
    { 0x222A, "union" },
    { 0x222B, "integral" },
    { 0x2234, "therefore" },
    { 0x223C, "similar" },
    { 0x2245, "congruent" },
    { 0x2248, "approxequal" },
    { 0x2260, "notequal" },
    { 0x2261, "equivalence" },
    { 0x2264, "lessequal" },
    { 0x2265, "greaterequal" },
    { 0x2282, "propersubset" },
    { 0x2283, "propersuperset" },
    { 0x2284, "notsubset" },
    { 0x2286, "reflexsubset" },
    { 0x2287, "reflexsuperset" },
    { 0x2295, "circleplus" },
    { 0x2297, "circlemultiply" },
    { 0x22A5, "perpendicular" },
    { 0x22C5, "dotmath" },
    { 0x2302, "house" },
    { 0x2310, "revlogicalnot" },
    { 0x2320, "integraltp" },
    { 0x2321, "integralbt" },
    { 0x2329, "angleleft" },
    { 0x232A, "angleright" },
    { 0x2500, "SF100000" },
    { 0x2502, "SF110000" },
    { 0x250C, "SF010000" },
    { 0x2510, "SF030000" },
    { 0x2514, "SF020000" },
    { 0x2518, "SF040000" },
    { 0x251C, "SF080000" },
    { 0x2524, "SF090000" },
    { 0x252C, "SF060000" },
    { 0x2534, "SF070000" },
    { 0x253C, "SF050000" },
    { 0x2550, "SF430000" },
    { 0x2551, "SF240000" },
    { 0x2552, "SF510000" },
    { 0x2553, "SF520000" },
    { 0x2554, "SF390000" },
    { 0x2555, "SF220000" },
    { 0x2556, "SF210000" },
    { 0x2557, "SF250000" },
    { 0x2558, "SF500000" },
    { 0x2559, "SF490000" },
    { 0x255A, "SF380000" },
    { 0x255B, "SF280000" },
    { 0x255C, "SF270000" },
    { 0x255D, "SF260000" },
    { 0x255E, "SF360000" },
    { 0x255F, "SF370000" },
    { 0x2560, "SF420000" },
    { 0x2561, "SF190000" },
    { 0x2562, "SF200000" },
    { 0x2563, "SF230000" },
    { 0x2564, "SF470000" },
    { 0x2565, "SF480000" },
    { 0x2566, "SF410000" },
    { 0x2567, "SF450000" },
    { 0x2568, "SF460000" },
    { 0x2569, "SF400000" },
    { 0x256A, "SF540000" },
    { 0x256B, "SF530000" },
    { 0x256C, "SF440000" },
    { 0x2580, "upblock" },
    { 0x2584, "dnblock" },
    { 0x2588, "block" },
    { 0x258C, "lfblock" },
    { 0x2590, "rtblock" },
    { 0x2591, "ltshade" },
    { 0x2592, "shade" },
    { 0x2593, "dkshade" },
    { 0x25A0, "filledbox" },
    { 0x25A1, "H22073" },
    { 0x25AA, "H18543" },
    { 0x25AB, "H18551" },
    { 0x25AC, "filledrect" },
    { 0x25B2, "triagup" },
    { 0x25BA, "triagrt" },
    { 0x25BC, "triagdn" },
    { 0x25C4, "triaglf" },
    { 0x25CA, "lozenge" },
    { 0x25CB, "circle" },
    { 0x25CF, "H18533" },
    { 0x25D8, "invbullet" },
    { 0x25D9, "invcircle" },
    { 0x25E6, "openbullet" },
    { 0x263A, "smileface" },
    { 0x263B, "invsmileface" },
    { 0x263C, "sun" },
    { 0x2640, "female" },
    { 0x2642, "male" },
    { 0x2660, "spade" },
    { 0x2663, "club" },
    { 0x2665, "heart" },
    { 0x2666, "diamond" },
    { 0x266A, "musicalnote" },
    { 0x266B, "musicalnotedbl" },
    { 0xF6BF, "LL" },
    { 0xF6C0, "ll" },
    { 0xF6C1, "Scedilla" },
    { 0xF6C2, "scedilla" },
    { 0xF6C3, "commaaccent" },
    { 0xF6C4, "afii10063" },
    { 0xF6C5, "afii10064" },
    { 0xF6C6, "afii10192" },
    { 0xF6C7, "afii10831" },
    { 0xF6C8, "afii10832" },
    { 0xF6C9, "Acute" },
    { 0xF6CA, "Caron" },
    { 0xF6CB, "Dieresis" },
    { 0xF6CC, "DieresisAcute" },
    { 0xF6CD, "DieresisGrave" },
    { 0xF6CE, "Grave" },
    { 0xF6CF, "Hungarumlaut" },
    { 0xF6D0, "Macron" },
    { 0xF6D1, "cyrBreve" },
    { 0xF6D2, "cyrFlex" },
    { 0xF6D3, "dblGrave" },
    { 0xF6D4, "cyrbreve" },
    { 0xF6D5, "cyrflex" },
    { 0xF6D6, "dblgrave" },
    { 0xF6D7, "dieresisacute" },
    { 0xF6D8, "dieresisgrave" },
    { 0xF6D9, "copyrightserif" },
    { 0xF6DA, "registerserif" },
    { 0xF6DB, "trademarkserif" },
    { 0xF6DC, "onefitted" },
    { 0xF6DD, "rupiah" },
    { 0xF6DE, "threequartersemdash" },
    { 0xF6DF, "centinferior" },
    { 0xF6E0, "centsuperior" },
    { 0xF6E1, "commainferior" },
    { 0xF6E2, "commasuperior" },
    { 0xF6E3, "dollarinferior" },
    { 0xF6E4, "dollarsuperior" },
    { 0xF6E5, "hypheninferior" },
    { 0xF6E6, "hyphensuperior" },
    { 0xF6E7, "periodinferior" },
    { 0xF6E8, "periodsuperior" },
    { 0xF6E9, "asuperior" },
    { 0xF6EA, "bsuperior" },
    { 0xF6EB, "dsuperior" },
    { 0xF6EC, "esuperior" },
    { 0xF6ED, "isuperior" },
    { 0xF6EE, "lsuperior" },
    { 0xF6EF, "msuperior" },
    { 0xF6F0, "osuperior" },
    { 0xF6F1, "rsuperior" },
    { 0xF6F2, "ssuperior" },
    { 0xF6F3, "tsuperior" },
    { 0xF6F4, "Brevesmall" },
    { 0xF6F5, "Caronsmall" },
    { 0xF6F6, "Circumflexsmall" },
    { 0xF6F7, "Dotaccentsmall" },
    { 0xF6F8, "Hungarumlautsmall" },
    { 0xF6F9, "Lslashsmall" },
    { 0xF6FA, "OEsmall" },
    { 0xF6FB, "Ogoneksmall" },
    { 0xF6FC, "Ringsmall" },
    { 0xF6FD, "Scaronsmall" },
    { 0xF6FE, "Tildesmall" },
    { 0xF6FF, "Zcaronsmall" },
    { 0xF721, "exclamsmall" },
    { 0xF724, "dollaroldstyle" },
    { 0xF726, "ampersandsmall" },
    { 0xF730, "zerooldstyle" },
    { 0xF731, "oneoldstyle" },
    { 0xF732, "twooldstyle" },
    { 0xF733, "threeoldstyle" },
    { 0xF734, "fouroldstyle" },
    { 0xF735, "fiveoldstyle" },
    { 0xF736, "sixoldstyle" },
    { 0xF737, "sevenoldstyle" },
    { 0xF738, "eightoldstyle" },
    { 0xF739, "nineoldstyle" },
    { 0xF73F, "questionsmall" },
    { 0xF760, "Gravesmall" },
    { 0xF761, "Asmall" },
    { 0xF762, "Bsmall" },
    { 0xF763, "Csmall" },
    { 0xF764, "Dsmall" },
    { 0xF765, "Esmall" },
    { 0xF766, "Fsmall" },
    { 0xF767, "Gsmall" },
    { 0xF768, "Hsmall" },
    { 0xF769, "Ismall" },
    { 0xF76A, "Jsmall" },
    { 0xF76B, "Ksmall" },
    { 0xF76C, "Lsmall" },
    { 0xF76D, "Msmall" },
    { 0xF76E, "Nsmall" },
    { 0xF76F, "Osmall" },
    { 0xF770, "Psmall" },
    { 0xF771, "Qsmall" },
    { 0xF772, "Rsmall" },
    { 0xF773, "Ssmall" },
    { 0xF774, "Tsmall" },
    { 0xF775, "Usmall" },
    { 0xF776, "Vsmall" },
    { 0xF777, "Wsmall" },
    { 0xF778, "Xsmall" },
    { 0xF779, "Ysmall" },
    { 0xF77A, "Zsmall" },
    { 0xF7A1, "exclamdownsmall" },
    { 0xF7A2, "centoldstyle" },
    { 0xF7A8, "Dieresissmall" },
    { 0xF7AF, "Macronsmall" },
    { 0xF7B4, "Acutesmall" },
    { 0xF7B8, "Cedillasmall" },
    { 0xF7BF, "questiondownsmall" },
    { 0xF7E0, "Agravesmall" },
    { 0xF7E1, "Aacutesmall" },
    { 0xF7E2, "Acircumflexsmall" },
    { 0xF7E3, "Atildesmall" },
    { 0xF7E4, "Adieresissmall" },
    { 0xF7E5, "Aringsmall" },
    { 0xF7E6, "AEsmall" },
    { 0xF7E7, "Ccedillasmall" },
    { 0xF7E8, "Egravesmall" },
    { 0xF7E9, "Eacutesmall" },
    { 0xF7EA, "Ecircumflexsmall" },
    { 0xF7EB, "Edieresissmall" },
    { 0xF7EC, "Igravesmall" },
    { 0xF7ED, "Iacutesmall" },
    { 0xF7EE, "Icircumflexsmall" },
    { 0xF7EF, "Idieresissmall" },
    { 0xF7F0, "Ethsmall" },
    { 0xF7F1, "Ntildesmall" },
    { 0xF7F2, "Ogravesmall" },
    { 0xF7F3, "Oacutesmall" },
    { 0xF7F4, "Ocircumflexsmall" },
    { 0xF7F5, "Otildesmall" },
    { 0xF7F6, "Odieresissmall" },
    { 0xF7F8, "Oslashsmall" },
    { 0xF7F9, "Ugravesmall" },
    { 0xF7FA, "Uacutesmall" },
    { 0xF7FB, "Ucircumflexsmall" },
    { 0xF7FC, "Udieresissmall" },
    { 0xF7FD, "Yacutesmall" },
    { 0xF7FE, "Thornsmall" },
    { 0xF7FF, "Ydieresissmall" },
    { 0xF8E5, "radicalex" },
    { 0xF8E6, "arrowvertex" },
    { 0xF8E7, "arrowhorizex" },
    { 0xF8E8, "registersans" },
    { 0xF8E9, "copyrightsans" },
    { 0xF8EA, "trademarksans" },
    { 0xF8EB, "parenlefttp" },
    { 0xF8EC, "parenleftex" },
    { 0xF8ED, "parenleftbt" },
    { 0xF8EE, "bracketlefttp" },
    { 0xF8EF, "bracketleftex" },
    { 0xF8F0, "bracketleftbt" },
    { 0xF8F1, "bracelefttp" },
    { 0xF8F2, "braceleftmid" },
    { 0xF8F3, "braceleftbt" },
    { 0xF8F4, "braceex" },
    { 0xF8F5, "integralex" },
    { 0xF8F6, "parenrighttp" },
    { 0xF8F7, "parenrightex" },
    { 0xF8F8, "parenrightbt" },
    { 0xF8F9, "bracketrighttp" },
    { 0xF8FA, "bracketrightex" },
    { 0xF8FB, "bracketrightbt" },
    { 0xF8FC, "bracerighttp" },
    { 0xF8FD, "bracerightmid" },
    { 0xF8FE, "bracerightbt" },
    { 0xFB00, "ff" },
    { 0xFB01, "fi" },
    { 0xFB02, "fl" },
    { 0xFB03, "ffi" },
    { 0xFB04, "ffl" },
    { 0xFB1F, "afii57705" },
    { 0xFB2A, "afii57694" },
    { 0xFB2B, "afii57695" },
    { 0xFB35, "afii57723" },
    { 0xFB4B, "afii57700" },
    // end of stuff from glyphlist.txt
    { 0xFFFF, 0 }
};


static struct {
    QFont::CharSet cs;
    Q_UINT16 values[128];
} unicodevalues[] = {
    // from RFC 1489, http://ds.internic.net/rfc/rfc1489.txt
    { QFont::KOI8R,
      { 0x2500, 0x2502, 0x250C, 0x2510, 0x2514, 0x2518, 0x251C, 0x2524,
	0x252C, 0x2534, 0x253C, 0x2580, 0x2584, 0x2588, 0x258C, 0x2590,
	0x2591, 0x2592, 0x2593, 0x2320, 0x25A0, 0x2219, 0x221A, 0x2248,
	0x2264, 0x2265, 0x00A0, 0x2321, 0x00B0, 0x00B2, 0x00B7, 0x00F7,
	0x2550, 0x2551, 0x2552, 0x0451, 0x2553, 0x2554, 0x2555, 0x2556,
	0x2557, 0x2558, 0x2559, 0x255A, 0x255B, 0x255C, 0x255D, 0x255E,
	0x255F, 0x2560, 0x2561, 0x0401, 0x2562, 0x2563, 0x2564, 0x2565,
	0x2566, 0x2567, 0x2568, 0x2569, 0x256A, 0x256B, 0x256C, 0x00A9,
	0x044E, 0x0430, 0x0431, 0x0446, 0x0434, 0x0435, 0x0444, 0x0433,
	0x0445, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E,
	0x043F, 0x044F, 0x0440, 0x0441, 0x0442, 0x0443, 0x0436, 0x0432,
	0x044C, 0x044B, 0x0437, 0x0448, 0x044D, 0x0449, 0x0447, 0x044A,
	0x042E, 0x0410, 0x0411, 0x0426, 0x0414, 0x0415, 0x0424, 0x0413,
	0x0425, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E,
	0x041F, 0x042F, 0x0420, 0x0421, 0x0422, 0x0423, 0x0416, 0x0412,
	0x042C, 0x042B, 0x0417, 0x0428, 0x042D, 0x0429, 0x0427, 0x042A } },

    // next bits generated from tables on the Unicode 2.0 CD.  we can
    // use these tables since this is part of the transition to using
    // unicode everywhere in qt.

    // $ for A in 8 9 A B C D E F ; do for B in 0 1 2 3 4 5 6 7 8 9 A B C D E F ; do echo 0x${A}${B} 0xFFFD ; done ; done > /tmp/digits ; for a in 8859-* ; do ( awk '/^0x[89ABCDEF]/{ print $1, $2 }' < $a ; cat /tmp/digits ) | sort | uniq -w4 | cut -c6- | paste '-d ' - - - - - - - - | sed -e 's/ /, /g' -e 's/$/,/' -e '$ s/,$/} },/' -e '1 s/^/{ /' > /tmp/$a ; done

    // then I inserted the files manually.
    { QFont::Latin1,
      { 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
	0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
	0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
	0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
	0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
	0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
	0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
	0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
	0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
	0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
	0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
	0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF } },
    { QFont::Latin2,
      { 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0x00A0, 0x0104, 0x02D8, 0x0141, 0x00A4, 0x013D, 0x015A, 0x00A7,
	0x00A8, 0x0160, 0x015E, 0x0164, 0x0179, 0x00AD, 0x017D, 0x017B,
	0x00B0, 0x0105, 0x02DB, 0x0142, 0x00B4, 0x013E, 0x015B, 0x02C7,
	0x00B8, 0x0161, 0x015F, 0x0165, 0x017A, 0x02DD, 0x017E, 0x017C,
	0x0154, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139, 0x0106, 0x00C7,
	0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD, 0x00CE, 0x010E,
	0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7,
	0x0158, 0x016E, 0x00DA, 0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF,
	0x0155, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7,
	0x010D, 0x00E9, 0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F,
	0x0111, 0x0144, 0x0148, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x00F7,
	0x0159, 0x016F, 0x00FA, 0x0171, 0x00FC, 0x00FD, 0x0163, 0x02D9 } },
    { QFont::Latin3,
      { 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0x00A0, 0x0126, 0x02D8, 0x00A3, 0x00A4, 0xFFFD, 0x0124, 0x00A7,
	0x00A8, 0x0130, 0x015E, 0x011E, 0x0134, 0x00AD, 0xFFFD, 0x017B,
	0x00B0, 0x0127, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x0125, 0x00B7,
	0x00B8, 0x0131, 0x015F, 0x011F, 0x0135, 0x00BD, 0xFFFD, 0x017C,
	0x00C0, 0x00C1, 0x00C2, 0xFFFD, 0x00C4, 0x010A, 0x0108, 0x00C7,
	0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
	0xFFFD, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x0120, 0x00D6, 0x00D7,
	0x011C, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x016C, 0x015C, 0x00DF,
	0x00E0, 0x00E1, 0x00E2, 0xFFFD, 0x00E4, 0x010B, 0x0109, 0x00E7,
	0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
	0xFFFD, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x0121, 0x00F6, 0x00F7,
	0x011D, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x016D, 0x015D, 0x02D9 } },
    { QFont::Latin4,
      { 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0x00A0, 0x0104, 0x0138, 0x0156, 0x00A4, 0x0128, 0x013B, 0x00A7,
	0x00A8, 0x0160, 0x0112, 0x0122, 0x0166, 0x00AD, 0x017D, 0x00AF,
	0x00B0, 0x0105, 0x02DB, 0x0157, 0x00B4, 0x0129, 0x013C, 0x02C7,
	0x00B8, 0x0161, 0x0113, 0x0123, 0x0167, 0x014A, 0x017E, 0x014B,
	0x0100, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x012E,
	0x010C, 0x00C9, 0x0118, 0x00CB, 0x0116, 0x00CD, 0x00CE, 0x012A,
	0x0110, 0x0145, 0x014C, 0x0136, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
	0x00D8, 0x0172, 0x00DA, 0x00DB, 0x00DC, 0x0168, 0x016A, 0x00DF,
	0x0101, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x012F,
	0x010D, 0x00E9, 0x0119, 0x00EB, 0x0117, 0x00ED, 0x00EE, 0x012B,
	0x0111, 0x0146, 0x014D, 0x0137, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
	0x00F8, 0x0173, 0x00FA, 0x00FB, 0x00FC, 0x0169, 0x016B, 0x02D9 } },
    { QFont::Latin5,
      { 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0x00A0, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407,
	0x0408, 0x0409, 0x040A, 0x040B, 0x040C, 0x00AD, 0x040E, 0x040F,
	0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
	0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
	0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
	0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
	0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
	0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
	0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
	0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F,
	0x2116, 0x0451, 0x0452, 0x0453, 0x0454, 0x0455, 0x0456, 0x0457,
	0x0458, 0x0459, 0x045A, 0x045B, 0x045C, 0x00A7, 0x045E, 0x045F } },
    { QFont::Latin6,
      { 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0x00A0, 0xFFFD, 0xFFFD, 0xFFFD, 0x00A4, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x060C, 0x00AD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0x061B, 0xFFFD, 0xFFFD, 0xFFFD, 0x061F,
	0xFFFD, 0x0621, 0x0622, 0x0623, 0x0624, 0x0625, 0x0626, 0x0627,
	0x0628, 0x0629, 0x062A, 0x062B, 0x062C, 0x062D, 0x062E, 0x062F,
	0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635, 0x0636, 0x0637,
	0x0638, 0x0639, 0x063A, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0x0640, 0x0641, 0x0642, 0x0643, 0x0644, 0x0645, 0x0646, 0x0647,
	0x0648, 0x0649, 0x064A, 0x064B, 0x064C, 0x064D, 0x064E, 0x064F,
	0x0650, 0x0651, 0x0652, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD } },
    { QFont::Latin7,
      { 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0x00A0, 0x02BD, 0x02BC, 0x00A3, 0xFFFD, 0xFFFD, 0x00A6, 0x00A7,
	0x00A8, 0x00A9, 0xFFFD, 0x00AB, 0x00AC, 0x00AD, 0xFFFD, 0x2015,
	0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x0384, 0x0385, 0x0386, 0x00B7,
	0x0388, 0x0389, 0x038A, 0x00BB, 0x038C, 0x00BD, 0x038E, 0x038F,
	0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397,
	0x0398, 0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F,
	0x03A0, 0x03A1, 0xFFFD, 0x03A3, 0x03A4, 0x03A5, 0x03A6, 0x03A7,
	0x03A8, 0x03A9, 0x03AA, 0x03AB, 0x03AC, 0x03AD, 0x03AE, 0x03AF,
	0x03B0, 0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03B6, 0x03B7,
	0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BE, 0x03BF,
	0x03C0, 0x03C1, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7,
	0x03C8, 0x03C9, 0x03CA, 0x03CB, 0x03CC, 0x03CD, 0x03CE, 0xFFFD } },
    { QFont::Latin8,
      { 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0x00A0, 0xFFFD, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
	0x00A8, 0x00A9, 0x00D7, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x203E,
	0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
	0x00B8, 0x00B9, 0x00F7, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x2017,
	0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7,
	0x05D8, 0x05D9, 0x05DA, 0x05DB, 0x05DC, 0x05DD, 0x05DE, 0x05DF,
	0x05E0, 0x05E1, 0x05E2, 0x05E3, 0x05E4, 0x05E5, 0x05E6, 0x05E7,
	0x05E8, 0x05E9, 0x05EA, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD } },
    // makeFixedStrings() below assumes that latin9 is last
    { QFont::Latin9,
      { 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
	0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
	0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
	0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
	0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
	0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
	0x011E, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
	0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x0130, 0x015E, 0x00DF,
	0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
	0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
	0x011F, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
	0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x0131, 0x015F, 0x00FF } },
};




static char * fixed_ps_header = 0;
static QIntDict<QString> * font_vectors = 0;

static void cleanup()
{
    delete[] fixed_ps_header;
    fixed_ps_header = 0;
    delete font_vectors;
    font_vectors = 0;
}


static void wordwrap( char * s )
{
    int ip = 0, ilp = 0, op = 0, olp = 0, oline = 0;
    bool needws = FALSE, insertws = FALSE;

    while( s[ip] ) {
	if ( ilp && op - oline > 79 ) {
	    // we have a possible line start position and a long line - let's
	    // use it.
	    ip = ilp;
	    s[olp] = '\n';
	    op = olp;
	    op++;
	    oline = op;
	    needws = FALSE;
	}

	if ( isspace( s[ip] ) ) {
	    if ( needws )
		insertws = TRUE;
	    olp = op;
	    ilp = ip++;
	    needws = FALSE;
	} else if ( s[ip] == '/' || s[ip] == '{' || s[ip] == '}' ||
		    s[ip] == '[' || s[ip] == ']' ) {
	    if ( insertws ) {
		// if there was whitespace, we can start a new line here
		ilp = ip;
		olp = op;
		// but we don't need whitespace before it
		insertws = FALSE;
	    }
	    // don't need ws after it either
	    needws = FALSE;
	    s[op++] = s[ip++];
	} else {
	    if ( insertws ) {
		ilp = ip;
		olp = op;
		s[op++] = ' ';
		insertws = FALSE;
	    }
	    needws = TRUE;
	    s[op++] = s[ip++];
	}
    }
    s[op] = '\0';
}


static void makeFixedStrings()
{
    if ( fixed_ps_header )
	return;
    qAddPostRoutine( cleanup );

    {
	QString psh;
	const char** l = ps_header;
	while (*l) {
	    psh += *l++;
	    psh += '\n';
	}
	fixed_ps_header = qstrdup( psh );
    }
    wordwrap( fixed_ps_header );

    // fonts.
    font_vectors = new QIntDict<QString>( 17 );
    font_vectors->setAutoDelete( TRUE );

    int i = 0;
    int k;
    int l = 0; // unicode to glyph accumulator
    QString vector;
    const char * glyphname;
    do {
	vector.sprintf( "/FE%d [", (int)unicodevalues[i].cs );
	glyphname = 0;
	l = 0;
	for( k=0; k<128; k++ ) {
	    while( unicodetoglyph[l].u < k )
		l++;
	    if ( unicodetoglyph[l].u == k )
		glyphname = unicodetoglyph[l].g;
	    else
		glyphname = ".notdef";
	    vector += " /";
	    vector += glyphname;
	}
	for( k=0; k<128; k++ ) {
	    if ( unicodevalues[i].values[k] == 0xFFFD ) {
		glyphname = ".notdef";
	    } else {
		if ( l && unicodetoglyph[l].u > unicodevalues[i].values[k] )
		    l = 0;
		while( unicodetoglyph[l].u < unicodevalues[i].values[k] )
		    l++;
		if ( unicodetoglyph[l].u == unicodevalues[i].values[k] )
		    glyphname = unicodetoglyph[l].g;
		else
		    glyphname = ".notdef";
	    }
	    vector += " /";
	    vector += glyphname;
	}
	vector += " ] def";
	wordwrap( vector.data() );
	font_vectors->insert( (int)(unicodevalues[i].cs),
			      new QString( vector ) );
	vector.detach(); // ### remove in 2.0
    } while ( unicodevalues[i++].cs != QFont::Latin9 );
}


struct QPSPrinterPrivate {
    QPSPrinterPrivate( int filedes )
	: buffer( 0 ), realDevice( 0 ), fd( filedes ), savedImage( 0 )
    {
	headerFontNames.setAutoDelete( TRUE );
	pageFontNames.setAutoDelete( TRUE );
	headerEncodings.setAutoDelete( FALSE );
	pageEncodings.setAutoDelete( FALSE );
    }

    QBuffer * buffer;
    int pagesInBuffer;
    QIODevice * realDevice;
    int fd;
    QDict<QString> headerFontNames;
    QDict<QString> pageFontNames;
    QIntDict<void> headerEncodings;
    QIntDict<void> pageEncodings;
    int headerFontNumber;
    int pageFontNumber;
    QBuffer * fontBuffer;
    QTextStream fontStream;
    bool dirtyClipping;
    bool firstClipOnPage;
    QRect boundingBox;
    QImage * savedImage;
};



QPSPrinter::QPSPrinter( QPrinter *prt, int fd )
    : QPaintDevice( PDT_PRINTER | PDF_EXTDEV )
{
    printer = prt;
    d = new QPSPrinterPrivate( fd );
}


QPSPrinter::~QPSPrinter()
{
    if ( d->fd >= 0 )
	::close( d->fd );
    delete d;
}


static struct {
    const char * input;
    const char * roman;
    const char * italic;
    const char * bold;
    const char * boldItalic;
    const char * light;
    const char * lightItalic;
} postscriptFontNames[] = {
    { "avantgarde", "AvantGarde-Book", 0, 0, 0, 0, 0 },
    { "charter", "CharterBT-Roman", 0, 0, 0, 0, 0 },
    { "garamond", "Garamond-Regular", 0, 0, 0, 0, 0 },
    { "gillsans", "GillSans", 0, 0, 0, 0, 0 },
    { "helvetica",
      "Helvetica", "Helvetica-Oblique",
      "Helvetica-Bold", "Helvetica-BoldOblique",
      "Helvetica", "Helvetica-Oblique" },
    { "new century schoolbook", "NewCenturySchlbk-Roman", 0, 0, 0, 0, 0 },
    { "symbol", "Symbol", "Symbol", "Symbol", "Symbol", "Symbol", "Symbol" },
    { "terminal", "Courier", 0, 0, 0, 0, 0 },
    { "utopia", "Utopia-Regular", 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 }
};


//static void ps_setFont( QTextStream *s, const QFont *f, QString *fonts )
void QPSPrinter::setFont( const QFont & f )
{
    if ( f.rawMode() ) {
	QFont fnt( "Helvetica", 12 );
	setFont( fnt );
	return;
    }
    if ( f.pointSize() == 0 ) {
#if defined(CHECK_RANGE)
	warning( "QPrinter: Cannot set a font with zero point size." );
#endif
	return;
    }

    if ( !fixed_ps_header )
	makeFixedStrings();

    QString family = f.family();
    QString ps;
    int	 weight = f.weight();
    bool italic = f.italic();

    family = family.lower();

    int i;

    // try to make a "good" postscript name
    ps = family.simplifyWhiteSpace();
    i = 0;
    while( (unsigned int)i < ps.length() ) {
	if ( i == 0 || ps[i-1] == ' ' ) {
	    ps[i] = toupper( ps[i] );
	    if ( i )
		ps.remove( i-1, 1 );
	    else
		i++;
	} else {
	    i++;
	}
    }

    // see if the table has a better name
    i = 0;
    while( postscriptFontNames[i].input &&
	   qstrcmp( postscriptFontNames[i].input, family ) )
	i++;
    if ( postscriptFontNames[i].roman ) {
	ps = postscriptFontNames[i].roman;
	int p = ps.find( '-' );
	if ( p > -1 )
	    ps.truncate( p );
    }

    // get the right modification, or build something
    if ( weight >= QFont::Bold && italic ) {
	if ( postscriptFontNames[i].boldItalic )
	    ps = postscriptFontNames[i].boldItalic;
	else
	    ps.append( "-BoldItalic" );
    } else if ( weight >= QFont::Bold ) {
	if ( postscriptFontNames[i].bold )
	    ps = postscriptFontNames[i].bold;
	else
	    ps.append( "-Bold" );
    } else if ( weight >= QFont::DemiBold && italic ) {
	if ( postscriptFontNames[i].italic )
	    ps = postscriptFontNames[i].italic;
	else
	    ps.append( "-Italic" ); // possibly suboptimal
    } else if ( weight <= QFont::Light && italic ) {
	if ( postscriptFontNames[i].lightItalic )
	    ps = postscriptFontNames[i].lightItalic;
	else
	    ps.append( "-LightItalic" );
    } else if ( weight <= QFont::Light ) {
	if ( postscriptFontNames[i].light )
	    ps = postscriptFontNames[i].light;
	else
	    ps.append( "-Light" );
    } else if ( italic ) {
	if ( postscriptFontNames[i].italic )
	    ps = postscriptFontNames[i].italic;
	else
	    ps.append( "-Italic" );
    } else {
	if ( postscriptFontNames[i].roman )
	    ps = postscriptFontNames[i].roman;
	else
	    ps.append( "-Roman" );
    }

    QString key;
    int cs = (int)f.charSet();
    if ( cs == QFont::AnyCharSet ) {
	QIntDictIterator<void> it( d->headerEncodings );
	if ( it.current() )
	    cs = it.currentKey();
	else
	    cs = QFont::Latin1;
    }

    key.sprintf( "%s %d %d", ps.data(), f.pointSize(), cs );
    QString * tmp;
    tmp = d->headerFontNames.find( key );
    if ( !tmp && !d->buffer )
	tmp = d->pageFontNames.find( key );

    QString fontName;
    if ( tmp )
	fontName = *tmp;

    if ( fontName.isEmpty() ) {
	QString key2;
	key2.sprintf( "%s %d", ps.data(), cs );
	tmp = d->headerFontNames.find( key );

	QString fontEncoding;
	fontEncoding.sprintf( " FE%d", cs );
	if ( d->buffer ) {
	    if ( !d->headerEncodings.find( cs ) ) {
		QString * vector = font_vectors->find( cs );
		if ( vector ) {
		    d->fontStream << *vector << "\n";
		    d->headerEncodings.insert( cs, (void*)42 );
		} else {
		    d->fontStream << "% wanted font encoding "
				  << cs << "\n";
		}
	    }
	    if ( tmp ) {
		fontName = *tmp;
	    } else {
		fontName.sprintf( "/F%d", ++d->headerFontNumber );
		d->fontStream << fontName << fontEncoding << "/"
			      << ps << " MF\n";
		d->headerFontNames.insert( key2, new QString( fontName ) );
	    }
	    ++d->headerFontNumber;
	    d->fontStream << "/F" << d->headerFontNumber << " "
			  << f.pointSize() << fontName << " DF\n";
	    fontName.sprintf( "F%d", d->headerFontNumber );
	    d->headerFontNames.insert( key, new QString( fontName ) );
	} else {
	    if ( !d->headerEncodings.find( cs ) &&
		 !d->pageEncodings.find( cs ) ) {
		QString * vector = font_vectors->find( cs );
		if ( !vector )
		    vector = font_vectors->find( QFont::Latin1 );
		stream << *vector << "\n";
		d->pageEncodings.insert( cs, (void*)42 );
	    }
	    if ( !tmp )
		tmp = d->pageFontNames.find( key );
	    if ( tmp ) {
		fontName = *tmp;
	    } else {
		fontName.sprintf( "/F%d", ++d->pageFontNumber );
		stream << fontName << fontEncoding << "/" << ps << " MF\n";
		d->pageFontNames.insert( key2, new QString( fontName ) );
	    }
	    ++d->pageFontNumber;
	    stream << "/F" << d->pageFontNumber << " "
		   << f.pointSize() << fontName << " DF\n";
	    fontName.sprintf( "F%d", d->pageFontNumber );
	    d->pageFontNames.insert( key, new QString( fontName ) );
	}
    }
    stream << fontName << " F\n";

    ps.append( " " );
    ps.prepend( " " );
    if ( !fontsUsed.contains( ps ) )
	fontsUsed += ps;
}


static void hexOut( QTextStream &stream, int i )
{
    if ( i < 0x10 )
	stream << '0';
    stream << i;
}


static void ps_dumpTransparentBitmapData( QTextStream &stream,
					  const QImage &img )
{
    stream.setf( QTextStream::hex, QTextStream::basefield );

    int width  = img.width();
    int height = img.height();
    int numBytes = (width + 7)/8;
    uchar *scanLine;
    int x,y;
    int count = -1;
    for( y = 0 ; y < height ; y++ ) {
	scanLine = img.scanLine(y);
	for( x = 0 ; x < numBytes ; x++ ) {
	    hexOut( stream, scanLine[x] );
	    if ( !(count++ % 66) )
		stream << '\n';
	}
    }
    if ( --count % 66 )
	stream << '\n';

    stream.setf( QTextStream::dec, QTextStream::basefield );
}


static void ps_dumpPixmapData( QTextStream &stream, QImage img,
			       const QColor fgCol, const QColor bgCol )
{
    stream.setf( QTextStream::hex, QTextStream::basefield );

    if ( img.depth() == 1 ) {
	img = img.convertDepth( 8 );
	if ( img.color(0) == 0 ) {			// black
	    img.setColor( 0, fgCol.rgb() );
	    img.setColor( 1, bgCol.rgb() );
	} else {
	    img.setColor( 0, bgCol.rgb() );
	    img.setColor( 1, fgCol.rgb() );
	}
    }

    int width  = img.width();
    int height = img.height();
    int pixWidth = img.depth() == 8 ? 1 : 4;
    uchar *scanLine;
    uint cval;
    int x,y;
    int count = -1;
    for( y = 0 ; y < height ; y++ ) {
	scanLine = img.scanLine(y);
	for( x = 0 ; x < width ; x++ ) {
	    if ( pixWidth == 1 ) {
		cval = img.color( scanLine[x] );
	    } else {
		cval = ((QRgb*) scanLine)[x];
	    }
	    hexOut( stream, qRed(cval) );
	    hexOut( stream, qGreen(cval) );
	    hexOut( stream, qBlue(cval) );
	    if ( !(count++ % 13) )
		stream << '\n';
	}
    }
    if ( --count % 13 )
	stream << '\n';

    stream.setf( QTextStream::dec, QTextStream::basefield );
}

#undef XCOORD
#undef YCOORD
#undef WIDTH
#undef HEIGHT
#undef POINT
#undef RECT
#undef INT_ARG
#undef COLOR

#define XCOORD(x)	(float)(x)
#define YCOORD(y)	(float)(y)
#define WIDTH(w)	(float)(w)
#define HEIGHT(h)	(float)(h)

#define POINT(index)	XCOORD(p[index].point->x()) << ' ' <<		\
			YCOORD(p[index].point->y()) << ' '
#define RECT(index)	XCOORD(p[index].rect->x())  << ' ' <<		\
			YCOORD(p[index].rect->y())  << ' ' <<		\
			WIDTH (p[index].rect->width())	<< ' ' <<	\
			HEIGHT(p[index].rect->height()) << ' '
#define INT_ARG(index)	p[index].ival << ' '
#define COLOR(x)	(x).red()   << ' ' <<	\
			(x).green() << ' ' <<	\
			(x).blue()  << ' '


bool QPSPrinter::cmd( int c , QPainter *paint, QPDevCmdParam *p )
{
    if ( c == PDC_BEGIN ) {		// start painting
	d->pagesInBuffer = 0;
	d->buffer = new QBuffer();
	d->buffer->open( IO_WriteOnly );
	stream.setDevice( d->buffer );
	d->fontBuffer = new QBuffer();
	d->fontBuffer->open( IO_WriteOnly );
	d->fontStream.setDevice( d->fontBuffer );
	d->headerFontNumber = 0;
	pageCount           = 1;		// initialize state
	dirtyMatrix         = TRUE;
	d->dirtyClipping    = FALSE;		// No clipping is default.
	dirtyNewPage        = FALSE;		// setup done by QPainter
	                                        // for the first page.
	d->firstClipOnPage  = TRUE;
	d->boundingBox = QRect( 0, 0, -1, -1 );
	fontsUsed = "";

	stream << "%%Page: " << pageCount << ' ' << pageCount << endl
	       << "QI\n";
	orientationSetup();
	stream << "GS\n";
	return TRUE;
    }

    if ( c == PDC_END ) {			// painting done
	bool pageCountAtEnd = (d->buffer == 0);
	if ( !pageCountAtEnd )
	    emitHeader( TRUE );
	stream << "GR\n"
	       << "QP\n"
	       << "%%Trailer\n";
	if ( pageCountAtEnd )
	    stream << "%%Pages: " << pageCount << "\n%%DocumentFonts: "
		   << fontsUsed.simplifyWhiteSpace() << '\n';
	stream.unsetDevice();
	d->realDevice->close();
	if ( d->fd >= 0 )
	    ::close( d->fd );
	d->fd = -1;
	delete d->realDevice;
	d->realDevice = 0;
    }

    if ( c >= PDC_DRAW_FIRST && c <= PDC_DRAW_LAST ) {
	if ( dirtyMatrix )
	    matrixSetup( paint );
	if ( dirtyNewPage )
	    newPageSetup( paint );
	if ( d->dirtyClipping )	// Must be after matrixSetup and newPageSetup
	    clippingSetup( paint );
    }

    switch( c ) {
	case PDC_DRAWPOINT:
	    stream << POINT(0) << "P\n";
	    break;
	case PDC_MOVETO:
	    stream << POINT(0) << "M\n";
	    break;
	case PDC_LINETO:
	    stream << POINT(0) << "L\n";
	    break;
	case PDC_DRAWLINE:
	    stream << POINT(0) << POINT(1) << "DL\n";
	    break;
	case PDC_DRAWRECT:
	    stream << RECT(0) << "R\n";
	    break;
	case PDC_DRAWROUNDRECT:
	    stream << RECT(0) << INT_ARG(1) << INT_ARG(2) << "RR\n";
	    break;
	case PDC_DRAWELLIPSE:
	    stream << RECT(0) << "E\n";
	    break;
	case PDC_DRAWARC:
	    stream << RECT(0) << INT_ARG(1) << INT_ARG(2) << "A\n";
	    break;
	case PDC_DRAWPIE:
	    stream << RECT(0) << INT_ARG(1) << INT_ARG(2) << "PIE\n";
	    break;
	case PDC_DRAWCHORD:
	    stream << RECT(0) << INT_ARG(1) << INT_ARG(2) << "CH\n";
	    break;
	case PDC_DRAWLINESEGS:
	    if ( p[0].ptarr->size() > 0 ) {
		QPointArray a = *p[0].ptarr;
		QPoint pt;
		stream << "NP\n";
		for ( int i=0; i<(int)a.size(); i+=2 ) {
		    pt = a.point( i );
		    stream << XCOORD(pt.x()) << ' '
			   << YCOORD(pt.y()) << " MT\n";
		    pt = a.point( i+1 );
		    stream << XCOORD(pt.x()) << ' '
			   << YCOORD(pt.y()) << " LT\n";
		}
		stream << "QS\n";
	    }
	    break;
	case PDC_DRAWPOLYLINE:
	    if ( p[0].ptarr->size() > 1 ) {
		QPointArray a = *p[0].ptarr;
		QPoint pt = a.point( 0 );
		stream << "NP\n"
		       << XCOORD(pt.x()) << ' ' << YCOORD(pt.y()) << " MT\n";
		for ( int i=1; i<(int)a.size(); i++ ) {
		    pt = a.point( i );
		    stream << XCOORD(pt.x()) << ' '
			   << YCOORD(pt.y()) << " LT\n";
		}
		stream << "QS\n";
	    }
	    break;
	case PDC_DRAWPOLYGON:
	    if ( p[0].ptarr->size() > 2 ) {
		QPointArray a = *p[0].ptarr;
		if ( p[1].ival )
		    stream << "/WFi true def\n";
		QPoint pt = a.point(0);
		stream << "NP\n";
		stream << XCOORD(pt.x()) << ' '
		       << YCOORD(pt.y()) << " MT\n";
		for( int i=1; i<(int)a.size(); i++) {
		    pt = a.point( i );
		    stream << XCOORD(pt.x()) << ' '
			   << YCOORD(pt.y()) << " LT\n";
		}
		stream << "CP PF QS\n";
		if ( p[1].ival )
		    stream << "/WFi false def\n";
	    }
	    break;
	case PDC_DRAWQUADBEZIER:
	    if ( p[0].ptarr->size() == 4 ) {
		stream << "NP\n";
		QPointArray a = *p[0].ptarr;
		stream << XCOORD(a[0].x()) << ' '
		       << YCOORD(a[0].y()) << " MT ";
		for ( int i=1; i<4; i++ ) {
		    stream << XCOORD(a[i].x()) << ' '
			   << YCOORD(a[i].y()) << ' ';
		}
		stream << "BZ\n";
	    }
	    break;
	case PDC_DRAWTEXT:
	    if ( p[1].str && strlen( p[1].str ) ) {
		char * tmp = new char[ strlen( p[1].str ) * 2 + 2 ];
#if defined(CHECK_NULL)
		CHECK_PTR( tmp );
#endif
		const char * from = p[1].str;
		char * to = tmp;
		while ( *from ) {
		    if ( *from == '\\' || *from == '(' || *from == ')' )
			*to++ = '\\';		// escape special chars
		    *to++ = *from++;
		}
		*to = '\0';
		stream<< "(" << tmp << ")" << POINT(0) << "T\n";
		delete [] tmp;
	    }
	    break;
	case PDC_DRAWTEXTFRMT:;
	    return FALSE;			// uses QPainter instead
	case PDC_DRAWPIXMAP: {
	    if ( p[1].pixmap->isNull() )
		break;
	    QPoint pnt = *(p[0].point);
	    QImage img;
	    img = *(p[1].pixmap);
	    drawImage( paint, pnt, img );
	    break;
	}
	case PDC_DRAWIMAGE: {
	    if ( p[1].image->isNull() )
		break;
	    QPoint pnt = *(p[0].point);
	    QImage img = *(p[1].image);
	    drawImage( paint, pnt, img );
	    break;
	}
	case PDC_SETBKCOLOR:
	    stream << COLOR(*(p[0].color)) << "BC\n";
	    break;
	case PDC_SETBKMODE:
	    if ( p[0].ival == TransparentMode )
		stream << "/OMo false def\n";
	    else
		stream << "/OMo true def\n";
	    break;
	case PDC_SETROP:
#if defined(DEBUG)
	    if ( p[0].ival != CopyROP )
		debug( "QPrinter: Raster operation setting not supported" );
#endif
	    break;
	case PDC_SETBRUSHORIGIN:
	    break;
	case PDC_SETFONT:
	    setFont( *(p[0].font) );
	    break;
	case PDC_SETPEN:
	    stream << (int)p[0].pen->style() << ' ' << p[0].pen->width()
		   << ' ' << COLOR(p[0].pen->color()) << "PE\n";
	    break;
	case PDC_SETBRUSH:
	    if ( p[0].brush->style() == CustomPattern ) {
#if defined(DEBUG)
		warning( "QPrinter: Pixmap brush not supported" );
#endif
		return FALSE;
	    }
	    stream << (int)p[0].brush->style()	 << ' '
		   << COLOR(p[0].brush->color()) << "B\n";
	    break;
	case PDC_SETTABSTOPS:
	case PDC_SETTABARRAY:
	    return FALSE;
	case PDC_SETUNIT:
	    break;
	case PDC_SETVXFORM:
	case PDC_SETWINDOW:
	case PDC_SETVIEWPORT:
	case PDC_SETWXFORM:
	case PDC_SETWMATRIX:
	    dirtyMatrix = TRUE;
	    break;
	case PDC_SETCLIP:
	    d->dirtyClipping = TRUE;
	    break;
	case PDC_SETCLIPRGN:
	    d->dirtyClipping = TRUE;
	    break;
	case PDC_PRT_NEWPAGE:
	    pageCount++;
	    stream << "GR\nQP\n%%Page: "
		   << pageCount << ' ' << pageCount
		   << "\nQI\n";
	    dirtyNewPage       = TRUE;
	    d->dirtyClipping   = TRUE;
	    d->firstClipOnPage = TRUE;
	    delete d->savedImage;
	    d->savedImage = 0;
	    orientationSetup();
	    stream << "GS\n";
	    break;
	case PDC_PRT_ABORT:
	    break;
	default:
	    break;
    }
    return TRUE;
}


void QPSPrinter::drawImage( QPainter *paint, const QPoint &pnt,
			    const QImage &img )
{
    stream << pnt.x() << " " << pnt.y() << " TR\n";

    //bool mask  = FALSE;
    int width  = img.width();
    int height = img.height();

    QColor fgCol = paint->pen().color();
    QColor bgCol = paint->backgroundColor();

    if ( width * height > 21840 ) { // 65535/3, tolerance for broken printers
	delete d->savedImage;
	d->savedImage = 0;
	stream << "/sl " << width*3 << " string def\n"
	       << width << ' ' << height << " 8[1 0 0 1 0 0]"
	       << "{currentfile sl readhexstring pop}QCI\n";
	ps_dumpPixmapData( stream, img, fgCol, bgCol );
    } else if ( d->savedImage && img == *d->savedImage ) {
	stream << width << ' ' << height << " 8[1 0 0 1 0 0]{sl}QCI\n";
    } else {
	if ( !d->savedImage ||
	     d->savedImage->width()*d->savedImage->height() != width*height )
	    stream << "/sl " << width*3*height << " string def\n";
	stream << "currentfile sl readhexstring\n";
	ps_dumpPixmapData( stream, img, fgCol, bgCol );
	stream << "pop pop\n";
	delete d->savedImage;
	d->savedImage = new QImage( img );
	d->savedImage->detach();
	stream << width << ' ' << height << " 8[1 0 0 1 0 0]{sl}QCI\n";
    }
    stream << -pnt.x() << " " << -pnt.y() << " TR\n";
}

void QPSPrinter::matrixSetup( QPainter *paint )
{
    QWMatrix tmp;
    if ( paint->hasViewXForm() ) {
	QRect viewport = paint->viewport();
	QRect window   = paint->window();
	tmp.translate( viewport.x(), viewport.y() );
	tmp.scale( 1.0 * viewport.width()  / window.width(),
		   1.0 * viewport.height() / window.height() );
	tmp.translate( -window.x(), -window.y() );
    }
    if ( paint->hasWorldXForm() ) {
	tmp = paint->worldMatrix() * tmp;
    }
    stream << "["
	   << tmp.m11() << ' ' << tmp.m12() << ' '
	   << tmp.m21() << ' ' << tmp.m22() << ' '
	   << tmp.dx()	<< ' ' << tmp.dy()
	   << "]ST\n";
    dirtyMatrix = FALSE;
}

void QPSPrinter::orientationSetup()
{
    if ( printer->orientation() == QPrinter::Landscape ) {
	stream << "PageW 0 TR 90 rotate\n";
	stream << "/defM matrix CM def\n";
	stream << "PageW PageH /PageW def /PageH def\n";
    }

}


void QPSPrinter::emitHeader( bool finished )
{
    const char *title   = printer->docName();
    const char *creator = printer->creator();
    if ( !title )				// default document names
	title = "Unknown";
    if ( !creator )				// default creator
	creator = "Qt " QT_VERSION_STR;
    d->realDevice = new QFile();
    (void)((QFile *)d->realDevice)->open( IO_WriteOnly, d->fd );
    stream.setDevice( d->realDevice );
    stream << "%!PS-Adobe-1.0";
    if ( finished && pageCount == 1 && printer->numCopies() == 1 ) {
	QPaintDeviceMetrics m( printer );
	if ( !d->boundingBox.isValid() )
	    d->boundingBox.setRect( 0, 0, m.width(), m.height() );
	stream << " EPSF-3.0\n%%BoundingBox: "
	       << d->boundingBox.left() << " "
	       << m.height() - d->boundingBox.bottom() - 1 << " "
	       << d->boundingBox.right() + 1 << " "
	       << m.height() - d->boundingBox.top();
    }
    stream << "\n%%Creator: " << creator
	   << "\n%%Title: " << title
	   << "\n%%CreationDate: " << QDateTime::currentDateTime().toString();
    if ( finished )
	stream << "\n%%Pages: " << pageCount << "\n%%DocumentFonts: "
	       << fontsUsed.simplifyWhiteSpace();
    else
	stream << "\n%%Pages: (atend)"
	       << "\n%%DocumentFonts: (atend)";
    stream << "\n%%EndComments\n\n";

    if ( printer->numCopies() > 1 )
	stream << "/#copies " << printer->numCopies() << " def\n";

    if ( !fixed_ps_header )
	makeFixedStrings();

    stream << "% Standard Qt prolog\n" << fixed_ps_header << "\n";
    if ( d->fontBuffer->buffer().size() ) {
	if ( pageCount == 1 )
	    stream << "% Fonts and encodings used\n";
	else
	    stream << "% Fonts and encodings used on pages 1-"
		   << pageCount << "\n";
	stream.writeRawBytes( (const char *)(d->fontBuffer->buffer().data()),
			      d->fontBuffer->buffer().size() );
    }
    stream << "%%EndProlog\n";
    stream.writeRawBytes( (const char *)(d->buffer->buffer().data()),
			  d->buffer->buffer().size() );

    delete d->buffer;
    d->buffer = 0;
    d->fontStream.unsetDevice();
    delete d->fontBuffer;
    d->fontBuffer = 0;
}

void QPSPrinter::newPageSetup( QPainter *paint )
{
    if ( d->buffer && d->pagesInBuffer++ > 4 )
	emitHeader( FALSE );

    if ( !d->buffer ) {
	d->pageEncodings.clear();
	d->pageFontNames.clear();
    }
    resetDrawingTools( paint );
    dirtyNewPage      = FALSE;
    d->pageFontNumber = d->headerFontNumber;
}


/*
  Called whenever a restore has been done. Currently done at the top of a
  new page and whenever clipping is turned off.
 */
void QPSPrinter::resetDrawingTools( QPainter *paint )
{
    QPDevCmdParam param[1];
    QPen   defaultPen;			// default drawing tools
    QBrush defaultBrush;

    param[0].color = &paint->backgroundColor();
    if ( *param[0].color != white )
	cmd( PDC_SETBKCOLOR, paint, param );

    param[0].ival = paint->backgroundMode();
    if (param[0].ival != TransparentMode )
	cmd( PDC_SETBKMODE, paint, param );

    param[0].font = &paint->font();
    cmd( PDC_SETFONT, paint, param );

    param[0].pen = &paint->pen();
    if (*param[0].pen != defaultPen )
	cmd( PDC_SETPEN, paint,param );

    param[0].brush = &paint->brush();
    if (*param[0].brush != defaultBrush )
	cmd( PDC_SETBRUSH, paint, param);

    if ( paint->hasViewXForm() || paint->hasWorldXForm() )
	matrixSetup( paint );
}

static void putRect( QTextStream &stream, const QRect &r )
{
    stream << r.x() << " "
	   << r.y() << " "
	   << r.width() << " "
	   << r.height() << " ";
}

void QPSPrinter::setClippingOff( QPainter *paint )
{
	stream << "CLO\n";		// clipping off, includes a restore
	resetDrawingTools( paint );     // so drawing tools must be reset
}

void QPSPrinter::clippingSetup( QPainter *paint )
{
    if ( paint->hasClipping() ) {
	if ( !d->firstClipOnPage ) {
	    setClippingOff( paint );
	}
	const QRegion rgn = paint->clipRegion();
	QArray<QRect> rects = rgn.rects();
	int i;
	stream<< "CLSTART\n";		// start clipping
	for( i = 0 ; i < (int)rects.size() ; i++ ) {
	    putRect( stream, rects[i] );
	    stream << "ACR\n";		// add clip rect
	    if ( pageCount == 1 )
		d->boundingBox = d->boundingBox.unite( rects[i] );
	}
	stream << "CLEND\n";		// end clipping
	d->firstClipOnPage = FALSE;
    } else {
	if ( !d->firstClipOnPage )	// no need to turn off if first on page
	    setClippingOff( paint );
    }
    d->dirtyClipping = FALSE;
}
