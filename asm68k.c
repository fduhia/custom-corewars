// V4.0 8.17.03 Added address to DS directive and open labels in list file.
//              Added ASM68K version line to list file.
//              Then took it out.

// V3.5 9.15.99 Ignore lines after END bug fixed.

// V3.4 1.11.99 Forced 32-bit address generation on mode-7
//              addresses. Increased symbol table size to 1024.

// V3.3 11.2.98 Fixed forward reference problem with LEA
//              Added EQU value display to .LST file

// V3.2 4.10.98 Added data/address registers to symbol table
//		Chnaged symbol table display (two sorted columns)
//		Changed warning/fatal output (first error only)
//		Took SP stuff back out

// V3.1 3.12.98 Fixed skipped blanks in DC
//              Fixed EQU evaluation

// V3.0 2.14.98 Added date to signon message
//              Took line numbers out of list file
//              Added RTD instruction
//              Added SP recognition (in place of A7)

// V2.2 11.4.96 Changed tabs to blanks in list file

// V2.1 9.11.96 Fixed Srecord problem with DS directive

// Version 2.0a, 3.8.96
// Fixed BGT/BLT opcodes, MOVEM's comma problem

/* 68000 Assembler, Version 2.0 */
/* C1995 James L. Antonakos */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "asm.h"

void writequlist(unsigned long equval);

	int rsym;
	int ENDOPfound,linesignored;
	int elabel;

main(int argc, char *argv[])
{
	printf("ASM68K Version 4.0, 8/17/03\n");
	srcfile[0] = '\0';
	lstfile[0] = '\0';
	objfile[0] = '\0';
	hexfile[0] = '\0';
	if (argc > 1)
	{
		strcat(srcfile,argv[1]);
		strcat(srcfile,".asm");
		strcat(lstfile,argv[1]);
		strcat(lstfile,".lst");
		strcat(objfile,argv[1]);
		strcat(objfile,".obj");
		strcat(hexfile,argv[1]);
		strcat(hexfile,".hex");
		srcptr = fopen(srcfile,"r");
		if (srcptr == NULL)
		{
			printf("Cannot open %s.\n",srcfile);
			exit(1);
		}
		lstptr = fopen(lstfile,"w");
		if (lstptr == NULL)
		{
			printf("Cannot open %s.\n",lstfile);
			exit(1);
		}
		objptr = fopen(objfile,"wb");
		if (objptr == NULL)
		{
			printf("Cannot open %s.\n",objfile);
			exit(1);
		}
		hexptr = fopen(hexfile,"w");
		if (hexptr == NULL)
		{
			printf("Cannot open %s.\n",hexfile);
			exit(1);
		}
	}
	else
	{
		printf("No source file specified.\n");
		printf("Usage: asm68k <filename>\n");
		printf("Example: asm68k hello\n");
		printf("where hello is an .asm file\n");
		exit(2);
	}
	initsymtab();
	initpass(1);
	makeapass();
	srcptr = fopen(srcfile,"r");
	initpass(2);
//        tolst(lstptr,";%s created by ASM68K Version 4.0\n\n",lstfile);
	s0rec();
	makeapass();
	s9rec();
	setsymbol("PC",pc,'R');
	showsymbol();
	printf("\n%d lines processed.\n",line);
	if(linesignored)        //9.15.99
		printf("%d lines after END ignored.\n",linesignored);
	printf("%d warnings.\n",warnings);
	printf("%d fatals.\n",fatals);
	tolst(lstptr,"%d lines processed.\n",line);
	if(linesignored)        //9.15.99
		tolst(lstptr,"%d lines after END ignored.\n",linesignored);
	tolst(lstptr,"%d warnings.\n",warnings);
	tolst(lstptr,"%d fatals.\n",fatals);
	fclose(lstptr);
	fclose(objptr);
	fclose(hexptr);
}

void s0rec()
{
	unsigned char len,k,odat;

	fprintf(hexptr,"S0");
	odat = 0;
	fprintf(objptr,"%c",odat);
	len = strlen(hexfile) - 4;
	fprintf(hexptr,"%02X0000",len + 3);
	fprintf(objptr,"%c%c%c",len+3,odat,odat);
	srecsum = len + 3;
	for (k = 0; k < len; k++)
	{
		fprintf(hexptr,"%02X",hexfile[k]);
		srecsum += hexfile[k];
		odat = hexfile[k];
		fprintf(objptr,"%c",odat);
	}
	srecsum = 0xff - srecsum;
	fprintf(hexptr,"%02X\n",srecsum);
	odat = srecsum;
	fprintf(objptr,"%c",odat);
}

void writes1rec()
{
	int k;
	unsigned char odat;
	fprintf(hexptr,"S1%02X%04X",sreclen,srecpc);
	odat = 1;
	fprintf(objptr,"%c",odat);
	odat = sreclen;
	fprintf(objptr,"%c",odat);
	odat = (srecpc >> 8) & 0xff;
	fprintf(objptr,"%c",odat);
	odat = srecpc & 0xff;
	fprintf(objptr,"%c",odat);
	for (k = 0; k < sreclen-3; k++)
	{
		fprintf(hexptr,"%02X",srecbuff[k]);
		odat = srecbuff[k];
		fprintf(objptr,"%c",odat);
	}
	srecsum += sreclen;
	srecsum = 0xff - srecsum;
	fprintf(hexptr,"%02X\n",srecsum);
	odat = srecsum;
	fprintf(objptr,"%c",odat);
	s1pending = FALSE;
}

void s1rec(int oper, unsigned char data)
{
	int k;

	if (oper == SORG)
	{
		if (s1pending) writes1rec();
		sreclen = 3;
		srecsum = ((pc >> 8) & 0xff) + (pc & 0xff);
		srecpc = pc & 0xffff;
		srecptr = 0;
		s1pending = TRUE;
	}
	if ((oper == SCLOSE) && s1pending) writes1rec();
	if (oper == SWRITE)
	{
		if (!s1pending)
		{
			sreclen = 3;
			srecsum = ((pc >> 8) & 0xff) + (pc & 0xff);
			srecpc = pc & 0xffff;
			srecptr = 0;
			s1pending = TRUE;
		}
		srecbuff[srecptr] = data;
		srecptr++;
		sreclen++;
		srecsum += data;
		if (srecptr == 0x22) writes1rec();
	}
}

void s9rec()
{
	unsigned char odat;

	if (s1pending) s1rec(SCLOSE,0);
	fprintf(hexptr,"S903%04X",startaddress);
	odat = 9;
	fprintf(objptr,"%c",odat);
	odat = 3;
	fprintf(objptr,"%c",odat);
	odat = (startaddress >> 8) & 0xff;
	fprintf(objptr,"%c",odat);
	odat = startaddress & 0xff;
	fprintf(objptr,"%c",odat);
	srecsum = 3;
	srecsum += (startaddress >> 8) & 0xff;
	srecsum += startaddress & 0xff;
	srecsum = 0xff - srecsum;
	fprintf(hexptr,"%02X\n",srecsum);
	odat = srecsum;
	fprintf(objptr,"%c",odat);
}

void initpass(int passval)
{
	line = 0;
	lineptr = 0;
	pass = passval;
	fatals = 0;
	warnings = 0;
	pc = 0;
	oldpc = 0;
	startaddress = 0;
	stopped = FALSE;
	gotlabel = FALSE;
	s1pending = FALSE;
	srecptr = 0;
	ENDOPfound = FALSE;
	linesignored = 0;
}

void makeapass()
{
	char fchar;

	fchar = getc(srcptr);
	while (fchar != EOF)
	{
		linetxt[lineptr] = fchar;
		lineptr++;
		if (fchar == '\n')
		{
			line++;
			linetxt[lineptr] = '\0';
			lineptr = 0;
			if (!ENDOPfound)         //9.15.99
				parse();
			else
			{
				skipblank();                    // 8.17.03
				if((linetxt[lineptr] != '\0')    // 8.17.03
					&& (linetxt[lineptr] != 0x0a)
					&& (linetxt[lineptr] != 0x0d))
						linesignored++;
			}
			lineptr = 0;
		}
		fchar = getc(srcptr);
	}
	fclose(srcptr);
}

void parse()
{
	lbuffptr = -1;
	writeflag = FALSE;
	comment = FALSE;
	warnptr = -1;
	fatalptr = -1;
	elabel = FALSE;
	process_symbol();
	skipblank();
	process_op();                       
	skipblank();
	if (openlabel) setsymbol(label,pc,'L');
	if ((!comment) && ('\n' != linetxt[lineptr]))
		fatalerror(24);
	lineptr = 0;
	showwarnings();
	showfatals();
}

void process_op()
{
	if (!comment)
	{
		getopcode();
		if (whichop != 254)
		{
			elabel = FALSE;         // 8.17.03
			getsize();
			switch(opiden)
			{
				case PSEUDO : do_pseudo(); break;
				case CPUOP  : do_68kop(); break;
			}
		}
		else
		{
			writelist();
		}
	}
	else
		writelist();
}

