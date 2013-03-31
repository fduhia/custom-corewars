// COM68K.C: Simple C compiler
// Generates 68000 assembly language

// Version 2.1: Added array handling.
// Version 2.0: Pass-by-value function paramerters working.
// Version 1.0: First version, no arrays or function parameters

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define FALSE  0
#define TRUE   1
#define BACKUP 2
#define NORMAL 3

// tVOID, tMAIN, etc.
#define NUMRESWRD 23

#define match(y) (ctoken == (y))

//	char singles[] = "!&;{},=:()[]?'\"\\+-*/;><";
//	char doubles[] = "*= /= %= += -= || && == != <= >= << >> ++ -- //";

	enum tokens {LNOT, ADDR, SEMI, LBRACE, RBRACE, COMMA, ASSIGN, COLON,
	     LPAREN, RPAREN, LBRACKET, RBRACKET, QMARK, SQUOTE,
	     DQUOTE, FSLASH, PLUSOP, MINUSOP, TIMESOP, DIVOP,
	     LTHAN, GTHAN, TIMESEQ,
	     DIVEQ, MODEQ, PLUSEQ, MINUSEQ,
	     LOGICOR, LOGICAND, EQUAL, NOTEQUAL, LESSEQ, GREQ, SHIFTL,
	     SHIFTR, PLUSPLUS, MINUSMINUS, COMMENT, NOTOKEN,
	     tVOID, tMAIN, tINT, tCHAR,
	     tUNSIGNED, tIF, tELSE, tRETURN,
	     tDO, tWHILE, tSWITCH, tCASE,
	     tBREAK, tDEFAULT, tPRINTF, tNULL, tATOI,
	     tSCANF, tEXIT, tGETS, tSTRLEN, tGETCHAR, tASM,
	     ALPHA, NUMBER, CHAR, STRING, OTHER, EOIF};

	enum tokens ctokengl;
	enum tokens ctoken2gl;
	char tokenstrgl[33],fcnbak[33];

enum tokens GetToken(void);
void PreMainStuff(void);
void PostMainStuff(void);
void fatal(char *msg);
void ProcessFunctionBody(int tmode, enum tokens tok, char ts[]);
int GetParmList(int fnum, char p1type, char p1name[]);
int CheckParmList(int fnum);
void ProcessGlobals(void);
void ProcessLocalVars(int fnum);
void ShowFunctionStats(void);
void EatSemi(void);
int isFunction(char fcn[]);
int isGlobal(char tok[]);
int isLocal(char fcn[], char tok[]);
void LoadLibs(void);
void ParseLocalVars(int fn);
void ReadVar(char fcn[], char tok[]);
void WriteVar(char fcn[], char tok[]);
int isArray(char fcn[], char var[]);

	FILE *srcfile;
	char srcname[32];

	FILE *asmfile;
	char asmname[32];

	char input[132];
	char tokenstr[132];
	char *rwords[] = {"void", "main", "int", "char",
		       "unsigned", "if", "else", "return",
		       "do", "while", "switch", "case",
		       "break", "default", "printf", "NULL",
		       "atoi", "scanf", "exit", "gets", "strlen",
		       "getchar", "asm"};
	int pfs;

	int glvars,currglobal;
	struct
	{
		char mode;
		char type;
		char name[33];
		int size;
	} globals[10];

	int functs,currfn,currparm;
	struct
	{
		char mode;
		char type;
		char name[33];
		int nparms;
		int defined;
		struct
		{
		     char mode;
		     char type;
		     char name[33];
		} parms[4];
	} fn[10];

	int nlocals,currlocal;
	char localfn[33];
	struct
	{
		char mode;
		char type;
		char fcn[33];
		char name[33];
		int size;
	} localvar[50];

	char cfn[33], cvar[33], aindex[33], rindex[33];
	char term1[33], t1index[33], term2[33], t2index[33];
	char aitype,ritype,t1itype,t2itype;
	int doloops,ifstmts,gsknt;
	int loadprintf,loadstrlen,loadatoi;
	int pos;
	char ch;
	int lines;

int main(int argc, char *argv[])
{
	enum tokens ctoken;
	int k;

	if (argc == 1)
	{
		printf("COM68K C Compiler, Version 2.1, 8/11/03\n");
		printf("Usage: com68k sourcefile\n");
		printf("Do not enter the '.c' extension.\n");
		exit(-1);
	}
	strcpy(srcname,argv[1]);
	strcat(srcname,".c");
	srcfile = fopen(srcname,"r");
	if (srcfile == NULL)
	{
		printf("Cannot open %s\n",srcname);
		exit(-2);
	}
	strcpy(asmname,argv[1]);
	strcat(asmname,".asm");
	asmfile = fopen(asmname,"w");
	if (asmfile == NULL)
	{
		printf("Cannot open %s\n",asmname);
		exit(-2);
	}

	fprintf(asmfile,";%s created from %s\n\n",asmname,srcname);
	fprintf(asmfile,"\tORG\t$8000\n\n");


	PreMainStuff();         //this handles global variable and
				//function declarations

	ShowFunctionStats();

	ctoken = GetToken();
	if( !match(LPAREN) )
	    fatal("Missing '('");
	ctoken = GetToken();
	if( !match(RPAREN) )
	    fatal("Missing ')'");
	ctoken = GetToken();
	if( !match(LBRACE) )
	    fatal("Missing '{'");
	fprintf(asmfile,"\nStartMain\n");
	fprintf(asmfile,"\tMOVEA.L\t#PStack-32,A7\t;setup stack\n");
	fprintf(asmfile,"\tMOVEA.L\t#PStack,A0\t;setup frame ptr\n\n");

	strcpy(localfn,"main");
	strcpy(cfn,"main");
	strcpy(fn[functs].name,"main");
	fn[functs].nparms = 0;
	functs++;

	ParseLocalVars(0);

	ProcessFunctionBody(BACKUP,ctokengl,tokenstrgl);

	fprintf(asmfile,"\nMainExit\n");
	fprintf(asmfile,"\tTRAP\t#2\t;clean up display\n");
	fprintf(asmfile,"\tTRAP\t#9\t;return to monitor\n");

	ProcessLocalVars(functs-1);

	fprintf(asmfile,";-------------------------\n\n");

	PostMainStuff();        //this handles function definitions

	LoadLibs();

	fprintf(asmfile,"\n\tDS.L\t256\t;reserve room for stack\n");
	fprintf(asmfile,"PStack\n\tDC.L\t0\n\n");

	ProcessGlobals();

	fprintf(asmfile,"\tEND\tStartMain\n");

	fclose(srcfile);
	fclose(asmfile);
	printf("\n%d lines compiled.\n",lines);
	printf("No errors.\n");
}

void ShowFunctionStats()
{
       int k,p;

       if(functs > 1)
		fprintf(asmfile,"\n;Function declarations\n\n");
       for(k = 1; k < functs; k++)
       {
	      fprintf(asmfile,";%c: %s\n",fn[k].type,fn[k].name);
	      for(p = 0; p < fn[k].nparms; p++)
		     fprintf(asmfile,";\t%c: %s\n",fn[k].parms[p].type,
			    fn[k].parms[p].name);
	      fprintf(asmfile,"\n");
       }
}

