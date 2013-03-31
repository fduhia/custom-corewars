#define SYMSIZE 16
#define TRUE 1
#define FALSE 0
#define PSEUDO 1
#define CPUOP 2
#define BYTE 0
#define WORD 1
#define LONG 2
#define SHORT 3
#define UNSIZED 4
#define ORGOP 0
#define DCOP 1
#define DSOP 2
#define EQUOP 3
#define ENDOP 4
#define SORG 0
#define SWRITE 1
#define SCLOSE 2
#define tolst if(pass==2)fprintf
#define xCCR 0xf
#define xSR 0x8000
#define xSP 0xfffe
#define xUSP 0x0
#define xSSP 0x0
#define xPC 0

FILE *srcptr, *lstptr, *objptr, *hexptr;
char temp, srcfile[16], lstfile[16], objfile[16];
char hexfile[16];
int line, lineptr, pass, fatals, warnings, stopped, writeflag;
int gotlabel, comment, optype, opsize, whichop, opiden, oppatt;
unsigned char label[SYMSIZE], opcode[SYMSIZE], lbuff[8];
int rval, value, newsymptr, optype, lbuffptr, openlabel;
char linetxt[80], digit, lchar;
unsigned long int itemp, number, disp, result, pc, oldpc, startaddress;
unsigned long int immdata, EA;
int Dn, An, Dx, Dy, Ax, Ay, Rx, Ry, mode, reg;
int warnptr, fatalptr, warningbuff[8], fatalbuff[8];
char eatsymbol, eater1[8], eater2[8], sdstr[13], amstr[13];
unsigned char srecsum;
int s1pending, sreclen, srecptr;
unsigned int srecpc, extword;
unsigned char srecbuff[40];
int valundef, negflag, eflag, foundR;
unsigned int rlist,rlist2;

#define MAXSYM 1024

struct symtab
{
     char name[SYMSIZE];
     char type;
     unsigned long int value;
} symbols[MAXSYM];
int symptr;

void s0rec(void);
void s1rec(int oper, unsigned char data);
void writes1rec(void);
void s9rec(void);
void initpass(int passval);
void makeapass(void);
void parse(void);
void process_symbol(void);
void getsymbol(char sname[]);
int addsymbol(char sname[]);
void showsymbol(void);
int findsymbol(char sname[]);
void setsymbol(char sname[], unsigned long int value, char def);
void skipblank(void);
void process_op(void);
void getopcode(void);
void do_pseudo(void);
void do_68kop(void);
void getop(char opcode[]);
void getsize(void);
int typer(unsigned long int number);
void gethex(void);
void getdeci(void);
int isreg(void);
long int eval();
void eat(char symbol);
int ischar(char lchr);
int mathop(char msym);
void getvalue();
void putter(int size, unsigned long int value);
void writelist(void);
void warnerror(int errnum);
void showwarnings(void);
void fatalerror(int errnum);
void showfatals(void);
void getDn(void);
void getAn(void);
void getImmediate(void);
void getamode(void);
void finishamode(void);
unsigned int sizebits(void);
void notamode(int invmode, char sdflag);
void getsuperamode(void);
void getXn(void);
void initsymtab();
void reglist(int start);

