/****************************************************************************
** $Id: moc.l,v 2.3.2.4 1998/09/24 15:36:49 eiriken Exp $
**
** Lexical analyzer for meta object compiler
**
** Created : 930417
**
** Copyright (C) 1993-1998 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** This file contains the lexical analyzer for the meta object compiler (moc)
** of the Qt development framework.
*****************************************************************************/

%{
#include "qstring.h"


#define input yyinput				// yyinput in C++

#define X if(lexDebug){fprintf(stderr,"LEX (%i) : %s\n",lineNo,yytext);};
#define Y if(lexDebug){fprintf(stderr,"LEX (%i) : %s updates level to %i\n"\
                               ,lineNo,yytext,templLevel);};
#define Z if(lexDebug){fprintf(stderr,"LEX (%i) : skipped the string %s\"\n"\
                               ,lineNo,yytext);};
#define BEGIN_INSIDE


#define  linput() (c = input()) == '\n' ? (lineNo++,c) : c

#include <string.h>
#include <stdlib.h>

/* char c,c1; */
int level      = 1;  /* Used to keep track of nested curly braces in IN_CLASS*/
int expLevel   = 1;  /* Used to keep track of nested parentheses in IN_EXPR */
int fctLevel   = 1;  /* Used to keep track of nested parentheses in IN_FCT */
int templLevel = 1;  /* Used to keep track of levels in IN_TEMPL_ARGS */


extern void addExpressionChar( char );
extern void addExpressionString( char * );
%}

%start OUTSIDE QT_DEF IN_CLASS IN_EXPR IN_FCT IN_TEMPL_ARGS GIMME_SEMICOLON

%%
<OUTSIDE>"class"		{ X;
				  BEGIN QT_DEF;
				  return CLASS; }
<OUTSIDE>"template"		{ X;
				  BEGIN QT_DEF;
				  return TEMPLATE; }
<QT_DEF>"signals"		{ X;return SIGNALS; }
<QT_DEF>"slots"			{ X;return SLOTS; }

<IN_FCT>"{"			{ fctLevel++;Y; }
<IN_FCT>"}"			{ fctLevel--;Y;if (fctLevel==0){X;return '}';}}
<IN_CLASS>"{"			{ level++;Y; }
<IN_CLASS>"}"			{ level--;Y;if (level == 0) {X;return '}';} }
<IN_CLASS>"public"		{ X;return PUBLIC; }
<IN_CLASS>"protected"		{ X;return PROTECTED; }
<IN_CLASS>"private"		{ X;return PRIVATE; }
<IN_CLASS>"signals"		{ X;return SIGNALS; }
<IN_CLASS>"slots"		{ X;return SLOTS; }
<IN_CLASS>"Q_OBJECT"		{ X;return Q_OBJECT; }
<QT_DEF>"Q_OBJECT"		{ X;return Q_OBJECT; }
<IN_EXPR>"("			{ expLevel++;Y; }
<IN_EXPR>")"			{ expLevel--;Y;if (expLevel == 0) 
					{ X; BEGIN QT_DEF; return ')';} }
<IN_EXPR>"["			{ expLevel++;Y; }
<IN_EXPR>"]"			{ expLevel--;Y;if (expLevel == 0) 
					{ X; BEGIN QT_DEF; return ']';} }
<IN_EXPR>","			{ if (expLevel == 0) 
					{ X; BEGIN QT_DEF;return ',' ;} }
<IN_EXPR>";"			{ if (expLevel == 0)
					{ X; BEGIN QT_DEF;return ';' ;} }
<IN_TEMPL_ARGS>"<"		{ templLevel++;Y; }
<IN_TEMPL_ARGS>">"		{ templLevel--;Y;if (templLevel == 0) 
					{ X; BEGIN QT_DEF; return '>';} }
<IN_TEMPL_ARGS>"("		{ templLevel++;Y; }
<IN_TEMPL_ARGS>")"		{ templLevel--;Y;if (templLevel == 0) 
					{ X; BEGIN QT_DEF; return ')';} }
<IN_TEMPL_ARGS>"["		{ templLevel++;Y; }
<IN_TEMPL_ARGS>"]"		{ templLevel--;Y;if (templLevel == 0) 
					{ X; BEGIN QT_DEF; return ']';} }
<QT_DEF>"friend"		{ X;return FRIEND; }
<QT_DEF>"typedef"		{ X;return TYPEDEF; }
<QT_DEF>"auto"			{ X;return AUTO; }
<QT_DEF>"register"		{ X;return REGISTER; }
<QT_DEF>"static"		{ X;return STATIC; }
<QT_DEF>"extern"		{ X;return EXTERN; }
<QT_DEF>"inline"		{ X;return INLINE; }
<QT_DEF>"virtual"		{ X;return VIRTUAL; }
<QT_DEF>"const"			{ X;return CONST; }
<QT_DEF>"volatile"		{ X;return VOLATILE; }
<QT_DEF>"char"			{ X;return CHAR; }
<QT_DEF>"short"			{ X;return SHORT; }
<QT_DEF>"int"			{ X;return INT; }
<QT_DEF>"long"			{ X;return LONG; }
<QT_DEF>"signed"		{ X;return SIGNED; }
<QT_DEF>"unsigned"		{ X;return UNSIGNED; }
<QT_DEF>"float"			{ X;return FLOAT; }
<QT_DEF>"double"		{ X;return DOUBLE; }
<QT_DEF>"void"			{ X;return VOID; }
<QT_DEF>"enum"			{ X;return ENUM; }
<QT_DEF>"class"			{ X;return CLASS; }
<QT_DEF>"struct"		{ X;return STRUCT; }
<QT_DEF>"union"			{ X;return UNION; }
<QT_DEF>"asm"			{ X;return ASM; }
<QT_DEF>"private"		{ X;return PRIVATE; }
<QT_DEF>"protected"		{ X;return PROTECTED; }
<QT_DEF>"public"		{ X;return PUBLIC; }
<QT_DEF>"operator"		{ X;return OPERATOR; }
<QT_DEF>"::"			{ X;return DBL_COLON; }
<QT_DEF>"..."			{ X;return TRIPLE_DOT; }
<QT_DEF>"template"		{ X;return TEMPLATE; }

