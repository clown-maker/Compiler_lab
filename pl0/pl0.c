// pl0 compiler source code

#pragma warning(disable : 4996)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "PL0.h"
#include "set.c"

int dx; // data allocation index

//////////////////////////////////////////////////////////////////////
// print error message.
void error(int n)
{
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error

//////////////////////////////////////////////////////////////////////
void getch(void)
{
	if (cc == ll)
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
		while ((!feof(infile)) // added & modified by alex 01-02-09
			   && ((ch = getc(infile)) != '\n'))
		{
			printf("%c", ch);  //print what has just been input
			line[++ll] = ch; // what's the function of line?
		}					 // while
		printf("\n");
		line[++ll] = ' ';
	}
	ch = line[++cc];
} // getch

//////////////////////////////////////////////////////////////////////
// gets a symbol from input stream.
void getsym(void)
{
	int i, k;
	char a[MAXIDLEN + 1];

	while (ch == ' ' || ch == '\t') // read in the whitspace but don't perform actions
		getch();

	if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		} while (isalpha(ch) || isdigit(ch));

		a[k] = 0;	   // the end of a string
		strcpy(id, a); // give the id its value
		word[0] = id;
		i = NRW;
		while (strcmp(id, word[i--]))
			;
		if (++i)
		{
			sym = wsym[i]; // symbol is a reserved word
		}
		else
		{
			sym = SYM_IDENTIFIER; // symbol is an identifier
		}
	}
	else if (isdigit(ch))
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do
		{
			num = num * 10 + ch - '0';
			k++;
			getch();
		} while (isdigit(ch));
		if (k > MAXNUMLEN)
			error(25); // The number is too great.
	}
	else if (ch == ':')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_BECOMES; // :=
			getch();
		}
		else
		{
			sym = SYM_NULL; // illegal?
		}
	}
	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_GEQ; // >=
			getch();
		}
		else
		{
			sym = SYM_GTR; // >
		}
	}
	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_LEQ; // <=
			getch();
		}
		else if (ch == '>')
		{
			sym = SYM_NEQ; // <>
			getch();
		}
		else
		{
			sym = SYM_LES; // <
		}
	}
	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch)
			;
		if (++i)
		{
			sym = ssym[i];
			getch();
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym

//////////////////////////////////////////////////////////////////////
// generates (assembles) an instruction.
void gen(int x, int y, int z)
{
	if (cx > CXMAX)
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
} // gen

//////////////////////////////////////////////////////////////////////
// tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n)
{ /*����������һ�������Ƿ�λ��s1�ڣ�����λ�ڣ�
   �򱨴�n��������������ţ�ֱ������ķ���λ��s1��s2��*/
	symset s;

	if (!inset(sym, s1))
	{
		error(n);
		s = uniteset(s1, s2);
		while (!inset(sym, s))
			getsym();
		destroyset(s);
	}
} // test

//////////////////////////////////////////////////////////////////////
/*
	��ʼ���ķ����£�
	initializer -> num | { initializer_list } | { initializer_list , }
	initializer_list -> initializer | initializer_list , initializer
*/
int caculate_depth(int basetx)
{
	dimensionHead *dhead = (dimensionHead *)&table[basetx];
	dimension *dim;

	if (dhead->dimnum == 1)
	{
		firstdimwith = initsize;
		return 1;
	}
	else
	{
		int i;
		for (i = dhead->dimnum; i > 1; i--)
		{
			if (initindex[i] != 0)
				return i;
		}
		if (i == 1)
		{
			firstdimwith++;
			return 1;
		}
	}
}

int adjust_index(int basetx)
{
	dimensionHead *dhead = (dimensionHead *)&table[basetx];
	dimension *dim;

	int i;
	for (i = dhead->dimnum; i > 1; i--)
	{
		dim = (dimension *)&table[basetx + i];
		if (initindex[i] == dim->width)
		{
			initindex[i - 1]++;
			initindex[i] = 0;
		}
	}
}

void enter_zero(int curdepth, int basetx)
{
	dimensionHead *dhead = (dimensionHead *)&table[basetx];
	dimension *dim;
	if (curdepth == dhead->dimnum + 1)
		return;
	if (curdepth == 1 && dhead->dimnum == 1)
		return;
	int begin = initsize + 1;
	int subsize = 1;
	int i = (curdepth == 1) ? 2 : curdepth;
	for (; i <= dhead->dimnum; i++)
	{
		dim = (dimension *)&table[i + basetx];
		subsize = subsize * dim->width;
	}

	if (initsize % subsize == 0)
		return;

	int end = (initsize + subsize) / subsize * subsize;
	for (i = begin; i <= end; i++)
		initable[i] = 0;
	initsize = end;

	for (i = curdepth; i < dhead->dimnum; i++)
	{
		dim = (dimension *)&table[i + basetx];
		initindex[i] = dim->width - 1;
	}

	dim = (dimension *)&table[dhead->dimnum + basetx];
	initindex[dhead->dimnum] = dim->width;
}

int initializer_list(int depth, int basetx);
int initializer(int depth, int basetx)
{
	dimensionHead *dhead = (dimensionHead *)&table[basetx];
	dimension *dim;
	if (sym == SYM_LBRACE)
	{
		initializer_list(depth + 1, basetx); 
		if (sym == SYM_COMMA)
		{
			getsym();
			if (sym != SYM_RBRACE)
				error(31);
		}
		else if (sym != SYM_RBRACE)
			error(31);
		else
		{
			adjust_index(basetx);
			depth = caculate_depth(basetx);
			return depth;
		}
	}
	else if (sym == SYM_NUMBER)
	{
		if (depth > dhead->dimnum + 1)
			error(35);
		else
			depth = dhead->dimnum;

		dim = (dimension *)&table[basetx + depth]; // the index may need to be adjusted
		if (dim->width <= initindex[depth] && depth > 1)
		{
			error(36);
		}
		initable[++initsize] = num;
		initindex[depth]++;

		adjust_index(basetx);
		depth = caculate_depth(basetx);
		return depth;
	}
}

int initializer_list(int depth, int basetx)
{
	dimensionHead *dhead = (dimensionHead *)&table[basetx];
	dimension *dim;

	int current_depth = depth;
	int count = 0;
	do
	{
		getsym();
		if (sym == SYM_RBRACE)
			break;
		else if (sym == SYM_NUMBER)
		{
			if (current_depth == dhead->dimnum +1)
            {
                count++;
            }
            if (count > 1)
            {
                error(36);
            }
            else
				depth = initializer(depth, basetx);
		}
		else if (sym == SYM_LBRACE)
		{
			dim = (dimension *)&table[basetx + dhead->dimnum];
			if (initindex[dhead->dimnum] == dim->width)
			{
				adjust_index(basetx);
				depth = caculate_depth(basetx);
			}
			depth = initializer(depth, basetx);
		}
		getsym();
	} while (sym == SYM_COMMA);

	enter_zero(current_depth, basetx);
}

void ini_gen(int x, int y, int z)
{
	if (ini_x > INIMAX)
	{
		printf("Fatal Error: Program too long in ini_gen.\n");
		exit(1);
	}
	ini_code[ini_x].f = x;
	ini_code[ini_x].l = y;
	ini_code[ini_x++].a = z;
} // gen

void initgen(int basetx)
{
	mask *array = (mask *)&table[basetx];

	int initaddr = array->address;
	for (int i = 1; i <= initsize; i++)
	{
		ini_gen(LIT, 0, i + initaddr);
		ini_gen(LIT, 0, initable[i]);
		ini_gen(STOA, 0, 0);
	}
	dx = dx + initsize;
}

//////////////////////////////////////////////////////////////////////

void enter_array()
{
	mask *mk;
	dimensionHead *dhead;
	dimension *dim;
	array_basetx = tx;
	int firstdim = 0, basetx = tx, flag = 0;

	int arraysize = 1;
	mk = (mask *)&table[tx];
	mk->level = level;
	mk->address = dx; // mabey dx+1

	tx++;
	strcpy(table[tx].name, id);
	table[tx].kind = ID_ARRAY;
	dhead = (dimensionHead *)&table[tx];
	dhead->dimnum = 0;
	do
	{
		dhead->dimnum++;
		getsym();
		if (sym == SYM_NUMBER)
		{
			if (dhead->dimnum == 1)
				firstdim = 1; //��һάδȱʡ
			tx++;
			strcpy(table[tx].name, id);
			table[tx].kind = ID_ARRAY;
			dim = (dimension *)&table[tx];
			dim->width = num;
			arraysize = arraysize * num;
			getsym();
		}
		else
		{
			if (dhead->dimnum != 1) //���е�һά��Ϊȱʡ��
				error(28);
			else if (sym == SYM_RSBRACKET)
			{ //��һάȱʡ����Ҫ�ڷ��ű���������Ӧ����
				tx++;
				strcpy(table[tx].name, id);
				table[tx].kind = ID_ARRAY;
				dim = (dimension *)&table[tx];
				dim->width = 0;
			}
		}

		if (sym == SYM_RSBRACKET)
			getsym();
		else
			error(29);
	} while (sym == SYM_LSBRACKET);

	if (sym == SYM_EQU)
	{
		flag = 1;
		initsize = 0;
		firstdimwith = 0;
		for (int i = 1; i <= dhead->dimnum; i++)
			initindex[i] = 0;
		getsym();
		initializer(0, basetx + 1);
		initgen(basetx);
		int subsize = 1;

		for (int i = 3; i <= dhead->dimnum + 1; i++)
		{
			dim = (dimension *)&table[i + basetx];
			subsize = subsize * dim->width;
		}
		dim = (dimension *)&table[basetx + 2];
		dim->width = initsize / subsize;
		getsym();
	}

	if (flag == 0)
		dx = dx + arraysize;
}

// enter object(constant, variable or procedre) into table.
void enter(int kind)
{
	mask *mk;

	tx++; // table index ++
	strcpy(table[tx].name, id);
	table[tx].kind = kind;
	switch (kind)
	{
	case ID_CONSTANT:
		if (num > MAXADDRESS)
		{
			error(25); // The number is too great.
			num = 0;
		}
		table[tx].value = num;
		break;
	case ID_VARIABLE:
		mk = (mask *)&table[tx];
		mk->level = level;
		mk->address = dx++;
		break;
	case ID_PROCEDURE:
		mk = (mask *)&table[tx];
		mk->level = level;
		break;
	case ID_ARRAY:
		enter_array();
		break;
	} // switch
} // enter

//////////////////////////////////////////////////////////////////////
// locates identifier in symbol table.
int position(char *id)
{ /*���ؼ���id�ڷ��ű��е�λ��(�±�)�����޸�id���򷵻�0*/
	int i;

	strcpy(table[0].name, id); // the table 0 is a place holder
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0)
		;
	if (i != 0 && table[i].kind == ID_ARRAY)
	{
		while (strcmp(table[i].name, id) == 0)
			i--;
		return i + 1;
	}
	return i;
} // position

