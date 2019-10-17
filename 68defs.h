#define EX 1
#define NOEX 0
#define BYTE 0
#define WORD 1
#define LONG 2
#define TRUE 1
#define FALSE 0
#define RD 0
#define WR 1
#define POST 0
#define PRE 1
#define NONE 2
#define pushpc temp_pc=pc;
#define poppc pc=temp_pc;

void bkpt(void);
void fill(void);
void hex_math(void);
int inrange(unsigned long int addr);
void skipblank(void);
void show_help(void);
void disp_reg(void);
void disp_reg_list(char reg_type);
void disp_mem(void);
void enter(void);
void go(void);
void init_data(void);
void load_hexfile(void);
void pushAn(void);
void popAn(void);
void addr_mode(int opword);
int op_search(int iword);
void read_src(int opword);
void write_dst(int opword);
void loadsrc(void);
void writemem(int size, unsigned long int addr);
void adrind(int rwflag, int opsize, int An, int incflag);
int btst(int bitnum, unsigned char pattern);
void rlist(char reg, unsigned char pattern);
unsigned char bitswap(unsigned char pattern);
void fixN(int opsize, unsigned long int result);
void fixZ(int opsize, unsigned long int result);
void savedreg(int reg, unsigned long int regvalue);
void readreg(unsigned long int regvalue);
int bitval(int bitnum, unsigned long int number);
void sflag(unsigned long int value);
void dflag(unsigned long int value);
void rflag(unsigned long int result);
unsigned char getbyte(void);
void condi(int cc);
void type0(int pattern);
void type111(void);
void type112(void);
void type2(void);
void type3(void);
void type4(int Dn);
void type5(int cc);
void type611(void);
void type612(int pattern);
void type7(void);
void type8(int trapnum);
void type9(void);
void type100(int pattern);
void type101(void);
void type11(void);
void type12(int cc);
void type13(int pattern);
void type14(void);
void type15(int pattern);
void type16(void);
void type170(void);
void type171(void);
void type18(int pattern);
void type19(void);
void type200(void);
void type201(int pattern);
void type202(int pattern);
void type21(int pattern);
void type220(void);
void type221(int pattern);
void type23(int pattern);
void type24(int pattern);
void type25(int pattern);
void type26(void);
void type27(int pattern);
void type28(int Dn);
void type29(void);
void type30(int Dn);
void type34(int cc);
void type35(void);
void type99(void);
void op_size(int sval, int inst);
void swaprm(int inst);
void fetch(int ex);
void trace();
void address_error(unsigned long int addr);
int readmem(int size, unsigned long int addr);
unsigned long int tohex(char hstr[]);
int daa(unsigned char *num1, unsigned char num2);
unsigned char tobcd(unsigned char num);
unsigned char frombcd(unsigned char num);
void modify_reg(void);
void unassemble(void);
void int21h(void);

     unsigned long int d_reg[] = {0,0,0,0,0,0,0,0};

     unsigned long int a_reg[] = {0,0,0,0,0,0,0,0xff0};

	unsigned long int tempAn[8];

     //unsigned char mem[8192];
     unsigned char opsize,chksum;
     long int ssp, usp, pc, unaspc, temp_pc, bpadr[4], tbp1, tbp2;
     int sr, inst,first,bpstat[4];
     unsigned char reg_num;
     unsigned char Xflag, Nflag, Zflag, Vflag, Cflag;
	unsigned char membyte, srcbyte;
	unsigned int memword, srcword;
	unsigned long int memlong, srclong, srcaddr, srcdata;
	unsigned long int temp, temp2, result;
	int stopem, ingo;
	int Sm, Dm, Rm, COND;
	FILE *fptr;
	char fname[20], fchr, fbyte[2];
	char cmd[32];
	int cptr;
	unsigned long int dump_ptr;
	int exflag;
	unsigned char *pbuf;

#define XF (0!=(sr&0x10))
#define NF (0!=(sr&0x08))
#define ZF (0!=(sr&0x04))
#define VF (0!=(sr&0x02))
#define CF (0!=(sr&0x01))

//#define loadX(exp) if(exp){sr|=0x10;}else{sr&=0xffef;}
//#define loadN(exp) if(exp){sr|=0x08;}else{sr&=0xfff7;}
//#define loadZ(exp) if(!exp){sr|=0x04;}else{sr&=0xfffb;}
//#define loadV(exp) if(exp){sr|=0x02;}else{sr&=0xfffd;}
//#define loadC(exp) if(exp){sr|=0x01;}else{sr&=0xfffe;}

