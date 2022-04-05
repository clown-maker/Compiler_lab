%{
#include <stdio.h>
#include <string.h>
int yylex(void);
void yyerror(char *);
%}

%union{
       int inum;
       double dnum;
       char operator;
       struct{
               int num_i;
               double num_d;
               char type;
               int  value;
      }node;
}
%token LT GT CR EQ NE GE LE
%token <inum> INUM
%token <dnum> DNUM
%type <node> E 
%type <node> T
%type <operator> OP

%%
	line_list : line
	          | line_list line
	          ;
	
	line : E CR	{if($1.value) printf("True\n"); else printf("False\n"); }
	
	E : T                        {$$.num_i = $1.num_i;  $$.num_d = $1.num_d; $$.type = $1.type;}
	    | E OP T
			{
				$$.num_i = $3.num_i;  $$.num_d = $3.num_d;  $$.type = $3.type;
				switch($2 + 256)
				{
					case LT:	
						if($1.type == 'I' &&  $3.type == 'I')
							$$.value = $1.value && ($1.num_i < $3.num_i)?1:0;
						else if($1.type == 'I' &&  $3.type == 'D')
							$$.value = $1.value && ($1.num_i < $3.num_d)?1:0;	
						else if($1.type == 'D' &&  $3.type == 'I')
							$$.value = $1.value && ($1.num_d < $3.num_i)?1:0;	
						else	
							$$.value = $1.value && ($1.num_d < $3.num_d)?1:0;	
						break;
					case GT:
						if($1.type == 'I' &&  $3.type == 'I')
							$$.value = $3.value && ($1.num_i > $3.num_i)?1:0;
						else if($1.type == 'I' &&  $3.type == 'D')
							$$.value = $3.value && ($1.num_i > $3.num_d)?1:0;	
						else if($1.type == 'D' &&  $3.type == 'I')
							$$.value = $3.value && ($1.num_d > $3.num_i)?1:0;	
						else	
							$$.value = $3.value && ($1.num_d > $3.num_d)?1:0;	
						break;	
					case EQ:
						if($1.type == 'I' &&  $3.type == 'I')
							$$.value = $3.value && ($1.num_i == $3.num_i)?1:0;
						else if($1.type == 'I' &&  $3.type == 'D')
							$$.value = $3.value && ($1.num_i == $3.num_d)?1:0;	
						else if($1.type == 'D' &&  $3.type == 'I')
							$$.value = $3.value && ($1.num_d == $3.num_i)?1:0;	
						else	
							$$.value = $3.value && ($1.num_d == $3.num_d)?1:0;	
						break;
					case NE:
						if($1.type == 'I' &&  $3.type == 'I')
							$$.value = $3.value && ($1.num_i != $3.num_i)?1:0;
						else if($1.type == 'I' &&  $3.type == 'D')
							$$.value = $3.value && ($1.num_i != $3.num_d)?1:0;	
						else if($1.type == 'D' &&  $3.type == 'I')
							$$.value = $3.value && ($1.num_d != $3.num_i)?1:0;	
						else	
							$$.value = $3.value && ($1.num_d != $3.num_d)?1:0;	
						break;
					case GE:
						if($1.type == 'I' &&  $3.type == 'I')
							$$.value = $3.value && ($1.num_i >= $3.num_i)?1:0;
						else if($1.type == 'I' &&  $3.type == 'D')
							$$.value = $3.value && ($1.num_i >= $3.num_d)?1:0;	
						else if($1.type == 'D' &&  $3.type == 'I')
							$$.value = $3.value && ($1.num_d >= $3.num_i)?1:0;	
						else	
							$$.value = $3.value && ($1.num_d >= $3.num_d)?1:0;	
						break;
					case LE:
						if($1.type == 'I' &&  $3.type == 'I')
							$$.value = $3.value && ($1.num_i <= $3.num_i)?1:0;
						else if($1.type == 'I' &&  $3.type == 'D')
							$$.value = $3.value && ($1.num_i <= $3.num_d)?1:0;	
						else if($1.type == 'D' &&  $3.type == 'I')
							$$.value = $3.value && ($1.num_d <= $3.num_i)?1:0;	
						else	
							$$.value = $3.value && ($1.num_d <= $3.num_d)?1:0;	
						break;																							
				}		
			}
		;

	T  : INUM { $$.num_i = $1;  $$.value = 1; $$.type = 'I'; }
	   | DNUM { $$.num_d = $1;  $$.value = 1; $$.type = 'D'; }
	   ;

	OP : LT {$$ = LT - 256;} 
	   | GT {$$ = GT - 256;}
	   | EQ {$$ = EQ - 256;}
	   | NE {$$ = NE - 256;}
	   | GE{$$ = GE - 256;}
	   | LE {$$ = LE - 256;}	
	   ;   
%%
void	yyerror(char *str){
	fprintf(stderr, "error:%s", str);
}

int	yywrap(){
	return 1;
}

int	main()
{
	yyparse();
}
