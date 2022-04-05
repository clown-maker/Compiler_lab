%{
#include <stdio.h>
#include "y.tab.h"
void yyerror(char *);
%}
DIGIT [1-9]+[0-9]*|0
INUM  [\-]?{DIGIT}
DNUM  [\-]?{DIGIT}+\.[0-9]+
%%

{DNUM}		sscanf(yytext, "%lf", &yylval.dnum); /*printf("scan %lf\n", yylval.dnum);*/ return DNUM;
{INUM}		sscanf(yytext, "%d", &yylval.inum);  /*printf("scan %d\n", yylval.inum);*/ return INUM;
"<"		    /*sscanf(yytext, "%c", &yylval.operator);  printf("scan %d\n", yylval.operator);*/ return LT;
">"		    /*sscanf(yytext, "%c", &yylval.operator);  printf("scan %d\n", yylval.operator);*/ return GT;
\n		    return CR;
"="         return EQ;
"!="        return NE;
">="        return GE;
"<="        return LE;
[ \t]+		/*ignore whitespace*/;
.           return 0;

%%