void getopcode()
{
	getop(opcode);
	if (0 == strlen(opcode))
		whichop = 254;
	else
		whichop = findop(opcode);
	if ((whichop != -1) && (whichop != 254))
	{
		optype = optab[whichop].type;
		opiden = optab[whichop].iden;
		oppatt = optab[whichop].pattern;
	}
	else
		opiden = -1;
}

void getop(char opcode[])
/* Called during both passes */
{
	int opptr = 0;
	while (isalpha(linetxt[lineptr]) && (opptr < SYMSIZE))
	{
		opcode[opptr] = toupper(linetxt[lineptr]);
		opptr++;
		lineptr++;
	}
	opcode[opptr] = '\0';
	if ((opptr == SYMSIZE) && (0 != isalpha(linetxt[lineptr])) && (pass == 1))
		fatalerror(25);
}

int findop(char opcode[])
/* Called during both passes */
{
	int k = 0, found = 0, numops;

	numops = sizeof(optab)/sizeof(inst);
	while((k < numops) && !found)
	{
		if (0 == strcmp(optab[k].opcode,opcode))
			found++;
		else
			k++;
	}
	if (!found)
		fatalerror(26);
	if (found)
		return k;
	else
		return -1;
}

void getsize()
{
	char sizext;

	if ('.' == linetxt[lineptr])
	{
		lineptr++;
		sizext = toupper(linetxt[lineptr]);
		lineptr++;
		if (0 != isalpha(sizext))
		{
			opsize = -1;
			switch(sizext)
			{
				case 'B' : opsize = BYTE; break;
				case 'W' : opsize = WORD; break;
				case 'L' : opsize = LONG; break;
				case 'S' : opsize = SHORT; break;
			}
			if (opsize == -1)
				fatalerror(27);
		}
		else
		{
			opsize = -1;
			fatalerror(27);
		}
	}
	else
		opsize = UNSIZED;
}

void do_pseudo()
{
	unsigned long int data;
	char moreflag;
        char savelabel[33];

	switch(optype)
	{
	case ORGOP: valundef = FALSE;
		result = eval();
		pc = result & 0xffffff;
		oldpc = pc;
		if (openlabel) setsymbol(label,pc,'L');
		if (pass == 2) s1rec(SORG,0);
		writelist();
		break;
	case DCOP: if (openlabel) {
		setsymbol(label,pc,'L');
//                if (0 == strcmpi(label,"CMD_ADR"))
  //                      printf("%s = %08lX\n",label,pc);
		}
		if (opsize == UNSIZED) opsize = LONG;
		moreflag = TRUE;
		do
		{
			skipblank();
			if (lchar == '\'')
			{
	     //                  eat('\'');
			lineptr++;
			lchar = linetxt[lineptr];       //3.12.98
			       do
			       {
					if ((lchar != '\'') && (lchar != '\n'))
					{
						putter(opsize,lchar);
						lineptr++;
						lchar = linetxt[lineptr];
					}
			       } while ((lchar != '\'') && (lchar != '\n'));
			       eat('\'');
			}
			else
			{
				valundef = FALSE;
				data = eval();
				if ((opsize == BYTE) && (BYTE != typer(data)))
				{
					fatalerror(29);
					data = 0;
				}
				if ((opsize == WORD) && (LONG == typer(data)))
				{
					fatalerror(30);
					data = 0;
				}
				putter(opsize,data);
			}
			skipblank();
			if (ischar(','))
				eat(',');
			else
				moreflag = FALSE;

		} while (moreflag);
		writelist();
		break;
	case DSOP: if (openlabel) setsymbol(label,pc,'L');
		valundef = FALSE;
		result = eval();
		switch(opsize)
		{
		case WORD: result <<= 1; break;
		case LONG: result <<= 2; break;
		case UNSIZED: result <<= 2; break;
		}
		pc += result;
		s1rec(SCLOSE,0);       //9.11.96 S-record fix
		writelist();
		break;
	case EQUOP: valundef = TRUE;    // 3.12.98
		savelabel[0] = '\0';
                strcat(savelabel,label);
		result = eval();
//		printf("line %d, label %s, EQU result = %08lX, valundef = %d\n",
  //			line,savelabel,result,valundef);
		if ((pass == 1) && !openlabel)
			fatalerror(10);
		//else    // 3.12.98
		if (!valundef)
                        setsymbol(savelabel,result,'E');
		writequlist(result);
		break;
	case ENDOP: if (openlabel) setsymbol(label,pc,'L');
		skipblank();
		if (isalpha(lchar) || isdigit(lchar) || (lchar == '$'))
		{
			valundef = FALSE;
			startaddress = eval() & 0xffff;
		}
		writelist();
		ENDOPfound = TRUE;      //9.15.99
		break;
	}
}

void do_68kop()
{
	if (openlabel) setsymbol(label,pc,'L');

	switch(optype)
	{
	case  1: ___abcdSbcd(); break;
	case 201: ___addAndOrSub(); break;
	case 22: ___addaCmpaSuba(); break;
	case 600: ___addiCmpiSubi(); break;
	case 601: ___addiCmpiSubi(); break;
	case 602: ___addiCmpiSubi(); break;
	case 11: ___addqSubq(); break;
	case 19: ___addxSubx(); break;
	case 230: ___andiEoriOri(); break;
	case 231: ___andiEoriOri(); break;
	case 232: ___andiEoriOri(); break;
	case 17: ___asdLsdRodRoxd(); break;
	case 181: ___bitops(); break;
	case 182: ___bitops(); break;
	case 183: ___bitops(); break;
	case 184: ___bitops(); break;
	case 12: ___branches(); break;
	case  0: ___NopResetRteRtrRtsTrapv(); break;
	case 99: ___Rtd(); break;
	case 21: ___clrNegNegxNotTst(); break;
	case 202: ___cmp(); break;
	case 26: ___cmpm(); break;
	case  5: ___dbcc(); break;
	case 203: ___eor(); break;
	case 27: ___exg(); break;
	case 28: ___ext(); break;
	case 15: ___jumps(); break;
	case 29: ___lea(); break;
	case 16: ___link(); break;
	case 31: ___move(); break;
	case  3: ___movea(); break;
	case 35: ___movem(); break;
	case 25: ___movep(); break;
	case 30: ___moveq(); break;
	case 10: ___muluMulsDivuDivsChk(); break;
	case 14: ___pea(); break;
	case 34: ___scc(); break;
	case  9: ___stop(); break;
	case  4: ___swap(); break;
	case 13: ___tasNbcd(); break;
	case  8: ___trap(); break;
	case  7: ___unlk(); break;
	}
	writelist();
}

void process_symbol()
{
	lchar = linetxt[lineptr];
	if (isalpha(lchar))
	{
		getsymbol(label);
		if (pass == 1)
			newsymptr = addsymbol(label);
		if (pass == 2)
			newsymptr = findsymbol(label);
	}
	else
	if ((' ' != lchar) && ('\t' != lchar) &&
		('\n' != lchar) && (';' != lchar))
			fatalerror(28);
}

void getsymbol(char sname[])
/* Called during both passes */
{
	int sptr = 0;
	do
	{
		sname[sptr] = toupper(linetxt[lineptr]);
		sptr++;
		lineptr++;
	} while ((isalnum(linetxt[lineptr]) || (linetxt[lineptr] == '_'))
		&& (sptr < SYMSIZE));
	sname[sptr] = '\0';
	gotlabel = TRUE;
	elabel = TRUE;          // 8.17.03

	if ((sptr == SYMSIZE) && (0 != isalnum(linetxt[lineptr]))
		&& (linetxt[lineptr] != '_'))
	{
		fatalerror(31);
		while (isalnum(linetxt[lineptr]) || (linetxt[lineptr] == '_'))
			lineptr++;
	}
}

void initsymtab()
{
	symptr = 0;
	addsymbol("CCR");
	setsymbol("CCR",xCCR,'R');
	addsymbol("SR");
	setsymbol("SR",xSR,'R');
	addsymbol("USP");
	setsymbol("USP",xUSP,'R');
	addsymbol("SSP");
	setsymbol("SSP",xSSP,'R');
	addsymbol("SP");
	setsymbol("SP",xSP,'R');
	addsymbol("PC");
	setsymbol("PC",0,'R');

	addsymbol("D0");
	setsymbol("D0",0,'D');
	addsymbol("D1");
	setsymbol("D1",1,'D');
	addsymbol("D2");
	setsymbol("D2",2,'D');
	addsymbol("D3");
	setsymbol("D3",3,'D');
	addsymbol("D4");
	setsymbol("D4",4,'D');
	addsymbol("D5");
	setsymbol("D5",5,'D');
	addsymbol("D6");
	setsymbol("D6",6,'D');
	addsymbol("D7");
	setsymbol("D7",7,'D');

	addsymbol("A0");
	setsymbol("A0",0,'A');
	addsymbol("A1");
	setsymbol("A1",1,'A');
	addsymbol("A2");
	setsymbol("A2",2,'A');
	addsymbol("A3");
	setsymbol("A3",3,'A');
	addsymbol("A4");
	setsymbol("A4",4,'A');
	addsymbol("A5");
	setsymbol("A5",5,'A');
	addsymbol("A6");
	setsymbol("A6",6,'A');
	addsymbol("A7");
	setsymbol("A7",7,'A');
	rsym = symptr;

	openlabel = FALSE;
}

