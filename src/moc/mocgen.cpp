#ifndef lint
/*static char yysccsid[] = "from: @(#)yaccpar	1.9 (Berkeley) 02/21/93";*/
static char yyrcsid[] = "$Id: skeleton.c,v 1.4 1993/12/21 18:45:32 jtc Exp $";
#endif
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define yyclearin (yychar=(-1))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING (yyerrflag!=0)
#define YYPREFIX "yy"
#line 31 "moc.y"
void yyerror( char *msg );

#include "qlist.h"
#include "qstring.h"
#include "qdatetime.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

static QString rmWS( const char * );

enum AccessPerm { _PRIVATE, _PROTECTED, _PUBLIC };


struct Argument					/* single arg meta data*/
{
    Argument( char *left, char *right )
	{ leftType  = rmWS( left );
	  rightType = rmWS( right );
	  if ( leftType == "void" && rightType.isEmpty() )
	      leftType = "";
	}
    QString leftType;
    QString rightType;
};

Q_DECLARE(QListM,Argument);

class ArgList : public QListM(Argument) {	/* member function arg list*/
public:
    ArgList() { setAutoDelete(TRUE); }
};


struct Function					/* member function meta data*/
{						/*   used for signals and slots*/
    AccessPerm accessPerm;
    QString    qualifier;			/* const or volatile*/
    QString    name;
    QString    type;
    int	       lineNo;
    ArgList   *args;
    Function() { args=0; }
   ~Function() { delete args; }
};

Q_DECLARE(QListM,Function);

class FuncList : public QListM(Function) {	/* list of member functions*/
public:
    FuncList() { setAutoDelete(TRUE); }
};


ArgList *addArg( Argument * );			/* add arg to tmpArgList*/
void	 addMember( char );			/* add tmpFunc to current class*/

char	*strnew( const char * );		/* returns a new string (copy)*/
char	*stradd( const char *, const char * );	/* add two strings*/
char	*stradd( const char *, const char *,	/* add three strings*/
			       const char * );
char	*straddSpc( const char *, const char * );
char	*straddSpc( const char *, const char *,
			       const char * );
char	*straddSpc( const char *, const char *,
		    const char *, const char * );

extern int yydebug;
bool	   lexDebug	   = FALSE;
bool	   grammarDebug	   = FALSE;
int	   lineNo;				/* current line number*/
bool	   errorControl	   = FALSE;		/* controlled errors*/
bool	   displayWarnings = TRUE;
bool	   skipClass;				/* don't generate for class*/
bool	   skipFunc;				/* don't generate for func*/
bool	   templateClass;			/* class is a template*/

ArgList	  *tmpArgList;				/* current argument list*/
Function  *tmpFunc;				/* current member function*/
AccessPerm tmpAccessPerm;			/* current access permission*/
AccessPerm subClassPerm;			/* current access permission*/
bool	   Q_OBJECTdetected;			/* TRUE if current class*/
						/* contains the Q_OBJECT macro*/
QString	   tmpExpression;

const int  formatRevision = 2;			/* moc output format revision*/