//////////////////////////////////////////////////////////////////////
void constdeclaration()
{
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_EQU || sym == SYM_BECOMES)
		{
			if (sym == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter(ID_CONSTANT);
				getsym();
			}
			else
			{
				error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			error(3); // There must be an '=' to follow the identifier.
		}
	}
	else
		error(4);
	// There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration

//////////////////////////////////////////////////////////////////////
void vardeclaration(void)
{
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_LSBRACKET)
			enter(ID_ARRAY);
		else
			enter(ID_VARIABLE);
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration

//////////////////////////////////////////////////////////////////////
void listcode(int from, int to)
{
	int i;

	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

//////////////////////////////////////////////////////////////////////
void factor(symset fsys)
{
	void expression(symset fsys);
	int i;
	symset set, set1;

	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

	if (inset(sym, facbegsys))
	{
		if (sym == SYM_IDENTIFIER)
		{
			if ((i = position(id)) == 0)
			{
				error(11); // Undeclared identifier.
			}
			else
			{
				switch (table[i].kind)
				{
					mask *mk;
				case ID_CONSTANT:
					gen(LIT, 0, table[i].value);
					break;
				case ID_VARIABLE:
					mk = (mask *)&table[i];
					gen(LOD, level - mk->level, mk->address);
					break;
				case ID_ARRAY:
					mk = (mask *)&table[i];
					dimensionHead *dhead;
					dimension *dim;
					gen(LEA, level - mk->level, mk->address);
					dhead = (dimensionHead *)&table[++i];
					i++;
					int depth = 1;
					getsym(); // read '['
					if (sym != SYM_LSBRACKET)
						error(30);
					getsym(); // read the first symbol of the expression of the first dimension
					set1 = createset(SYM_LSBRACKET, SYM_RSBRACKET, SYM_NULL);
					set = uniteset(set1, fsys);
					expression(set); // generate one instruction  which adds one number into the stack of the top
					while (sym == SYM_RSBRACKET && depth < dhead->dimnum)
					{
						getsym(); // read '['
						if (sym != SYM_LSBRACKET)
							error(30);
						getsym();
						depth++;
						dim = (dimension *)&table[++i];
						gen(LIT, 0, dim->width);
						gen(OPR, 0, OPR_MUL);
						expression(set);
						gen(OPR, 0, OPR_ADD);
					}
					gen(OPR, 0, OPR_ADD);
					gen(LODA, 0, 0);
					destroyset(set1);
					destroyset(set);
					break;
				case ID_PROCEDURE:
					error(21); // Procedure identifier can not be in an expression.
					break;
				} // switch
			}
			getsym();
		}
		else if (sym == SYM_NUMBER)
		{
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			gen(LIT, 0, num);
			getsym();
		}
		else if (sym == SYM_LPAREN)
		{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			expression(set);
			destroyset(set);
			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
		else if (sym == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
		{
			getsym();
			factor(fsys);
			gen(OPR, 0, OPR_NEG);
		}
		test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);
	} // if
} // factor

//////////////////////////////////////////////////////////////////////
void term(symset fsys)
{
	int mulop;
	symset set;

	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_NULL));
	factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH)
	{
		mulop = sym;
		getsym();
		factor(set);
		if (mulop == SYM_TIMES)
		{
			gen(OPR, 0, OPR_MUL);
		}
		else
		{
			gen(OPR, 0, OPR_DIV);
		}
	} // while
	destroyset(set);
} // term