int addsymbol(char sname[])
/* Called during pass 1 */
{
	int sympos;

	sympos = findsymbol(sname);
	if (sympos == -1)
	{
		if (symptr < MAXSYM)
		{
			symbols[symptr].name[0] = '\0';
			strcat(symbols[symptr].name,sname);
			symbols[symptr].type = 'U';
			symbols[symptr].value = 0;
			symptr++;
			openlabel = TRUE;
//                        if (0 == strcmpi(sname,"CMD_ADR"))
  //                              printf("%s ... %08lX\n",sname,pc);
		}
		else
		{
			fatalerror(32);
			sympos = -1;
			openlabel = FALSE;
		}
	}
	else
	{
		fatalerror(33);
		sympos = -1;
		openlabel = FALSE;
	}
}

void showsymbol()
{
	int k,n,i,j;
	char ost[128], cst[64],sc;
	unsigned long int ad;

	if (0 == symptr)
	{
		tolst(lstptr,"No symbols defined.\n");
	}
	else
	{
		tolst(lstptr,"\nSymbol Table, %d reserved symbols, "
			"%d user symbols\n\n",rsym,symptr-rsym);
		for(i = rsym; i < symptr; i++)
			for(j = rsym; j < symptr-1; j++)
				if(strcmp(symbols[j].name,
					symbols[j+1].name) > 0)
				{
					strcpy(cst,symbols[j].name);
					sc = symbols[j].type;
					ad = symbols[j].value;
					strcpy(symbols[j].name,
						symbols[j+1].name);
					symbols[j].type = symbols[j+1].type;
					symbols[j].value = symbols[j+1].value;
					strcpy(symbols[j+1].name,cst);
					symbols[j+1].type = sc;
					symbols[j+1].value = ad;
				}
		ost[0] = '\0';
		n = 0;
		tolst(lstptr,"                     ----- Reserved Symbols -----\n");
		for (k = 0; k < symptr; k++)
		{
			if (k == rsym)
				tolst(lstptr,"                       ----- User Symbols -----\n");
			sprintf(cst,"%16s  %c  %06lX   ",symbols[k].name,
				symbols[k].type,symbols[k].value);
			strcat(ost,cst);
			n++;
			if (n == 2)
			{
				tolst(lstptr,"%s\n",ost);
				ost[0] = '\0';
				n = 0;
			}
		}
		if (n != 0)
		{
			tolst(lstptr,"%s\n",ost);
		}
	}
	tolst(lstptr,"\n");
}

int findsymbol(char sname[])
/* Called during both passes */
{
	int k = 0, found = 0;

	while((k < symptr) && !found)
	{
		if (0 == strcmp(symbols[k].name,sname))
			found++;
		else
			k++;
	}
	if (!found)
		fatalerror(34);
	if (found)
		return k;
	else
		return -1;
}

void setsymbol(char sname[], unsigned long int value, char def)
{
	newsymptr = findsymbol(sname);
	symbols[newsymptr].type = def;
	symbols[newsymptr].value = value;
	openlabel = FALSE;
}

void skipblank()
{
	while ((' ' == linetxt[lineptr]) || ('\t' == linetxt[lineptr]))
		lineptr++;
	if (';' == linetxt[lineptr])
		comment = TRUE;
	lchar = linetxt[lineptr];
}

int typer(unsigned long int number)
{
	if (0 == (number & 0xffffff00))
		return BYTE;
	if ((0xffffff00 == (number & 0xffffff00)) && negflag)
		return BYTE;
	if (0 == (number & 0xffff0000))
		return WORD;
	if ((0xffff0000 == (number & 0xffff0000)) && negflag)
		return WORD;
	else
		return LONG;
}

void gethex()
{
	number = 0;
	digit = linetxt[lineptr];
	while (isxdigit(digit))
	{
		number <<= 4;
		digit = toupper(digit) - 0x30;
		if (digit > 9)
			digit -= 7;
		number += digit;
		lineptr++;
		digit = linetxt[lineptr];
	}
	valundef = FALSE;	//4.10.98
}

void getdeci()
{
	number = 0;
	digit = linetxt[lineptr];
	while (isdigit(digit))
	{
		number *= 10;
		digit -= 0x30;
		number += digit;
		lineptr++;
		digit = linetxt[lineptr];
	}
	valundef = FALSE;	// 4.10.98
}

int isreg()
{
	if ((digit >= '0' ) && (digit <= '7'))
		return TRUE;
	else
		return FALSE;
}

int mathop(char msym)
{
	int mflag;

	mflag = FALSE;
	switch(msym)
	{
		case '+': mflag = TRUE; break;
		case '-': mflag = TRUE; break;
		case '*': mflag = TRUE; break;
		case '/': mflag = TRUE; break;
	}
	return mflag;
}

long int eval()
{
	long int expval,temp;
	char mathbak;

	skipblank();
	lchar = linetxt[lineptr];
	if ('(' == lchar)
	{
		eat('(');
		expval = eval();
		eat(')');
		skipblank();
		lchar = linetxt[lineptr];
	}
	else
	{
		getvalue();
		expval = number;
//		printf("line %d, number = %X\n",line,number);
		skipblank();
		lchar = linetxt[lineptr];
	}
	if (mathop(lchar))
	{
		do
		{
			mathbak = lchar;
			eat(lchar);
			skipblank();
			lchar = linetxt[lineptr];
			if ('(' == lchar)
			{
				eat('(');
				temp = eval();
				eat(')');
				skipblank();
				lchar = linetxt[lineptr];
			}
			else
			{
				getvalue();
				temp = number;
				skipblank();
				lchar = linetxt[lineptr];
			}
			switch(mathbak)
			{
				case '+': expval += temp; break;
				case '-': expval -= temp; break;
				case '*': expval *= temp; break;
				case '/': expval /= temp; break;
			}
		} while (mathop(lchar));
	}
	if (!valundef) return expval;
	else return 0;
}

void getvalue()
{
	int gotit;

	if ('-' == lchar)
	{
		negflag = TRUE;
		eat('-');
		skipblank();
		lchar = linetxt[lineptr];
	}
	else
		negflag = FALSE;
	if (isdigit(lchar))
		getdeci();
	else
	if (ischar('$'))
	{
		eat('$');
		skipblank();
		lchar = linetxt[lineptr];
		gethex();
	}
	else
	if (isalpha(lchar))
	{
		getsymbol(label);
		gotit = findsymbol(label);
		if (-1 != gotit)
		{
			number = symbols[gotit].value;
			if ('U' == symbols[gotit].type) valundef = TRUE;
				else valundef = FALSE;
			if ('R' == symbols[gotit].type) foundR = TRUE;
		}
		//else
		  //      valundef = TRUE;
	}
	else
	if ('\'' == lchar)
	{
		number = 0;
	 //       eat('\'');
		lineptr++;      // 3.12.98
		lchar = linetxt[lineptr];
		while ('\'' != lchar)
		{
			number <<= 8;
			number |= lchar;
			lineptr++;      // 3.12.98
			lchar = linetxt[lineptr];
		}
		eat('\'');       // 3.12.98
		valundef = FALSE;
	}
	if (negflag)
		number = 0 - number;
}

void eat(char symbol)
{
	skipblank();
	if (toupper(symbol) == toupper(lchar))
	{
		lineptr++;
		lchar = linetxt[lineptr];
		skipblank();   //MOVEM fix 3.8.96
	}
	else
	{
		eatsymbol = symbol;
		fatalerror(7);
	}
}

int ischar(char lchr)
{
	if (lchr == linetxt[lineptr])
		return TRUE;
	else
		return FALSE;
}

// int pphase[2048];