#line 121 "moc.y"
typedef union {
    char	char_val;
    int		int_val;
    double	double_val;
    char       *string;
    AccessPerm	access;
    Function   *function;
    ArgList    *arg_list;
    Argument   *arg;
} YYSTYPE;
#line 112 "y.tab.c"
#define CHAR_VAL 257
#define INT_VAL 258
#define DOUBLE_VAL 259
#define STRING 260
#define IDENTIFIER 261
#define FRIEND 262
#define TYPEDEF 263
#define AUTO 264
#define REGISTER 265
#define STATIC 266
#define EXTERN 267
#define INLINE 268
#define VIRTUAL 269
#define CONST 270
#define VOLATILE 271
#define CHAR 272
#define SHORT 273
#define INT 274
#define LONG 275
#define SIGNED 276
#define UNSIGNED 277
#define FLOAT 278
#define DOUBLE 279
#define VOID 280
#define ENUM 281
#define CLASS 282
#define STRUCT 283
#define UNION 284
#define ASM 285
#define PRIVATE 286
#define PROTECTED 287
#define PUBLIC 288
#define OPERATOR 289
#define DBL_COLON 290
#define TRIPLE_DOT 291
#define TEMPLATE 292
#define SIGNALS 293
#define SLOTS 294
#define Q_OBJECT 295
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    0,   38,   36,    1,    1,    2,   39,   40,   28,
   28,   28,   28,   28,   27,   29,   29,   30,   30,   41,
   41,   41,   41,   42,   42,   31,   31,   11,   11,   11,
   12,   12,   13,   13,   13,   13,   13,   13,   13,   13,
   13,    3,   43,   43,   14,   14,   15,   15,   16,   16,
   17,   17,   17,   18,   18,   20,   20,   44,   44,   19,
   19,   23,   45,   23,   23,   47,   23,   21,   21,   22,
   48,   22,   49,   22,   22,   22,   32,   32,   50,   32,
   32,   46,   51,   10,   10,   54,   10,   55,   10,   56,
   53,   57,   53,   35,   35,   34,   34,   33,   33,   24,
   24,   25,   25,   26,   26,   52,   52,   52,   59,   58,
   62,   37,   37,   37,   37,   64,   64,   64,   64,   64,
   63,   63,   60,    4,    4,   61,   61,   65,   65,   67,
   67,   69,   66,   70,   66,   66,   68,   68,   73,   68,
   68,   71,   71,   74,   74,   75,   72,   72,   77,   77,
   78,   79,   79,    5,    6,    6,    7,    7,    8,    8,
    8,    8,    8,    8,    8,    8,    8,    8,    9,    9,
    9,   80,   80,   80,   80,   80,   80,   80,   80,   80,
   80,   80,   80,   80,   80,   80,   80,   80,   80,   80,
   80,   80,   80,   80,   80,   80,   80,   80,   80,   80,
   80,   80,   80,   80,   80,   80,   80,   80,   80,   80,
   80,   80,   81,   81,   82,   82,   82,   82,   82,   82,
   82,   82,   82,   82,   82,   82,   76,   76,   76,   76,
   84,   84,   86,   87,   86,   88,   86,   83,   89,   83,
   91,   85,   90,   90,
};
short yylen[] = {                                         2,
    0,    2,    0,    3,    1,    1,    4,    0,    0,    1,
    1,    1,    1,    1,    3,    0,    1,    1,    2,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    2,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    4,    0,    1,    2,    2,    1,    2,    3,    1,
    2,    2,    2,    2,    3,    0,    1,    0,    1,    3,
    1,    2,    0,    5,    4,    0,    7,    0,    1,    2,
    0,    4,    0,    5,    1,    3,    1,    2,    0,    5,
    3,    1,    5,    1,    2,    0,    4,    0,    5,    0,
    4,    0,    5,    0,    1,    1,    2,    2,    2,    0,
    1,    1,    2,    1,    1,    1,    1,    3,    0,    3,
    0,    5,    1,    4,    2,    1,    1,    1,    1,    1,
    2,    3,    2,    0,    1,    0,    1,    2,    1,    1,
    1,    0,    3,    0,    4,    1,    2,    3,    0,    3,
    1,    0,    1,    2,    1,    1,    0,    1,    2,    1,
    1,    0,    2,    2,    3,    1,    4,    4,    1,    3,
    2,    3,    2,    1,    3,    2,    3,    2,    1,    1,
    1,    3,    3,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    3,    3,    2,    2,    2,    2,    1,    3,    2,
    2,    2,    0,    1,    2,    1,    3,    5,    2,    3,
    4,    3,    2,    6,    4,    5,    3,    4,    6,    3,
    1,    3,    1,    0,    4,    0,    3,    0,    0,    3,
    0,    5,    0,    1,
};
short yydefred[] = {                                      1,
    3,    2,    0,    0,    0,    0,    0,    0,    0,    0,
    8,  116,   20,   21,   22,   23,   24,   25,   26,   27,
   33,   34,   35,   36,   37,   38,   39,   40,   41,  117,
  118,  119,  120,  115,    0,  121,    6,    4,   45,   46,
  111,    0,    0,  123,  125,    0,    0,    8,  122,    0,
    0,    0,    0,  169,  170,  171,    0,   50,    0,  164,
  156,    0,  159,    0,   42,    0,  134,  131,  136,  130,
    0,    0,  129,  132,  114,    0,  166,    0,  161,    0,
    0,    0,  168,  163,    0,    7,    0,  112,  128,    0,
    0,    0,  165,  160,  155,  167,  162,   49,    0,  141,
    0,    0,  139,  133,  157,  158,    0,   13,   14,    0,
    0,    0,    0,   44,  216,    0,    0,   32,    0,   29,
   28,   18,    0,   11,   10,   12,  135,    0,  145,  146,
    0,    0,    0,  137,    0,    0,   86,   90,    0,    0,
    0,   53,    0,    0,  208,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  223,    0,    0,    0,    0,  215,    0,   96,    0,   31,
   51,    0,    0,   19,  144,    0,    0,  239,  152,    0,
  152,  138,  151,    0,  150,  140,    9,    9,   88,   92,
  241,    0,  198,    0,  199,  197,  212,  211,  189,  193,
  204,  187,  206,    0,  188,  207,  190,  191,  192,  194,
  205,  195,  196,    0,    0,  222,  104,  105,   98,    0,
  102,   99,    0,    0,    0,  220,   97,   52,    0,  217,
    0,    0,    0,   61,    0,    0,    9,    0,  152,    0,
    0,  149,   87,    0,    9,    9,    0,  202,  203,  209,
  172,  173,  103,    0,  221,  225,    0,    0,    0,   59,
   54,   71,    0,    0,    0,   75,    0,  240,  153,    0,
    0,    0,  236,    0,   77,    0,  231,   91,   89,    0,
  242,  226,    0,  218,    0,   55,   60,    9,    0,   82,
   63,    0,   73,   70,   15,  234,    0,    9,   79,   78,
  152,    0,   93,  224,  106,    0,  109,   83,  107,    0,
   76,    9,    0,    9,    9,   81,  237,    9,    0,  232,
    0,    0,   72,   64,   66,    0,  235,    0,  108,  110,
    9,   74,   80,   67,
};
short yydgoto[] = {                                       1,
   58,   37,  114,   44,   45,   59,   60,   61,   62,  115,
  116,  117,  118,  119,  120,   64,  121,  231,  232,  233,
  264,  265,  234,  219,  220,  221,  235,  122,  159,  160,
  124,  274,  168,  223,  224,    2,    7,    3,   46,  243,
  125,  126,    8,  261,  312,  275,  331,  288,  314,  318,
  179,  308,  139,  187,  245,  188,  246,  309,  322,    9,
   71,   50,   10,   34,   72,   73,   74,  104,   90,   87,
  127,  182,  136,  128,  129,  183,  184,  185,  238,  161,
  131,  132,  180,  276,  133,  277,  315,  298,  237,  141,
  247,
};
short yysindex[] = {                                      0,
    0,    0, -265,   53,  710, -194,   98, -222,   -6,  292,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  -59,    0,    0,    0,    0,    0,
    0,  -90,  105,    0,    0,  111,  135,    0,    0,  590,
  171,  166,   72,    0,    0,    0,  -47,    0,  172,    0,
    0,  -87,    0,  -71,    0,  163,    0,    0,    0,    0,
  168,  590,    0,    0,    0,  586,    0, -244,    0,  -71,
  105, -244,    0,    0,  -47,    0,  225,    0,    0,  -46,
  256,  281,    0,    0,    0,    0,    0,    0,  627,    0,
  291,  303,    0,    0,    0,    0,   79,    0,    0,    0,
  117,  144,  419,    0,    0,  180,  823,    0,  145,    0,
    0,    0,  659,    0,    0,    0,    0,  627,    0,    0,
  270,  294,  365,    0,  627,  627,    0,    0,  -20,    0,
  302,    0,    9,  309,    0,  392,  333,  413,  399,   -3,
  329,  285,  404,  409,  412,  -39,  416,  417,  730,  822,
    0,  107,  419,  129,  129,    0,  252,    0,   21,    0,
    0,  213,  822,    0,    0,  237,  822,    0,    0,   18,
    0,    0,    0,  627,    0,    0,    0,    0,    0,    0,
    0,  438,    0,  439,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  462,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  -18,  -18,    0,    0,    0,    0,  129,
    0,    0,  -18, -217,  419,    0,    0,    0,  -18,    0,
  465,  463,  217,    0,  141,  535,    0,  450,    0,  -30,
  450,    0,    0,  434,    0,    0,  406,    0,    0,    0,
    0,    0,    0,  419,    0,    0,  -69,  129,  466,    0,
    0,    0,  141,  -48,  246,    0,  822,    0,    0,  450,
  474,  -32,    0,  253,    0,  452,    0,    0,    0,  440,
    0,    0,  419,    0,  -40,    0,    0,    0,  113,    0,
    0,  141,    0,    0,    0,    0,  158,    0,    0,    0,
    0,  -30,    0,    0,    0,  286,    0,    0,    0,  446,
    0,    0,  481,    0,    0,    0,    0,    0,  450,    0,
  487,  422,    0,    0,    0,  455,    0,  456,    0,    0,
    0,    0,    0,    0,
};
short yyrindex[] = {                                      0,
    0,    0,  119,    0,  121,    0,    0,    0,    0,   36,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  132,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   -4,    0,    0,  425,
    0,   66,    0,    0,    0,    0,    0,    0,  428,    0,
    0,    0,    0,    7,    0,    0,    0,    0,    0,    0,
    0,  429,    0,    0,    0,    0,    0,    0,    0,   38,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  200,    0,
    0,    0,    0,    0,    0,    0,  118,    0,    0,  507,
  459,    0,  739,    0,    0,    0,   50,    0,    0,    0,
    0,    0,  119,    0,    0,    0,    0,  242,    0,    0,
    0,  453,    0,    0,  468,  468,    0,    0,  284,  169,
    0,    0,  296,  431,    0,  494,    0,    0,  701,  780,
  793,  980,  982,  985,  987,  990,  992, 1012,    0,  -38,
    0, 1014,  739,   74,   74,    0,  -68,    0,    0,    0,
    0,    0,  -33,    0,    0,    0,  137,    0,    0,    0,
    0,    0,    0,  548,    0,    0,    0,    0,    0,    0,
    0, 1017,    0, 1019,    0,    0,    0,    0,    0,    0,
    0,    0,    0, 1022,    0,    0,    0,    0,    0,    0,
    0,    0,    0, 1024, 1024,    0,    0,    0,    0,   62,
    0,    0,  108,    0,  739,    0,    0,    0,  -68,    0,
    0,  -10,  516,    0,  -11,  119,    0,  297,    0,    0,
  342,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  739,    0,    0,    0,   14,  680,    0,
    0,    0,    0,  -12,  103,    0,   84,    0,    0,  377,
  142,    0,    0,  482,    0,    0,    0,    0,    0,    0,
    0,    0,  739,    0,    0,    0,    0,    0,    0,    0,
    0,   40,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   -2,    0,    0,    0,    0,    0,  430,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
   58,    0,  585,    0,    0,    0,  423,  514,  155,  -27,
 -105,    0,   10,  593,  295,  540,    0,    0,    0,    0,
  306,  336,  341, -151,    0,  381,    0,   27, -121,  356,
  597,  355,  583,  546,   -5,    0,    0,    0,  620,  556,
  670,  671,    0,    0,    0,  414,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  605,    0,    0,    0,    0,
    0,  543,    0,    0,  552,   89,    0,  531, -165, -120,
    0,    0,    0,    0,    0,  424,    0,    0,    0,    0,
    0,
};
#define YYTABLESIZE 1103
short yytable[] = {                                      17,
   48,   17,   17,   17,   16,   17,  219,  272,   16,  272,
  219,  103,  291,  222,   30,  241,   52,  173,  305,  165,
  306,  210,   17,  164,  219,  219,    4,  273,   62,   68,
   57,   62,   68,    5,  201,    5,    5,    5,   65,    5,
  189,   65,  216,  162,   47,   57,   47,   47,   47,   68,
   47,  229,   17,    5,    5,  236,    5,  200,  165,   39,
   40,  240,  164,   36,   47,   47,   35,   47,  192,  193,
  190,  254,  100,  270,  100,   48,  239,   48,   48,   48,
   68,   48,  307,   68,  211,   92,    5,   30,  166,   30,
   30,   30,   49,   30,  113,   48,   48,   47,   48,  101,
   68,  101,  101,  101,  256,  101,  285,   30,   30,    5,
   30,  100,   11,  100,  100,  100,   41,  100,    5,  101,
  101,   16,  101,   16,   16,   16,  170,   16,   48,   47,
  267,  100,  100,  282,  100,  319,  100,  236,   48,  137,
   30,  226,   98,   69,   16,  295,   69,   95,  230,  174,
  165,   95,  101,  311,  164,    5,   38,   84,  124,    5,
   48,   84,  304,   69,  100,   95,   95,  137,  215,  138,
   51,    5,   65,   52,   16,   84,   84,   56,  165,   82,
  263,   82,  164,   82,  101,   82,  174,  130,    5,    5,
    5,  162,   94,  174,   48,  165,  255,  138,  316,  164,
   82,   47,   57,  293,   70,   76,   52,   78,  251,  252,
   52,   75,  290,   47,  100,   81,  130,  165,   85,  283,
   94,  164,   17,  257,   86,   48,   70,   16,  290,  284,
  271,  262,   82,   17,   17,   17,   17,   17,   17,   17,
   17,   17,   17,   17,   17,   17,  101,  102,  299,   68,
   17,   17,   17,   17,    5,   16,    5,    5,    5,    5,
    5,    5,    5,    5,    5,    5,    5,   47,   47,   47,
   47,   47,   47,   47,   47,   47,   47,   47,   62,   68,
   57,  162,   99,  165,    5,    5,    5,  164,   65,  165,
  165,  244,   88,  164,  164,   47,  105,   47,   48,   48,
   48,   48,   48,   48,   48,   48,   48,   48,   48,  225,
   30,   30,   30,   30,   30,   30,   30,   30,   30,   30,
   30,  106,  101,   85,  142,  213,   48,   85,   48,  206,
   68,   42,   52,  177,  100,  185,  293,   63,   30,  185,
   30,   85,   85,  299,   16,  205,  204,   79,  134,   43,
  101,  178,  101,  185,  185,    5,   84,   54,   55,   56,
  135,   57,  100,   69,  100,   52,  143,  213,   95,  195,
  194,  203,   94,   53,   16,   63,   97,  140,    5,    5,
    5,    5,    5,    5,    5,    5,    5,    5,    5,  202,
   54,   55,   56,   69,   57,  176,   95,   16,  217,  218,
   43,   43,   44,   44,  142,  171,    5,    5,   16,   16,
   16,   16,   16,   16,   16,   16,   16,   16,   16,   16,
   16,  227,  227,  181,  191,  197,   16,   56,   16,   52,
   52,   52,   52,   52,   52,   52,   52,   52,   52,   52,
  162,  108,  109,   13,   14,   15,   16,   17,   18,   19,
   20,  158,  196,  198,  123,  154,  150,   52,  148,  199,
  149,  151,  145,  152,  207,  153,  230,  230,  163,  208,
  186,  167,  209,  228,  186,   77,  212,  213,  143,  146,
  144,   43,   43,  123,   83,  142,  142,  142,  186,  186,
  123,  123,  142,  142,  142,  302,  238,  162,  248,  249,
   93,  228,  228,  250,   96,  258,  259,  260,  269,  147,
  301,  238,  155,  108,  109,   13,   14,   15,   16,   17,
   18,   19,   20,   43,   43,  233,  278,  143,  143,  143,
  281,  296,  303,  184,  143,  143,  143,  184,  323,  123,
  233,  325,  156,  321,  157,  329,  330,  332,  333,  126,
  154,  184,  184,  127,  229,  229,   58,  227,  227,  227,
  227,  227,  227,  227,  227,  227,  227,  227,  227,  227,
  227,  227,  227,  227,  227,  227,  227,  227,  227,  227,
  227,  243,  227,  227,  227,  227,  227,    5,  227,  227,
  227,  227,  147,  213,   95,    6,   80,  313,  289,  287,
  253,   31,  230,  230,  230,  230,  230,  230,  230,  230,
  230,  230,  230,  230,  230,  230,  230,  230,  230,  230,
  230,  230,  230,  230,  230,  230,  297,  230,  230,  230,
  230,  230,  214,  230,  230,  230,  230,  228,  228,  228,
  228,  228,  228,  228,  228,  228,  228,  228,  228,  228,
  228,  228,  228,  228,  228,  228,  228,  228,  228,  228,
  228,  169,  228,  228,  228,  228,  228,   66,  228,  228,
  228,  228,  148,  213,   32,   33,   89,  292,  186,  175,
  108,  109,   13,   14,   15,   16,   17,   18,   19,   20,
  229,  229,  229,  229,  229,  229,  229,  229,  229,  229,
  229,  229,  229,  229,  229,  229,  229,  229,  229,  229,
  229,  229,  229,  229,  242,  229,  229,  229,  229,  229,
    0,  229,  229,  229,  229,  320,    0,  108,  109,   13,
   14,   15,   16,   17,   18,   19,   20,    0,    0,    0,
  176,    0,    0,  244,  176,    0,    0,    0,    0,   43,
   43,  227,    0,  147,  147,  147,  286,    0,  176,  176,
  147,  147,  147,    0,    0,    0,    0,   25,   25,   25,
   25,   25,   25,   25,   25,   25,   25,   25,   25,   25,
   25,   25,   25,   25,   25,   25,   25,   25,   25,   25,
   25,    0,  268,    0,    0,   47,   25,    0,   25,    0,
  279,  280,    0,    0,    0,  227,   21,   22,   23,   24,
   25,   26,   27,   28,   29,  172,    0,  266,  112,  180,
    0,    0,    0,  180,   57,    0,    4,    0,    0,   43,
   43,    0,  174,  148,  148,  148,  174,  180,  180,    0,
  148,  148,  148,  310,    0,  266,   91,  294,    0,    0,
  174,  174,    0,  317,    0,    0,  300,   21,   22,   23,
   24,   25,   26,   27,   28,   29,    0,  324,    0,  326,
  327,  294,    0,  328,  266,   54,   55,   56,    0,  300,
    0,    0,   67,   68,   69,    0,  334,  107,  108,  109,
   13,   14,   15,   16,   17,  110,   19,   20,   21,   22,
   23,   24,   25,   26,   27,   28,   29,  111,    0,    0,
  112,    0,    0,    0,    0,  113,   57,    0,    4,   47,
  108,  109,   13,   14,   15,   16,   17,   18,   19,   20,
   21,   22,   23,   24,   25,   26,   27,   28,   29,  172,
   16,    0,  112,    0,    0,    0,    0,    0,   57,    0,
    4,   16,   16,   16,   16,   16,   16,   16,   16,   16,
   16,   16,   16,   16,    0,    0,    0,    0,    0,   16,
   12,   16,    0,   13,   14,   15,   16,   17,   18,   19,
   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
  214,    0,    0,    0,    0,    0,    0,    0,    0,   16,
    0,   21,   22,   23,   24,   25,   26,   27,   28,   29,
   16,   16,   16,   16,   16,   16,   16,   16,   16,  175,
    0,  177,    0,  175,  178,  177,  179,    0,  178,  181,
  179,  182,    0,  181,    0,  182,    0,  175,  175,  177,
  177,    0,  178,  178,  179,  179,    0,  181,  181,  182,
  182,  183,    0,   84,    0,  183,  200,   84,  201,    0,
  200,  210,  201,   94,    0,  210,    0,   94,    0,  183,
  183,   84,   84,    0,  200,  200,  201,  201,    0,  210,
  210,   94,   94,  108,  109,   13,   14,   15,   16,   17,
   18,   19,   20,    0,   21,   22,   23,   24,   25,   26,
   27,   28,   29,
};
short yycheck[] = {                                      38,
   60,   40,   41,   42,   38,   44,   40,   40,   42,   40,
   44,   58,   61,  165,    5,  181,  261,  123,   59,   38,
   61,   61,   61,   42,   58,   59,  292,   58,   41,   41,
   41,   44,   44,   38,   38,   40,   41,   42,   41,   44,
   61,   44,  163,  261,   38,  290,   40,   41,   42,   61,
   44,  173,   91,   58,   59,  177,   61,   61,   38,  282,
  283,   44,   42,    6,   58,   59,  261,   61,   60,   61,
   91,  289,   59,  239,   61,   38,   59,   40,   41,   42,
   41,   44,  123,   44,  124,   76,   91,   38,  116,   40,
   41,   42,   35,   44,   59,   58,   59,   91,   61,   38,
   61,   40,   41,   42,  225,   44,  258,   58,   59,   44,
   61,   38,   60,   40,   41,   42,  123,   44,  123,   58,
   59,   38,   61,   40,   41,   42,  117,   44,   91,  123,
  236,   58,   59,  254,   61,  301,  123,  259,   60,   61,
   91,  169,   85,   41,   61,  267,   44,   40,  176,  123,
   38,   44,   91,   41,   42,   38,   59,   40,  123,   42,
  123,   44,  283,   61,   91,   58,   59,   61,  159,   91,
  261,   40,   62,  261,   91,   58,   59,   41,   38,   38,
   40,  269,   42,   42,  123,   44,  160,   99,  123,   58,
   59,  261,  261,  167,   60,   38,  224,   91,   41,   42,
   59,  261,  290,   91,   50,   40,   38,   53,  214,  215,
   42,   41,  261,  261,  261,   44,  128,   38,  290,  289,
  289,   42,  261,  229,   62,   60,   72,  261,  261,  257,
  261,   91,   91,  272,  273,  274,  275,  276,  277,  278,
  279,  280,  281,  282,  283,  284,  293,  294,   91,  261,
  289,  290,  291,  292,  123,  289,  261,  262,  263,  264,
  265,  266,  267,  268,  269,  270,  271,  261,  262,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  291,  291,
  291,  261,   58,   38,  289,  290,  291,   42,  291,   38,
   38,  123,  125,   42,   42,  289,   41,  291,  261,  262,
  263,  264,  265,  266,  267,  268,  269,  270,  271,  289,
  261,  262,  263,  264,  265,  266,  267,  268,  269,  270,
  271,   41,  261,   40,  125,  126,  289,   44,  291,   45,
  291,   40,  261,   40,  261,   40,   91,   43,  289,   44,
  291,   58,   59,   91,  261,   61,   62,   53,   58,   58,
  289,   58,  291,   58,   59,  290,   62,  286,  287,  288,
   58,  290,  289,  261,  291,  261,  125,  126,  261,   61,
   62,   43,   78,  269,  291,   81,   82,  261,  261,  262,
  263,  264,  265,  266,  267,  268,  269,  270,  271,   61,
  286,  287,  288,  291,  290,  126,  289,  261,  270,  271,
  282,  283,  282,  283,  261,  261,  289,  290,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,  125,  126,   59,  123,   93,  290,  291,  292,  261,
  262,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  261,  262,  263,  264,  265,  266,  267,  268,  269,  270,
  271,   33,   61,   41,   99,   37,   38,  289,   40,   61,
   42,   43,   44,   45,   61,   47,  125,  126,  289,   61,
   40,  116,   61,  261,   44,   53,   61,   61,   60,   61,
   62,  282,  283,  128,   62,  286,  287,  288,   58,   59,
  135,  136,  293,  294,  295,   44,   44,  261,   61,   61,
   78,  125,  126,   42,   82,   41,   44,  291,   59,   91,
   59,   59,   94,  262,  263,  264,  265,  266,  267,  268,
  269,  270,  271,  282,  283,   44,   93,  286,  287,  288,
  125,   58,   93,   40,  293,  294,  295,   44,   93,  184,
   59,   61,  124,  258,  126,   59,  125,   93,   93,  125,
  123,   58,   59,  125,  125,  126,   41,  261,  262,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,  123,  286,  287,  288,  289,  290,    3,  292,  293,
  294,  295,  125,  126,   81,    3,   57,  292,  263,  259,
  220,    5,  261,  262,  263,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  280,  281,  282,  283,  284,  272,  286,  287,  288,
  289,  290,  126,  292,  293,  294,  295,  261,  262,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,  116,  286,  287,  288,  289,  290,   48,  292,  293,
  294,  295,  125,  126,    5,    5,   72,  264,  136,  128,
  262,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  261,  262,  263,  264,  265,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  281,  282,  283,  284,  184,  286,  287,  288,  289,  290,
   -1,  292,  293,  294,  295,  302,   -1,  262,  263,  264,
  265,  266,  267,  268,  269,  270,  271,   -1,   -1,   -1,
   40,   -1,   -1,  188,   44,   -1,   -1,   -1,   -1,  282,
  283,  169,   -1,  286,  287,  288,  291,   -1,   58,   59,
  293,  294,  295,   -1,   -1,   -1,   -1,  261,  262,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,  237,   -1,   -1,  261,  290,   -1,  292,   -1,
  245,  246,   -1,   -1,   -1,  223,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,   -1,  235,  284,   40,
   -1,   -1,   -1,   44,  290,   -1,  292,   -1,   -1,  282,
  283,   -1,   40,  286,  287,  288,   44,   58,   59,   -1,
  293,  294,  295,  288,   -1,  263,  261,  265,   -1,   -1,
   58,   59,   -1,  298,   -1,   -1,  274,  272,  273,  274,
  275,  276,  277,  278,  279,  280,   -1,  312,   -1,  314,
  315,  289,   -1,  318,  292,  286,  287,  288,   -1,  297,
   -1,   -1,  293,  294,  295,   -1,  331,  261,  262,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,   -1,   -1,
  284,   -1,   -1,   -1,   -1,  289,  290,   -1,  292,  261,
  262,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  261,   -1,  284,   -1,   -1,   -1,   -1,   -1,  290,   -1,
  292,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,  290,
  261,  292,   -1,  264,  265,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  261,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  261,
   -1,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  272,  273,  274,  275,  276,  277,  278,  279,  280,   40,
   -1,   40,   -1,   44,   40,   44,   40,   -1,   44,   40,
   44,   40,   -1,   44,   -1,   44,   -1,   58,   59,   58,
   59,   -1,   58,   59,   58,   59,   -1,   58,   59,   58,
   59,   40,   -1,   40,   -1,   44,   40,   44,   40,   -1,
   44,   40,   44,   40,   -1,   44,   -1,   44,   -1,   58,
   59,   58,   59,   -1,   58,   59,   58,   59,   -1,   58,
   59,   58,   59,  262,  263,  264,  265,  266,  267,  268,
  269,  270,  271,   -1,  272,  273,  274,  275,  276,  277,
  278,  279,  280,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 295
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'!'",0,0,0,"'%'","'&'",0,"'('","')'","'*'","'+'","','","'-'",0,"'/'",0,0,0,0,0,
0,0,0,0,0,"':'","';'","'<'","'='","'>'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,"'['",0,"']'","'^'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,"'{'","'|'","'}'","'~'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"CHAR_VAL","INT_VAL",
"DOUBLE_VAL","STRING","IDENTIFIER","FRIEND","TYPEDEF","AUTO","REGISTER",
"STATIC","EXTERN","INLINE","VIRTUAL","CONST","VOLATILE","CHAR","SHORT","INT",
"LONG","SIGNED","UNSIGNED","FLOAT","DOUBLE","VOID","ENUM","CLASS","STRUCT",
"UNION","ASM","PRIVATE","PROTECTED","PUBLIC","OPERATOR","DBL_COLON",
"TRIPLE_DOT","TEMPLATE","SIGNALS","SLOTS","Q_OBJECT",
};
char *yyrule[] = {
"$accept : class_defs",
"class_defs :",
"class_defs : class_defs class_def",
"$$1 :",
"class_def : $$1 class_specifier ';'",
"class_name : IDENTIFIER",
"class_name : template_class_name",
"template_class_name : IDENTIFIER '<' template_args '>'",
"template_args :",
"const_expression :",
"decl_specifier : storage_class_specifier",
"decl_specifier : type_specifier",
"decl_specifier : fct_specifier",
"decl_specifier : FRIEND",
"decl_specifier : TYPEDEF",
"decl_specifiers : decl_specs_opt type_name decl_specs_opt",
"decl_specs_opt :",
"decl_specs_opt : decl_specs",
"decl_specs : decl_specifier",
"decl_specs : decl_specs decl_specifier",
"storage_class_specifier : AUTO",
"storage_class_specifier : REGISTER",
"storage_class_specifier : STATIC",
"storage_class_specifier : EXTERN",
"fct_specifier : INLINE",
"fct_specifier : VIRTUAL",
"type_specifier : CONST",
"type_specifier : VOLATILE",
"type_name : elaborated_type_specifier",
"type_name : complete_class_name",
"type_name : simple_type_names",
"simple_type_names : simple_type_names simple_type_name",
"simple_type_names : simple_type_name",
"simple_type_name : CHAR",
"simple_type_name : SHORT",
"simple_type_name : INT",
"simple_type_name : LONG",
"simple_type_name : SIGNED",
"simple_type_name : UNSIGNED",
"simple_type_name : FLOAT",
"simple_type_name : DOUBLE",
"simple_type_name : VOID",
"template_spec : TEMPLATE '<' template_args '>'",
"opt_template_spec :",
"opt_template_spec : template_spec",
"class_key : opt_template_spec CLASS",
"class_key : opt_template_spec STRUCT",
"complete_class_name : qualified_class_name",
"complete_class_name : DBL_COLON qualified_class_name",
"qualified_class_name : qualified_class_name DBL_COLON class_name",
"qualified_class_name : class_name",
"elaborated_type_specifier : class_key IDENTIFIER",
"elaborated_type_specifier : ENUM IDENTIFIER",
"elaborated_type_specifier : UNION IDENTIFIER",
"argument_declaration_list : arg_declaration_list_opt triple_dot_opt",
"argument_declaration_list : arg_declaration_list ',' TRIPLE_DOT",
"arg_declaration_list_opt :",
"arg_declaration_list_opt : arg_declaration_list",
"triple_dot_opt :",
"triple_dot_opt : TRIPLE_DOT",
"arg_declaration_list : arg_declaration_list ',' argument_declaration",
"arg_declaration_list : argument_declaration",
"argument_declaration : decl_specifiers abstract_decl_opt",
"$$2 :",
"argument_declaration : decl_specifiers abstract_decl_opt '=' $$2 const_expression",
"argument_declaration : decl_specifiers abstract_decl_opt dname abstract_decl_opt",
"$$3 :",
"argument_declaration : decl_specifiers abstract_decl_opt dname abstract_decl_opt '=' $$3 const_expression",
"abstract_decl_opt :",
"abstract_decl_opt : abstract_decl",
"abstract_decl : abstract_decl ptr_operator",
"$$4 :",
"abstract_decl : '[' $$4 const_expression ']'",
"$$5 :",
"abstract_decl : abstract_decl '[' $$5 const_expression ']'",
"abstract_decl : ptr_operator",
"abstract_decl : '(' abstract_decl ')'",
"declarator : dname",
"declarator : declarator ptr_operator",
"$$6 :",
"declarator : declarator '[' $$6 const_expression ']'",
"declarator : '(' declarator ')'",
"dname : IDENTIFIER",
"fct_decl : '(' argument_declaration_list ')' cv_qualifier_list_opt fct_body_or_semicolon",
"fct_name : IDENTIFIER",
"fct_name : IDENTIFIER array_decls",
"$$7 :",
"fct_name : IDENTIFIER '=' $$7 const_expression",
"$$8 :",
"fct_name : IDENTIFIER array_decls '=' $$8 const_expression",
"$$9 :",
"array_decls : '[' $$9 const_expression ']'",
"$$10 :",
"array_decls : array_decls '[' $$10 const_expression ']'",
"ptr_operators_opt :",
"ptr_operators_opt : ptr_operators",
"ptr_operators : ptr_operator",
"ptr_operators : ptr_operators ptr_operator",
"ptr_operator : '*' cv_qualifier_list_opt",
"ptr_operator : '&' cv_qualifier_list_opt",
"cv_qualifier_list_opt :",
"cv_qualifier_list_opt : cv_qualifier_list",
"cv_qualifier_list : cv_qualifier",
"cv_qualifier_list : cv_qualifier_list cv_qualifier",
"cv_qualifier : CONST",
"cv_qualifier : VOLATILE",
"fct_body_or_semicolon : ';'",
"fct_body_or_semicolon : fct_body",
"fct_body_or_semicolon : '=' INT_VAL ';'",
"$$11 :",
"fct_body : '{' $$11 '}'",
"$$12 :",
"class_specifier : full_class_head '{' $$12 opt_obj_member_list '}'",
"class_specifier : class_head",
"class_specifier : class_head '(' IDENTIFIER ')'",
"class_specifier : template_spec whatever",
"whatever : IDENTIFIER",
"whatever : simple_type_name",
"whatever : type_specifier",
"whatever : storage_class_specifier",
"whatever : fct_specifier",
"class_head : class_key class_name",
"class_head : class_key IDENTIFIER class_name",
"full_class_head : class_head opt_base_spec",
"opt_base_spec :",
"opt_base_spec : base_spec",
"opt_obj_member_list :",
"opt_obj_member_list : obj_member_list",
"obj_member_list : obj_member_list obj_member_area",
"obj_member_list : obj_member_area",
"qt_access_specifier : access_specifier",
"qt_access_specifier : SLOTS",
"$$13 :",
"obj_member_area : qt_access_specifier $$13 slot_area",
"$$14 :",
"obj_member_area : SIGNALS $$14 ':' opt_signal_declarations",
"obj_member_area : Q_OBJECT",
"slot_area : SIGNALS ':'",
"slot_area : SLOTS ':' opt_slot_declarations",
"$$15 :",
"slot_area : ':' $$15 opt_slot_declarations",
"slot_area : IDENTIFIER",
"opt_signal_declarations :",
"opt_signal_declarations : signal_declarations",
"signal_declarations : signal_declarations signal_declaration",
"signal_declarations : signal_declaration",
"signal_declaration : signal_or_slot",
"opt_slot_declarations :",
"opt_slot_declarations : slot_declarations",
"slot_declarations : slot_declarations slot_declaration",
"slot_declarations : slot_declaration",
"slot_declaration : signal_or_slot",
"opt_semicolons :",
"opt_semicolons : opt_semicolons ';'",
"base_spec : ':' base_list",
"base_list : base_list ',' base_specifier",
"base_list : base_specifier",
"qt_macro_name : IDENTIFIER '(' IDENTIFIER ')'",
"qt_macro_name : IDENTIFIER '(' simple_type_name ')'",
"base_specifier : complete_class_name",
"base_specifier : VIRTUAL access_specifier complete_class_name",
"base_specifier : VIRTUAL complete_class_name",
"base_specifier : access_specifier VIRTUAL complete_class_name",
"base_specifier : access_specifier complete_class_name",
"base_specifier : qt_macro_name",
"base_specifier : VIRTUAL access_specifier qt_macro_name",
"base_specifier : VIRTUAL qt_macro_name",
"base_specifier : access_specifier VIRTUAL qt_macro_name",
"base_specifier : access_specifier qt_macro_name",
"access_specifier : PRIVATE",
"access_specifier : PROTECTED",
"access_specifier : PUBLIC",
"operator_name : decl_specs_opt IDENTIFIER ptr_operators_opt",
"operator_name : decl_specs_opt simple_type_name ptr_operators_opt",
"operator_name : '+'",
"operator_name : '-'",
"operator_name : '*'",
"operator_name : '/'",
"operator_name : '%'",
"operator_name : '^'",
"operator_name : '&'",
"operator_name : '|'",
"operator_name : '~'",
"operator_name : '!'",
"operator_name : '='",
"operator_name : '<'",
"operator_name : '>'",
"operator_name : '+' '='",
"operator_name : '-' '='",
"operator_name : '*' '='",
"operator_name : '/' '='",
"operator_name : '%' '='",
"operator_name : '^' '='",
"operator_name : '&' '='",
"operator_name : '|' '='",
"operator_name : '~' '='",
"operator_name : '!' '='",
"operator_name : '=' '='",
"operator_name : '<' '='",
"operator_name : '>' '='",
"operator_name : '<' '<'",
"operator_name : '>' '>'",
"operator_name : '<' '<' '='",
"operator_name : '>' '>' '='",
"operator_name : '&' '&'",
"operator_name : '|' '|'",
"operator_name : '+' '+'",
"operator_name : '-' '-'",
"operator_name : ','",
"operator_name : '-' '>' '*'",
"operator_name : '-' '>'",
"operator_name : '(' ')'",
"operator_name : '[' ']'",
"opt_virtual :",
"opt_virtual : VIRTUAL",
"type_and_name : type_name fct_name",
"type_and_name : fct_name",
"type_and_name : opt_virtual '~' fct_name",
"type_and_name : decl_specs type_name decl_specs_opt ptr_operators_opt fct_name",
"type_and_name : decl_specs type_name",
"type_and_name : type_name ptr_operators fct_name",
"type_and_name : type_name decl_specs ptr_operators_opt fct_name",
"type_and_name : type_name OPERATOR operator_name",
"type_and_name : OPERATOR operator_name",
"type_and_name : decl_specs type_name decl_specs_opt ptr_operators_opt OPERATOR operator_name",
"type_and_name : type_name ptr_operators OPERATOR operator_name",
"type_and_name : type_name decl_specs ptr_operators_opt OPERATOR operator_name",
"signal_or_slot : type_and_name fct_decl opt_semicolons",
"signal_or_slot : type_and_name opt_bitfield ';' opt_semicolons",
"signal_or_slot : type_and_name opt_bitfield ',' member_declarator_list ';' opt_semicolons",
"signal_or_slot : enum_specifier ';' opt_semicolons",
"member_declarator_list : member_declarator",
"member_declarator_list : member_declarator_list ',' member_declarator",
"member_declarator : declarator",
"$$16 :",
"member_declarator : IDENTIFIER ':' $$16 const_expression",
"$$17 :",
"member_declarator : ':' $$17 const_expression",
"opt_bitfield :",
"$$18 :",
"opt_bitfield : ':' $$18 const_expression",
"$$19 :",
"enum_specifier : ENUM opt_identifier '{' $$19 '}'",
"opt_identifier :",
"opt_identifier : IDENTIFIER",
};
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH 500
#endif
#endif
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short yyss[YYSTACKSIZE];
YYSTYPE yyvs[YYSTACKSIZE];
#define yystacksize YYSTACKSIZE
#line 764 "moc.y"