void ProcessGlobals()
{
	int k;

	if(glvars)
	{
		fprintf(asmfile,";global variables\n");
		for(k = 0; k < glvars; k++)
		{
		      fprintf(asmfile,"%s\n",globals[k].name);
		      if(globals[k].size == -1)
			   switch(globals[k].type)
			   {
			   case 'C': fprintf(asmfile,"\tDC.B\t0,0\t;char\n"); break;
			   case 'I': fprintf(asmfile,"\tDC.W\t0\t;int\n"); break;
			   }
		      else
		      {
			   switch(globals[k].type)
			   {
			   case 'C': fprintf(asmfile,"\tDS.B\t%d\t;char",
				(globals[k].size & 1) ? 1+globals[k].size :
				globals[k].size); break;
			   case 'I': fprintf(asmfile,"\tDS.W\t%d\t;int",
				globals[k].size); break;
			   }
			   fprintf(asmfile," array [%d]\n",globals[k].size);
		      }
		}
		fprintf(asmfile,"\n");
	}
}

void PreMainStuff()
{
	enum tokens ctoken;
	char ttype;
	char ttoken[33];

	ctoken = GetToken();
	do
	{
		if(match(tCHAR) || match(tINT) || match(tVOID))
		{
			switch(ctoken)
			{
			case tCHAR: ttype = 'C'; break;
			case tINT: ttype = 'I'; break;
			case tVOID: ttype = 'V'; break;
			}
			ctoken = GetToken();
			if(!match(ALPHA) && !match(tMAIN))
				fatal("missing variable/function name");
			strcpy(ttoken,tokenstr);
			if((ttype == 'V') && (0 == strcmp(tokenstr,"main")))
			       return;
			ctoken = GetToken();
			if(!match(SEMI) && !match(LBRACKET) && !match(LPAREN))
				fatal("parse error");
			if(match(LBRACKET))
			{
				globals[glvars].type = ttype;
				strcpy(globals[glvars].name,ttoken);
				globals[glvars].size = -1;
				ctoken = GetToken();
				if(!match(NUMBER))
					fatal("missing array range");
				globals[glvars].size = atoi(tokenstr);
				ctoken = GetToken();
				if(!match(RBRACKET))
					fatal("missing ']'");
				EatSemi();
				glvars++;
			}
			else
			if(match(LPAREN))      //must be function dec.
			{
				fn[functs].nparms = GetParmList(functs,ttype,ttoken);
				functs++;
				EatSemi();
			}
			else
			{
				globals[glvars].type = ttype;
				strcpy(globals[glvars].name,ttoken);
				globals[glvars].size = -1;
				glvars++;
			}
			ctoken = GetToken();
		}
		else
			fatal("missing type");
	} while(TRUE);
}

void ParseLocalVars(int fn)
{
	enum tokens ctoken;
	char ttype;
	char ttoken[33];
	int lvk = 1;

	ctoken = GetToken();
	do
	{
		if(match(tCHAR) || match(tINT))
		{
			switch(ctoken)
			{
				case tCHAR: ttype = 'C'; break;
				case tINT: ttype = 'I'; break;
			}
			ctoken = GetToken();
			if(!match(ALPHA))
				fatal("missing variable name");
			strcpy(ttoken,tokenstr);
			ctoken = GetToken();
			if(!match(SEMI) && !match(LBRACKET) && !match(LPAREN))
				fatal("parse error");
			if(match(LBRACKET))
			{
				localvar[nlocals].type = ttype;
				strcpy(localvar[nlocals].name,ttoken);
				strcpy(localvar[nlocals].fcn,localfn);
				ctoken = GetToken();
				if(!match(NUMBER))
					fatal("missing array range");
				localvar[nlocals].size = atoi(tokenstr);
				ctoken = GetToken();
				if(!match(RBRACKET))
					fatal("missing ']'");
				EatSemi();
				nlocals++;
			}
			else
			{
				localvar[nlocals].type = ttype;
				strcpy(localvar[nlocals].name,ttoken);
				strcpy(localvar[nlocals].fcn,localfn);
				localvar[nlocals].size = -1;
				localvar[nlocals].mode = -2*lvk;
				lvk++;
				nlocals++;
			}
			ctoken = GetToken();
		}
	} while(match(tCHAR) || match(tINT));
	ctokengl = ctoken;
	strcpy(tokenstrgl,tokenstr);
}

int GetParmList(int fnum, char p1type, char p1name[])
{
	enum tokens ctoken;
	int pknt = 0;

	fn[fnum].type = p1type;
	strcpy(fn[fnum].name,p1name);
	fn[fnum].defined = FALSE;
	ctoken = GetToken();
	if(match(RPAREN))
	       return(0);
	if(match(tVOID))
	{
	       ctoken = GetToken();
	       if(!match(RPAREN))
		      fatal("Missing ')'");
	       return(0);
	}
	do
	{
	       switch(ctoken)
	       {
	       case tCHAR: fn[fnum].parms[pknt].type = 'C'; break;
	       case tINT: fn[fnum].parms[pknt].type = 'I'; break;
	       default: fatal("Illegal type");
	       }
	       ctoken = GetToken();
	       if(!match(ALPHA))
		      fatal("Missing variable");
	       strcpy(fn[fnum].parms[pknt].name,tokenstr);
	       pknt++;
	       ctoken = GetToken();
	       if(match(RPAREN))
		      return(pknt);
	       if(!match(COMMA))
		      fatal("Missing comma");
	       ctoken = GetToken();
	} while(pknt != 4);
	fatal("Too many parameters");
}

int CheckParmList(int fnum)
{
	enum tokens ctoken;
	char ttoken;
	int k;

	ctoken = GetToken();
	if(match(RPAREN))
	       return(0);
	for(k = 0; k < fn[fnum].nparms; k++)
	{
	       switch(ctoken)
	       {
	       case tCHAR: ttoken = 'C'; break;
	       case tINT: ttoken = 'I'; break;
	       case tVOID: ttoken = 'V'; break;
	       }
	       if(fn[fnum].parms[k].type != ttoken)
		    fatal("wrong type");

	       ctoken = GetToken();
	       if(0 != strcmp(fn[fnum].parms[k].name,tokenstr))
		      fatal("variable does not match");
	       if(k < (fn[fnum].nparms - 1))
	       {
		     ctoken = GetToken();
		     if(!match(COMMA))
			    fatal("Missing comma");
	       }
	       ctoken = GetToken();
	}
	if(!match(RPAREN))
	     fatal("missing ')'");
	return(fn[fnum].nparms);
}