//////////////////////////////////////////////////////////////////////
void expression(symset fsys)
{
	int addop;
	symset set;

	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));

	term(set);
	while (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;
		getsym();
		term(set);
		if (addop == SYM_PLUS)
		{
			gen(OPR, 0, OPR_ADD);
		}
		else
		{
			gen(OPR, 0, OPR_MIN);
		}
	} // while

	destroyset(set);
} // expression

//////////////////////////////////////////////////////////////////////
void condition(symset fsys)
{
	int relop;
	symset set;

	if (sym == SYM_ODD)
	{
		getsym();
		expression(fsys);
		gen(OPR, 0, 6);
	}
	else
	{
		set = uniteset(relset, fsys);
		expression(set);
		destroyset(set);
		if (!inset(sym, relset))
		{
			error(20);
		}
		else
		{
			relop = sym;
			getsym();
			expression(fsys);
			switch (relop)
			{
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
		}	  // else
	}		  // else
} // condition

//////////////////////////////////////////////////////////////////////
void statement(symset fsys)
{
	int i, cx1, cx2;
	symset set1, set;

	if (sym == SYM_IDENTIFIER)
	{ // variable assignment or element of array assignment
		mask *mk;
		if (!(i = position(id)))
		{
			error(11); // Undeclared identifier.
		}
		else if (table[i].kind == ID_ARRAY)
		{
			mk = (mask *)&table[i];
			dimensionHead *dhead;
			dimension *dim;
			gen(LEA, level - mk->level, mk->address);
			dhead = (dimensionHead *)&table[++i];
			i++;
			int depth = 1;
			getsym(); // read '['
			if (sym != SYM_LSBRACKET)
				error(30);
			getsym(); // read the first symbol of the expression of the first dimension
			set1 = createset(SYM_LSBRACKET, SYM_RSBRACKET, SYM_NULL);
			set = uniteset(set1, fsys);
			expression(set); // generate one instruction  which adds one number into the stack of the top
			while (sym == SYM_RSBRACKET && depth < dhead->dimnum)
			{
				getsym(); // read '['
				if (sym != SYM_LSBRACKET)
					error(30);
				getsym();
				depth++;
				dim = (dimension *)&table[++i];
				gen(LIT, 0, dim->width);
				gen(OPR, 0, OPR_MUL);
				expression(set);
				gen(OPR, 0, OPR_ADD);
			}
			gen(OPR, 0, OPR_ADD);
			destroyset(set1);
			destroyset(set);
		}
		else if (table[i].kind != ID_VARIABLE && table[i].kind != ID_ARRAY)
		{
			error(12); // Illegal assignment.
			i = 0;
		}
		getsym();
		if (sym == SYM_BECOMES)
		{
			getsym();
		}
		else
		{
			error(13); // ':=' expected.
		}
		expression(fsys);
		mk = (mask *)&table[i];
		if (i && table[i].kind == ID_VARIABLE)
		{
			gen(STO, level - mk->level, mk->address);
		}
		else if (i && table[i].kind == ID_ARRAY)
		{
			gen(STOA, 0, 0);
		}
	}
	else if (sym == SYM_CALL)
	{ // procedure call
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			error(14); // There must be an identifier to follow the 'call'.
		}
		else
		{
			if (!(i = position(id)))
			{
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
				mask *mk;
				mk = (mask *)&table[i];
				gen(CAL, level - mk->level, mk->address);
			}
			else
			{
				error(15); // A constant or variable can not be called.
			}
			getsym();
		}
	}
	else if (sym == SYM_IF)
	{ // if statement
		getsym();
		set1 = createset(SYM_THEN, SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_THEN)
		{
			getsym();
		}
		else
		{
			error(16); // 'then' expected.
		}
		cx1 = cx;
		gen(JPC, 0, 0);
		statement(fsys);
		code[cx1].a = cx;
	}
	else if (sym == SYM_BEGIN)
	{ // block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		while (sym == SYM_SEMICOLON || inset(sym, statbegsys))
		{
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(10);
			}
			statement(set);
		} // while
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_END)
		{
			getsym();
		}
		else
		{
			error(17); // ';' or 'end' expected.
		}
	}
	else if (sym == SYM_WHILE)
	{ // while statement
		cx1 = cx;
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		cx2 = cx;
		gen(JPC, 0, 0);
		if (sym == SYM_DO)
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.
		}
		statement(fsys);
		gen(JMP, 0, cx1);
		code[cx2].a = cx;
	}
	else if (sym == SYM_PRINT)
	{ // print number
		getsym();
		if (sym != SYM_LPAREN)
			error(26);
		else
		{
			getsym();
			set = uniteset(createset(SYM_COMMA, SYM_RPAREN, SYM_NULL), fsys);
			symset p = set;
			expression(set);
			gen(OUT, 0, 0);
			while (sym == SYM_COMMA)
			{
				getsym();
				expression(set);
				gen(OUT, 0, 0);
			}
			destroyset(set);
			gen(OUT, 0, 1);
			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
	}
	test(fsys, phi, 19);
} // statement