#if defined(_OS_WIN32_)
#include <io.h>
#undef isatty
extern "C" int hack_isatty( int )
{
    return 0;
}
#define isatty hack_isatty
#else
#include <unistd.h>
#endif

#include "lex.yy.c"

void 	  init();				// initialize
void 	  initClass();				// prepare for new class
void 	  generateClass();			// generate C++ code for class
void 	  initExpression();			// prepare for new expression
QString	  combinePath( const char *, const char * );

QString	  fileName;				// file name
QString	  outputFile;				// output file name
QString   includeFile;				// name of #include file
QString	  includePath;				// #include file path
QString	  qtPath;				// #include qt file path
bool	  noInclude     = FALSE;		// no #include <filename>
bool	  generatedCode = FALSE;		// no code generated
bool	  mocError = FALSE;			// moc parsing error occurred
QString	  className;				// name of parsed class
QString	  superclassName;			// name of super class
FuncList  signals;				// signal interface
FuncList  slots;				// slots interface

FILE  *out;					// output file

int yyparse();

void replace( char *s, char c1, char c2 );

int main( int argc, char **argv )
{
    bool autoInclude = TRUE;
    char *error	     = 0;
    qtPath = "";
    for ( int n=1; n<argc && error==0; n++ ) {
	QString arg = argv[n];
	if ( arg[0] == '-' ) {			// option
	    QString opt = &arg[1];
	    if ( opt[0] == 'o' ) {		// output redirection
		if ( opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing output-file name";
			break;
		    }
		    outputFile = argv[++n];
		} else
		    outputFile = &opt[1];
	    } else if ( opt == "i" ) {		// no #include statement
		noInclude   = TRUE;
		autoInclude = FALSE;
	    } else if ( opt[0] == 'f' ) {	// produce #include statement
		noInclude   = FALSE;
		autoInclude = FALSE;
		if ( opt[1] ) {			// -fsomething.h
		    includeFile = &opt[1];
		}
	    } else if ( opt[0] == 'p' ) {	// include file path
		if ( opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing path name for the -p option.";
			break;
		    }
		    includePath = argv[++n];
		} else {
		    includePath = &opt[1];
		}
	    } else if ( opt[0] == 'q' ) {	// qt include file path
		if ( opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing path name for the -q option.";
			break;
		    }
		    qtPath = argv[++n];
		} else {
		    qtPath = &opt[1];
		}
		replace(qtPath.data(),'\\','/');
		if ( qtPath.right(1) != "/" )
		    qtPath += '/';
	    } else if ( opt == "k" ) {		// don't stop on errors
		errorControl = TRUE;
	    } else if ( opt == "nw" ) {		// don't display warnings
		displayWarnings = FALSE;
	    } else if ( opt == "ldbg" ) {	// lex debug output
		lexDebug = TRUE;
	    } else if ( opt == "ydbg" ) {	// yacc debug output
		yydebug = TRUE;
	    } else if ( opt == "dbg" ) {	// non-signal members are slots
		grammarDebug = TRUE;
	    } else {
		error = "Invalid argument";
	    }
	} else {
	    if ( !fileName.isNull() )		// can handle only one file
		error	 = "Too many input files specified";
	    else
		fileName = arg.copy();
	}
    }

    if ( autoInclude ) {
	int ppos = fileName.findRev('.');
	if ( ppos != -1 && tolower( fileName[ppos + 1] ) == 'h' )
	    noInclude = FALSE;
	else
	    noInclude = TRUE;
    }
    if ( !fileName.isEmpty() && !outputFile.isEmpty() &&
	 includeFile.isEmpty() && includePath.isEmpty() ) {
	includeFile = combinePath(fileName,outputFile);
    }
    if ( includeFile.isEmpty() )
	includeFile = fileName.copy();
    if ( !includePath.isEmpty() ) {
	if ( includePath.right(1) != "/" )
	    includePath += '/';
	includeFile = includePath + includeFile;
    }
    if ( fileName.isNull() && !error ) {
	fileName = "standard input";
	yyin	 = stdin;
    } else if ( argc < 2 || error ) {		// incomplete/wrong args
	fprintf( stderr, "Qt meta object compiler\n" );
	if ( error )
	    fprintf( stderr, "moc: %s\n", error );
	fprintf( stderr, "Usage:  moc [options] <header-file>\n"
		 "\t-o file  Write output to file rather than stdout\n"
		 "\t-i       Do not generate an #include statement\n"
		 "\t-f[file] Force #include, optional file name\n"
		 "\t-p path  Path prefix for included file\n"
		 "\t-k       Do not stop on errors\n"
		 "\t-nw      Do not display warnings\n"
#if 0
		 "\t-ldbg    lex debug output\n"
		 "\t-ydbg    yacc debug output\n"
		 "\t-dbg     test parser, all non-signal members are slots\n"
#endif
		 );
	return 1;
    } else {
	yyin = fopen( (const char *)fileName, "r" );
	if ( !yyin ) {
	    fprintf( stderr, "moc: %s: No such file\n", (const char*)fileName);
	    return 1;
	}
    }
    if ( !outputFile.isEmpty() ) {		// output file specified
	out = fopen( (const char *)outputFile, "w" );	// create output file
	if ( !out ) {
	    fprintf( stderr, "moc: Cannot create %s\n",
		     (const char*)outputFile );
	    return 1;
	}
    } else {					// use stdout
	out = stdout;
    }
    init();
    yyparse();
    fclose( yyin );
    if ( !outputFile.isNull() )
	fclose( out );

    if ( !generatedCode && displayWarnings && !mocError ) {
        fprintf( stderr, "%s:%d: Warning: %s\n", fileName.data(), 0,
		 "No relevant classes found. No output generated." );
    }

    slots.clear();
    signals.clear();

    return mocError ? 1 : 0;
}