void PostMainStuff()
{
	enum tokens ctoken;
	char ttype;
	char ttoken[33];
	int k,j,found;

	if(functs == 1)
		return;

	ctoken = GetToken();
	k = 0;
	do
	{
		if(match(tCHAR) || match(tINT) || match(tVOID))
		{
			switch(ctoken)
			{
			case tCHAR: ttype = 'C'; break;
			case tINT: ttype = 'I'; break;
			case tVOID: ttype = 'V'; break;
			}
			ctoken = GetToken();
			j = 0;
			found = FALSE;
			do
			{
			     if((ttype == fn[j].type)
			     && (0 == strcmp(tokenstr,fn[j].name)))
				    found = TRUE;
			     else
				    j++;
			} while((j < functs) && !found);
			if(!found)
				fatal("undeclared function");
			fn[j].defined = TRUE;
			fprintf(asmfile,"%s\n",fn[j].name);
			fprintf(asmfile,"\tLINK\tA0,#-32\n");
//			       4 + 2 * fn[j].nparms);
			strcpy(localfn,fn[j].name);
			strcpy(cfn,tokenstr);

			ctoken = GetToken();
			if(!match(LPAREN))
				fatal("missing '('");
			if(fn[j].nparms != CheckParmList(j))
			       fatal("parameter error");
			ctoken = GetToken();
			if(!match(LBRACE))
			       fatal("missing '{'");

			// parse local vars

			ParseLocalVars(j);

			ProcessFunctionBody(BACKUP,ctokengl,tokenstrgl);

			fprintf(asmfile,"\tUNLK\tA0\n");
			fprintf(asmfile,"\tRTS\n");

			ProcessLocalVars(j);

			fprintf(asmfile,";-------------------------\n\n");

			ctoken = GetToken();
		}
		else
			fatal("missing type");
		k++;
	} while(k < (functs-1));
}

enum tokens GetToken()
{
	char singles[] = "&;{},=:()[]?'\"\\+-*/><";
	char doubles[] = "*= /= %= += -= || && == != <= >= << >> ++ -- //";
	char ds[3];
	int tsptr = 0;
	int p,k,found,i;

	if (lines == 0)
	{
		do
		{
			fgets(input,131,srcfile);
			if(feof(srcfile))
				return(EOIF);
			if( (1 == strlen(input)) && (0x0a == input[0]) )
			{
			//do not process a blank line
			}
			else
			{
				printf("\nLine %d: %s",lines++,input);
				p = 0;
				do
				{
					ch = input[p];
					p++;
				} while((ch == ' ') || (ch == '\t'));     /* Skip blanks */
				fprintf(asmfile,";\t%s",&input[p-1]);
				pos = 0;
			}
		} while (0 == lines);
	}

	if((pos >= strlen(input)) || (0x0a == input[pos]))
	{
	//get a new line
		do
		{
			fgets(input,131,srcfile);
			if(feof(srcfile))
				return(EOIF);
			if( (1 == strlen(input)) && (0x0a == input[0]) )
			{
			//do not process a blank line
			}
			else
			{
				printf("\nLine %d: %s",lines++,input);
				p = 0;
				do
				{
					ch = input[p];
					p++;
				} while((ch == ' ') || (ch == '\t'));     /* Skip blanks */
				fprintf(asmfile,";\t%s",&input[p-1]);
				pos = 0;
			}
		} while (0 != pos);

	}
	do
	{
		ch = input[pos];
		pos++;
	} while(ch == ' ');     /* Skip blanks */

	/* Do we have a symbol name? */
	if(isalpha(ch) != 0)
	{
		printf("%c",ch);        /* Print first letter */
		tokenstr[tsptr++] = ch;

		/* Print remaining letters/digits */
		while(isalnum(ch = input[pos]))
		{
			printf("%c",input[pos]);
			tokenstr[tsptr++] = ch;
			pos++;
		}
		tokenstr[tsptr] = '\0';
		printf("<%s>",tokenstr);
		found = FALSE;
		k = 0;
		do
		{
			if (0 == strcmp(tokenstr,rwords[k]))
			      found = TRUE;
			else
			    k++;
		} while ((k < NUMRESWRD) && !found);
		if(found)
			return(tVOID + k);
		else
			return(ALPHA);
	}
	/* Do we have a number? */
	if(isdigit(ch))
	{
		printf("%c",ch);        /* Print first digit */
		tokenstr[tsptr++] = ch;

		/* Print remaining digits */
		while(isdigit(ch = input[pos]))
		{
			printf("%c",input[pos]);
			tokenstr[tsptr++] = ch;
			pos++;
		}
		tokenstr[tsptr] = '\0';
		return(NUMBER);
	}

	/* Check for double token */

	ds[0] = ch;            /* Load double token */
	ds[1] = input[pos];
	if(ds[1] == ' ')
		ds[1] = 0x07;
	ds[2] = '\0';
	if(strstr(doubles,ds) != '\0')
	{
		printf("<<%s>>",ds);        /* Print it */
		strcpy(tokenstr,ds);
		pos++;
		for(i = 0; i < 46; i+=3)
		{
			if((ds[0] == doubles[i]) &&
			 (ds[1] == doubles[i+1]))
				p = i;
		}
		switch(p)
		{
		case 0: return(TIMESEQ);
		case 3: return(DIVEQ);
		case 6: return(MODEQ);
		case 9: return(PLUSEQ);
		case 12: return(MINUSEQ);
		case 15: return(LOGICOR);
		case 18: return(LOGICAND);
		case 21: return(EQUAL);
		case 24: return(NOTEQUAL);
		case 27: return(LESSEQ);
		case 30: return(GREQ);
		case 33: return(SHIFTL);
		case 36: return(SHIFTR);
		case 39: return(PLUSPLUS);
		case 42: return(MINUSMINUS);
		case 45:
			do
			{
				ch = input[pos];
				pos++;
			} while (ch != 0x0a);
			return( GetToken() );
		}
	}

	/* Check for a single token */
	if(strchr(singles,ch) != '\0')
	{
		printf("%c",ch);        /* Print token */
		tokenstr[tsptr++] = ch;

		/* String or character token? */
		if((ch == '\"') || (ch == '\''))
		{
			do      /* Print remaining token text */
			{
				ch = input[pos];
				printf("%c",ch);
				tokenstr[tsptr++] = ch;
				pos++;
			} while((ch != '\"') && (ch != '\''));
			tokenstr[tsptr] = '\0';
			if( tokenstr[0] == '\"')
				return(STRING);
			else
				return(CHAR);
		}

		/* Check for == token */
		if((ch == '=') && (input[pos] == '='))
		{
			printf("=");
			pos++;
			return(EQUAL);
		}
		//select single token value
		switch(ch)
		{
		case '!': return(LNOT);
		case '&': return(ADDR);
		case ';': return(SEMI);
		case '{': return(LBRACE);
		case '}': return(RBRACE);
		case ',': return(COMMA);
		case '=': return(ASSIGN);
		case ':': return(COLON);
		case '(': return(LPAREN);
		case ')': return(RPAREN);
		case '[': return(LBRACKET);
		case ']': return(RBRACKET);
		case '?': return(QMARK);
		case '\'': return(SQUOTE);
		case '\"': return(DQUOTE);
		case '\\': return(FSLASH);
		case '+': return(PLUSOP);
		case '-': return(MINUSOP);
		case '*': return(TIMESOP);
		case '/': return(DIVOP);
		case '>': return(GTHAN);
		case '<': return(LTHAN);
		}
	}
	else
	{
		printf("%c",ch);
		tokenstr[tsptr++] = ch;
		tokenstr[tsptr] = '\0';
		return(OTHER);
	}
}

void fatal(char *msg)
{
	printf("%s\n",msg);
	fclose(srcfile);
	fclose(asmfile);
	exit(-1);
}


void EatSemi()
{
       enum tokens ctoken;

       ctoken = GetToken();
       if(!match(SEMI))
	     fatal("missing ';'");
}

int isFunction(char fcn[])
{
     int k,found;

     k = 0;
     found = FALSE;
     do
     {
	    if(0 == strcmp(fcn,fn[k].name))
	    {
		   found = TRUE;
		   currfn = k;
	    }
	    else
		   k++;
     } while ((k < functs) && !found);
     return(found);
}