void putter(int size, unsigned long int value)
{
	int k;
	unsigned long int temp;
//        static int pp;

	switch(size)
	{
		case BYTE: k = 1; break;
		case WORD: k = 2;
			temp = (value >> 8) & 0xff;
			value = (value << 8) & 0xff00;
			value |= temp;
			break;
		case LONG: k = 4;
			temp = value;
			value = (value << 24) & 0xff000000;
			value |= (temp << 8) & 0xff0000;
			value |= (temp >> 8) & 0xff00;
			value |= (temp >> 24) & 0xff;
			break;
		default: k = 4; break;
	}
	while (k > 0)
	{
		if (lbuffptr == 5)
			writelist();
		else
		{
//                        if (pass == 1)
  //                              pphase[pp++] = pc & 0xffff;  // 10/98
			lbuffptr++;
			lbuff[lbuffptr] = value & 0xff;
			if (pass == 2)
			{
				s1rec(SWRITE, value & 0xff);
    //                            if (pphase[pp] != (pc & 0xffff))
      //                          {
	//                                printf("Phase error: %08lX, %08lX\n",pphase[pp],(pc & 0xffff));
	  //                      }
	    //                    pp++;
			}
			value >>= 8;
			pc += 1;
			k--;
		}
	}
}

void writelist()
{
	int k,tc;

	if (((linetxt[0] == 0x0a) || (lbuffptr == -1))
		&& (optype != ORGOP) && (optype != DSOP)
		&& !elabel)     // 8.17.03
	{
		tolst(lstptr,"        ");
	}
	else
	{
		optype = -1;
		tolst(lstptr,"%06lX  ",oldpc);
	}

	switch(lbuffptr)
	{
		case -1: tolst(lstptr,"              "); break;
		case 0: tolst(lstptr,"%02X            ",lbuff[0]); break;
		case 1: tolst(lstptr,"%02X%02X          ",lbuff[0],
			lbuff[1]); break;
		case 2: tolst(lstptr,"%02X%02X %02X       ",lbuff[0],
			lbuff[1],lbuff[2]); break;
		case 3: tolst(lstptr,"%02X%02X %02X%02X     ",lbuff[0],
			lbuff[1],lbuff[2],lbuff[3]); break;
		case 4: tolst(lstptr,"%02X%02X %02X%02X %02X  ",lbuff[0],
			lbuff[1],lbuff[2],lbuff[3],lbuff[4]); break;
		case 5: tolst(lstptr,"%02X%02X %02X%02X %02X%02X",lbuff[0],
			lbuff[1],lbuff[2],lbuff[3],lbuff[4],
			lbuff[5]); break;
	}
	if (!writeflag)
	{
		//tolst(lstptr,"  %5d  ",line); // Sol said no! 2.14.98
		tolst(lstptr,"  ");
		tc = 0;
		for(k = 0; k < strlen(linetxt); k++)
		{
			if (linetxt[k] != 0x09)
			{
				tolst(lstptr,"%c",linetxt[k]);
				tc++;
			}
			else
			{
				tolst(lstptr," ");
				tc++;
				while (tc % 8)
				{
					tolst(lstptr," ");
					tc++;
				}
			}
		}
		//tolst(lstptr,"\n");
	}
	else
	{
		tolst(lstptr,"\n");
	}
	oldpc = pc;
	lbuffptr = -1;
	writeflag = TRUE;
}

void writequlist(unsigned long equval)
{
	int k,tc;

	switch(typer(equval))
	{
	case BYTE: tolst(lstptr,"<%02lX>",equval);
		   tolst(lstptr,"                  ");
		   break;
	case WORD: tolst(lstptr,"<%04lX>",equval);
		   tolst(lstptr,"                ");
		   break;
	case LONG: tolst(lstptr,"<%08lX>",equval);
		   tolst(lstptr,"            ");
	}
	if (!writeflag)
	{
		//tolst(lstptr,"  %5d  ",line); // Sol said no! 2.14.98
		tolst(lstptr,"  ");
		tc = 0;
		for(k = 0; k < strlen(linetxt); k++)
		{
			if (linetxt[k] != 0x09)
			{
				tolst(lstptr,"%c",linetxt[k]);
				tc++;
			}
			else
			{
				tolst(lstptr," ");
				tc++;
				while (tc % 8)
				{
					tolst(lstptr," ");
					tc++;
				}
			}
		}
		//tolst(lstptr,"\n");
	}
	else
	{
		tolst(lstptr,"\n");
	}
	oldpc = pc;
	lbuffptr = -1;
	writeflag = TRUE;
}

void getDn()
{
	eat('D');
	digit = lchar;
	eat(digit);
	if (!isreg())
	{
		fatalerror(3);
		digit = '0';
	}
	Dn = digit & 0x7;
}

void getAn()
{
/*	if ((lchar == 's') || (lchar == 'S'))   // 2.14.98
	{
		eat(lchar);
		eat('P');
		An = 7;
	}
	else */  //4.10.98
	{
		eat('A');
		digit = lchar;
		eat(digit);
		if (!isreg())
		{
			fatalerror(3);
			digit = '0';
		}
		An = digit & 0x7;
	}
}

void getImmediate()
{
	eat('#');
	valundef = FALSE;
	immdata = eval();
}

void getamode()
{
	char nchar;

	mode = 255;
	EA = 0;
	foundR = FALSE;
	skipblank();
	digit = linetxt[lineptr+1];
	nchar = digit;
	if (('D' == toupper(lchar)) && isreg())
	{
		getDn();
		mode = 0;
		reg = Dn;
	}
	else
	if (('A' == toupper(lchar)) && isreg())
	{
		getAn();
		mode = 1;
		reg = An;
	}
/*	else
	if ('S' == toupper(lchar))
	{
		getAn();
		mode = 1;
		reg = An;
	}  */ 		//4.10.98 Sorry Sol!
	else
	if ('#' == lchar)
	{
		getImmediate();
		mode = 7;
		reg = 4;
	}
	else
	if (('-' == lchar) && ('(' == nchar))
	{
		eat('-');
		eat('(');
		getAn();
		eat(')');
		mode = 4;
		reg = An;
	}
	else
	if (('$' == lchar) || isdigit(lchar) || isalpha(lchar) ||
		(('-' == lchar) && ('(' != nchar)))
	{
		valundef = FALSE;
		EA = eval();
		if ('(' == lchar) getsuperamode();
		else
		{
			mode = 7;
     //                   if (LONG == typer(EA)) reg = 1;
     //                   else reg = 0;
			reg = 1;   // force 32-bit 1.98
		}
	}
	else
	if ('(' == lchar)
	{
		getsuperamode();
		if (lchar == '+')
		{
			eat('+');
			mode = 3;
		}
	}
	else
	{
		fatalerror(16);
		mode = 0;
		reg = 0;
	}
}

void getsuperamode()
{
	eat('(');
//	if (('A' == toupper(lchar)) || ('S' == toupper(lchar))) //2.14.98
	if ('A' == toupper(lchar)) //4.10.98
	{
		getAn();
		reg = An;
		if (',' == lchar)
		{
			getXn();
			if (mode != 0) mode = 6;
		}
		else
		{
			eat(')');
			if (LONG == typer(EA))
			{
				fatalerror(0);
				EA = 0;
			}
			if (0 == EA) mode = 2;
			else mode = 5;
		}
	}
	else
	if ('P' == toupper(lchar))
	{
		EA -= (pc + 2);
		eat('P');
		eat('C');
		if (',' == lchar)
		{
			getXn();
			if (mode != 0)
			{
				mode = 7;
				reg = 3;
			}
		}
		else
		{
			eat(')');
			if (LONG == typer(EA))
			{
				fatalerror(0);
				EA = 0;
			}
			mode = 7;
			reg = 2;
		}
	}
	else
	{
		fatalerror(16);
		mode = 0;
		reg = 0;
	}
}

void getXn()
{
	eat(',');
	if ('D' == toupper(lchar))
	{
		getDn();
		extword = (Dn << 12);
		mode = 100;
	}
	else
//	if (('A' == toupper(lchar)) || ('S' == toupper(lchar)))  // 2.14.98
	if ('A' == toupper(lchar))  // 4.10.98
	{
		getAn();
		extword = 0x8000 | (An << 12);
		mode = 101;
	}
	else
	{
		fatalerror(18);
		mode = 0;
		reg = 0;
	}
	if ((mode == 100) || (mode == 101))
	{
		if (BYTE != typer(EA))
		{
			fatalerror(0);
			EA = 0;
		}
		extword |= EA & 0xff;
		if ('.' == lchar)
		{
			eat('.');
			if ('W' == toupper(lchar))
				eat('W');
			else
			if ('L' == toupper(lchar))
			{
				eat('L');
				extword |= 0x800;
			}
			else
				fatalerror(17);
		}
		else extword |= 0x800;
		eat(')');
	}
}

void finishamode()
{
	switch(mode)
	{
	case 5: putter(WORD,EA); break;
	case 6: putter(WORD,extword); break;
	case 7: switch(reg)
		{
		case 0: putter(WORD,EA); break;
		case 1: putter(LONG,EA); break;
		case 2: putter(WORD,EA); break;
		case 3: putter(WORD,extword); break;
		case 4: if (opsize == LONG) putter(LONG,immdata);
			else putter(WORD,immdata);
			break;
		}
		break;
	}
}

unsigned int sizebits()
{
	switch(opsize)
	{
	case BYTE: return 0; break;
	case WORD: return 0x40; break;
	case LONG: return 0x80; break;
	default: return 0x80; break;
	}
}