void replace( char *s, char c1, char c2 )
{
    if ( !s )
	return;
    while ( *s ) {
	if ( *s == c1 )
	    *s = c2;
	s++;
    }
}

/*
  This function looks at two file names and returns the name of the
  infile, with a path relative to outfile. Examples:
    /tmp/abc	/tmp/bcd	->	abc
    xyz/a/bc	xyz/b/ac	->	../a/bc
    /tmp/abc	xyz/klm		-)	/tmp/abc
 */

QString combinePath( const char *infile, const char *outfile )
{
    QString a = infile;  replace(a.data(),'\\','/');
    QString b = outfile; replace(b.data(),'\\','/');
    a = a.stripWhiteSpace();
    b = b.stripWhiteSpace();
#if !defined(UNIX)
    a = a.lower();
    b = b.lower();
#endif
    QString r;
    int i = 0;
    int ncommondirs = 0;
    while ( a[i] && a[i] == b[i] ) {
	if ( a[i] == '/' && i > 0 )
	    ncommondirs++;
	i++;
    }
    if ( ncommondirs > 0 ) {			// common base directory
	while ( i>=0 ) {
	    if ( a[i] == '/' && b[i] == '/' )
		break;
	    --i;
	}
	++i;
	a = &a[i];
	b = &b[i];
    } else {
#if defined(UNIX)
	if ( a[0] == '/' )
#else
	if ( (a[0] == '/') || (isalpha(a[0]) && a[1] == ':') )
#endif
	    return a;
	b = &b[i];
    }
    i = b.contains('/');
    while ( i-- > 0 )
	r += "../";
    r += a;
    return r;
}


