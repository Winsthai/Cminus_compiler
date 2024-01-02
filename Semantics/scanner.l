%{
    // C code (will be placed at the begining of the scanner.c)

    #include "parser.h"
%}

%option yylineno

DIGIT   [0-9]
LETTER  [a-zA-Z]
COMMENT ([/][*][^*]*[*]+([^*/][^*]*[*]+)*[/])
INVALID_IDNUM ({LETTER}+{DIGIT}+({LETTER}|{DIGIT})*)|({DIGIT}+{LETTER}+({LETTER}|{DIGIT})*)

%%
"else"  {return ELSE;}
"if"    {return IF;}
"int"   {return INT;}
"return"    {return RETURN;}
"void"  {return VOID;}
"while" {return WHILE;}

"+"     {return PLUS;}
"-"     {return SUB;}
"*"     {return MULT;}
"/"     {return DIV;}
"<"     {return LT;}
"<="    {return LEQ;}
">"     {return GT;}
">="    {return GEQ;}
"=="    {return EQ;}
"|="    {return NEQ;}
"="     {return SET;}
";"     {return SEM_COL;}
","     {return COMMA;}
"("     {return O_PAREN;}
")"     {return C_PAREN;}
"["     {return O_BRACK;}
"]"     {return C_BRACK;}
"{"     {return O_BRACE;}
"}"     {return C_BRACE;}

{INVALID_IDNUM}    {printf("error: invalid ID/NUM \"%s\" at or near line %d\n", yytext, yylineno);
                   exit(-1);
                   } /* detects if nums and ids are not separated */

{LETTER}+   {yylval.str = strdup(yytext); return ID;}
{DIGIT}+    {yylval.num = atoi(yytext); return NUM;}

(" "|\t|\n)     /* skip whitespaces */
{COMMENT}       /* skip comments */
"/*"  {printf("error: unterminated comment at or near line %d\n", yylineno);
        exit(-1);
        }
"*/"  {printf("error: end of comment with no matching start at or near line %d\n", yylineno);
        exit(-1);
        }

.   { printf("skipping unknown character '%c' at or near line %d\n", yytext[0], yylineno);}   /* detects unknown characters */
%%

// C code
// will be placed at the end of the scanner.c
int yywrap(){
    return 1;
}