#define CC !CF
#define CS CF
#define EQ ZF
#define GE (NF&&VF)||(!NF&&!VF)
#define GT GE&&!ZF
#define HI !CF&&!ZF
#define LT (NF&&!VF)||(!NF&&VF)
#define LS CF||ZF
#define LE ZF||LT
#define MI NF
#define NE !ZF
#define PL !NF
#define VC !VF
#define VS VF

     struct ucode
     {
	  int mask, pattern, type;
	  unsigned char opcode[10];
     } optab[] = {
	  {0xffff, 0x4afc, 0,  "ILLEGAL"},
	  {0xffff, 0x4e71, 0,  "NOP"},
	  {0xffff, 0x4e70, 0,  "RESET"},
          {0xffff, 0x4e74, 99, "RTD"},
	  {0xffff, 0x4e73, 0,  "RTE"},
	  {0xffff, 0x4e77, 0,  "RTR"},
	  {0xffff, 0x4e75, 0,  "RTS"},
	  {0xffff, 0x4e76, 0,  "TRAPV"},
	{0xffff, 0x4e72, 9, "STOP"},
	{0xffff, 0x023c, 23, "ANDI.B"},
	{0xffff, 0x0a3c, 23, "EORI.B"},
	{0xffff, 0x003c, 23, "ORI.B"},
	{0xffff, 0x027c, 24, "ANDI.W"},
	{0xffff, 0x0a7c, 24, "EORI.W"},
	{0xffff, 0x007c, 24, "ORI.W"},
	{0xfff8, 0x4880, 28, "EXT.W"},
	{0xfff8, 0x48c0, 28, "EXT.L"},
	  {0xfff0, 0x4e60, 31, "MOVE.L"},
	  {0xffc0, 0x42c0, 32, "MOVE.W"},
	  {0xffc0, 0x44c0, 32, "MOVE.W"},
	  {0xffc0, 0x40c0, 33, "MOVE.W"},
	  {0xffc0, 0x46c0, 33, "MOVE.W"},
	{0xfb80, 0x4880, 35, "MOVEM"},
	{0xf1f0, 0xc100, 111, "ABCD.B"},
	{0xf1f0, 0x8100, 112, "SBCD.B"},
	{0xf1c0, 0x3040, 3, "MOVEA"},
	{0xf1c0, 0x2040, 3, "MOVEA"},
	  {0xfff8, 0x4840, 4,  "SWAP.W"},
	  {0xf0f8, 0x50c8, 5, "DB"},
	  {0xf0c0, 0x50c0, 34, "S"},
	{0xffc0, 0x0600, 611, "ADDI"},
	{0xffc0, 0x0640, 611, "ADDI"},
	{0xffc0, 0x0680, 611, "ADDI"},
	{0xffc0, 0x0200, 612, "ANDI"},
	{0xffc0, 0x0240, 612, "ANDI"},
	{0xffc0, 0x0280, 612, "ANDI"},
	{0xffc0, 0x0c00, 611, "CMPI"},
	{0xffc0, 0x0c40, 611, "CMPI"},
	{0xffc0, 0x0c80, 611, "CMPI"},
	{0xffc0, 0x0a00, 612, "EORI"},
	{0xffc0, 0x0a40, 612, "EORI"},
	{0xffc0, 0x0a80, 612, "EORI"},
	{0xffc0, 0x0000, 612, "ORI"},
	{0xffc0, 0x0040, 612, "ORI"},
	{0xffc0, 0x0080, 612, "ORI"},
	{0xffc0, 0x0400, 611, "SUBI"},
	{0xffc0, 0x0440, 611, "SUBI"},
	{0xffc0, 0x0480, 611, "SUBI"},
	{0xffc0, 0x4800, 13, "NBCD.B"},
	{0xffc0, 0x4ac0, 13, "TAS.B"},
	  {0xff00, 0x4200, 21, "CLR"},
	  {0xff00, 0x4400, 21, "NEG"},
	  {0xff00, 0x4000, 21, "NEGX"},
	  {0xff00, 0x4600, 21, "NOT"},
	  {0xff00, 0x4a00, 21, "TST"},
	{0xfff8, 0x4e58, 7, "UNLK"},
	{0xfff0, 0x4e40, 8, "TRAP"},
	{0xf1f8, 0x0108, 25, "MOVEP.W"},
	{0xf1f8, 0x0148, 25, "MOVEP.L"},
	{0xf1f8, 0x0188, 25, "MOVEP.W"},
	{0xf1f8, 0x01c8, 25, "MOVEP.L"},
	{0xf1c0, 0x81c0, 100, "DIVS.W"},
	{0xf1c0, 0x80c0, 100, "DIVU.W"},
	{0xf1c0, 0xc1c0, 100, "MULS.W"},
	{0xf1c0, 0xc0c0, 100, "MULU.W"},
	{0xf1c0, 0x4180, 101, "CHK.W"},
	{0xf1c0, 0x41c0, 29, "LEA.L"},
     {0xf100, 0x5000, 11, "ADDQ"},
     {0xf100, 0x5100, 11, "SUBQ"},
	{0xf100, 0x7000, 30, "MOVEQ.L"},
	{0xf000, 0x6000, 12, "B"},
	{0xffc0, 0x4840, 14, "PEA.L"},
	{0xffc0, 0x4e80, 15, "JSR"},
	{0xffc0, 0x4ec0, 15, "JMP"},
	{0xfff8, 0x4e50, 16, "LINK"},
	{0xfec0, 0xe0c0, 170, "AS"},
	{0xfec0, 0xe2c0, 170, "LS"},
	{0xfec0, 0xe6c0, 171, "RO"},
	{0xfec0, 0xe4c0, 171, "ROX"},
	{0xf1f8, 0xc140, 27, "EXG.L"},
	{0xf1f8, 0xc148, 27, "EXG.L"},
	{0xf1f8, 0xc188, 27, "EXG.L"},
	{0xf018, 0xe000, 170, "AS"},
	{0xf018, 0xe008, 170, "LS"},
	{0xf018, 0xe018, 171, "RO"},
	{0xf018, 0xe010, 171, "ROX"},
	{0xf1c0, 0x0140, 18, "BCHG"},
	{0xf1c0, 0x0180, 18, "BCLR"},
	{0xf1c0, 0x01c0, 18, "BSET"},
	{0xf1c0, 0x0100, 18, "BTST"},
	{0xffc0, 0x0840, 18, "BCHG"},
	{0xffc0, 0x0880, 18, "BCLR"},
	{0xffc0, 0x08c0, 18, "BSET"},
	{0xffc0, 0x0800, 18, "BTST"},
	{0xf1f8, 0xb108, 26, "CMPM"},
	{0xf1f8, 0xb148, 26, "CMPM"},
	{0xf1f8, 0xb188, 26, "CMPM"},
	{0xf1f0, 0xd100, 19, "ADDX"},
	{0xf1f0, 0xd140, 19, "ADDX"},
	{0xf1f0, 0xd180, 19, "ADDX"},
	{0xf1f0, 0x9100, 19, "SUBX"},
	{0xf1f0, 0x9140, 19, "SUBX"},
	{0xf1f0, 0x9180, 19, "SUBX"},
	{0xf1c0, 0xd000, 201, "ADD"},
	{0xf1c0, 0xd040, 201, "ADD"},
	{0xf1c0, 0xd080, 201, "ADD"},
	{0xf1c0, 0xd100, 201, "ADD"},
	{0xf1c0, 0xd140, 201, "ADD"},
	{0xf1c0, 0xd180, 201, "ADD"},
	{0xf1c0, 0x9000, 201, "SUB"},
	{0xf1c0, 0x9040, 201, "SUB"},
	{0xf1c0, 0x9080, 201, "SUB"},
	{0xf1c0, 0x9100, 201, "SUB"},
	{0xf1c0, 0x9140, 201, "SUB"},
	{0xf1c0, 0x9180, 201, "SUB"},
	{0xf1c0, 0xb000, 200, "CMP"},
	{0xf1c0, 0xb040, 200, "CMP"},
	{0xf1c0, 0xb080, 200, "CMP"},
	{0xf1c0, 0xc000, 202, "AND"},
	{0xf1c0, 0xc040, 202, "AND"},
	{0xf1c0, 0xc080, 202, "AND"},
	{0xf1c0, 0xc100, 202, "AND"},
	{0xf1c0, 0xc140, 202, "AND"},
	{0xf1c0, 0xc180, 202, "AND"},
	{0xf1c0, 0x8000, 202, "OR"},
	{0xf1c0, 0x8040, 202, "OR"},
	{0xf1c0, 0x8080, 202, "OR"},
	{0xf1c0, 0x8100, 202, "OR"},
	{0xf1c0, 0x8140, 202, "OR"},
	{0xf1c0, 0x8180, 202, "OR"},
	{0xf1c0, 0xb100, 202, "EOR"},
	{0xf1c0, 0xb140, 202, "EOR"},
	{0xf1c0, 0xb180, 202, "EOR"},
	{0xf1c0, 0xd0c0, 221, "ADDA.W"},
	{0xf1c0, 0xd1c0, 221, "ADDA.L"},
	{0xf1c0, 0x90c0, 221, "SUBA.W"},
	{0xf1c0, 0x91c0, 221, "SUBA.L"},
	{0xf1c0, 0xb0c0, 220, "CMPA.W"},
	{0xf1c0, 0xb1c0, 220, "CMPA.L"},
	{0xc000, 0,      2,  "MOVE"},
	{0x0000, 0x0000, 0,  "  done  "}
	};
     /*
     word 0f1f8h
     byte 4,27
     word 0e000h,t_as
     */