#define getenv hack_getenv			// workaround for byacc
char *getenv()		     { return 0; }
char *getenv( const char * ) { return 0; }


void init()					// initialize
{
    BEGIN OUTSIDE;
    lineNo	 = 1;
    skipClass	 = FALSE;
    skipFunc	 = FALSE;
    signals.setAutoDelete( TRUE );
    slots.setAutoDelete( TRUE );

    tmpArgList	 = new ArgList;
    CHECK_PTR( tmpArgList );
    tmpFunc	 = new Function;
    CHECK_PTR( tmpFunc );
}

void initClass()				 // prepare for new class
{
    tmpAccessPerm    = _PRIVATE;
    subClassPerm     = _PRIVATE;
    Q_OBJECTdetected = FALSE;
    skipClass	     = FALSE;
    templateClass    = FALSE;
    slots.clear();
    signals.clear();
}

//
// Remove white space from SIGNAL and SLOT names.
// This function has been copied from qobject.cpp.
//

inline bool isIdentChar( char x )
{						// Avoid bug in isalnum
    return x == '_' || (x >= '0' && x <= '9') ||
	 (x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z');
}

inline bool isSpace( char x )
{
#if defined(_CC_BOR_)
  /*
    Borland C++ 4.5 has a weird isspace() bug.
    isspace() usually works, but not here.
    This implementation is sufficient for our internal use: rmWS()
  */
    return (uchar)x <= 32;
#else
    return isspace( x );
#endif
}

static QString rmWS( const char *src )
{
    QString result( strlen(src)+1 );
    char *d = result.data();
    char *s = (char *)src;
    char last = 0;
    while( *s && isSpace(*s) )			// skip leading space
	s++;
    while ( *s ) {
	while ( *s && !isSpace(*s) )
	    last = *d++ = *s++;
	while ( *s && isSpace(*s) )
	    s++;
	if ( *s && isIdentChar(*s) && isIdentChar(last) )
	    last = *d++ = ' ';
    }
    result.truncate( (int)(d - result.data()) );
    return result;
}


void initExpression()
{
    tmpExpression = "";
}

void addExpressionString( char *s )
{
    tmpExpression += s;
}

void addExpressionChar( char c )
{
    tmpExpression += c;
}

void yyerror( char *msg )			// print yacc error message
{
    mocError = TRUE;
    fprintf( stderr, "%s:%d: Error: %s\n", fileName.data(), lineNo, msg );
}

void moc_err( char *s )
{
    yyerror( s );
    if ( errorControl )
	exit( -1 );
}

void moc_err( char *s1, char *s2 )
{
    static char tmp[1024];
    sprintf( tmp, s1, s2 );
    yyerror( tmp );
    if ( errorControl )
	exit( -1 );
}

void moc_warn( char *msg )
{
    if ( displayWarnings )
	fprintf( stderr, "%s:%d: Warning: %s\n", fileName.data(), lineNo, msg);
}

void func_warn( char *msg )
{
    moc_warn( msg );
    skipFunc = TRUE;
}

void operatorError()
{
    moc_warn("Operator functions cannot be signals or slots.");
    skipFunc = TRUE;
}

#ifndef yywrap
int yywrap()					// more files?
{
    return 1;					// end of file
}
#endif

char *stradd( const char *s1, const char *s2 )	// adds two strings
{
    char *n = new char[strlen(s1)+strlen(s2)+1];
    CHECK_PTR( n );
    strcpy( n, s1 );
    strcat( n, s2 );
    return n;
}

char *stradd( const char *s1, const char *s2, const char *s3 )// adds 3 strings
{
    char *n = new char[strlen(s1)+strlen(s2)+strlen(s3)+1];
    CHECK_PTR( n );
    strcpy( n, s1 );
    strcat( n, s2 );
    strcat( n, s3 );
    return n;
}

char *stradd( const char *s1, const char *s2,
	      const char *s3, const char *s4 )// adds 4 strings
{
    char *n = new char[strlen(s1)+strlen(s2)+strlen(s3)+strlen(s4)+1];
    CHECK_PTR( n );
    strcpy( n, s1 );
    strcat( n, s2 );
    strcat( n, s3 );
    strcat( n, s4 );
    return n;
}


char *straddSpc( const char *s1, const char *s2 )
{
    char *n = new char[strlen(s1)+strlen(s2)+2];
    CHECK_PTR( n );
    strcpy( n, s1 );
    strcat( n, " " );
    strcat( n, s2 );
    return n;
}

char *straddSpc( const char *s1, const char *s2, const char *s3 )
{
    char *n = new char[strlen(s1)+strlen(s2)+strlen(s3)+3];
    CHECK_PTR( n );
    strcpy( n, s1 );
    strcat( n, " " );
    strcat( n, s2 );
    strcat( n, " " );
    strcat( n, s3 );
    return n;
}

char *straddSpc( const char *s1, const char *s2,
	      const char *s3, const char *s4 )
{
    char *n = new char[strlen(s1)+strlen(s2)+strlen(s3)+strlen(s4)+4];
    CHECK_PTR( n );
    strcpy( n, s1 );
    strcat( n, " " );
    strcat( n, s2 );
    strcat( n, " " );
    strcat( n, s3 );
    strcat( n, " " );
    strcat( n, s4 );
    return n;
}

// Generate C++ code for building member function table

const int Slot_Num   = 1;
const int Signal_Num = 2;

void generateFuncs( FuncList *list, char *functype, int num )
{
    Function *f;
    for ( f=list->first(); f; f=list->next() ) {
	QString typstr = "";
	int count = 0;
	Argument *a = f->args->first();
	while ( a ) {
	    if ( !a->leftType.isEmpty() || ! a->rightType.isEmpty() ) {
		if ( count++ )
		    typstr += ",";
		typstr += a->leftType;
		typstr += a->rightType;
	    }
	    a = f->args->next();
	}
	fprintf( out, "    typedef %s(%s::*m%d_t%d)(%s)%s;\n",
		 (const char*)f->type, (const char*)className, num, list->at(),
		 (const char*)typstr,  (const char*)f->qualifier );
	f->type = f->name.copy();
	f->type += "(";
	f->type += typstr;
	f->type += ")";
    }
    for ( f=list->first(); f; f=list->next() )
	fprintf( out, "    m%d_t%d v%d_%d = &%s::%s;\n", num, list->at(),
		 num, list->at(), (const char*)className,(const char*)f->name);
    if ( list->count() )
	fprintf( out, "    QMetaData *%s_tbl = new QMetaData[%d];\n",
		 functype, list->count() );
    for ( f=list->first(); f; f=list->next() )
	fprintf( out, "    %s_tbl[%d].name = \"%s\";\n",
		 functype, list->at(), (const char*)f->type );
    for ( f=list->first(); f; f=list->next() )
	fprintf( out, "    %s_tbl[%d].ptr = *((QMember*)&v%d_%d);\n",
		 functype, list->at(), num, list->at() );
}

void generateClass()		      // generate C++ source code for a class
{
    static int gen_count = 0;
    char *hdr1 = "/****************************************************************************\n"
		 "** %s meta object code from reading C++ file '%s'\n**\n";
    char *hdr2 = "** Created: %s\n"
		 "**      by: The Qt Meta Object Compiler ($Revision: 2.25.2.12 $)\n**\n";
    char *hdr3 = "** WARNING! All changes made in this file will be lost!\n";
    char *hdr4 = "*****************************************************************************/\n\n";
    int   i;

    if ( skipClass )				// don't generate for class
	return;
    if ( !Q_OBJECTdetected ) {
	if ( signals.count() == 0 && slots.count() == 0 )
	    return;
	generatedCode = TRUE;
	if ( displayWarnings )
	    moc_err("The declaration of the class \"%s\" contains slots "
		    "and/or signals\n\t but no Q_OBJECT macro!", className.data());
    } else {
	if ( superclassName.isEmpty() )
	    moc_err("The declaration of the class \"%s\" contains the\n"
		    "\tQ_OBJECT macro but does not inherit from any class!\n"
		    "\tInherit from QObject or one of its descendants"
		    " or remove Q_OBJECT. ", className.data() );
    }
    if ( templateClass ) {			// don't generate for class
	moc_err( "Sorry, Qt does not support templates that contain\n"
		 "signals, slots or Q_OBJECT. This will be supported soon." );
	return;
    }
    generatedCode = TRUE;
    if ( gen_count++ == 0 ) {			// first class to be generated
	QDateTime dt = QDateTime::currentDateTime();
	QString dstr = dt.toString();
	QString fn = fileName;
	i = fileName.length()-1;
	while ( i>0 && fileName[i-1] != '/' && fileName[i-1] != '\\' )
	    i--;				// skip path
	if ( i >= 0 )
	    fn = &fileName[i];
	fprintf( out, hdr1, (const char*)className, (const char*)fn );
	fprintf( out, hdr2, (const char*)dstr );
	fprintf( out, hdr3 );
	fprintf( out, hdr4 );
	fprintf( out, "#if !defined(Q_MOC_OUTPUT_REVISION)\n" );
	fprintf( out, "#define Q_MOC_OUTPUT_REVISION %d\n", formatRevision );
	fprintf( out, "#elif Q_MOC_OUTPUT_REVISION != %d\n", formatRevision );
	fprintf( out, "#error \"Moc format conflict - "
		 "please regenerate all moc files\"\n" );
	fprintf( out, "#endif\n\n" );
	if ( !noInclude )
	    fprintf( out, "#include \"%s\"\n", (const char*)includeFile );
	fprintf( out, "#include <%sqmetaobject.h>\n", (const char*)qtPath );
	fprintf( out, "\n\n" );
    } else {
	fprintf( out, "\n\n" );
    }

//
// Generate virtual function className()
//
    fprintf( out, "const char *%s::className() const\n{\n    ",
	     (const char*)className );
    fprintf( out, "return \"%s\";\n}\n\n", (const char*)className );

//
// Generate static metaObj variable
//
    fprintf( out, "QMetaObject *%s::metaObj = 0;\n\n", (const char*)className);

//
// Generate static meta-object constructor-object (we don't rely on
// it, except for QBuilder).
//
    fprintf( out, "\n#if QT_VERSION >= 200\n" );
    fprintf( out, "static QMetaObjectInit init_%s(&%s::staticMetaObject);\n\n",
	(const char*)className, (const char*)className );
    fprintf( out, "#endif\n\n" );

//
// Generate initMetaObject member function
//
    fprintf( out, "void %s::initMetaObject()\n{\n", (const char*)className );
    fprintf( out, "    if ( metaObj )\n\treturn;\n" );
    fprintf( out, "    if ( strcmp(%s::className(), \"%s\") != 0 )\n"
	          "\tbadSuperclassWarning(\"%s\",\"%s\");\n",
             (const char*)superclassName, (const char*)superclassName,
             (const char*)className, (const char*)superclassName );
    fprintf( out, "\n#if QT_VERSION >= 200\n" );
    fprintf( out, "    staticMetaObject();\n");
    fprintf( out, "}\n\n");

//
// Generate staticMetaObject member function
//
    fprintf( out, "void %s::staticMetaObject()\n{\n", (const char*)className );
    fprintf( out, "    if ( metaObj )\n\treturn;\n" );
    fprintf( out, "    %s::staticMetaObject();\n", (const char*)superclassName );
    fprintf( out, "#else\n\n" );
    fprintf( out, "    %s::initMetaObject();\n", (const char*)superclassName );
    fprintf( out, "#endif\n\n" );
//
// Build slots array in staticMetaObject()
//
    generateFuncs( &slots, "slot", Slot_Num );

//
// Build signals array in staticMetaObject()
//
    generateFuncs( &signals, "signal", Signal_Num );

//
// Finally code to create and return meta object
//
    fprintf( out, "    metaObj = new QMetaObject( \"%s\", \"%s\",\n",
	     (const char*)className, (const char*)superclassName );
    if ( slots.count() )
	fprintf( out, "\tslot_tbl, %d,\n", slots.count() );
    else
	fprintf( out, "\t0, 0,\n" );
    if ( signals.count() )
	fprintf( out, "\tsignal_tbl, %d );\n", signals.count());
    else
	fprintf( out, "\t0, 0 );\n" );

    fprintf( out, "}\n" );

//
// End of function initMetaObject()
//

//
// Generate internal signal functions
//
    Function *f;
    f = signals.first();			// make internal signal methods
    static bool included_list_stuff = FALSE;
    while ( f ) {
	QString typstr = "";			// type string
	QString valstr = "";			// value string
	QString argstr = "";			// argument string (type+value)
	char	buf[12];
	Argument *a = f->args->first();
	QString typvec[32], valvec[32], argvec[32];
	typvec[0] = "";
	valvec[0] = "";
	argvec[0] = "";

	i = 0;
	while ( a ) {				// argument list
	    if ( !a->leftType.isEmpty() || !a->rightType.isEmpty() ) {
		if ( i ) {
		    typstr += ",";
		    valstr += ", ";
		    argstr += ", ";
		}
		typstr += a->leftType;
		typstr += a->rightType;
		argstr += a->leftType;
		argstr += " ";
		sprintf( buf, "t%d", i );
		valstr += buf;
		argstr += buf;
		argstr += a->rightType;
		++i;
		typvec[i] = typstr.copy();
		valvec[i] = valstr.copy();
		argvec[i] = argstr.copy();
	    }
	    a = f->args->next();
	}

	bool predef_call = FALSE;
	if ( typstr.isEmpty() || typstr == "short" || typstr == "int" ||
	     typstr == "long" || typstr == "char*" || typstr == "const char*"){
	    predef_call = TRUE;
	}
	if ( !predef_call && !included_list_stuff ) {
	    // yes we need it, because otherwise QT_VERSION may not be defined
	    fprintf( out, "\n#include <%sqobjectdefs.h>\n", (const char*)qtPath );
	    fprintf( out, "#if QT_VERSION >= 141\n" );
	    fprintf( out, "/" "/ newer implementation\n" );
	    fprintf( out, "#include <%sqsignalslotimp.h>\n", (const char*)qtPath );
	    fprintf( out, "#else\n" );
	    fprintf( out, "/" "/ for late-model 1.x header files\n" );
	    fprintf( out, "#if !defined(Q_MOC_CONNECTIONLIST_DECLARED)\n" );
	    fprintf( out, "#define Q_MOC_CONNECTIONLIST_DECLARED\n" );
	    fprintf( out, "#include <%sqlist.h>\n", (const char*)qtPath );
	    fprintf( out, "Q_DECLARE(QListM,QConnection);\n" );
	    fprintf( out, "Q_DECLARE(QListIteratorM,QConnection);\n" );
	    fprintf( out, "#endif\n" );
	    fprintf( out, "#endif\n" );
	    included_list_stuff = TRUE;
	}

	fprintf( out, "\n/" /* c++ */ "/ SIGNAL %s\n", (const char*)f->name );
	fprintf( out, "void %s::%s(", (const char*)className,
		 (const char*)f->name );

	if ( argstr.isEmpty() )
	    fprintf( out, ")\n{\n" );
	else
	    fprintf( out, " %s )\n{\n", (const char*)argstr );

	if ( predef_call ) {
	    fprintf( out, "    activate_signal( \"%s(%s)\"",
		     (const char*)f->name, (const char*)typstr );
	    if ( !valstr.isEmpty() )
		fprintf( out, ", %s", (const char*)valstr );
	    fprintf( out, " );\n}\n" );
	    f = signals.next();
	    continue;
	}

	int nargs = f->args->count();
	fprintf( out, "    QConnectionList *clist = receivers(\"%s(%s)\");\n",
		 (const char*)f->name, (const char*)typstr );
	fprintf( out, "    if ( !clist || signalsBlocked() )\n\treturn;\n" );
	if ( nargs ) {
	    for ( i=0; i<=nargs; i++ ) {
		fprintf( out, "    typedef void (QObject::*RT%d)(%s);\n",
			 i, (const char*)typvec[i] );
		fprintf( out, "    typedef RT%d *PRT%d;\n", i, i );
	    }
	} else {
	    fprintf( out, "    typedef void (QObject::*RT)(%s);\n",
		     (const char*)typstr);
	    fprintf( out, "    typedef RT *PRT;\n" );	
	}
	if ( nargs ) {
	    for ( i=0; i<=nargs; i++ )
		fprintf( out, "    RT%d r%d;\n", i, i );
	} else {
	    fprintf( out, "    RT r;\n" );
	}
	fprintf( out, "    QConnectionListIt it(*clist);\n" );
	fprintf( out, "    QConnection   *c;\n" );
	fprintf( out, "    QSenderObject *object;\n" );
	fprintf( out, "    while ( (c=it.current()) ) {\n" );
	fprintf( out, "\t++it;\n" );
	fprintf( out, "\tobject = (QSenderObject*)c->object();\n" );
	fprintf( out, "\tobject->setSender( this );\n" );
	if ( nargs ) {
	    fprintf( out, "\tswitch ( c->numArgs() ) {\n" );
	    for ( i=0; i<=nargs; i++ ) {
		fprintf( out, "\t    case %d:\n", i );
		fprintf( out, "\t\tr%d = *((PRT%d)(c->member()));\n", i, i );
		fprintf( out, "\t\t(object->*r%d)(%s);\n",
			 i, (const char*)valvec[i] );
		fprintf( out, "\t\tbreak;\n" );
	    }
	    fprintf( out, "\t}\n" );
	} else {
	    fprintf( out, "\tr = *((PRT)(c->member()));\n" );
	    fprintf( out, "\t(object->*r)(%s);\n", (const char*)valstr );
	}
	fprintf( out, "    }\n}\n" );
	f = signals.next();
    }

}


ArgList *addArg( Argument *a )			// add argument to list
{
    tmpArgList->append( a );
    return tmpArgList;
}

void addMember( char m )
{
    if ( skipFunc ) {
	tmpArgList  = new ArgList;   // ugly but works!
	CHECK_PTR( tmpArgList );
	tmpFunc	    = new Function;
	CHECK_PTR( tmpFunc );
	skipFunc    = FALSE;
	return;
    }
    if ( m == 's' && tmpFunc->type != "void" ) {
	moc_err( "Signals must have \"void\" as their return type" );
	return;
    }
    tmpFunc->accessPerm = tmpAccessPerm;
    tmpFunc->args	= tmpArgList;
    tmpFunc->lineNo	= lineNo;
    tmpArgList		= new ArgList;
    CHECK_PTR( tmpArgList );
    switch( m ) {
	case 's': signals.append( tmpFunc ); break;
	case 't': slots.  append( tmpFunc ); break;
    }
    skipFunc = FALSE;
    tmpFunc  = new Function;
    CHECK_PTR( tmpFunc );
}
#line 1654 "y.tab.c"
#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
#if defined(__STDC__)
yyparse(void)
#else
yyparse()
#endif
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register char *yys;
    extern char *getenv();

    if (yys = getenv("YYDEBUG"))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yyss + yystacksize - 1)
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
    yyerror("syntax error");