//////////////////////////////////////////////////////////////////////
void block(symset fsys)
{
	int cx0; // initial code index
	mask *mk;
	int block_dx;
	int savedTx;
	symset set1, set;

	dx = 3;
	block_dx = dx;
	mk = (mask *)&table[tx];
	mk->address = cx;
	ini_x = 0;
	gen(JMP, 0, 0);
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	do
	{
		if (sym == SYM_CONST) //��ȡ��const�ؼ���
		{					  // constant declarations
			getsym();
			do
			{
				constdeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					constdeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			} while (sym == SYM_IDENTIFIER);
		} // if

		if (sym == SYM_VAR)
		{ // variable declarations
			getsym();
			do
			{
				vardeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			} while (sym == SYM_IDENTIFIER);
		}			   // if
		block_dx = dx; // save dx before handling procedure call!
		while (sym == SYM_PROCEDURE)
		{ // procedure declarations
			getsym();
			if (sym == SYM_IDENTIFIER)
			{
				enter(ID_PROCEDURE);
				getsym();
			}
			else
			{
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}

			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}

			level++;
			savedTx = tx;
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);
			destroyset(set1);
			destroyset(set);
			tx = savedTx;
			level--;

			if (sym == SYM_SEMICOLON)
			{
				getsym();
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);
				test(set, fsys, 6);
				destroyset(set1);
				destroyset(set);
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}
		}			   // while of procedure
		dx = block_dx; // restore dx after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	} while (inset(sym, declbegsys));

	code[mk->address].a = cx;
	mk->address = cx;
	cx0 = cx;
	gen(INT, 0, block_dx);
	for (int i = 0; i < ini_x; i++)
		gen(ini_code[i].f, ini_code[i].l, ini_code[i].a);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8);	  // test for error: Follow the statement is an incorrect symbol.
	//listcode(cx0, cx);
} // block