int isGlobal(char tok[])
{
     int k,found;

     k = 0;
     found = FALSE;
     do
     {
	    if(0 == strcmp(tok,globals[k].name))
	    {
		   found = TRUE;
		   currglobal = k;
	    }
	    else
		   k++;
     } while ((k < glvars) && !found);
     return(found);
}

int isLocal(char fcn[], char tok[])
{
     int k,found;

     k = 0;
     found = FALSE;
     do
     {
	    if((0 == strcmp(tok,localvar[k].name)) &&
	     (0 == strcmp(fcn,localvar[k].fcn)))
	    {
		   found = TRUE;
		   currlocal = k;
	    }
	    else
		   k++;
     } while ((k < nlocals) && !found);
     return(found);
}

int isArray(char fcn[], char var[])
{
	int found = FALSE;

	if(isLocal(fcn,var))
	{
		if(localvar[currlocal].size != -1)
		{
			found = TRUE;
		}
	}
	else
	if(isGlobal(var))
	{
		if(globals[currglobal].size != -1)
		{
			found = TRUE;
		}
	}
	return(found);
}

int isFparm(char fcn[], char par[])
{
     int j,k,found;

     k = 0;
     found = FALSE;
     do
     {
	    if(0 == strcmp(fcn,fn[k].name))
	    {
		j = 0;
		do
		{
			if(0 == strcmp(par,fn[k].parms[j].name))
			{
				currparm = j;
				currfn = k;
				found = TRUE;
			}
			else
				j++;
		} while ((j < fn[k].nparms) && !found);
	    }
	    else
		   k++;
     } while ((k < (functs-1)) && !found);
     return(found);
}

void ProcessLocalVars(int fnum)
{
	int k,first = TRUE;

	if(!nlocals)
	     return;

	for(k = 0; k < nlocals; k++)
	{
		if(0 == strcmp(localfn,localvar[k].fcn))
		{
			if(first)
			{
				fprintf(asmfile,"\n;Local variables for %s()\n\n",localfn);
				first = FALSE;
			}
			if(localvar[k].size == -1)
				fprintf(asmfile,";%s\n;\t<stack var>, %d(A0)\n\n",
					localvar[k].name,localvar[k].mode);
			else
			{
				fprintf(asmfile,"F%d%s\n\tDS.",fnum,localvar[k].name);
				if(localvar[k].type == 'C')
					fprintf(asmfile,"B\t");
				else
					fprintf(asmfile,"W\t");
				if(localvar[k].type == 'C')
				{
					fprintf(asmfile,"%d\t; char array [%d]\n",
						localvar[k].size,localvar[k].size);
					if(localvar[k].size & 1)
						fprintf(asmfile,"\tDS.B\t1\n");
					fprintf(asmfile,"\n");
				}
				else
					fprintf(asmfile,"%d\t; int array [%d]\n\n",
						localvar[k].size,localvar[k].size);
			}
		}
	}
	fprintf(asmfile,"\n");
}