void ___abcdSbcd(void);
void ___addAndOrSub(void);
void ___addaCmpaSuba(void);
void ___addiCmpiSubi(void);
void ___addqSubq(void);
void ___addxSubx(void);
void ___andiEoriOri(void);
void ___asdLsdRodRoxd(void);
void ___bitops(void);
void ___branches(void);
void ___NopResetRteRtrRtsTrapv(void);
void ___clrNegNegxNotTst(void);
void ___cmp(void);
void ___cmpm(void);
void ___dbcc(void);
void ___eor(void);
void ___exg(void);
void ___ext(void);
void ___jumps(void);
void ___lea(void);
void ___link(void);
void ___move(void);
void ___movea(void);
void ___movem(void);
void ___movep(void);
void ___moveq(void);
void ___muluMulsDivuDivsChk(void);
void ___pea(void);
void ___Rtd(void);
void ___scc(void);
void ___stop(void);
void ___swap(void);
void ___tasNbcd(void);
void ___trap(void);
void ___unlk(void);

     struct ucode
     {
       int mask, pattern, type, iden;
       unsigned char opcode[SYMSIZE];
     } optab[] = {
     {0,0,ORGOP,1,"ORG"},
     {0,0,DCOP,1,"DC"},
     {0,0,DSOP,1,"DS"},
     {0,0,EQUOP,1,"EQU"},
     {0,0,ENDOP,1,"END"},
     {0xf1f0, 0xc100, 1,2, "ABCD"},
     {0xf1c0, 0xd000, 201,2, "ADD"},
     {0xf1c0, 0xd0c0, 22,2, "ADDA"},
     {0xff00, 0x0600, 600,2, "ADDI"},
     {0xf100, 0x5000, 11,2, "ADDQ"},
     {0xf130, 0xd100, 19,2, "ADDX"},
     {0xf1c0, 0xc000, 201,2, "AND"},
     {0xffff, 0x0200, 230,2, "ANDI"},
     {0xfec0, 0xe100, 17,2, "ASL"},
     {0xfec0, 0xe000, 17,2, "ASR"},

     {0xf000, 0x6000, 12,2, "BRA"},
     {0xf000, 0x6100, 12,2, "BSR"},
     {0xf000, 0x6200, 12,2, "BHI"},
     {0xf000, 0x6300, 12,2, "BLS"},
     {0xf000, 0x6400, 12,2, "BCC"},
     {0xf000, 0x6500, 12,2, "BCS"},
     {0xf000, 0x6600, 12,2, "BNE"},
     {0xf000, 0x6700, 12,2, "BEQ"},
     {0xf000, 0x6800, 12,2, "BVC"},
     {0xf000, 0x6900, 12,2, "BVS"},
     {0xf000, 0x6a00, 12,2, "BPL"},
     {0xf000, 0x6b00, 12,2, "BMI"},
     {0xf000, 0x6c00, 12,2, "BGE"},
     {0xf000, 0x6d00, 12,2, "BLT"},
     {0xf000, 0x6e00, 12,2, "BGT"},
     {0xf000, 0x6f00, 12,2, "BLE"},

     {0xf1c0, 0x0140, 181,2, "BCHG"},
     {0xf1c0, 0x0180, 182,2, "BCLR"},
     {0xf1c0, 0x01c0, 183,2, "BSET"},
     {0xf1c0, 0x0100, 184,2, "BTST"},
     {0xf1c0, 0x4180, 10,2, "CHK"},
       {0xff00, 0x4200, 21,2, "CLR"},
     {0xf1c0, 0xb000, 202,2, "CMP"},
      {0xf1c0, 0xb0c0, 22,2, "CMPA"},
     {0xff00, 0x0c00, 601,2, "CMPI"},
     {0xf138, 0xb108, 26,2, "CMPM"},

     {0,      0x50c8, 5,2, "DBT"},
     {0xf000, 0x51c8, 5,2, "DBRA"},
     {0xf000, 0x51c8, 5,2, "DBF"},
     {0xf000, 0x52c8, 5,2, "DBHI"},
     {0xf000, 0x53c8, 5,2, "DBLS"},
     {0xf000, 0x54c8, 5,2, "DBCC"},
     {0xf000, 0x55c8, 5,2, "DBCS"},
     {0xf000, 0x56c8, 5,2, "DBNE"},
     {0xf000, 0x57c8, 5,2, "DBEQ"},
     {0xf000, 0x58c8, 5,2, "DBVC"},
     {0xf000, 0x59c8, 5,2, "DBVS"},
     {0xf000, 0x5ac8, 5,2, "DBPL"},
     {0xf000, 0x5bc8, 5,2, "DBMI"},
     {0xf000, 0x5cc8, 5,2, "DBGE"},
     {0xf000, 0x5dc8, 5,2, "DBLT"},
     {0xf000, 0x5ec8, 5,2, "DBGT"},
     {0xf000, 0x5fc8, 5,2, "DBLE"},

     {0xf1c0, 0x81c0, 10,2, "DIVS"},
     {0xf1c0, 0x80c0, 10,2, "DIVU"},
     {0xf1c0, 0xb100, 203,2, "EOR"},
     {0xffff, 0x0a00, 231,2, "EORI"},
     {0xf1f8, 0xc100, 27,2, "EXG"},
     {0xfff8, 0x4880, 28,2, "EXT"},
       {0xffff, 0x4afc, 0, 2, "ILLEGAL"},
     {0xffc0, 0x4ec0, 15,2, "JMP"},
     {0xffc0, 0x4e80, 15,2, "JSR"},
     {0xf1c0, 0x41c0, 29,2, "LEA"},
     {0xfff8, 0x4e50, 16,2, "LINK"},
     {0xfec0, 0xe308, 17,2, "LSL"},
     {0xfec0, 0xe208, 17,2, "LSR"},
       {0xfff0, 0x4e60, 31,2, "MOVE"},
     {0xc1c0, 0x0040, 3,2, "MOVEA"},
     {0xfb80, 0x4880, 35,2, "MOVEM"},
     {0xf1f8, 0x0108, 25,2, "MOVEP"},
     {0xf100, 0x7000, 30,2, "MOVEQ"},
     {0xf1c0, 0xc1c0, 10,2, "MULS"},
     {0xf1c0, 0xc0c0, 10,2, "MULU"},
     {0xffc0, 0x4800, 13,2, "NBCD"},
       {0xff00, 0x4400, 21,2, "NEG"},
       {0xff00, 0x4000, 21,2, "NEGX"},
       {0xffff, 0x4e71, 0, 2, "NOP"},
       {0xff00, 0x4600, 21,2, "NOT"},
     {0xf1c0, 0x8000, 201,2, "OR"},
     {0xffff, 0x0000, 232,2, "ORI"},
     {0xffc0, 0x4840, 14,2, "PEA"},
       {0xffff, 0x4e70, 0, 2, "RESET"},
     {0xfec0, 0xe718, 17,2, "ROL"},
     {0xfec0, 0xe618, 17,2, "ROR"},
     {0xfec0, 0xe510, 17,2, "ROXL"},
     {0xfec0, 0xe410, 17,2, "ROXR"},
     {0xffff, 0x4e74, 99, 2, "RTD"},
       {0xffff, 0x4e73, 0, 2, "RTE"},
       {0xffff, 0x4e77, 0, 2, "RTR"},
       {0xffff, 0x4e75, 0, 2, "RTS"},
     {0xf1f0, 0x8100, 1,2, "SBCD"},

     {0,      0x50c0, 34,2, "ST"},
     {0,      0x51c0, 34,2, "SF"},
     {0xf000, 0x52c0, 34,2, "SHI"},
     {0xf000, 0x53c0, 34,2, "SLS"},
     {0xf000, 0x54c0, 34,2, "SCC"},
     {0xf000, 0x55c0, 34,2, "SCS"},
     {0xf000, 0x56c0, 34,2, "SNE"},
     {0xf000, 0x57c0, 34,2, "SEQ"},
     {0xf000, 0x58c0, 34,2, "SVC"},
     {0xf000, 0x59c0, 34,2, "SVS"},
     {0xf000, 0x5ac0, 34,2, "SPL"},
     {0xf000, 0x5bc0, 34,2, "SMI"},
     {0xf000, 0x5cc0, 34,2, "SGE"},
     {0xf000, 0x5dc0, 34,2, "SLT"},
     {0xf000, 0x5ec0, 34,2, "SGT"},
     {0xf000, 0x5fc0, 34,2, "SLE"},

     {0xffff, 0x4e72, 9,2, "STOP"},
     {0xf1c0, 0x9000, 201,2, "SUB"},
     {0xf1c0, 0x90c0, 22,2, "SUBA"},
     {0xff00, 0x0400, 602,2, "SUBI"},
     {0xf100, 0x5100, 11,2, "SUBQ"},
     {0xf130, 0x9100, 19,2, "SUBX"},
       {0xfff8, 0x4840, 4, 2, "SWAP"},
     {0xffc0, 0x4ac0, 13,2, "TAS"},
     {0xfff0, 0x4e40, 8, 2, "TRAP"},
       {0xffff, 0x4e76, 0, 2, "TRAPV"},
       {0xff00, 0x4a00, 21,2, "TST"},
     {0xfff8, 0x4e58, 7,2, "UNLK"},
    };

struct ucode inst;