<QT_DEF>[_a-zA-Z][_a-zA-Z0-9]* {
			  X;
			  yylval.string = qstrdup(yytext);
			  return IDENTIFIER;
			}

<OUTSIDE>[_a-zA-Z][_a-zA-Z0-9]*  ;
<IN_CLASS>[_a-zA-Z][_a-zA-Z0-9]* ;

<OUTSIDE>\"[^"]*	{			/* discard strings */
			  if (yytext[yyleng - 1] == '\\') {
			    yymore();
			  }
			  else {
			    Z;
			    input();		/* discard the '"' */
			  }
			}

<IN_CLASS>\"[^"]*	{			/* discard strings */
			  if (yytext[yyleng - 1] == '\\') {
			    yymore();
			  }
			  else {
			    Z;
			    input();		/* discard the '"' */
			  }
			}

<IN_FCT>\"[^"]*		{			/* discard strings */
			  if (yytext[yyleng - 1] == '\\') {
			    yymore();
			  }
			  else {
			    Z;
			    input();		/* discard the '"' */
			  }
			}

<IN_EXPR>\"[^"]*        { 
			  if (yytext[yyleng - 1] == '\\') {
			    yymore();
			  }
			  else {
			    X;
			    addExpressionString(yytext);
			    input();		/* discard the '"' */
			    return STRING;
			  }
			}


<IN_TEMPL_ARGS>\"[^"]*	  { if (yytext[yyleng - 1] == '\\') {
			      yymore();
			  }
			  else {
			      X;
			      addExpressionString(yytext);
			      input();		/* discard the '"' */
			      return STRING;
			 }
			}


<QT_DEF>\"[^"]*	     { if (yytext[yyleng - 1] == '\\') {
			      yymore();
			  }
			  else {
			      X;
			      yylval.string = qstrdup(yytext);
			      input();		/* discard the '"' */
			      return STRING;
			 }
			}


<QT_DEF>'.'	     { X;
			  yylval.char_val = yytext[1];
			  return CHAR_VAL;
			}

<QT_DEF>'\\a'	    { X;
			 yylval.char_val = '\a';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\b'	    { X;
			 yylval.char_val = '\b';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\f'	    { X;
			 yylval.char_val = '\f';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\n'	    { X;
			 yylval.char_val = '\n';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\r'	    { X;
			 yylval.char_val = '\r';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\t'	    { X;
			 yylval.char_val = '\t';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\v'	    { X;
			 yylval.char_val = '\v';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\\\'	    { X;
			 yylval.char_val = '\\';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\?'	    { X;
			 yylval.char_val = '\?';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\''	    { X;
			 yylval.char_val = '\'';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\\"'	    { X;
			 yylval.char_val = '\"';	/* " */
			 return CHAR_VAL;
		       }

<QT_DEF>'\\0'	    { X;
			 yylval.char_val = '\0';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\[0-7]+'  { X;
			 yylval.char_val =
			   (char)strtol( &yytext[1], 0, 8 );
			 return CHAR_VAL;
		       }

<QT_DEF>'\\x[0-9a-fA-F]+' { X;
			 yylval.char_val =
			   (char)strtol( &yytext[2], 0, 16 );
			 return CHAR_VAL;
		       }

<QT_DEF>'\\.'	    { X;
			 yylval.char_val = ' ';
			 return CHAR_VAL;
		       }

<QT_DEF>[0-9]+	     { X;
			  yylval.int_val = atoi(yytext);
			  return INT_VAL;
			}

<QT_DEF>[0-9]+\.[0-9]* { X;
			  yylval.double_val = atof(yytext);
			  return DOUBLE_VAL;
			}

<QT_DEF>\.[0-9]+     { X;
			  yylval.double_val = atof(yytext);
			  return DOUBLE_VAL;
			}


^#define.*\\$		{ /* skip multi-line macro-definitions */
			  char c, c1;
			  input();   /* Discard the '\n'. */
			  do {
			      c1=' ';
			      while((c = linput()) != '\n' && c != 0) c1=c;
			      if (c == 0) break;
			  } while(c1=='\\');
			  unput(c);  /* put back the '\n' or the EOF */
			}

^[ \t]*#.*		{ /* preprocessor commands are skipped */}
"//"[^\n]*		{ /* C++ comment */ }
"/*"			{ /* C	 comment */
			  char c = ' ';
			  do {
			      if ( c!= '*' ) {
				  while((c = linput()) != '*' && c != 0)
				      ;
			      }
			      if (c == 0) break;
			  } while(((c = linput())) != '/' && c != 0);
			  if (c == 0)
			      unput(c);
			}

[ \t\r\b\f]+   		;
<IN_CLASS>.		;
<IN_FCT>.		;
<IN_EXPR>.		  { addExpressionChar( yytext[0] ); }
<IN_TEMPL_ARGS>.	  { addExpressionChar( yytext[0] ); }
<OUTSIDE>.		;
<QT_DEF>.		{
			  X;
			  return yytext[0];
			}
<GIMME_SEMICOLON>.	{
			  X;
			  return ';';
			}
\n			{
			  lineNo++;
			}

%%