void ProcessFunctionBody(int tmode,enum tokens tok, char ts[])
{
	enum tokens ctoken;
	enum tokens ctoken2;
	enum tokens tstack[5];
	char type;
	int val;
	int dlknt,ifknt;
	char rvar[33],fcall[33],rvtype;
	int tsptr,k;
	char tstr[5][33];
	int vps;
	int spalpha;

	if(tmode != BACKUP)
		ctoken = GetToken();
	else
	{
		ctoken = ctokengl;
		strcpy(tokenstr,tokenstrgl);
	}
	do
	{
	spalpha = FALSE;
	if(match(tIF))
	{
		ifknt = ifstmts;
		ifstmts++;
		ctoken = GetToken();
		if(!match(LPAREN))
			fatal("missing '('");
		ctoken = GetToken();
		if(match(ALPHA))
		{
			ReadVar(cfn,tokenstr);
			ctoken = GetToken();
			ctoken2 = ctoken;
			ctoken = GetToken();
			if(match(CHAR) || match(NUMBER) || match(ALPHA))	// if(var <op> number)
			{
				if(match(CHAR))
					fprintf(asmfile,"\tCMPI.B\t#%s,D7\n",tokenstr);
				else
				if(match(NUMBER))
					fprintf(asmfile,"\tCMPI.W\t#%s,D7\n",tokenstr);
				else
				{
					fprintf(asmfile,"\tMOVE.W\tD7,-(A7)\n");
					ReadVar(cfn,tokenstr);
					fprintf(asmfile,"\tMOVE.W\tD7,D4\n");
					fprintf(asmfile,"\tMOVE.W\t(A7)+,D7\n");
					fprintf(asmfile,"\tCMP.W\tD4,D7\n",tokenstr);

				}
				ctoken = ctoken2;
				if(match(EQUAL))
				{
					fprintf(asmfile,"\tBNE\tEndIF%d\n\n",ifknt);
				}
				else
				if(match(NOTEQUAL))
				{
					fprintf(asmfile,"\tBEQ\tEndIF%d\n\n",ifknt);
				}
				else
				if(match(LTHAN))
				{
					fprintf(asmfile,"\tBGT\tEndIF%d\n\n",ifknt);
				}
				else
				if(match(GTHAN))
				{
					fprintf(asmfile,"\tBLT\tEndIF%d\n\n",ifknt);
				}
				if(match(LOGICAND))
				{
					fprintf(asmfile,"\tBEQ\tEndIF%d\n\n",ifknt);
				}
				if(match(LOGICOR))
				{
					fprintf(asmfile,"\tBEQ\tEndIF%d\n\n",ifknt);
				}
				ctoken = GetToken();
				if(!match(RPAREN))
					fatal("missing ')'");
				ctoken = GetToken();
				if(!match(LBRACE))
					fatal("missing '{'");
				ProcessFunctionBody(NORMAL,NOTOKEN,"");
			}
			else
			if(match(CHAR))	// if(var <op> char)
			{
				fprintf(asmfile,"\tCMPI.B\t#%s,D7\n",tokenstr);
				ctoken = ctoken2;
				if(match(EQUAL))
				{
					fprintf(asmfile,"\tBNE\tEndIF%d\n\n",ifknt);
				}
				else
				if(match(NOTEQUAL))
				{
					fprintf(asmfile,"\tBEQ\tEndIF%d\n\n",ifknt);
				}
				if(match(LOGICAND))
				{
					fprintf(asmfile,"\tBEQ\tEndIF%d\n\n",ifknt);
				}
				if(match(LOGICOR))
				{
					fprintf(asmfile,"\tBEQ\tEndIF%d\n\n",ifknt);
				}
				ctoken = GetToken();
				if(!match(RPAREN))
					fatal("missing ')'");
				ctoken = GetToken();
				if(!match(LBRACE))
					fatal("missing '{'");
				ProcessFunctionBody(NORMAL,NOTOKEN,"");
			}
			else
			if(match(RPAREN))	// if(var)
			{
			}
			else
			if(match(LNOT))		// if(!var)
			{
			}
		}
		ctoken = GetToken();
		if(match(tELSE))
		{
			fprintf(asmfile,"\n\tBRA\tEndIF%db\n",ifknt);
			fprintf(asmfile,"EndIF%d\n",ifknt);

			ctoken = GetToken();
			if(!match(LBRACE))
				fatal("missing '{'");
			ProcessFunctionBody(NORMAL,NOTOKEN,"");
			fprintf(asmfile,"\nEndIF%db\n\n",ifknt);
			ctoken = GetToken();
		}
		else
			fprintf(asmfile,"\nEndIF%d\n",ifknt);
	}
	else
	if(match(tDO))
	{
		dlknt = doloops;
		doloops++;
		ctoken = GetToken();
		if(!match(LBRACE))
			fatal("missing '{'");
		fprintf(asmfile,"\nDLoop%d\n\n",dlknt);
		ProcessFunctionBody(NORMAL,NOTOKEN,"");
		ctoken = GetToken();
		if(!match(tWHILE))
			fatal("missing while");
		ctoken = GetToken();
		if(!match(LPAREN))
			fatal("missing '('");
		ctoken = GetToken();
		if(match(ALPHA))
		{
			ReadVar(cfn,tokenstr);
			ctoken = GetToken();
			ctoken2 = ctoken;
			ctoken = GetToken();
			if(match(NUMBER))
			{
				fprintf(asmfile,"\tCMPI.W\t#%s,D7\n",tokenstr);
				ctoken = ctoken2;
				if(match(EQUAL))
				{
					fprintf(asmfile,"\tBEQ\tDLoop%d\n\n",dlknt);
				}
				else
				if(match(NOTEQUAL))
				{
					fprintf(asmfile,"\tBNE\tDLoop%d\n\n",dlknt);
				}
			}
			else
			if(match(CHAR))
			{
				fprintf(asmfile,"\tCMPI.B\t#%s,D7\n",tokenstr);
				ctoken = ctoken2;
				if(match(EQUAL))
				{
					fprintf(asmfile,"\tBEQ\tDLoop%d\n\n",dlknt);
				}
				else
				if(match(NOTEQUAL))
				{
					fprintf(asmfile,"\tBNE\tDLoop%d\n\n",dlknt);
				}
			}
			else
			if(match(ALPHA))
			{
				fprintf(asmfile,"\tMOVE.W\tD7,-(A7)\n");
				ReadVar(cfn,tokenstr);
				fprintf(asmfile,"\tMOVE.W\tD7,D4\n");
				fprintf(asmfile,"\tMOVE.W\t(A7)+,D7\n");
				fprintf(asmfile,"\tCMP.W\tD4,D7\n",tokenstr);
				ctoken = ctoken2;
				if(match(EQUAL))
				{
					fprintf(asmfile,"\tBEQ\tDLoop%d\n\n",dlknt);
				}
				else
				if(match(NOTEQUAL))
				{
					fprintf(asmfile,"\tBNE\tDLoop%d\n\n",dlknt);
				}
				else
				if(match(LTHAN))
				{
					fprintf(asmfile,"\tBLT\tDLoop%d\n\n",dlknt);
				}
				else
				if(match(GTHAN))
				{
					fprintf(asmfile,"\tBGT\tDLoop%d\n\n",dlknt);
				}
			}
		}
		ctoken = GetToken();
		if(!match(RPAREN))
			fatal("missing ')'");
		EatSemi();
		ctoken = GetToken();
	}
	else
	if(match(tGETS))
	{
		ctoken = GetToken();
		if(!match(LPAREN))
			fatal("missing '('");
		ctoken = GetToken();
		if(!match(ALPHA))
			fatal("missing variable");
		if(isGlobal(tokenstr))
			fprintf(asmfile,"\n\tLEA\t%s,A3\t;initial character pointer\n",tokenstr);
		else
		if(isLocal(cfn,tokenstr))
		{
			if(localvar[currlocal].size == -1)
			{
			}
			else
			{
				fprintf(asmfile,"\n\tLEA\tF%d%s,A3\t;initial char position\n",
					currfn,tokenstr);
			}
		}
		else
			fatal("unknown variable");
		fprintf(asmfile,"Gstr%d\tTRAP\t#0\t;read character\n",gsknt);
		fprintf(asmfile,"\tCMPI.B\t#13,D1\t;Enter pressed?\n");
		fprintf(asmfile,"\tBEQ.S\tGstr%db\n",gsknt);
		fprintf(asmfile,"\tTRAP\t#1\t;echo character\n");
		fprintf(asmfile,"\tMOVE.B\tD1,(A3)+\t;save char in array\n");
		fprintf(asmfile,"\tBRA\tGstr%d\n",gsknt);
		fprintf(asmfile,"Gstr%db\tMOVE.B\t#0,(A3)\t;terminate string\n",gsknt);
		fprintf(asmfile,"\tTRAP\t#2\t;newline\n\n");
		gsknt++;
		ctoken = GetToken();
		if(!match(RPAREN))
			fatal("missing ')'");
		EatSemi();
		ctoken = GetToken();
	}
	else
	if(match(tGETCHAR))
	{
		ctoken = GetToken();
		if(!match(LPAREN))
			fatal("missing '('");
		ctoken = GetToken();
		if(!match(ALPHA))
			fatal("missing variable");
		fprintf(asmfile,"\n\tTRAP\t#0\t;read character\n");
		fprintf(asmfile,"\tTRAP\t#1\t;echo character\n");
		fprintf(asmfile,"\tMOVE.B\tD1,D7\n");
		WriteVar(cfn,tokenstr);
		ctoken = GetToken();
		if(!match(RPAREN))
			fatal("missing ')'");
		EatSemi();
		ctoken = GetToken();
	}
	else
	if(match(tASM))
	{
		ctoken = GetToken();
		if(!match(LPAREN))
			fatal("missing '('");
		ctoken = GetToken();
		if(!match(STRING))
			fatal("missing asm string");
		tokenstr[0] = ' ';
		tokenstr[strlen(tokenstr)-1] = ' ';
		fprintf(asmfile,"\n\t%s\n",tokenstr);
		ctoken = GetToken();
		if(!match(RPAREN))
			fatal("missing ')'");
		EatSemi();
		ctoken = GetToken();
	}
	else
	if(match(tSCANF))
	{
		ctoken = GetToken();
		if(!match(LPAREN))
			fatal("missing '('");
		ctoken = GetToken();
		if(!match(STRING))
			fatal("missing string");
		if(tokenstr[1] == '%')
		{
			type = tokenstr[2];
			ctoken = GetToken();
			if(!match(COMMA))
				fatal("missing comma");
			ctoken = GetToken();
			if(!match(ADDR))
				fatal("missing '&'");
			ctoken = GetToken();
//			type = globals[1].type;
			switch(type)
			{
			case 'c':
			case 'C':
			fprintf(asmfile,"\n\tTRAP\t#0\t;get character\n");
			fprintf(asmfile,"\tMOVE.B\tD1,D7\n");
			WriteVar(cfn,tokenstr);
			break;
			case 'I':
			break;
			}
		}
		ctoken = GetToken();
		if(!match(RPAREN))
			fatal("missing ')'");
		EatSemi();
		ctoken = GetToken();
	}
	else
	if(match(tPRINTF))
	{
		ctoken = GetToken();
		if(!match(LPAREN))
			fatal("missing '('");
		ctoken = GetToken();
		if(!match(STRING))
			fatal("missing string");
		if((tokenstr[1] == '%') && (tokenstr[2] == 'x'))
		{
			ctoken = GetToken();
			if(!match(COMMA))
				fatal("missing comma");
			ctoken = GetToken();
			ReadVar(cfn,tokenstr);
			fprintf(asmfile,"\n\tMOVE.W\tD7,D2\n");
			fprintf(asmfile,"\tTRAP\t#5\t;display word in hex\n\n");
		}
		else
		if((tokenstr[1] == '%') && (tokenstr[2] == 'd'))
		{
			ctoken = GetToken();
			if(!match(COMMA))
				fatal("missing comma");
			ctoken = GetToken();
			ReadVar(cfn,tokenstr);
			fprintf(asmfile,"\n\tMOVE.W\tD7,D6\n");
			fprintf(asmfile,"\tJSR\tBCD16\t;display byte in decimal\n\n");
			loadprintf = TRUE;
		}
		else
		if((tokenstr[1] == '%') && (tokenstr[2] == 's'))
		{
			ctoken = GetToken();
			if(!match(COMMA))
				fatal("missing comma");
			ctoken = GetToken();
			if(match(ALPHA))
			{
				if(isGlobal(tokenstr))
					fprintf(asmfile,"\n\tLEA\t%s,A3\t;initial character pointer\n",tokenstr);
				else
				if(isLocal(cfn,tokenstr))
				{
					if(localvar[currlocal].size == -1)
					{
					}
					else
					{
						fprintf(asmfile,"\n\tLEA\tF%d%s,A3\t;initial char position\n",
							currfn,tokenstr);
					}
				}
				else
					fatal("unknown variable");
				fprintf(asmfile,"Pstr%d\tMOVE.B\t(A3)+,D1\t;get next char\n",gsknt);
				fprintf(asmfile,"\tCMPI.B\t#0,D1\t;End of string?\n");
				fprintf(asmfile,"\tBEQ.S\tPstr%db\n",gsknt);
				fprintf(asmfile,"\tTRAP\t#1\t;echo character\n");
				fprintf(asmfile,"\tBRA.S\tPstr%d\n",gsknt);
				fprintf(asmfile,"Pstr%db\n\n",gsknt);
				gsknt++;
			}
		}
		else
		if((tokenstr[1] == '%') && (tokenstr[2] == 'c'))
		{
			ctoken = GetToken();
			if(!match(COMMA))
				fatal("missing comma");
			ctoken = GetToken();
			if(match(ALPHA))
			{
				ReadVar(cfn,tokenstr);
				fprintf(asmfile,"\tMOVE.B\tD7,D1\t;get char\n");
				fprintf(asmfile,"\tTRAP\t#1\t;echo character\n\n");
			}
		}
		else
		if(tokenstr[1] == '\\')
		{
			fprintf(asmfile,"\n\tTRAP\t#2\t;output CR, LF\n\n");
		}
		else
		{
			tokenstr[0] = '\'';
			tokenstr[strlen(tokenstr)-1] = '\'';
			fprintf(asmfile,"\n\tBRA.S\tMout%d\t;skip over message text\n",pfs);
			fprintf(asmfile,"Msg%d\tDC.B\t%s,0",pfs,tokenstr);
			if(0 == (1 & strlen(tokenstr)))
			     fprintf(asmfile,",0");
			fprintf(asmfile,"\nMout%d\tMOVEA.L\t#Msg%d,A3\n",pfs,pfs);
			pfs++;
			fprintf(asmfile,"\tTRAP\t#3\t;display ASCII text\n\n");
		}
		ctoken = GetToken();
		if(!match(RPAREN))
			fatal("missing ')'");
		EatSemi();
		ctoken = GetToken();
	}
	else
	if(match(tRETURN))
	{
		ctoken = GetToken();
		if(!match(LPAREN))
			fatal("missing '('");
		ctoken = GetToken();
		if(match(ALPHA) || match(NUMBER) || match(CHAR))
		{
			if(match(ALPHA))
				ReadVar(cfn,tokenstr);
			else
				fprintf(asmfile,"\n\tMOVE.W\t#%s,D7\n",tokenstr);
			ctoken = GetToken();
		}
		if(!match(RPAREN))
			fatal("missing ')'");
		fprintf(asmfile,"\tUNLK\tA0\n");
		fprintf(asmfile,"\tRTS\n\n");
		EatSemi();
		ctoken = GetToken();
	}
	else
	if(match(ALPHA))
	{
		if(isFunction(tokenstr))
		{
		       vps = -1;
		       tsptr = 0;
		       strcpy(fcall,tokenstr);
		       ctoken = GetToken();
		       if(!match(LPAREN))
			      fatal("missing '(')");
		       ctoken = GetToken();
		       if(match(ALPHA) || match(NUMBER) || match(CHAR))
		       {
			      do
			      {
					tstack[tsptr] = ctoken;
					strcpy(tstr[tsptr],tokenstr);
					tsptr++;
					ctoken = GetToken();
					if(ctoken == COMMA)
						ctoken = GetToken();
			      } while(match(ALPHA) || match(NUMBER) || match(CHAR));
			      fprintf(asmfile,"\n");
			      vps = tsptr;
			      for(k = vps-1; k >= 0; k--)
			      {
					switch(tstack[k])
					{
					case ALPHA:   ReadVar(cfn,tstr[k]);
						break;
					case NUMBER:  fprintf(asmfile,"\tMOVE.W\t#%s,D7\n",tstr[k]);
						fprintf(asmfile,"\tMOVE.W\tD7,-(A7)\n");
						break;
					case CHAR:    fprintf(asmfile,"\tMOVE.B\t#%s,D7\n",tstr[k]);
						fprintf(asmfile,"\tMOVE.W\tD7,-(A7)\n");
					}
			      }
		       }

		       if(match(RPAREN))
		       {
			      fprintf(asmfile,"\n\tJSR\t%s\n\n",fcall);
			      for(k = 0; k < vps; k++)
				fprintf(asmfile,"\tMOVE.W\t(A7)+,D6\n");
		       }
		       EatSemi();
		       ctoken = GetToken();
		}
		else                    //must be var name
		{
			strcpy(rvar,tokenstr);
			if(isLocal(cfn,rvar))
				rvtype = localvar[currlocal].type;
			else
			if(isGlobal(rvar))
				rvtype = globals[currglobal].type;
			if(isArray(cfn,rvar))
			{
				ctoken = GetToken();
				if(!match(LBRACKET))
					fatal("missing '['");
				ctoken = GetToken();
				if(match(NUMBER))
				{
					ritype = 'N';
				}
				if(match(ALPHA))
				{
					ritype = 'A';
				}
				strcpy(rindex,tokenstr);
				ctoken = GetToken();
				if(!match(RBRACKET))
					fatal("missing ']'");
			}

			ctoken = GetToken();
			if(match(ASSIGN))
			{
				ctoken = GetToken();
				if(match(NUMBER))
				{
					fprintf(asmfile,"\tMOVE.W\t#%s,D7\n",tokenstr);
					strcpy(aindex,rindex);
					aitype = ritype;
					WriteVar(cfn,rvar);
				}
				else
				if(match(CHAR))
				{
					fprintf(asmfile,"\tMOVE.B\t#%s,D7\n",tokenstr);
					strcpy(aindex,rindex);
					aitype = ritype;
					WriteVar(cfn,rvar);
				}
				else
				if(match(tSTRLEN))
				{
					ctoken = GetToken();
					if(!match(LPAREN))
						fatal("missing '('");
					ctoken = GetToken();
					if(match(ALPHA))
					{
						if(isGlobal(tokenstr))
							fprintf(asmfile,"\n\tLEA\t%s,A3\n",tokenstr);
						else
						if(isLocal(cfn,tokenstr))
						{
							if(localvar[currlocal].size == -1)
							{
							}
							else
							{
								fprintf(asmfile,"\n\tLEA\tF%d%s,A3\n",
									currfn,tokenstr);
							}
						}
						else
							fatal("unknown variable");
						fprintf(asmfile,"\tJSR\tSTRLEN\n");
						aitype = ritype;
						strcpy(aindex,rindex);
						WriteVar(cfn,rvar);
						loadstrlen = TRUE;
						ctoken = GetToken();
						if(!match(RPAREN))
							fatal("missing ')'");
					}
				}
				else
				if(match(tATOI))
				{
					ctoken = GetToken();
					if(!match(LPAREN))
						fatal("missing '('");
					ctoken = GetToken();
					if(match(ALPHA))
					{
						if(isGlobal(tokenstr))
							fprintf(asmfile,"\n\tLEA\t%s,A3\n",tokenstr);
						else
						if(isLocal(cfn,tokenstr))
						{
							if(localvar[currlocal].size == -1)
							{
							}
							else
							{
								fprintf(asmfile,"\n\tLEA\tF%d%s,A3\n",
									currfn,tokenstr);
							}
						}
						else
							fatal("unknown variable");

						fprintf(asmfile,"\tJSR\tATOI\n");
						aitype = ritype;
						strcpy(aindex,rindex);
						WriteVar(cfn,rvar);
						loadatoi = TRUE;
						ctoken = GetToken();
						if(!match(RPAREN))
							fatal("missing ')'");
					}
				}
				else
				if(match(ALPHA))
				{
					spalpha = FALSE;
					if(isFunction(tokenstr))
					{
						strcpy(fcnbak,tokenstr);
						ctoken = GetToken();
						if(!match(LPAREN))
							fatal("missing '('");
						ctoken = GetToken();
						if(match(NUMBER))
						{
							fprintf(asmfile,"\n\tMOVE.W\t#%s,-(A7)",tokenstr);
							ctoken = GetToken();
						}
						else
						if(match(ALPHA))
						{
							ReadVar(cfn,tokenstr);
							fprintf(asmfile,"\n\tMOVE.W\tD7,-(A7)",tokenstr);
							ctoken = GetToken();

						}
						fprintf(asmfile,"\n\tJSR\t%s\n",fcnbak);
						strcpy(aindex,rindex);
						aitype = ritype;
						WriteVar(cfn,rvar);
//						ctoken = GetToken();
						if(!match(RPAREN))
							fatal("missing ')'");

					}
					else

					{
						strcpy(term1,tokenstr);
						if(isArray(cfn,term1))
						{
							ctoken = GetToken();
							if(!match(LBRACKET))
								fatal("missing '['");
							ctoken = GetToken();
							if(match(NUMBER))
							{
								t1itype = 'N';
							}
							else
							if(match(ALPHA))
							{
								t1itype = 'A';
							}
							strcpy(t1index,tokenstr);
							ctoken = GetToken();
							if(!match(RBRACKET))
								fatal("missing ']'");
						}
						aitype = t1itype;
						strcpy(aindex,t1index);
						ReadVar(cfn,term1);
						ctoken = GetToken();
						ctoken2 = ctoken;
						if(match(PLUSOP) || match(MINUSOP) ||
						 match(TIMESOP) || match(DIVOP) ||
						 match(SHIFTL) || match(SHIFTR) ||
						 match(ADDR))
						{
							ctoken = GetToken();
							if(match(NUMBER))
							{
								fprintf(asmfile,"\tMOVE.W\t#%s,D4\n",tokenstr);
							}
							else
							if(match(ALPHA))
							{
								fprintf(asmfile,"\tMOVE.W\tD7,-(A7)\n");
								ReadVar(cfn,tokenstr);
								fprintf(asmfile,"\tMOVE.W\tD7,D4\n");
								fprintf(asmfile,"\tMOVE.W\t(A7)+,D7\n");
							}
							switch(ctoken2)
							{
								case PLUSOP: fprintf(asmfile,"\tADD.W\tD4,D7\n");
								break;
							case MINUSOP: fprintf(asmfile,"\tSUB.W\tD4,D7\n");
								break;
							case TIMESOP: fprintf(asmfile,"\tMULU.W\tD4,D7\n");
								break;
							case DIVOP: fprintf(asmfile,"\tDIVU.W\tD4,D7\n");
								break;
							case SHIFTL: fprintf(asmfile,"\tASL.W\tD4,D7\n");
								break;
							case SHIFTR: fprintf(asmfile,"\tASR.W\tD4,D7\n");
								break;
							case ADDR: fprintf(asmfile,"\tAND.W\tD4,D7\n");
								break;
							}
							aitype = ritype;
							strcpy(aindex,rindex);
							WriteVar(cfn,rvar);
						}
						else
						if(match(SEMI))
						{
							spalpha = TRUE;
							aitype = ritype;
							strcpy(aindex,rindex);
							WriteVar(cfn,rvar);
						}
					}
				}
			}
			else
			if(match(PLUSPLUS))
			{
				strcpy(aindex,rindex);
				aitype = ritype;
				ReadVar(cfn,rvar);
				fprintf(asmfile,"\tADDI.W\t#1,D7\n");
				WriteVar(cfn,rvar);
			}
			else
			if(match(MINUSMINUS))
			{
				strcpy(aindex,rindex);
				aitype = ritype;
				ReadVar(cfn,rvar);
				fprintf(asmfile,"\n\tSUBI.");
				if(rvtype == 'C')
					fprintf(asmfile,"B");
				else
					fprintf(asmfile,"W");
				fprintf(asmfile,"\t#1,D7\n");
				WriteVar(cfn,rvar);
			}
			if(!spalpha)
				EatSemi();
			ctoken = GetToken();
		}
	}
	} while(!match(RBRACE));

}