void notamode(int invmode, char sdflag)
{
	if (eflag) return;
	sdstr[0] = '\0';
	amstr[0] = '\0';
	if ('S' == sdflag) strcat(sdstr,"Source");
	else strcat(sdstr,"Destination");
	switch(invmode)
	{
	case 0: if (mode == 0)
		{
			strcat(amstr,"Dn");
			fatalerror(11);
			eflag = TRUE;
		}
		break;
	case 1: if (mode == 1)
		{
			strcat(amstr,"An");
			fatalerror(11);
			eflag = TRUE;
		}
		break;
	case 2: if (mode == 2)
		{
			strcat(amstr,"(An)");
			fatalerror(11);
			eflag = TRUE;
		}
		break;
	case 3: if (mode == 3)
		{
			strcat(amstr,"(An)+");
			fatalerror(11);
			eflag = TRUE;
		}
		break;
	case 4: if (mode == 4)
		{
			strcat(amstr,"-(An)");
			fatalerror(11);
			eflag = TRUE;
		}
		break;
	case 5: if (mode == 5)
		{
			strcat(amstr,"d16(An)");
			fatalerror(11);
			eflag = TRUE;
		}
		break;
	case 6: if (mode == 6)
		{
			strcat(amstr,"d8(An,Xn)");
			fatalerror(11);
			eflag = TRUE;
		}
		break;
	case 70: if ((mode == 7) && (reg == 0))
		{
			strcat(amstr,"XXX.W");
			fatalerror(11);
			eflag = TRUE;
		}
		break;
	case 71: if ((mode == 7) && (reg == 1))
		{
			strcat(amstr,"XXX.L");
			fatalerror(11);
			eflag = TRUE;
		}
		break;
	case 72: if ((mode == 7) && (reg == 2))
		{
			strcat(amstr,"d16(PC)");
			fatalerror(11);
			eflag = TRUE;
		}
		break;
	case 73: if ((mode == 7) && (reg == 3))
		{
			strcat(amstr,"d8(PC,Xn)");
			fatalerror(11);
			eflag = TRUE;
		}
		break;
	case 74: if ((mode == 7) && (reg == 4))
		{
			strcat(amstr,"#data");
			fatalerror(11);
			eflag = TRUE;
		}
		break;
	}
	if (eflag)
	{
		mode = 0;
		reg = 0;
	}
}

void ___abcdSbcd()
{
	int m2;

	if ((opsize != UNSIZED) && (opsize != BYTE))
		warnerror(4);
	getamode();
	m2 = mode;
	Rx = reg;
	if ((mode != 0) && (mode != 4))
	{
		fatalerror(12);
		m2 = 0;
		Rx = 0;
	}
	eat(',');
	getamode();
	Ry = reg;
	if ((mode != 0) && (mode != 4))
	{
		fatalerror(12);
		mode = 0;
		Ry = 0;
	}
	if (mode != m2) fatalerror(13);
	itemp = oppatt;
	if (mode == 4) itemp |= 0x8;
	itemp |= (Ry << 9) | Rx;
	putter(WORD,itemp);
}

void ___addAndOrSub()
{
	int sop,sreg;
	unsigned long int ea1;
	unsigned int ext1;

	if (opsize == UNSIZED)
	{
		warnerror(1);
		opsize = LONG;
	}
	if (opsize == SHORT)
	{
		warnerror(6);
		opsize = LONG;
	}
	getamode();
	sop = mode;
	sreg = reg;
	ea1 = EA;
	ext1 = extword;
	eat(',');
	getamode();
	if (mode == 0)
	{
		itemp = oppatt | (sop << 3) | sreg;
		itemp |= (reg << 9);
		itemp |= sizebits();
		putter(WORD,itemp);
		mode = sop;
		reg = sreg;
		EA = ea1;
		extword = ext1;
		finishamode();
	}
	else
	{
		eflag = FALSE;
		notamode(1,'D');
		notamode(72,'D');
		notamode(73,'D');
		notamode(74,'D');
		if (sop != 0)
		{
			fatalerror(15);
			sop = 0;
			sreg = 0;
		}
		itemp = oppatt | (mode << 3) | reg;
		itemp |= (sreg << 9);
		itemp |= sizebits();
		itemp |= 0x100;
		putter(WORD,itemp);
		finishamode();
	}
}

void ___addaCmpaSuba()
{
	if ((opsize != WORD) && (opsize != LONG))
	{
		warnerror(8);
		opsize = LONG;
	}
	itemp = oppatt;
	if (opsize == LONG) itemp |= 0x100;
	getamode();
	itemp |= (mode << 3) | reg;
	eat(',');
	getAn();
	itemp |= (An << 9);
	putter(WORD,itemp);
	finishamode();
}

void ___addiCmpiSubi()
{
	unsigned long int idat;

	if ((opsize == UNSIZED) || (opsize == SHORT))
	{
		warnerror(6);
		opsize = LONG;
	}
	eat('#');
	valundef = FALSE;
	idat = eval();
	eat(',');
	getamode();
	eflag = FALSE;
	notamode(1,'D');
	notamode(74,'D');
	if (optype != 601)
	{
		notamode(72,'D');
		notamode(72,'D');
	}
	itemp = oppatt | sizebits() | (mode << 3) | reg;
	putter(WORD,itemp);
	if (opsize == BYTE) idat &= 0xff;
	if (opsize == LONG) putter(LONG,idat);
	else putter(WORD,idat);
	finishamode();
}

void ___Rtd()
{
	unsigned long int idat;

	if (opsize != UNSIZED)
	{
		warnerror(2);
		opsize = WORD;
	}
	eat('#');
	valundef = FALSE;
	idat = eval();
	itemp = oppatt;
	putter(WORD,itemp);
	putter(WORD,idat);
}

void ___addqSubq()
{
	unsigned long int qval;

	if ((opsize == UNSIZED) || (opsize == SHORT))
	{
		warnerror(6);
		opsize = LONG;
	}
	eat('#');
	valundef = FALSE;
	qval = eval();
	if ((qval < 1) || (qval > 8))
	{
		fatalerror(0);
		qval = 1;
	}
	if (qval == 8) qval = 0;
	itemp = oppatt | (qval << 9) | sizebits();
	eat(',');
	getamode();
	eflag = FALSE;
	notamode(72,'D');
	notamode(73,'D');
	notamode(74,'D');
	itemp |= (mode << 3) | reg;
	putter(WORD,itemp);
	finishamode();
}

void ___addxSubx()
{
	int m1;

	if ((opsize == UNSIZED) || (opsize == SHORT))
	{
		warnerror(6);
		opsize = LONG;
	}
	getamode();
	if ((mode != 0) && (mode != 4))
	{
		fatalerror(20);
		mode = 0;
		reg = 0;
	}
	m1 = mode;
	itemp = oppatt | reg | sizebits();
	eat(',');
	getamode();
	if ((mode != 0) && (mode != 4))
	{
		fatalerror(20);
		mode = 0;
		reg = 0;
	}
	itemp |= (reg << 9);
	if (m1 != mode) fatalerror(13);
	if (m1 == 4) itemp |= 0x8;
	putter(WORD,itemp);
}

void ___andiEoriOri()
{
	unsigned long int idat;

	if (opsize == SHORT)
	{
		warnerror(6);
		opsize = LONG;
	}
	eat('#');
	valundef = FALSE;
	idat = eval();
	eat(',');
	getamode();
	if (foundR)
	{
		if (EA == xCCR)
		{
			if (opsize != BYTE) warnerror(4);
			switch(optype)
			{
			case 230: itemp = 0x023c; break;
			case 231: itemp = 0x0a3c; break;
			case 232: itemp = 0x003c; break;
			}
			putter(WORD,itemp);
			if (BYTE != typer(idat)) warnerror(4);
			idat &= 0xff;
			putter(WORD,idat);
		}
		else
		if (EA == xSR)
		{
			if (opsize != WORD) warnerror(2);
			switch(optype)
			{
			case 230: itemp = 0x027c; break;
			case 231: itemp = 0x0a7c; break;
			case 232: itemp = 0x007c; break;
			}
			putter(WORD,itemp);
			if (LONG == typer(idat)) warnerror(2);
			idat &= 0xffff;
			putter(WORD,idat);
		}
		else
		{
			fatalerror(21);
			putter(WORD,0);
		}
	}
	else
	{
		itemp = oppatt | sizebits();
		eflag = FALSE;
		notamode(1,'D');
		notamode(72,'D');
		notamode(73,'D');
		notamode(74,'D');
		itemp |= (mode << 3) | reg;
		putter(WORD,itemp);
		if (opsize == BYTE) idat &= 0xff;
		if (opsize == LONG) putter(LONG,idat);
		else putter(WORD,idat);
		finishamode();
	}
}

