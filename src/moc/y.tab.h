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
extern YYSTYPE yylval;