void LoadLibs()
{
	if(loadprintf || loadstrlen || loadatoi)
		fprintf(asmfile,"\n;Library Subroutines\n");

	if(loadprintf)
	{
	  fprintf(asmfile,"\nBCD16\tMOVE.W\t#10000,D5\t;do 10,000's digit\n");
	  fprintf(asmfile,"\tBSR\tDODIG\n");
	  fprintf(asmfile,"\tMOVE.W\t#1000,D5\t;do 1,000's digit\n");
	  fprintf(asmfile,"\tBSR\tDODIG\n");
	  fprintf(asmfile,"BCD8\tMOVE.W\t#100,D5\t;do 100's digit\n");
	  fprintf(asmfile,"\tBSR\tDODIG\n");
	  fprintf(asmfile,"\tMOVE.W\t#10,D5\t;do 10's digit\n");
	  fprintf(asmfile,"\tBSR\tDODIG\n");
	  fprintf(asmfile,"\tMOVE.B\tD6,D1\t;load 1's digit\n");
	  fprintf(asmfile,"\tADDI.B\t#$30,D1\t;add ASCII bias\n");
	  fprintf(asmfile,"\tTRAP\t#1\t;and display\n");
	  fprintf(asmfile,"\tRTS\n\n");
	  fprintf(asmfile,"DODIG\tANDI.L\t#$FFFF,D6\t;clear upper word of D6\n");
	  fprintf(asmfile,"\tDIVU\tD5,D6\t;divide D6 by D5\n");
	  fprintf(asmfile,"\tMOVE.B\tD6,D1\t;load result digit\n");
	  fprintf(asmfile,"\tADDI.B\t#$30,D1\t;add ASCII bias\n");
	  fprintf(asmfile,"\tTRAP\t#1\t;display digit\n");
	  fprintf(asmfile,"\tSWAP\tD6\t;get remainder\n");
	  fprintf(asmfile,"\tRTS\n");
	}

	if(loadstrlen)
	{
		fprintf(asmfile,"\nSTRLEN\tCLR.W\tD7\n");
		fprintf(asmfile,"FNULL\tCMPI.B\t#0,(A3)+\n");
		fprintf(asmfile,"\tBEQ.S\tRSLEN\n");
		fprintf(asmfile,"\tADDI.W\t#1,D7\n");
		fprintf(asmfile,"\tBRA.S\tFNULL\n");
		fprintf(asmfile,"RSLEN\tRTS\n");
	}

	if(loadatoi)
	{
		fprintf(asmfile,"\nATOI\tCLR.W\tD7\n");
		fprintf(asmfile,"GNXTC\tMOVE.B\t(A3)+,D3\n");
		fprintf(asmfile,"\tCMPI.B\t#0,D3\n");
		fprintf(asmfile,"\tBEQ.S\tXATOI\n");
		fprintf(asmfile,"\tMULU.W\t#10,D7\n");
		fprintf(asmfile,"\tANDI.W\t#$000F,D3\n");
		fprintf(asmfile,"\tADD.W\tD3,D7\n");
		fprintf(asmfile,"\tBRA.S\tGNXTC\n");
		fprintf(asmfile,"XATOI\tRTS\n");
	}
}