void ___asdLsdRodRoxd()
{
	getamode();
	if (mode == 0)
	{
		if (opsize == UNSIZED)
		{
			opsize == LONG;
			warnerror(1);
		}
		if (opsize == SHORT)
		{
			opsize = LONG;
			warnerror(6);
		}
		eat(',');
		itemp = oppatt & 0xf1ff;
		itemp |= 0x20;
		itemp |= sizebits();
		itemp |= (Dn << 9);
		getDn();
		itemp |= Dn;
		putter(WORD,itemp);
	}
	else
	if ((mode == 7) && (reg == 4))
	{
		if (opsize == UNSIZED)
		{
			opsize == LONG;
			warnerror(1);
		}
		if (opsize == SHORT)
		{
			opsize = LONG;
			warnerror(6);
		}
		if ((immdata < 1) || (immdata > 8))
		{
			fatalerror(0);
			immdata = 1;
		}
		eat(',');
		itemp = oppatt & 0xf1ff;
		itemp |= sizebits();
		if (immdata == 8) immdata = 0;
		itemp |= (immdata << 9);
		getDn();
		itemp |= Dn;
		putter(WORD,itemp);
	}
	else
	{
		if ((opsize != UNSIZED) && (opsize != WORD))
		{
			warnerror(2);
			opsize = WORD;
		}
		eflag = FALSE;
		notamode(1,'D');
		notamode(72,'D');
		notamode(73,'D');
		notamode(74,'D');
		itemp = oppatt & 0xff00;
		itemp |= 0xc0;
		itemp |= (mode << 3) | reg;
		putter(WORD,itemp);
		finishamode();
	}
}

void ___bitops()
{
	unsigned long int bitnum;

	if ((opsize == SHORT) || (opsize == WORD))
		warnerror(7);
	getamode();
	if (mode == 0)
	{
		switch(optype)
		{
		case 181: itemp = 0x0140; break;
		case 182: itemp = 0x0180; break;
		case 183: itemp = 0x01c0; break;
		case 184: itemp = 0x0100; break;
		}
		itemp |= (Dn << 9);
		eat(',');
		getamode();
		eflag = FALSE;
		notamode(1,'D');
		notamode(74,'D');
		if (optype != 184)
		{
			notamode(72,'D');
			notamode(72,'D');
		}
		itemp |= (mode << 3) | reg;
		putter(WORD,itemp);
		finishamode();
	}
	else
	if ((mode == 7) && (reg == 4))
	{
		switch(optype)
		{
		case 181: itemp = 0x0840; break;
		case 182: itemp = 0x0880; break;
		case 183: itemp = 0x08c0; break;
		case 184: itemp = 0x0800; break;
		}
		bitnum = immdata;
		eat(',');
		getamode();
		eflag = FALSE;
		notamode(1,'D');
		notamode(74,'D');
		if (optype != 184)
		{
			notamode(72,'D');
			notamode(72,'D');
		}
		if (bitnum > 31)
		{
			fatalerror(0);
			bitnum = 0;
		}
		itemp |= (mode << 3) | reg;
		putter(WORD,itemp);
		if (mode == 0) putter(WORD,bitnum);
		else
		{
			if (bitnum > 7)
			{
				fatalerror(0);
				bitnum = 0;
			}
			putter(WORD,bitnum);
			finishamode();
		}
	}
	else
	{
		fatalerror(14);
		putter(WORD,0);
	}
}

void ___branches()
{
	unsigned long int offset, target;

	if (opsize == UNSIZED) opsize = WORD;
	if (opsize == SHORT) opsize = BYTE;
	if (opsize == LONG)
	{
		warnerror(5);
		opsize = WORD;
	}
	valundef = FALSE;
	target = eval();
	offset = target - (pc + 2);
	if (opsize == BYTE)
	{

		if (offset == 0)
		{
			fatalerror(9);
			itemp = oppatt | 0xfe;
			putter(WORD,itemp);
		}
		else
		{
			if ((0 != (0xff00 & offset)) &&
				(0xff00 != (0xff00 & offset)))
			{
				fatalerror(8);
				offset = 0xfe;
			}
			itemp = oppatt | (offset & 0xff);
			putter(WORD,itemp);
		}
	}
	else
	{
		putter(WORD,oppatt);
		if ((0 != (0xffff0000 & offset)) &&
			(0xffff0000 != (0xffff0000 & offset)))
		{
			fatalerror(8);
			offset = 0;
		}
		putter(WORD,offset);
	}
}

void ___NopResetRteRtrRtsTrapv(void)
{
	if (opsize != UNSIZED) warnerror(0);
	putter(WORD,oppatt);
}

void ___clrNegNegxNotTst()
{
	getamode();
	eflag = FALSE;
	notamode(1,'D');
	notamode(72,'D');
	notamode(73,'D');
	notamode(74,'D');
	itemp = oppatt | sizebits();
	itemp = itemp | (mode << 3) | reg;
	putter(WORD,itemp);
	finishamode();
}

void ___cmp()
{
	if (opsize == UNSIZED)
	{
		warnerror(1);
		opsize = LONG;
	}
	if (opsize == SHORT)
	{
		warnerror(6);
		opsize = LONG;
	}
	getamode();
	eat(',');
	itemp = oppatt | sizebits();
	itemp |= (mode << 3) | reg;
	getDn();
	itemp |= (Dn << 9);
	putter(WORD,itemp);
	finishamode();
}

void ___cmpm()
{
	if ((opsize == UNSIZED) || (opsize == SHORT))
	{
		warnerror(6);
		opsize = LONG;
	}
	itemp = oppatt | sizebits();
	getamode();
	if (mode != 3)
	{
		fatalerror(19);
		mode = 3;
		reg = 0;
	}
	itemp |= An;
	eat(',');
	getamode();
	if (mode != 3)
	{
		fatalerror(19);
		mode = 3;
		reg = 0;
	}
	itemp |= (An << 9);
	putter(WORD,itemp);
}

void ___dbcc()
{
	unsigned long int offset, target;

	if (opsize != UNSIZED) warnerror(0);
	getDn();
	itemp = oppatt | Dn;
	eat(',');
	valundef = FALSE;
	target = eval();
	offset = target - (pc + 2);
	putter(WORD,itemp);
	if ((0 != (0xffff0000 & offset)) &&
		(0xffff0000 != (0xffff0000 & offset)))
	{
		fatalerror(8);
		offset = 0;
	}
	putter(WORD,offset);
}

void ___eor()
{
	if (opsize == UNSIZED)
	{
		warnerror(1);
		opsize = LONG;
	}
	if (opsize == SHORT)
	{
		warnerror(6);
		opsize = LONG;
	}
	getDn();
	itemp = oppatt | (Dn << 9) | sizebits();
	eat(',');
	getamode();
	eflag = FALSE;
	notamode(1,'D');
	notamode(72,'D');
	notamode(73,'D');
	notamode(74,'D');
	itemp |= (mode << 3) | reg;
	putter(WORD,itemp);
	finishamode();
}

void ___exg()
{
	int m1;

	if ((opsize != UNSIZED) && (opsize != LONG))
		warnerror(3);
	getamode();
	m1 = mode;
	Rx = reg;
	if ((mode != 0) && (mode != 1))
	{
		fatalerror(5);
		mode = 0;
		Rx = 0;
	}
	eat(',');
	getamode();
	Ry = reg;
	if ((mode != 0) && (mode != 1))
	{
		fatalerror(6);
		mode = 0;
		Ry = 0;
	}
	itemp = oppatt;
	if ((m1 == mode) && (m1 == 0))
	{
		itemp |= 0x40;
		itemp |= (Rx << 9);
		itemp |= Ry;
	}
	if ((m1 == mode) && (m1 == 1))
	{
		itemp |= 0x48;
		itemp |= (Rx << 9);
		itemp |= Ry;
	}
	if ((m1 != mode) && (m1 == 0))
	{
		itemp |= 0x88;
		itemp |= (Rx << 9);
		itemp |= Ry;
	}
	if ((m1 != mode) && (m1 == 1))
	{
		itemp |= 0x88;
		itemp |= (Ry << 9);
		itemp |= Rx;
	}
	putter(WORD,itemp);
}

void ___ext()
{
	if (opsize == UNSIZED)
	{
		warnerror(1);
		opsize = LONG;
	}
	else
	if ((opsize != WORD) && (opsize != LONG))
	{
		fatalerror(2);
		opsize = LONG;
	}
	getDn();
	itemp = oppatt | Dn;
	if (opsize == LONG) itemp |= 0x40;
	putter(WORD,itemp);
}

void ___jumps()
{
	if (opsize != UNSIZED) warnerror(0);
	getamode();
	eflag = FALSE;
	notamode(0,'D');
	notamode(1,'D');
	notamode(3,'D');
	notamode(4,'D');
	notamode(74,'D');
	itemp = oppatt | (mode << 3) | reg;
	putter(WORD,itemp);
	finishamode();
}