#ifdef lint
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yyss + yystacksize - 1)
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 3:
#line 219 "moc.y"
{ initClass(); }
break;
case 4:
#line 220 "moc.y"
{ generateClass();
						BEGIN OUTSIDE; }
break;
case 5:
#line 227 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 6:
#line 228 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 7:
#line 232 "moc.y"
{ yyval.string = stradd( yyvsp[-3].string, "<",
				     tmpExpression =
				     tmpExpression.stripWhiteSpace(), ">" ); }
break;
case 8:
#line 243 "moc.y"
{ initExpression();
						    templLevel = 1;
						    BEGIN IN_TEMPL_ARGS; }
break;
case 9:
#line 256 "moc.y"
{ initExpression();
						    BEGIN IN_EXPR; }
break;
case 10:
#line 262 "moc.y"
{ yyval.string = ""; }
break;
case 11:
#line 263 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 12:
#line 264 "moc.y"
{ yyval.string = ""; }
break;
case 13:
#line 265 "moc.y"
{ skipFunc = TRUE; yyval.string = ""; }
break;
case 14:
#line 266 "moc.y"
{ skipFunc = TRUE; yyval.string = ""; }
break;
case 15:
#line 270 "moc.y"
{ yyval.string = straddSpc(yyvsp[-2].string,yyvsp[-1].string,yyvsp[0].string); }
break;
case 16:
#line 273 "moc.y"
{ yyval.string = ""; }
break;
case 17:
#line 274 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 18:
#line 277 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 19:
#line 278 "moc.y"
{ yyval.string = straddSpc(yyvsp[-1].string,yyvsp[0].string); }
break;
case 22:
#line 283 "moc.y"
{ skipFunc = TRUE; }
break;
case 26:
#line 291 "moc.y"
{ yyval.string = "const"; }
break;
case 27:
#line 292 "moc.y"
{ yyval.string = "volatile"; }
break;
case 28:
#line 295 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 29:
#line 296 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 30:
#line 297 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 31:
#line 301 "moc.y"
{ yyval.string = straddSpc(yyvsp[-1].string,yyvsp[0].string); }
break;
case 32:
#line 302 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 33:
#line 304 "moc.y"
{ yyval.string = "char"; }
break;
case 34:
#line 305 "moc.y"
{ yyval.string = "short"; }
break;
case 35:
#line 306 "moc.y"
{ yyval.string = "int"; }
break;
case 36:
#line 307 "moc.y"
{ yyval.string = "long"; }
break;
case 37:
#line 308 "moc.y"
{ yyval.string = "signed"; }
break;
case 38:
#line 309 "moc.y"
{ yyval.string = "unsigned"; }
break;
case 39:
#line 310 "moc.y"
{ yyval.string = "float"; }
break;
case 40:
#line 311 "moc.y"
{ yyval.string = "double"; }
break;
case 41:
#line 312 "moc.y"
{ yyval.string = "void"; }
break;
case 42:
#line 316 "moc.y"
{ yyval.string = stradd( "template<",
				     tmpExpression =
				     tmpExpression.stripWhiteSpace(), ">" ); }