void ReadVar(char fcn[], char tok[])
{
	char clt;

	if(isLocal(fcn,tok))
	{
		clt = localvar[currlocal].type;
		if(localvar[currlocal].size == -1)
		{
			// local scalar
			fprintf(asmfile,"\tMOVE.W\t%d(A0),D7\n",
				localvar[currlocal].mode);
		}
		else
		{
			// local array
			if(isFunction(fcn))
			{
				fprintf(asmfile,"\tLEA\tF%d%s,A3\n",
					currfn,tok);
				if(aitype == 'N')
					fprintf(asmfile,"\tMOVE.W\t#%s,D3\n",aindex);
				else
				{
					ReadVar(fcn,aindex);
					fprintf(asmfile,"\tMOVE.W\tD7,D3\n");
				}
				if(clt == 'I')
					fprintf(asmfile,"\tADD.W\tD3,D3\n");
				fprintf(asmfile,"\tMOVE.");
				switch(clt)
				{
				case 'C': fprintf(asmfile,"B");
					break;
				case 'I': fprintf(asmfile,"W");
				}
				fprintf(asmfile,"\t(A3,D3),D7\n");
			}
			else
				fatal("unknown function");
		}
	}
	else
	if(isGlobal(tok))
	{
		clt = globals[currglobal].type;
		if(globals[currglobal].size == -1)
		{
			// global scalar
			fprintf(asmfile,"\tMOVE.");
			switch(globals[currglobal].type)
			{
			case 'C': fprintf(asmfile,"B");
				break;
			case 'I': fprintf(asmfile,"W");
			}
			fprintf(asmfile,"\t%s,D7\n",tok);
		}
		else
		{
			// global array
			fprintf(asmfile,"\tLEA\t%s,A3\n",tok);
			if(aitype == 'N')
				fprintf(asmfile,"\tMOVE.W\t#%s,D3\n",aindex);
			else
			{
				ReadVar(cfn,aindex);
				fprintf(asmfile,"\tMOVE.W\tD7,D3\n");
			}
			if(clt == 'I')
				fprintf(asmfile,"\tADD.W\tD3,D3\n");
			fprintf(asmfile,"\tMOVE.");
			switch(clt)
			{
			case 'C': fprintf(asmfile,"B");
				break;
			case 'I': fprintf(asmfile,"W");
			}
			fprintf(asmfile,"\t(A3,D3),D7\n");
		}
	}
	else
	if(isFparm(fcn,tok))
	{
		fprintf(asmfile,"\n\tMOVE.W\t%d(A0),D7\n",
			8 + 2*currparm);
	}
	else
		fatal("undefined variable 1");
}