//////////////////////////////////////////////////////////////////////
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;

	while (levelDiff--)
		b = stack[b];
	return b;
} // base

//////////////////////////////////////////////////////////////////////
// interprets and executes codes.
void interpret()
{
	int pc;				  // program counter
	int stack[STACKSIZE]; //����������ջ��ʽ��֯
	int top;			  // top of stack
	int b;				  // program, base, and top-stack register
	instruction i;		  // instruction register

	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{
		// printf("code %d  ", pc);
		i = code[pc++];

		switch (i.f)
		{
		case LIT:
			stack[++top] = i.a; //���ó���
			// printf("LIT: stack[%d] = %d\n", top, stack[top]);
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				top = b - 1;
				pc = stack[top + 3];
				b = stack[top + 2];
				break;
			case OPR_NEG: //ջ��ȡ��
				stack[top] = -stack[top];
				break;
			case OPR_ADD: //ջ���ʹ�ջ����ӣ���������ڴ�ջ����
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN: //��ջ����ȥջ������������ڴ�ջ����
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL: //���
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV: //��������жϳ����Ƿ�Ϊ0
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1];
				break;
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
				break;
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
				break;
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			// printf("%d\n", stack[top]);
			top--;
			break;
		case CAL:
			stack[top + 1] = base(stack, b, i.l);
			// generate new block mark
			stack[top + 2] = b;
			stack[top + 3] = pc;
			b = top + 1;
			pc = i.a;
			break;
		case INT:
			top += i.a;
			break;
		case JMP:
			pc = i.a;
			break;
		case JPC:
			if (stack[top] == 0)
				pc = i.a;
			top--;
			break;
		case OUT:
			if (i.a == 0)
				printf("%d ", stack[top--]);
			else
				printf("\n");
			break;
		case LEA:
			stack[++top] = base(stack, b, i.l) + i.a;
			break;
		case LODA:
			 //printf("LODA: stack[%d]=%d, stack[%d]=%d\n", top, stack[top], stack[top], stack[stack[top]]);
			stack[top] = stack[stack[top]];
			break;
		case STOA:
			stack[stack[top - 1]] = stack[top];
			top = top - 2;
			break;
		} // switch
	
	} while (pc);

	printf("End executing PL/0 program.\n");
} // interpret

//////////////////////////////////////////////////////////////////////
void main()
{
	FILE *hbin;
	char s[80];
	int i;
	symset set, set1, set2;

	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}

	phi = createset(SYM_NULL);
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);

	// create begin symbol sets
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_NULL);
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_NULL);

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;

	getsym();

	set1 = createset(SYM_PERIOD, SYM_NULL);
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);
	block(set);
	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

	if (sym != SYM_PERIOD)
		error(9); // '.' expected.
	if (err == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	/*for (int k = 0; k <= tx; k++)
	{
		printf("**%d: name=%s ,kind=%d,", k, table[k].name, table[k].kind);
		/*if(table[k].kind == 0)
			printf("value=%d\n", table[k].value);
		else
			printf("level=%d, add=%d\n", (0x1100&table[k].value) >>8, (0x0011&table[k].value));	
		printf("value=%x\n", table[k].value);
	}*/
	if (err == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	 listcode(0, cx);
} // main

//////////////////////////////////////////////////////////////////////
// eof pl0.c