break;
case 44:
#line 322 "moc.y"
{ templateClass = TRUE; }
break;
case 45:
#line 326 "moc.y"
{ yyval.string = "class"; }
break;
case 46:
#line 327 "moc.y"
{ yyval.string = "struct"; }
break;
case 47:
#line 330 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 48:
#line 332 "moc.y"
{ yyval.string = stradd( "::", yyvsp[0].string ); }
break;
case 49:
#line 336 "moc.y"
{ yyval.string = stradd( yyvsp[-2].string, "::", yyvsp[0].string );}
break;
case 50:
#line 337 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 51:
#line 341 "moc.y"
{ yyval.string = straddSpc(yyvsp[-1].string,yyvsp[0].string); }
break;
case 52:
#line 342 "moc.y"
{ yyval.string = stradd("enum ",yyvsp[0].string); }
break;
case 53:
#line 343 "moc.y"
{ yyval.string = stradd("union ",yyvsp[0].string); }
break;
case 54:
#line 348 "moc.y"
{ yyval.arg_list = yyvsp[-1].arg_list;}
break;
case 55:
#line 349 "moc.y"
{ yyval.arg_list = yyvsp[-2].arg_list;
				       moc_warn("Ellipsis not supported"
					       " in signals and slots.\n"
					       "Ellipsis argument ignored."); }
break;
case 56:
#line 355 "moc.y"
{ yyval.arg_list = tmpArgList; }
break;
case 57:
#line 356 "moc.y"
{ yyval.arg_list = yyvsp[0].arg_list; }
break;
case 59:
#line 360 "moc.y"
{ moc_warn("Ellipsis not supported"
					       " in signals and slots.\n"
					       "Ellipsis argument ignored."); }
break;
case 60:
#line 368 "moc.y"
{ yyval.arg_list = addArg(yyvsp[0].arg); }
break;
case 61:
#line 369 "moc.y"
{ yyval.arg_list = addArg(yyvsp[0].arg); }
break;
case 62:
#line 372 "moc.y"
{ yyval.arg = new Argument(straddSpc(yyvsp[-1].string,yyvsp[0].string),"");
				  CHECK_PTR(yyval.arg); }
break;
case 63:
#line 375 "moc.y"
{ expLevel = 1; }
break;
case 64:
#line 377 "moc.y"
{ yyval.arg = new Argument(straddSpc(yyvsp[-4].string,yyvsp[-3].string),"");
				  CHECK_PTR(yyval.arg); }