void ___lea()
{
	if ((opsize != UNSIZED) && (opsize != LONG))
		warnerror(3);
	opsize = LONG;
	getamode();
	if ((mode == 7) && (reg == 0))
		reg = 1;                 // 11.2.98
	//printf("%1d: addr: %08lX, EA = %08lX, mode: %1d\n",pass,pc,EA,mode);
	eflag = FALSE;
	notamode(0,'S');
	notamode(1,'S');
	notamode(3,'S');
	notamode(4,'S');
	notamode(74,'S');
	itemp = oppatt | (mode << 3) | reg;
	eat(',');
	getAn();
	itemp |= (An << 9);
	putter(WORD,itemp);
	finishamode();
}

void ___link()
{
	if (opsize != UNSIZED) warnerror(0);
	getAn();
	eat(',');
	eat('#');
	valundef = FALSE;
	disp = eval();
	if (LONG == typer(disp))
	{
		fatalerror(0);
		disp = 0;
	}
	itemp = oppatt | An;
	putter(WORD,itemp);
	putter(WORD,disp);
}

void ___move()
{
	int smode,sreg;
	unsigned long int sEA, sidat;
	unsigned int sextword;
	int dmode,dreg;
	unsigned long int dEA, didat;
	unsigned int dextword;

	switch(opsize)
	{
	case BYTE: itemp = 0x1000; break;
	case WORD: itemp = 0x3000; break;
	case LONG: itemp = 0x2000; break;
	default: warnerror(6);
		itemp = 0x2000;
		break;
	}
	getamode();
	if (foundR)
	{
		if (EA == xCCR)
		{
			if (opsize != WORD) warnerror(2);
			itemp = 0x42c0;
			eat(',');
			getamode();
			eflag = FALSE;
			notamode(1,'D');
			notamode(72,'D');
			notamode(73,'D');
			notamode(74,'D');
			itemp |= (mode << 3) | reg;
			putter(WORD,itemp);
			finishamode();
		}
		else
		if (EA == xSR)
		{
			if (opsize != WORD) warnerror(2);
			itemp = 0x40c0;
			eat(',');
			getamode();
			eflag = FALSE;
			notamode(1,'D');
			notamode(72,'D');
			notamode(73,'D');
			notamode(74,'D');
			itemp |= (mode << 3) | reg;
			putter(WORD,itemp);
			finishamode();
		}
		else
		if (EA == xUSP)
		{
			if (opsize != LONG) warnerror(3);
			itemp = 0x4e68;
			eat(',');
			getAn();
			itemp |= An;
			putter(WORD,itemp);
		}
		else
		{
			fatalerror(21);
			putter(WORD,0);
		}
	}
	else
	{
		smode = mode;
		sreg = reg;
		sEA = EA;
		sidat = immdata;
		sextword = extword;
		itemp |= (mode << 3) | reg;
		eat(',');
		getamode();
		if (foundR)
		{
			if (EA == xCCR)
			{
				if (opsize != WORD) warnerror(2);
				itemp = 0x44c0;
				mode = smode;
				reg = sreg;
				EA = sEA;
				immdata = sidat;
				extword = sextword;
				eflag = FALSE;
				notamode(1,'S');
				itemp |= (mode << 3) | reg;
				putter(WORD,itemp);
				finishamode();
			}
			else
			if (EA == xSR)
			{
				if (opsize != WORD) warnerror(2);
				itemp = 0x46c0;
				mode = smode;
				reg = sreg;
				EA = sEA;
				immdata = sidat;
				extword = sextword;
				eflag = FALSE;
				notamode(1,'S');
				itemp |= (mode << 3) | reg;
				putter(WORD,itemp);
				finishamode();
			}
			else
			if (EA == xUSP)
			{
				if (opsize != LONG) warnerror(3);
				itemp = 0x4e60;
				mode = smode;
				reg = sreg;
				if (mode != 1) fatalerror(35);
				itemp |= An;
				putter(WORD,itemp);
			}
			else
			{
				fatalerror(21);
				putter(WORD,0);
			}
		}
		else
		{
			eflag = FALSE;
			notamode(1,'D');
			notamode(72,'D');
			notamode(73,'D');
			notamode(74,'D');
			itemp |= (mode << 6) | (reg << 9);
			putter(WORD,itemp);
			dmode = mode;
			dreg = reg;
			dEA = EA;
			didat = immdata;
			dextword = extword;
			mode = smode;
			reg = sreg;
			EA = sEA;
			immdata = sidat;
			extword = sextword;
			finishamode();
			mode = dmode;
			reg = dreg;
			EA = dEA;
			immdata = didat;
			extword = dextword;
			finishamode();
		}
	}
}

void ___movea()
{
	if ((opsize != WORD) && (opsize != LONG))
	{
		warnerror(8);
		opsize = LONG;
	}
	itemp = oppatt;
	if (opsize == WORD) itemp |= 0x3000;
	else itemp |= 0x2000;
	getamode();
	itemp |= (mode << 3) | reg;
	eat(',');
	getAn();
	itemp |= (An << 9);
	putter(WORD,itemp);
	finishamode();
}

void ___movem()
{
	if ((opsize != WORD) && (opsize != LONG))
	{
		warnerror(8);
		opsize = LONG;
	}
	itemp = oppatt;
	if (opsize == LONG) itemp |= 0x40;
	getamode();
	if ((mode == 0) || (mode == 1))
	{
		reglist(1);
		eat(',');
		getamode();
		itemp |= (mode << 3) | reg;
		eflag = FALSE;
		notamode(0,'D');
		notamode(1,'D');
		notamode(3,'D');
		notamode(72,'D');
		notamode(73,'D');
		notamode(74,'D');
		putter(WORD,itemp);
		if (mode == 4) putter(WORD,rlist2);
		else putter(WORD,rlist);
		finishamode();
	}
	else
	{
		itemp |= 0x400;
		eflag = FALSE;
		notamode(4,'S');
		notamode(72,'S');
		itemp |= (mode << 3) | reg;
		putter(WORD,itemp);
		eat(',');
		reglist(0);
		putter(WORD,rlist);
		finishamode();
	}
}

void reglist(int start)
{
	int k,freg,lreg,inrange,lastmode,newmode;
	int rr[16];

	for (k = 0; k < 16; k++)
		rr[k] = 0;
	inrange = FALSE;
	if (start != 0)
	{
		switch(mode)
		{
		case 0: rr[reg] = 1; break;
		case 1: rr[reg+8] = 1; break;
		}
		freg = reg;
		lchar = linetxt[lineptr];
		if ('-' == lchar)
		{
			inrange = TRUE;
			eat('-');
		}
		else
		if ('/' == lchar) eat('/');
	}
	lchar = linetxt[lineptr];
	while (('A' == toupper(lchar)) || ('D' == toupper(lchar)))
	{
		switch(toupper(lchar))
		{
		case 'D': getDn();
			rr[Dn] = 1;
			if (inrange) lreg = Dn;
			else freg = Dn;
			break;
		case 'A': getAn();
			rr[An+8] = 1;
			if (inrange) lreg = An+8;
			else freg = An+8;
			break;
		}
		if (inrange)
		{
			if (((freg >= 0) && (freg <= 7)
			  && (lreg >= 0) && (lreg <= 7))
			  ||
			    ((freg >= 8) && (freg <= 15)
			  && (lreg >= 8) && (lreg <= 15)))
			{
				if (freg <= lreg)
				{
				for (k = freg; k <= lreg; k++)
					rr[k] = 1;
				}
				else
				{
				for (k = lreg; k <= freg; k++)
					rr[k] = 1;
				}
				inrange = FALSE;
				lchar = linetxt[lineptr];
				if ('/' == lchar) eat('/');
			}
			else
			{
				fatalerror(22);
				return;
			}
		}
		else
		if ('-' == lchar)
		{
			inrange = TRUE;
			eat('-');
		}
		else
		if ('/' == lchar)
			eat('/');
	}
	rlist = 0;
	rlist2 = 0;
	for (k = 0; k < 16; k++)
	{
		rlist >>= 1;
		if (rr[k]) rlist |= 0x8000;
		rlist2 <<= 1;
		if (rr[k]) rlist2 |= 1;
	}
}

void ___movep()
{
	if ((opsize != WORD) && (opsize != LONG))
	{
		warnerror(8);
		opsize = LONG;
	}
	itemp = oppatt;
	if (opsize == LONG) itemp |= 0x40;
	getamode();
	if (mode == 0)
	{
		itemp |= 0x80;
		itemp |= (reg << 9);
		eat(',');
		getamode();
		if ((mode == 5) || (mode == 2))
		{
			itemp |= reg;
			putter(WORD,itemp);
			putter(WORD,EA);
		}
		else
		{
			fatalerror(22);
			putter(WORD,itemp);
			putter(WORD,0);
		}
	}
	else
	if ((mode == 5) || (mode == 2))
	{
		itemp |= reg;
		eat(',');
		getDn();
		itemp |= (Dn << 9);
		putter(WORD,itemp);
		putter(WORD,EA);
	}
	else
	{
		fatalerror(22);
		putter(WORD,itemp);
	}
}