void WriteVar(char fcn[], char tok[])
{
	char clt;

	if(isLocal(fcn,tok))
	{
		clt = localvar[currlocal].type;
		if(localvar[currlocal].size == -1)
		{
			// local scalar
			fprintf(asmfile,"\tMOVE.W\tD7,%d(A0)\n",
				localvar[currlocal].mode);
		}
		else
		{
			// local array
			if(isFunction(fcn))
			{
				fprintf(asmfile,"\tLEA\tF%d%s,A3\n",
					currfn,tok);
				if(aitype == 'N')
					fprintf(asmfile,"\tMOVE.W\t#%s,D3\n",aindex);
				else
				{
					fprintf(asmfile,"\tMOVE.W\tD7,-(A7)\n");
					ReadVar(cfn,aindex);
					fprintf(asmfile,"\tMOVE.W\tD7,D3\n");
					fprintf(asmfile,"\tMOVE.W\t(A7)+,D7\n");
				}
				if(clt == 'I')
					fprintf(asmfile,"\tADD.W\tD3,D3\n");
				fprintf(asmfile,"\tMOVE.");
				switch(clt)
				{
				case 'C': fprintf(asmfile,"B");
					break;
				case 'I': fprintf(asmfile,"W");
				}
				fprintf(asmfile,"\tD7,(A3,D3)\n");
			}
			else
			{
				fatal("WriteVar(): unknown function");
			}
		}
	}
	else
	if(isGlobal(tok))
	{
		clt = globals[currglobal].type;
		if(globals[currglobal].size == -1)
		{
			// global scalar
			fprintf(asmfile,"\tMOVE.");
			switch(globals[currglobal].type)
			{
			case 'C': fprintf(asmfile,"B");
				break;
			case 'I': fprintf(asmfile,"W");
			}
			fprintf(asmfile,"\tD7,%s\n",tok);
		}
		else
		{
			//global array
			fprintf(asmfile,"\tLEA\t%s,A3\n",tok);
			if(aitype == 'N')
				fprintf(asmfile,"\tMOVE.W\t#%s,D3\n",aindex);
			else
			{
				fprintf(asmfile,"\tMOVE.W\tD7,-(A7)\n");
				ReadVar(cfn,aindex);
				fprintf(asmfile,"\tMOVE.W\tD7,D3\n");
				fprintf(asmfile,"\tMOVE.W\t(A7)+,D7\n");
			}
			if(clt == 'I')
				fprintf(asmfile,"\tADD.W\tD3,D3\n");
			fprintf(asmfile,"\tMOVE.");
			switch(clt)
			{
			case 'C': fprintf(asmfile,"B");
				break;
			case 'I': fprintf(asmfile,"W");
			}
			fprintf(asmfile,"\tD7,(A3,D3)\n");
		}
	}
	else
		fatal("undefined variable 2");
}