break;
case 65:
#line 381 "moc.y"
{ yyval.arg = new Argument(straddSpc(yyvsp[-3].string,yyvsp[-2].string),yyvsp[0].string);
				  CHECK_PTR(yyval.arg); }
break;
case 66:
#line 385 "moc.y"
{ expLevel = 1; }
break;
case 67:
#line 387 "moc.y"
{ yyval.arg = new Argument(straddSpc(yyvsp[-6].string,yyvsp[-5].string),yyvsp[-3].string);
				  CHECK_PTR(yyval.arg); }
break;
case 68:
#line 392 "moc.y"
{ yyval.string = ""; }
break;
case 69:
#line 393 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 70:
#line 397 "moc.y"
{ yyval.string = straddSpc(yyvsp[-1].string,yyvsp[0].string); }
break;
case 71:
#line 398 "moc.y"
{ expLevel = 1; }
break;
case 72:
#line 400 "moc.y"
{ yyval.string = stradd( "[",
				     tmpExpression =
				     tmpExpression.stripWhiteSpace(), "]" ); }
break;
case 73:
#line 403 "moc.y"
{ expLevel = 1; }
break;
case 74:
#line 405 "moc.y"
{ yyval.string = stradd( yyvsp[-4].string,"[",
				     tmpExpression =
				     tmpExpression.stripWhiteSpace(),"]" ); }
break;
case 75:
#line 408 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 76:
#line 409 "moc.y"
{ yyval.string = yyvsp[-1].string; }
break;
case 77:
#line 412 "moc.y"
{ yyval.string = ""; }
break;
case 78:
#line 414 "moc.y"
{ yyval.string = straddSpc(yyvsp[-1].string,yyvsp[0].string);}
break;
case 79:
#line 415 "moc.y"
{ expLevel = 1; }
break;
case 80:
#line 417 "moc.y"
{ yyval.string = stradd( yyvsp[-4].string,"[",
				     tmpExpression =
				     tmpExpression.stripWhiteSpace(),"]" ); }
break;
case 81:
#line 420 "moc.y"
{ yyval.string = yyvsp[-1].string; }
break;
case 83:
#line 431 "moc.y"
{ tmpFunc->args	     = yyvsp[-3].arg_list;
						  tmpFunc->qualifier = yyvsp[-1].string; }
break;
case 85:
#line 437 "moc.y"
{ func_warn("Variable as signal or slot."); }
break;
case 86:
#line 438 "moc.y"
{ expLevel=0; }
break;
case 87:
#line 440 "moc.y"
{ skipFunc = TRUE; }
break;
case 88:
#line 441 "moc.y"
{ expLevel=0; }
break;
case 89:
#line 443 "moc.y"
{ skipFunc = TRUE; }
break;
case 90:
#line 447 "moc.y"
{ expLevel = 1; }
break;
case 92:
#line 449 "moc.y"
{ expLevel = 1; }
break;
case 94:
#line 454 "moc.y"
{ yyval.string = ""; }
break;
case 95:
#line 455 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 96:
#line 458 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 97:
#line 459 "moc.y"
{ yyval.string = straddSpc(yyvsp[-1].string,yyvsp[0].string);}
break;
case 98:
#line 462 "moc.y"
{ yyval.string = straddSpc("*",yyvsp[0].string);}
break;
case 99:
#line 463 "moc.y"
{ yyval.string = stradd("&",yyvsp[0].string);}
break;
case 100:
#line 470 "moc.y"
{ yyval.string = ""; }
break;
case 101:
#line 471 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 102:
#line 474 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 103:
#line 476 "moc.y"
{ yyval.string = straddSpc(yyvsp[-1].string,yyvsp[0].string); }
break;
case 104:
#line 479 "moc.y"
{ yyval.string = "const"; }
break;
case 105:
#line 480 "moc.y"
{ yyval.string = "volatile"; }
break;
case 109:
#line 488 "moc.y"
{BEGIN IN_FCT; fctLevel = 1;}
break;
case 110:
#line 489 "moc.y"
{BEGIN QT_DEF; }
break;
case 111:
#line 496 "moc.y"
{ BEGIN IN_CLASS; level = 1; }
break;
case 112:
#line 498 "moc.y"
{ BEGIN QT_DEF; }
break;
case 113:
#line 499 "moc.y"
{ BEGIN QT_DEF;	  /* -- " -- */
						  skipClass = TRUE; }
break;
case 114:
#line 503 "moc.y"
{ BEGIN QT_DEF; /* catch ';' */
						  skipClass = TRUE; }
break;
case 115:
#line 505 "moc.y"
{ skipClass = TRUE;
						  BEGIN GIMME_SEMICOLON; }
break;
case 121:
#line 517 "moc.y"
{ className = yyvsp[0].string; }
break;
case 122:
#line 520 "moc.y"
{ className = yyvsp[0].string; }
break;
case 123:
#line 524 "moc.y"
{ superclassName = yyvsp[0].string; }
break;
case 124:
#line 527 "moc.y"
{ yyval.string = 0; }
break;
case 125:
#line 528 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 130:
#line 540 "moc.y"
{ tmpAccessPerm = yyvsp[0].access; }
break;
case 131:
#line 541 "moc.y"
{ moc_err( "Missing access specifier"
						   " before \"slots:\"." ); }
break;
case 132:
#line 545 "moc.y"
{ BEGIN QT_DEF; }
break;
case 134:
#line 547 "moc.y"
{ BEGIN QT_DEF; }
break;
case 136:
#line 549 "moc.y"
{ if ( tmpAccessPerm )
				moc_warn("Q_OBJECT is not in the private"
					" section of the class.\n"
					"Q_OBJECT is a macro that resets"
					" access permission to \"private\".");
						  Q_OBJECTdetected = TRUE; }
break;
case 137:
#line 557 "moc.y"
{ moc_err( "Signals cannot "
						 "have access specifiers" ); }
break;
case 139:
#line 560 "moc.y"
{ if ( grammarDebug )
						  BEGIN QT_DEF;
					      else
						  BEGIN IN_CLASS;
					 }
break;
case 141:
#line 566 "moc.y"
{ BEGIN IN_CLASS;
					   if ( level != 1 )
					       moc_warn( "unexpected access"
							 "specifier" );
					 }
break;
case 146:
#line 582 "moc.y"
{ addMember('s'); }
break;
case 151:
#line 593 "moc.y"
{ addMember('t'); }
break;
case 154:
#line 600 "moc.y"
{ yyval.string=yyvsp[0].string; }
break;
case 157:
#line 608 "moc.y"
{ yyval.string = stradd( yyvsp[-3].string, "(", yyvsp[-1].string, ")" ); }
break;
case 158:
#line 610 "moc.y"
{ yyval.string = stradd( yyvsp[-3].string, "(", yyvsp[-1].string, ")" ); }
break;
case 159:
#line 613 "moc.y"
{yyval.string=yyvsp[0].string;}
break;
case 160:
#line 614 "moc.y"
{yyval.string=yyvsp[0].string;}
break;
case 161:
#line 615 "moc.y"
{yyval.string=yyvsp[0].string;}
break;
case 162:
#line 616 "moc.y"
{yyval.string=yyvsp[0].string;}
break;
case 163:
#line 617 "moc.y"
{yyval.string=yyvsp[0].string;}
break;
case 164:
#line 618 "moc.y"
{yyval.string=yyvsp[0].string;}
break;
case 165:
#line 619 "moc.y"
{yyval.string=yyvsp[0].string;}
break;
case 166:
#line 620 "moc.y"
{yyval.string=yyvsp[0].string;}
break;
case 167:
#line 621 "moc.y"
{yyval.string=yyvsp[0].string;}
break;
case 168:
#line 622 "moc.y"
{yyval.string=yyvsp[0].string;}
break;
case 169:
#line 625 "moc.y"
{ yyval.access=_PRIVATE; }
break;
case 170:
#line 626 "moc.y"
{ yyval.access=_PROTECTED; }
break;
case 171:
#line 627 "moc.y"
{ yyval.access=_PUBLIC; }
break;
case 215:
#line 679 "moc.y"
{ tmpFunc->type = yyvsp[-1].string;
						  tmpFunc->name = yyvsp[0].string; }
break;
case 216:
#line 682 "moc.y"
{ tmpFunc->type = "int";
						  tmpFunc->name = yyvsp[0].string;
				  if ( tmpFunc->name == className )
				      func_warn( "Constructors cannot be"
						 " signals or slots.");
						}
break;
case 217:
#line 689 "moc.y"
{
				       func_warn( "Destructors cannot be"
						  " signals or slots.");
						}
break;
case 218:
#line 695 "moc.y"
{
						    char *tmp =
							straddSpc(yyvsp[-4].string,yyvsp[-3].string,yyvsp[-2].string,yyvsp[-1].string);
						    tmpFunc->type = rmWS(tmp);
						    delete tmp;
						    tmpFunc->name = yyvsp[0].string; }
break;
case 219:
#line 702 "moc.y"
{ skipFunc = TRUE; }
break;
case 220:
#line 704 "moc.y"
{ tmpFunc->type =
						      straddSpc(yyvsp[-2].string,yyvsp[-1].string);
						  tmpFunc->name = yyvsp[0].string; }
break;
case 221:
#line 709 "moc.y"
{ tmpFunc->type =
						      straddSpc(yyvsp[-3].string,yyvsp[-2].string,yyvsp[-1].string);
						  tmpFunc->name = yyvsp[0].string; }
break;
case 222:
#line 713 "moc.y"
{ operatorError();    }
break;
case 223:
#line 715 "moc.y"
{ operatorError();    }
break;
case 224:
#line 718 "moc.y"
{ operatorError();    }
break;
case 225:
#line 720 "moc.y"
{ operatorError();    }
break;
case 226:
#line 723 "moc.y"
{ operatorError();    }
break;
case 228:
#line 728 "moc.y"
{ func_warn("Variable as signal or slot."); }
break;
case 229:
#line 731 "moc.y"
{ func_warn("Variable as signal or slot."); }
break;
case 230:
#line 733 "moc.y"
{ func_warn("Enum declaration as signal or"
					  " slot."); }
break;
case 234:
#line 743 "moc.y"
{ expLevel = 0; }
break;
case 236:
#line 745 "moc.y"
{ expLevel = 0; }
break;
case 239:
#line 750 "moc.y"
{ expLevel = 0; }
break;
case 241:
#line 755 "moc.y"
{BEGIN IN_FCT; fctLevel=1;}
break;
case 242:
#line 756 "moc.y"
{BEGIN QT_DEF; }
break;
#line 2456 "y.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yyss + yystacksize - 1)
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