void ___moveq()
{
	unsigned long int idat;

	if ((opsize != UNSIZED) && (opsize != LONG))
	{
		warnerror(3);
		opsize = LONG;
	}
	eat('#');
	valundef = FALSE;
	idat = eval();
	if (BYTE != typer(idat))
	{
		fatalerror(0);
		idat = 0;
	}
	itemp = oppatt | (idat & 0xff);
	eat(',');
	getDn();
	itemp |= (Dn << 9);
	putter(WORD,itemp);
}

void ___muluMulsDivuDivsChk()
{
	if ((opsize != UNSIZED) && (opsize != WORD))
		warnerror(2);
	getamode();
	eflag = FALSE;
	notamode(1,'S');
	if ((mode == 7) && (reg == 4) && (LONG == typer(immdata)))
	{
		fatalerror(0);
		immdata = 0;
	}
	itemp = oppatt | (mode << 3) | reg;
	eat(',');
	getDn();
	itemp |= (Dn << 9);
	putter(WORD,itemp);
	finishamode();
}

void ___pea()
{
	if ((opsize != UNSIZED) && (opsize != LONG))
		warnerror(3);
	getamode();
	eflag = FALSE;
	notamode(0,'D');
	notamode(1,'D');
	notamode(3,'D');
	notamode(4,'D');
	notamode(74,'D');
	itemp = oppatt | (mode << 3) | reg;
	putter(WORD,itemp);
	finishamode();
}

void ___scc()
{
	if ((opsize != UNSIZED) && (opsize != BYTE))
		warnerror(4);
	getamode();
	eflag = FALSE;
	notamode(1,'D');
	notamode(72,'D');
	notamode(73,'D');
	notamode(74,'D');
	itemp = oppatt | (mode << 3) | reg;
	putter(WORD,itemp);
	finishamode();
}

void ___stop()
{
	if (opsize != UNSIZED) warnerror(0);
	eat('#');
	valundef = FALSE;
	immdata = eval();
	if (LONG == typer(immdata))
	{
		fatalerror(0);
		immdata = 0;
	}
	putter(WORD,oppatt);
	putter(WORD,immdata);
}

void ___swap()
{
	if ((opsize != UNSIZED) && (opsize != WORD))
		warnerror(2);
	getDn();
	itemp = oppatt | Dn;
	putter(WORD,itemp);
}

void ___tasNbcd(void)
{
	if ((opsize != UNSIZED) && (opsize != BYTE)) warnerror(4);
	getamode();
	eflag = FALSE;
	notamode(1,'D');
	notamode(72,'D');
	notamode(73,'D');
	notamode(74,'D');
	itemp = oppatt | (mode << 3);
	itemp |= reg;
	putter(WORD,itemp);
	finishamode();
}

void ___trap()
{
	if (opsize != UNSIZED) warnerror(0);
	eat('#');
	valundef = FALSE;
	itemp = eval();
	if (itemp & 0xfffffff0)
	{
		fatalerror(0);
		itemp = 0;
	}
	itemp = oppatt | (0xf & itemp);
	putter(WORD,itemp);
}

void ___unlk()
{
	if (opsize != UNSIZED) warnerror(0);
	getAn();
	itemp = oppatt | An;
	putter(WORD,itemp);
}

void warnerror(int errnum)
{
	if (warnptr == 7) return;
	warnptr++;
	warningbuff[warnptr] = errnum;
}

void showwarnings()
{
	int k;

	if (warnptr == -1) return;
	else warnptr = 0;	//4.10.98
	for (k = 0; k <= warnptr; k++)
	{
		tolst(lstptr,"W: ");
		switch(warningbuff[k])
		{
		case 0: tolst(lstptr,"Opcode does not require size extension.\n");
			break;
		case 1: tolst(lstptr,"Missing size extension, LONG assumed.\n");
			break;
		case 2: tolst(lstptr,"Size extension must be WORD.\n");
			break;
		case 3: tolst(lstptr,"Size extension must be LONG.\n");
			break;
		case 4: tolst(lstptr,"Size extension must be BYTE.\n");
			break;
		case 5: tolst(lstptr,"Size extension must be BYTE, WORD, or SHORT.\n");
			break;
		case 6: tolst(lstptr,"Size extension must be BYTE, WORD, or LONG.\n");
			break;
		case 7: tolst(lstptr,"Size extention must be BYTE or LONG.\n");
			break;
		case 8: tolst(lstptr,"Size extension must be WORD or LONG.\n");
			break;
		}
		warnings++;
	}
	warnptr = -1;
}

void fatalerror(int errnum)
{
	if (fatalptr == 7) return;
	fatalptr++;
	fatalbuff[fatalptr] = errnum;
	eater1[fatalptr] = lchar;
	eater2[fatalptr] = eatsymbol;
}

void showfatals()
{
	int k;
	char z;

	if (fatalptr == -1) return;
	else fatalptr = 0;  //4.10.98
	for (k = 0; k <= fatalptr; k++)
	{
		tolst(lstptr,"F: ");
		switch(fatalbuff[k])
		{
		case 0: tolst(lstptr,"Immediate data exceeds range.\n");
			break;
		case 1: tolst(lstptr,"Opcode requires size extension.\n");
			break;
		case 2: tolst(lstptr,"Illegal size extension.\n");
			break;
		case 3: tolst(lstptr,"Illegal register.\n");
			break;
		case 4: tolst(lstptr,"Illegal destination addressing mode.\n");
			break;
		case 5: tolst(lstptr,"Source operand must be Dn or An.\n");
			break;
		case 6: tolst(lstptr,"Destination operand must be Dn or An.\n");
			break;
		case 7: tolst(lstptr,"Found ");
			if (isprint(eater1[k]))
				tolst(lstptr,"%c",eater1[k]);
			else
			{
				switch(eater1[k])
				{
				case '\n': z = 'n'; break;
				case '\t': z = 't'; break;
				case '\r': z = 'r'; break;
				case '\b': z = 'b'; break;
				case '\f': z = 'f'; break;
				default  : z = '?'; break;
				}
				tolst(lstptr,"\'\\%c\'",z);
			}
			tolst(lstptr," when expecting \'%c\'.\n",eater2[k]);
			break;
		case 8: tolst(lstptr,"Target address out of range.\n");
			break;
		case 9: tolst(lstptr,"Zero displacement not allowed.\n");
			break;
		case 10:tolst(lstptr,"EQU requires a label.\n");
			break;
		case 11:tolst(lstptr,"%s addressing mode can not be %s.\n",
				sdstr,amstr);
			break;
		case 12:tolst(lstptr,"Operand must be Dn or -(An).\n");
			break;
		case 13:tolst(lstptr,"Mismatched addressing modes.\n");
			break;
		case 14:tolst(lstptr,"Source operand must be Dn or #data.\n");
			break;
		case 15:tolst(lstptr,"Source operand must be Dn.\n");
			break;
		case 16:tolst(lstptr,"Illegal addressing mode.\n");
			break;
		case 17:tolst(lstptr,"Index extension must be WORD or LONG.\n");
			break;
		case 18:tolst(lstptr,"Index register must be An or Dn.\n");
			break;
		case 19:tolst(lstptr,"Addressing mode must be (An)+.\n");
			break;
		case 20:tolst(lstptr,"Addressing mode must be Dn or -(An).\n");
			break;
		case 21:tolst(lstptr,"Operand must be CCR or SR.\n");
			break;
		case 22:tolst(lstptr,"Operand must be Dn or d16(An).\n");
			break;
		case 23:tolst(lstptr,"Error in register list.\n");
			break;
		case 24:tolst(lstptr,"Unexpected text encountered.\n");
			break;
		case 25:tolst(lstptr,"Opcode truncated.\n");
			break;
		case 26:tolst(lstptr,"Undefined opcode.\n");
			break;
		case 27:tolst(lstptr,"Illegal size extension.\n");
			break;
		case 28:tolst(lstptr,"Illegal label.\n");
			break;
		case 29:tolst(lstptr,"Data constant exceeds BYTE range.\n");
			break;
		case 30:tolst(lstptr,"Data constant exceeds WORD range.\n");
			break;
		case 31:tolst(lstptr,"Symbol truncated.\n");
			break;
		case 32:tolst(lstptr,"Symbol table full.\n");
			break;
		case 33:tolst(lstptr,"Duplicate symbol.\n");
			break;
		case 34:tolst(lstptr,"Undefined symbol.\n");
			break;
		case 35:tolst(lstptr,"Source operand must be An.\n");
			break;
		}
		fatals++;
	}
	fatalptr = -1;
}

