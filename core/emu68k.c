// V4.1 8.12.03  Added temporary breakpoints, changed TRAP #1 and TRAP #3
//               to output ASCII codes without restriction.
//
// V4.0 2.15.03  Cleanup. No functional changes.
//
// V3.2 11.28.98 Fixed flag updates in ANDI, SUBI, CMPI, ADD, SUBQ
//               ADD, SUB

// V3.11 3.3.98  Changed Dump command to start at hexfile pc.

// V3.1 3.1.98   Updated the help menu.
//               Changed U command to unassemble at initial load PC.
//               Fixed S0 record display when loading hexfile.
//               Added help message at startup.
//               Changed the way unassemble displays everything.

// V3.02 2.27.98 Added , to the U and T commands.
//                      along with a ^U bailout.

// V3.01 2.14.98 Modified the U command to retain last address
//               Fixed flag behavior in ADD (type 201)
//               Added date to signon message
//               Modified the E command to start at $400
//                      and quit when any illegal key is hit
//               Modified D command to allow number of lines to disp

// V3.00 8.17.97 Added DOS interrupt capability via TRAP #10
//               Added author identification command
//
//
// V2.10 2.4.97 Modified RAM to 64 KB by adding malloc'd block.
//              Fixed pc change in unas().
//              Changed breakpoint operation (disabled for
//                   tracing)
//              Fixed flag behavior in CMPM.
//              Fixed dis-asm of CMPM during go.

// V2.9 11.4.96 Fixed MOVEQ instruction (always displays D0)
//              Fixed unassemble bug in MOVE inst.

// V2.8 10.1.96 Added unassemble command

// V2.7 9.30.96 Fixed ABCD (via rewritten daa() function)

// V2.6 9.18.96 Added HEX command
//              Updated ENTER command to allow data skips
//              Added ASCII dump to DUMP command

// V2.5 9.17.96 Fixed MOVEM storage order when -(An) used.
//              Added block-fill command.
//              Added multiple breakpoints to bkpt command.

/* V2.4 9.11.96 Fixed -offset problem with LINK */
/*              Added starting address display */

/* V2.3 9.9.96 Added breakpoint feature */

/* V2.2 8.26.96 Fixed problem with zero-length S records. */

/* 68000 Simulator */
/* V2.1 First Version */
/* c1995 James L. Antonakos */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
//#include <malloc.h> replaced with stdlib.h
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include "68defs.h"
#include "srecord.h"

void loadX(int exp);
void loadN(int exp);
void loadZ(int exp);
void loadV(int exp);
void loadC(int exp);

     struct ucode icode;
     char isize, ops[32], tops[32];
     int mw;

	 static int SRecord_Address_Lengths[] = {
	4, // S0
	4, // S1
	6, // S2
	8, // S3
	8, // S4
	4, // S5
	6, // S6
	8, // S7
	6, // S8
	4, // S9
};

int main(int argc, char *argv[])
{
     char com;
	 char close;
	 FILE *fp;
	 char fname2[20];
	 fname2[0] = '\0';
	 const char * fileread;
	 SRecord srec;
	 uint32_t staddress[100];

     printf("Custom Corewars V1.0, 26/4/2013\n");
     init_data();

/*	 
     if (argc > 1)
     {
	cptr = 0;
	cmd[0] = '\0';
	strcat(cmd,argv[1]);
	load_hexfile();
     }
*/	 
	 
     printf("Enter '?' for help\n");
//     printf("Use 0x%04X for all DOS segment addresses.\n",FP_SEG(pbuf));

	printf("Enter warrior name: \n");
//	fname2 = getchar();
	scanf ("%s",fname2);
	printf("Warrior name is: \n",fname2);


			fp = fopen(fname2,"r");

					while (Read_SRecord(&srec, fp) == SRECORD_OK) 
			 {
				//recordCount++;
                //dataByteSum += srec.dataLen;
				Print_SRecord(&srec);
				
				printf("\n");
				
				for (unsigned long int mData = 0; mData < srec.dataLen; mData++)
				{
					unsigned long int effA, myaddr;
					myaddr = srec.address + mData;

					effA = myaddr & 0xffff;
							if (mData+1 < srec.dataLen)
//			printf("0x%02X, ", srec->data[i]);
					(pbuf)[effA] = srec.data[mData];
					printf("starting pc is: %x",srec.address);
				}
//				(pbuf)[ea] = srcbyte;
//				(pbuf)[srec.address] = srcbyte;
				
//				for (int mData = 0; mData < srec.dataLen; mData++) 
//			{
//				if (mData+1 < srec.dataLen)
//				(pbuf)[srec.address] = srec.data[mData];
//				printf("0x%02X, ", srec.data[mData]);
//				else
//				printf("0x%02X", srec.data[mData]);
			}
			
			
//			}
				
			

/*			
				for (int mData = 0; mData < srec.dataLen; mData++) 
			{
				if (mData+1 < srec.dataLen)
				printf("0x%02X, ", srec.data[mData]);
				else
				printf("0x%02X", srec.data[mData]);
			}
*/
			

		
		
		do
		{
		
			gets(cmd);
			cptr = 0;
			skipblank();
			com = toupper(cmd[cptr]);
			cptr++;
			skipblank();
		
		switch(com)
	  {
	       case 'A' : printf("Based on Motorola 68000 emulator by James L. Antonakos CC\n");
		printf("email: antonakos_j@sunybroome.edu for questions/comments\n");
		printf("http://www.sunybroome.edu/~antonakos_j for new version downloads\n");
			break;
	       case 'B' : bkpt(); break;
	       case 'D' : disp_mem(); break;
	       case 'E' : enter(); break;
	       case 'F' : fill(); break;
	       case 'G' : go(); break;
	       case 'H' : hex_math(); break;
	       case '?' : show_help(); break;
	       case 'L' : load_hexfile(); break;
	       case 'Q' : break;
	       case 'R' : modify_reg(); break;
	       case 'T' : trace(); break;
	       case 'U' : unassemble(); break;
	       case '\n': break;
	       case '\0': break;
	       default  : printf("Unknown command %s\n",cmd);
	  }
	  
		}while (com != 'Q');
//		getchar();
//		scanf ("%s",close);
//		int leave = 5;
//	if(leave != NULL)
		return 0;
}

/*
void int21h()
{
	union REGS inregs, outregs;
	struct SREGS segregs;

	//call DOS INT
	inregs.x.ax = d_reg[0] & 0xffff;
	inregs.x.bx = d_reg[1] & 0xffff;
	inregs.x.cx = d_reg[2] & 0xffff;
	inregs.x.dx = d_reg[3] & 0xffff;
	inregs.x.si = d_reg[4] & 0xffff;
	inregs.x.di = d_reg[5] & 0xffff;

//      segregs.cs = a_reg[0] & 0xffff;
//      segregs.ds = a_reg[1] & 0xffff;
//      segregs.es = a_reg[2] & 0xffff;
//      segregs.ss = a_reg[3] & 0xffff;
//      intdos( &inregs, &outregs );

	int86(d_reg[7] & 0xff, &inregs, &outregs);

//      inregs.h.ah = 0x9;
//      inregs.x.dx = FP_OFF( buffer );
//      segregs.ds = FP_SEG( pbuf );
//      intdosx( &inregs, &outregs, &segregs );
//      segread(&segregs);

	d_reg[0] = (d_reg[0] & 0xffff0000) | outregs.x.ax;
	d_reg[1] = (d_reg[1] & 0xffff0000) | outregs.x.bx;
	d_reg[2] = (d_reg[2] & 0xffff0000) | outregs.x.cx;
	d_reg[3] = (d_reg[3] & 0xffff0000) | outregs.x.dx;
	d_reg[4] = (d_reg[4] & 0xffff0000) | outregs.x.si;
	d_reg[5] = (d_reg[5] & 0xffff0000) | outregs.x.di;

//      a_reg[0] = (a_reg[0] & 0xffff0000) | segregs.cs;
//      a_reg[1] = (a_reg[1] & 0xffff0000) | segregs.ds;
//      a_reg[2] = (a_reg[2] & 0xffff0000) | segregs.es;
//      a_reg[3] = (a_reg[3] & 0xffff0000) | segregs.ss;
}
*/


void show_help()
{
     printf("Author       A\n");
     printf("Breakpoint   B number (1-4) address (0 to disable)\n");
     printf("Dump         D [address] [lines]\n");
     printf("Fill         F start-address stop-address pattern-byte\n");
     printf("Enter        E [address] (use <cr> to skip over a byte\n");
     printf("                    and any illegal key to exit)\n");
     printf("Go           G [address] , [break address 1] , [break address 2]\n");
     printf("               Ex: G 8200 (begin execution at $8200)\n");
     printf("               Ex: G 8200, 8212 (begin execution at $8200 with\n");
     printf("                   temporary breakpoint at $8212)\n");
     printf("               Ex: G 8200, 8212, 8348  (begin execution at $8200 with\n");
     printf("                   temporary breakpoints at $8212 and $8348)\n");
     printf("               Ex: G , 8212, 8348 (begin execution at current PC with)\n");
     printf("                   temporary breakpoints at $8212 and $8348)\n");

     printf("Help         ?\n");
     printf("Hex          H number1 number2\n");
     printf("Load         L filename\n");
     printf("Quit         Q\n");
     printf("Register     R [register]\n");
     printf("Trace        T [address] , [lines]\n");
     printf("               Ex: T 400   (begin trace at $400)\n");
     printf("               Ex: T 400,5 (trace 5 inst. starting at $400)\n");
     printf("               Ex: T ,5    (trace next 5 inst. at current pc)\n");
     printf("Unassemble   U [address] , [lines]\n");
     printf("               Ex: Same syntax as Trace command\n");
}

void skipblank()
{
	while (' ' == cmd[cptr])
		cptr++;
}

unsigned long int tohex(char hstr[])
{
	unsigned long int number;
	char digit;
	int k;

	number = 0;
	k = 0;
	while (('\0' != hstr[k])
		&& (' ' != hstr[k])     //9.17.96
		&& (',' != hstr[k]))    //2.27.98
	{
		digit = toupper(hstr[k]);
		if (isalpha(digit) != 0)
			digit -= 7;
		number <<= 4;
		number |= (digit & 0xf);
		k++;
		cptr++;  //9.17.96
	}
	return number;
}

int inrange(unsigned long int addr)
{
//        if (((addr >= 0x1000) && (addr < 0x8000)) || (addr >= 0x9000))
  //      {
    //            printf("No memory supported at address %06lX.\n",dump_ptr);
      //          return FALSE;
	//}
//        else
		return TRUE;
}

void hex_math()
{
	unsigned long int h1,h2;

	h1 = h2 = 0;
	h1 = tohex(&cmd[cptr]);
	skipblank();
	h2 = tohex(&cmd[cptr]);
	printf("Sum %08lX, Diff %08lX\n",h1+h2,h1-h2);
}

void disp_mem()
{
     int k,endk,j;

     if ('\0' != cmd[cptr])
	dump_ptr = tohex(&cmd[cptr]) & 0xfffff0;
     skipblank();
     endk = 16;
     if ('\0' != cmd[cptr])
	endk = tohex(&cmd[cptr]) & 0xffff;
     endk <<= 3;
     k = 0;
     printf("%06lX  ",dump_ptr);
     while ((k < endk) && inrange(dump_ptr))
     {
	  readmem(WORD,dump_ptr);
	  printf("%04X ",memword);
	  dump_ptr += 2;
	  k++;
	  if (0 == dump_ptr % 16)
	  {
		printf("   ");
		dump_ptr -= 16;
		for(j = 0; j < 16; j++)
		{
			readmem(BYTE,dump_ptr++);
			if ((membyte >= 32) && (membyte <= 127))
				printf("%c",membyte);
			else
				printf(".");
		}
		if (k != endk)
			printf("\n%06lX  ",dump_ptr);
	  }
     }
     printf("\n");
}

void disp_reg()
{
     disp_reg_list('D');
     disp_reg_list('A');
     printf("PC:   %06lX  SR: %04X\n",pc,sr);
     printf("Flags: X = %1d, N = %1d, Z = %1d, V = %1d, C = %1d\n",
	XF,NF,ZF,VF,CF);
}

void disp_reg_list(char reg_type)
{
     int reg;

     for(reg = 0; reg <= 7; reg++)
     {
	  printf("%c%1d: ",reg_type,reg);
	  (reg_type == 'D') ? printf("%08lX",d_reg[reg])
		      : printf("%08lX",a_reg[reg]);
	  printf("  ");
	  if(reg == 3)
	       printf("\n");
     }
     printf("\n");
}

void modify_reg(void)
{
	unsigned char reg,num, huh;
	char val[12];

	if ('\0' == cmd[cptr])
		disp_reg();
	else
	{
		huh = FALSE;
		reg = toupper(cmd[cptr]);
		num = (cmd[cptr+1] - 0x30) & 0xff;
		if ((reg != 'A') && (reg != 'D') && (reg != 'P')) huh = TRUE;
		if (reg == 'P')
		{
			num = toupper(cmd[cptr+1]);
			if (num != 'C') huh = TRUE;
			else num = 0;
		}
		if (num > 7) huh = TRUE;
		if (huh)
			printf("Huh?\n");
		else
		{
			switch(reg)
			{
				case 'A': printf("A%d: %08lX ? ",
					num,a_reg[num]); break;
				case 'D': printf("D%d: %08lX ? ",
					num,d_reg[num]); break;
				case 'P': printf("PC: %08lX ? ",
					pc); break;
			}
			gets(val);
			if (0 != strlen(val))
			{
				switch(reg)
				{
					case 'A': a_reg[num] = tohex(val); break;
					case 'D': d_reg[num] = tohex(val); break;
					case 'P': pc = tohex(val); break;
				}
			}
		}
	}
}

void fill()
{
	unsigned long int startadd, endadd;

	startadd = 0;
	endadd = 0;
	startadd = tohex(&cmd[cptr]) & 0xffffff;
	skipblank();
	endadd = tohex(&cmd[cptr]) & 0xffffff;
	skipblank();
	srcbyte = tohex(&cmd[cptr]) & 0xff;
	while (startadd <= endadd)
		writemem(BYTE,startadd++);
}

void enter()
{
	unsigned char val;
	char hexin[8];
	unsigned long int enter_ptr;
	int illeg;

	enter_ptr = 0x400;
	illeg = FALSE;
	if ('\0' != cmd[cptr])
		enter_ptr = tohex(&cmd[cptr]) & 0xffffff;
	do
	{
		if (inrange(enter_ptr))
		{
			readmem(BYTE,enter_ptr);
			val = membyte;
			printf("%06lX  %02X? ",enter_ptr,val);
			gets(hexin);
			if (0 != strlen(hexin))
			{
				illeg = isdigit(hexin[0]);
				illeg |= ((toupper(hexin[0]) >= 'A') &&
					  (toupper(hexin[0]) <= 'F'));
				illeg = !illeg;
				if (!illeg)
				{
					srcbyte = tohex(hexin) & 0xff;
					writemem(BYTE,enter_ptr);
				}
			}
			enter_ptr++;
		}
	} while (inrange && !illeg);
}

void init_data()
{
	unsigned count = 0xffff;
//	struct SREGS segregs;

     pc = 0;
     loadX(1);
     loadZ(0);
     loadC(1);
     loadN(0);
     loadV(0);

     ingo = FALSE;
     stopem = FALSE;
     dump_ptr = 0;

     if( (pbuf = (unsigned char *)malloc( (size_t)count )) == NULL )
     {
 //     count = _memmax();
		count = 0xfff7;
//        count = coreleft();

     if( (pbuf = (unsigned char *)malloc( (size_t)count )) == NULL )
	{
		printf("Fatal: Cannot allocate ANY memory.\n");
		exit(-1);
	}
     }
     printf("%u (0x%04X) bytes allocated for emulator memory.\n",count,count);
//        "at %04X:%04X.\n",count,FP_SEG(pbuf),FP_OFF(pbuf));
//        segread(&segregs);
//        printf("CS: %04X, DS: %04X, ES: %04X, SS: %04X\n",
//                segregs.cs,segregs.ds,segregs.es,segregs.ss);
}

int daa(unsigned char *num1, unsigned char num2)
{
	int CYout;
	unsigned int bcd1,bcd2;   //rewritten for ABCD fix 9.30.96

	bcd1 = ((*num1 >> 4) & 0xf) * 10;
	bcd1 += (*num1 & 0xf);
	bcd2 = ((num2 >> 4) & 0xf) * 10;
	bcd2 += (num2 & 0xf);
	bcd1 += bcd2;
	if (bcd1 > 99)
	{
		CYout = 1;
		bcd1 -= 100;
	}
	else
		CYout = 0;
	*num1 = tobcd((unsigned char)(bcd1 & 0xff));
	return CYout;
}

unsigned char tobcd(unsigned char num)
{
	unsigned char val;

	val = (num / 10) << 4;
	val += num % 10;
	return val;
}

unsigned char frombcd(unsigned char num)
{
	unsigned char val;

	val = (num >> 4) * 10;
	val += num & 0xf;
	return val;
}

void go()
{
	int k,nobp,firstloop,fetched;

	stopem = FALSE;
	ingo = TRUE;
	firstloop = TRUE;
	tbp1 = 1;
	tbp2 = 1;
	fetched = FALSE;

	if (',' == cmd[cptr])
		cptr++;
	else
	if ('\0' != cmd[cptr])
		pc = tohex(&cmd[cptr]) & 0xffffff;
	skipblank();
	if (',' == cmd[cptr])
		cptr++;
	skipblank();
	if ('\0' != cmd[cptr])
		tbp1 = tohex(&cmd[cptr]) & 0xffffff;
	skipblank();
	if (',' == cmd[cptr])
		cptr++;
	skipblank();
	if ('\0' != cmd[cptr])
		tbp2 = tohex(&cmd[cptr]) & 0xffffff;

	while (inrange(pc) && !stopem)
	{
		if (!bpstat[0] && !bpstat[1] && !bpstat[2] && !bpstat[3]
			&& (tbp1 == 1) && (tbp2 == 1))
			{
				fetch(EX);
				fetched = TRUE;
			}
		else
		{
			nobp = -1;
			for(k = 0; k < 4; k++)
				if (bpstat[k] && (pc == bpadr[k]))
				{
					if (!firstloop)
						nobp = k;
				}
			if (nobp == -1)
			{
				if (((tbp1 == pc) || (tbp2 == pc)) && fetched)
				{
					printf("\nTemporary breakpoint encountered at address $%08lX\n",pc);
					tbp1 = 1;
					tbp2 = 1;
					stopem = TRUE;
				}
				else
				{
					fetch(EX);
					fetched = TRUE;
				}
			}
			else
			{
				printf("\nBreakpoint %d encountered at "
					"address %08lX.\n",1+nobp,pc);
				stopem = TRUE;
			}
		}
		firstloop = FALSE;
	}
	ingo = FALSE;
}

void bkpt()
{
	char bnum;
	int k;

	bnum = cmd[cptr] - '1';
	if ((0 <= bnum) && (bnum <= 3))
	{
		cptr++;
		skipblank();
		if ('\0' != cmd[cptr])
			bpadr[bnum] = tohex(&cmd[cptr]) & 0xffffff;
		if (0 == bpadr[bnum])
		{
			bpstat[bnum] = 0;
			printf("Breakpoint %d cleared.\n",1+bnum);
		}
		else
		{
			bpstat[bnum] = 1;
			printf("Breakpoint %d active.\n",1+bnum);
		}
	}
	else
	if (cmd[cptr] == '\0')
	{
		for(k = 0; k < 4; k++)
		{
			printf("Breakpoint %d: ",1+k);
			if (bpstat[k])
				printf("%08lX\n",bpadr[k]);
			else
				printf("Not active\n");
		}
	}
	else
		printf("Error! Breakpoint number must be from 1 to 4.\n");
}

unsigned char getbyte(void)
{
	char fbyte[3];
	unsigned char dbyte;

	fbyte[0] = getc(fptr);
	printf("%c",fbyte[0]);
	fbyte[1] = getc(fptr);
	printf("%c",fbyte[1]);
	fbyte[2] = '\0';
	dbyte = tohex(fbyte) & 0xff;
	chksum += dbyte;
}

/* Utility function to read an S-Record from a file */
int Read_SRecord(SRecord *srec, FILE *in) {
	char recordBuff[SRECORD_RECORD_BUFF_SIZE];
	/* A temporary buffer to hold ASCII hex encoded data, set to the maximum length we would ever need */
	char hexBuff[SRECORD_MAX_ADDRESS_LEN+1];
	int asciiAddressLen, asciiDataLen, dataOffset, fieldDataCount, i;

	/* Check our record pointer and file pointer */
	if (srec == NULL || in == NULL)
		return SRECORD_ERROR_INVALID_ARGUMENTS;

	if (fgets(recordBuff, SRECORD_RECORD_BUFF_SIZE, in) == NULL) {
			/* In case we hit EOF, don't report a file error */
			if (feof(in) != 0)
				return SRECORD_ERROR_EOF;
			else
				return SRECORD_ERROR_FILE;
	}
	/* Null-terminate the string at the first sign of a \r or \n */
	for (i = 0; i < (int)strlen(recordBuff); i++) {
		if (recordBuff[i] == '\r' || recordBuff[i] == '\n') {
			recordBuff[i] = 0;
			break;
		}
	}

	/* Check if we hit a newline */
	if (strlen(recordBuff) == 0)
		return SRECORD_ERROR_NEWLINE;

	/* Size check for type and count fields */
	if (strlen(recordBuff) < SRECORD_TYPE_LEN + SRECORD_COUNT_LEN)
		return SRECORD_ERROR_INVALID_RECORD;

	/* Check for the S-Record start code at the beginning of every record */
	if (recordBuff[SRECORD_START_CODE_OFFSET] != SRECORD_START_CODE)
		return SRECORD_ERROR_INVALID_RECORD;

	/* Copy the ASCII hex encoding of the type field into hexBuff, convert it into a usable integer */
	strncpy(hexBuff, recordBuff+SRECORD_TYPE_OFFSET, SRECORD_TYPE_LEN);
	hexBuff[SRECORD_TYPE_LEN] = 0;
	srec->type = strtol(hexBuff, (char **)NULL, 16);

	/* Copy the ASCII hex encoding of the count field into hexBuff, convert it to a usable integer */
	strncpy(hexBuff, recordBuff+SRECORD_COUNT_OFFSET, SRECORD_COUNT_LEN);
	hexBuff[SRECORD_COUNT_LEN] = 0;
	fieldDataCount = strtol(hexBuff, (char **)NULL, 16);

	/* Check that our S-Record type is valid */
	if (srec->type < SRECORD_TYPE_S0 || srec->type > SRECORD_TYPE_S9)
		return SRECORD_ERROR_INVALID_RECORD;
	/* Get the ASCII hex address length of this particular S-Record type */
	asciiAddressLen = SRecord_Address_Lengths[srec->type];

	/* Size check for address field */
	if (strlen(recordBuff) < (unsigned int)(SRECORD_ADDRESS_OFFSET+asciiAddressLen))
		return SRECORD_ERROR_INVALID_RECORD;

	/* Copy the ASCII hex encoding of the count field into hexBuff, convert it to a usable integer */
	strncpy(hexBuff, recordBuff+SRECORD_ADDRESS_OFFSET, asciiAddressLen);
	hexBuff[asciiAddressLen] = 0;
	srec->address = strtol(hexBuff, (char **)NULL, 16);

	/* Compute the ASCII hex data length by subtracting the remaining field lengths from the S-Record
	 * count field (times 2 to account for the number of characters used in ASCII hex encoding) */
	asciiDataLen = (fieldDataCount*2) - asciiAddressLen - SRECORD_CHECKSUM_LEN;
	/* Bailout if we get an invalid data length */
	if (asciiDataLen < 0 || asciiDataLen > SRECORD_MAX_DATA_LEN)
		return SRECORD_ERROR_INVALID_RECORD;

	/* Size check for final data field and checksum field */
	if (strlen(recordBuff) < (unsigned int)(SRECORD_ADDRESS_OFFSET+asciiAddressLen+asciiDataLen+SRECORD_CHECKSUM_LEN))
		return SRECORD_ERROR_INVALID_RECORD;

	dataOffset = SRECORD_ADDRESS_OFFSET+asciiAddressLen;

	/* Loop through each ASCII hex byte of the data field, pull it out into hexBuff,
	 * convert it and store the result in the data buffer of the S-Record */
	for (i = 0; i < asciiDataLen/2; i++) {
		/* Times two i because every byte is represented by two ASCII hex characters */
		strncpy(hexBuff, recordBuff+dataOffset+2*i, SRECORD_ASCII_HEX_BYTE_LEN);
		hexBuff[SRECORD_ASCII_HEX_BYTE_LEN] = 0;
		srec->data[i] = strtol(hexBuff, (char **)NULL, 16);
	}
	/* Real data len is divided by two because every byte is represented by two ASCII hex characters */
	srec->dataLen = asciiDataLen/2;

	/* Copy out the checksum ASCII hex encoded byte, and convert it back to a usable integer */
	strncpy(hexBuff, recordBuff+dataOffset+asciiDataLen, SRECORD_CHECKSUM_LEN);
	hexBuff[SRECORD_CHECKSUM_LEN] = 0;
	srec->checksum = strtol(hexBuff, (char **)NULL, 16);

	if (srec->checksum != Checksum_SRecord(srec))
		return SRECORD_ERROR_INVALID_RECORD;

	return SRECORD_OK;
}


/* Utility function to print the information stored in an S-Record */
void Print_SRecord(const SRecord *srec) {
	int i;
	printf("S-Record Type: \t\tS%d\n", srec->type);
	printf("S-Record Address: \t0x%2.8X\n", srec->address);
	printf("S-Record Data: \t\t{");
	for (i = 0; i < srec->dataLen; i++) {
		if (i+1 < srec->dataLen)
			printf("0x%02X, ", srec->data[i]);
		else
			printf("0x%02X", srec->data[i]);
	}
	printf("}\n");
	printf("S-Record Checksum: \t0x%2.2X\n", srec->checksum);
}

uint8_t Checksum_SRecord(const SRecord *srec) {
	uint8_t checksum;
	int fieldDataCount, i;

	/* Compute the record count, address and checksum lengths are halved because record count
	 * is the number of bytes left in the record, not the length of the ASCII hex representation */
	fieldDataCount = SRecord_Address_Lengths[srec->type]/2 + srec->dataLen + SRECORD_CHECKSUM_LEN/2;

	/* Add the count, address, and data fields together */
	checksum = fieldDataCount;
	/* Add each byte of the address individually */
	checksum += (uint8_t)(srec->address & 0x000000FF);
	checksum += (uint8_t)((srec->address & 0x0000FF00) >> 8);
	checksum += (uint8_t)((srec->address & 0x00FF0000) >> 16);
	checksum += (uint8_t)((srec->address & 0xFF000000) >> 24);
	for (i = 0; i < srec->dataLen; i++)
		checksum += srec->data[i];

	/* One's complement the checksum */
	checksum = ~checksum;

	return checksum;
}

void load_hexfile()
{
	int k;
	unsigned char data, length, chkerr;
	unsigned long int adr;
//	FILE* fp;
//	char fname2[20];
	//make fptr NULL
	fptr = NULL;
//	fp = NULL;

	chkerr = FALSE;
	fname[0] = '\0';
//	fname2[0] = '\0';
	strcat(fname,&cmd[cptr]);
//	strcat(fname2,&cmd[cptr]);
	strcat(fname,".hex");
	
	fptr = fopen(fname,"r");
//	fp = fopen(fname2,"r");
//	SRecord srec;
//	uint32_t staddress[100];
	

	


	if (fptr == NULL)
	{
		printf("Error opening %s\n",fname);
		chkerr = TRUE;
	}
	else
	{
		fchr = getc(fptr);
		while (fchr != EOF)
		{
			printf("%c",fchr);
			if (fchr == 'S')
			{
				fchr = getc(fptr);
				printf("%c",fchr);
				switch(fchr)
				{
					case '0' : fchr = getc(fptr);  // 3.1.98
						   break;
					case '1' :
					{
						chksum = 0;
						length = getbyte();
						length -= 3;
//						printf("sourcebyte is %d\n",length);   //check length
						adr = getbyte();
						adr <<= 8;
						data = getbyte();
						adr = adr | data;
						while (length)  //was a do-loop
						{		//0-length Srecord 8.26.96
							srcbyte = getbyte();
//							printf("sourcebyte is %d",adr);  //testing if anything is written
							writemem(BYTE,adr);
							adr++;
							length--;
						};
						
						getbyte(); /* read checksum */
						if (chksum != 0xff)
						{
							printf("\nChecksum error!\n");
							chkerr = TRUE;
						}
						fchr = getc(fptr);
/*						
						for (int wip = 0; wip < srec.dataLen; wip++)
						{
							if (wip+1 < srec.dataLen)
							srcbyte = srec.data[wip];
							adr = staddress[wip];
							adr <<= 8;
							printf("addr is 0x%02X,\n ", adr);
							writemem(BYTE,adr);
						}
*/						
						break;
					}
					case '9' :
					{
						length = getbyte();
						length -= 3;
						adr = getbyte();
						adr <<= 8;
						data = getbyte();
						pc = adr | data;
						unaspc = pc;   // 3.1.98
						dump_ptr = pc;  // 3.3.98
						fchr = getc(fptr);
						while (fchr != EOF)
						{
							printf("%c",fchr);
							fchr = getc(fptr);
						}
						printf("Starting address: %04X\n", pc & 0xffff);
//						long int loc = (pc & 0xffff);
						long int loc = (pc);
						printf("PC Location: %04X\n", loc);
						break;
					}
				}
			}
			else
				fchr = getc(fptr);
		}
		fclose(fptr);
	}
	if (!chkerr)
		printf("%s loaded into emulator memory.\n",fname);
}

void pushAn()
{
	int k;
	for(k = 0; k < 8; k++)
		tempAn[k] = a_reg[k];
}

void popAn()
{
	int k;
	for(k = 0; k < 8; k++)
		a_reg[k] = tempAn[k];
}

void address_error(unsigned long int addr)
{
	printf("\nAddress error accessing location $%08lX\n",addr);
	exit(7);
}

int readmem(int size, unsigned long int addr)
{
	unsigned long int ea;

	ea = addr & 0xffff;
//        if ((ea >= 0x8000) && (ea < 0x9000))
  //      {
    //            ea &= 0xfff;
      //          ea |= 0x1000;
	//}
	switch(size)
	{
		case BYTE : membyte = (pbuf)[ea];
		//printf("\n-- %02X --\n",membyte);
			    return 0;
			    break;
		case WORD : if (0 == (ea & 1))
			    {
				    memword = ((pbuf)[ea] << 8) | (pbuf)[ea+1];
				    //printf("\n-- %04X --\n",memword);
				    return 0;
			    }
			    else
			    {
				    address_error(addr);
				    return 1;
			    }
			    break;
		case LONG : if (0 == (ea & 1))
			    {
				    memlong = ((pbuf)[ea] << 8) | (pbuf)[ea+1];
				    memlong <<= 16;
				    memlong |= (((pbuf)[ea+2] << 8) | (pbuf)[ea+3]);
				    //printf("\n-- %08lX --\n",memlong);
				    return 0;
			    }
			    else
			    {
				    address_error(addr);
				    return 1;
			    }
			    break;
	}
}

void addr_mode(int opword)
{
     unsigned char mode, reg;
     int error;

     mode = (opword >> 3) & 7;
     reg_num = reg = opword & 7;
     switch(mode)
     {
	  case 0 : if (!ingo) sprintf(tops,"D%1d",reg);
			strcat(ops,tops); break;
	  case 1 : if (!ingo) sprintf(tops,"A%1d",reg);
			strcat(ops,tops); break;
	  case 2 : if (!ingo) sprintf(tops,"(A%1d)",reg);
			strcat(ops,tops); break;
	  case 3 : if (!ingo) sprintf(tops,"(A%1d)+",reg);
			strcat(ops,tops); break;
	  case 4 : if (!ingo) sprintf(tops,"-(A%1d)",reg);
			strcat(ops,tops); break;
	  case 5 : error = readmem(WORD,pc);
			mw++;
		   if (!error)
		   {
			if (!ingo) printf("%04X ",memword);
			if (!ingo) sprintf(tops,"$%04X",memword);
			strcat(ops,tops);
			if (!ingo) sprintf(tops,"(A%1d)",reg);
			strcat(ops,tops);
			pc += 2;
		   }
		   break;
	  case 6 : error = readmem(WORD,pc);
			mw++;
		   if (!error)
		   {
			if (!ingo)
			{
				printf("%04X ",memword);
				sprintf(tops,"$%02X(A%1d,",memword & 0xff,reg);
				strcat(ops,tops);
				(0 == (memword & 0x8000)) ? strcat(ops,"D") : strcat(ops,"A");
				sprintf(tops,"%1d.",(memword >> 12) & 7);
				strcat(ops,tops);
				(0 == (memword & 0x800)) ? strcat(ops,"W)") : strcat(ops,"L)");
			}
			pc += 2;
		   }
		   break;
	  case 7 :
	  switch(reg)
	  {
	       case 0 : error = readmem(WORD,pc);
			mw++;
			if (!ingo) printf("%04X ",memword);
			if (!ingo) sprintf(tops,"$%04X",memword);
			strcat(ops,tops);
			pc += 2;
			break;
	       case 1 : error = readmem(LONG,pc);
			mw += 2;
			if (!ingo) printf("%04X %04X ",(int)(memlong >> 16),
				memlong & 0xffff);
			if (!ingo) sprintf(tops,"$%08lX",memlong);
			strcat(ops,tops);
			pc += 4;
			break;
	       case 2 : error = readmem(WORD,pc);
			mw++;
			if (!ingo) printf("%04X ",memword);
			if (!ingo) sprintf(tops,"$%04X(PC)",memword);
			strcat(ops,tops);
			pc += 2;
			break;
	       case 3 : error = readmem(WORD,pc);
			mw++;
			if (!ingo)
			{
				printf("%04X ",memword);
				sprintf(tops,"$%02X(PC,",memword & 0xff);
				strcat(ops,tops);
				(0 == (memword & 0x8000)) ? strcat(ops,"D") : strcat(ops,"A");
				sprintf(tops,"%1d.",(memword >> 12) & 7);
				strcat(ops,tops);
				(0 == (memword & 0x800)) ? strcat(ops,"W)") : strcat(ops,"L)");
			}
			pc += 2;
			break;
	       case 4 : switch(opsize)
			{
				case BYTE :
				{
					readmem(WORD,pc);
					mw++;
					if (!ingo) printf("%04X ",memword);
					membyte = memword & 0xff;
					if (!ingo) sprintf(tops,"#$%02X",membyte);
					strcat(ops,tops);
					pc += 2;
					break;
				}
				case WORD :
				{
					readmem(WORD,pc);
					mw++;
					if (!ingo) printf("%04X ",memword);
					if (!ingo) sprintf(tops,"#$%04X",memword);
					strcat(ops,tops);
					pc += 2;
					break;
				}
				case LONG :
				{
					readmem(LONG,pc);
					mw += 2;
					if (!ingo) printf("%04X %04X ",(int)(memlong >> 16), memlong & 0xffff);
					if (!ingo) sprintf(tops,"#$%08lX",memlong);
					strcat(ops,tops);
					pc += 4;
					break;
				}
			}
			break;
	  }
	  break;
     }
}

void read_src(int opword)
{
     int mode, reg, Dn, An, Xn, d8, d16;
     int error;
     unsigned long int disp, Xreg;

     mode = (opword >> 3) & 7;
     Dn = An = reg = opword & 7;
     switch(mode)
     {
	  case 0 : readreg(d_reg[Dn]);
		break;
	  case 1 : readreg(a_reg[An]);
		break;
	  case 2 : adrind(RD, opsize, An, NONE); break;
	  case 3 : adrind(RD, opsize, An, POST); break;
	  case 4 : adrind(RD, opsize, An, PRE); break;
	  case 5 : readmem(WORD,pc);
		pc += 2;
		if (!exflag) return;  //10.1.96
		disp = memword;
		if (0 != (0x8000 & disp))
			disp |= 0xffff0000;
		srcaddr = a_reg[An] + disp;
		readmem(opsize,srcaddr);
		loadsrc();
		   break;
	  case 6 : readmem(WORD,pc);
		pc += 2;
		if (!exflag) return;  //10.1.96
		disp = memword & 0xff;
		if (0 != (0x80 & disp))
			disp |= 0xffffff00;
		Xn = (memword >> 12) & 7;
		if (0 == (memword & 0x8000))
			Xreg = d_reg[Xn];
		else
			Xreg = a_reg[Xn];
		if (0 == (memword & 0x800))
		{
			if (0 != (0x8000 & Xreg))
				Xreg |= 0xffff0000;
			else Xreg &= 0xffff;
		}
		srcaddr = a_reg[An] + Xreg + disp;
		readmem(opsize,srcaddr);
		loadsrc();
		break;
	  case 7 :
	  switch(reg)
	  {
	       case 0 : readmem(WORD,pc);
			pc += 2;
		if (!exflag) return;  //10.1.96
			srcaddr = memword;
			if (0 != (srcaddr & 0x8000))
				srcaddr |= 0xffff0000;
			readmem(opsize,srcaddr);
			loadsrc();
			break;
	       case 1 : readmem(LONG,pc);
			pc += 4;
		if (!exflag) return;  //10.1.96
			srcaddr = memlong;
			readmem(opsize,srcaddr);
			loadsrc();
			break;
	       case 2 : readmem(WORD,pc);
			disp = memword;
			if (0 != (0x8000 & disp))
				disp |= 0xffff0000;
			srcaddr = pc + disp;
			pc += 2;
		if (!exflag) return;  //10.1.96
			readmem(opsize,srcaddr);
			loadsrc();
			break;
	       case 3 : readmem(WORD,pc);
			disp = memword & 0xff;
			if (0 != (0x80 & disp))
				disp |= 0xffffff00;
			Xn = (memword >> 12) & 7;
			if (0 == (memword & 0x8000))
				Xreg = d_reg[Xn];
			else
				Xreg = a_reg[Xn];
			if (0 == (memword & 0x800))
			{
				if (0 != (0x8000 & Xreg))
					Xreg |= 0xffff0000;
				else Xreg &= 0xffff;
			}
			srcaddr = pc + Xreg + disp;
			pc += 2;
		if (!exflag) return;  //10.1.96
			readmem(opsize,srcaddr);
			loadsrc();
			break;
	       case 4 : if (opsize != LONG)
			{
				readmem(WORD,pc);
				pc += 2;
		if (!exflag) return;  //10.1.96
				membyte = memword & 0xff;
			}
			else
			{
				readmem(LONG,pc);
				pc += 4;
		if (!exflag) return;  //10.1.96
			}
			loadsrc();
			break;
	  } break;
     }
}

void adrind(int rwflag, int opsize, int An, int incflag)
{
	int incr;

	if (!exflag) return;  //10.1.96
	switch(opsize)
	{
		case BYTE : incr = 1; break;
		case WORD : incr = 2; break;
		case LONG : incr = 4; break;
	}
	if (rwflag == RD)
	{
		switch(incflag)
		{
			case NONE : readmem(opsize,a_reg[An]);
				srcaddr = a_reg[An];
				break;
			case PRE : a_reg[An] -= incr;
				readmem(opsize,a_reg[An]);
				srcaddr = a_reg[An];
				break;
			case POST: readmem(opsize,a_reg[An]);
				srcaddr = a_reg[An];
				a_reg[An] += incr;
				break;
		}
		loadsrc();
	}
	else
	{
		switch(incflag)
		{
			case NONE : writemem(opsize,a_reg[An]);
				break;
			case PRE: a_reg[An] -= incr;
				writemem(opsize,a_reg[An]);
				break;
			case POST: writemem(opsize,a_reg[An]);
				a_reg[An] += incr;
				break;
		}
	}
}

void loadsrc()
{
	switch(opsize)
	{
		case BYTE : srcbyte = membyte;
			srcdata = srcbyte;
			break;
		case WORD : srcword = memword;
			srcdata = srcword;
			break;
		case LONG : srclong = memlong;
			srcdata = srclong;
			break;
	}
}

void writemem(int size, unsigned long int addr)
{
	unsigned long int ea;

	ea = addr & 0xffff;
//        if ((ea >= 0x8000) && (ea < 0x9000))
  //      {
    //            ea &= 0xfff;
      //          ea |= 0x1000;
	//}
	switch(size)
	{
		case BYTE : (pbuf)[ea] = srcbyte;
			break;
		case WORD :
			if (0 == (1 & ea))
			{
				(pbuf)[ea] = (srcword >> 8) & 0xff;
				(pbuf)[ea+1] = srcword & 0xff;
			}
			else
				address_error(addr);
			break;
		case LONG :
			if (0 == (1 & ea))
			{
				(pbuf)[ea] = (srclong >> 24) & 0xff;
				(pbuf)[ea+1] = (srclong >> 16) & 0xff;
				(pbuf)[ea+2] = (srclong >> 8) & 0xff;
				(pbuf)[ea+3] = srclong & 0xff;
			}
			else
				address_error(addr);
			break;
	}
}

void write_dst(int opword)
{
     int mode, reg, Dn, An, Xn, d8, d16;
     unsigned long int dstaddr, disp, Xreg;
     int error;

     switch(opsize)
     {
	case BYTE : result = srcbyte; break;
	case WORD : result = srcword; break;
	case LONG : result = srclong; break;
     }
     mode = (opword >> 3) & 7;
     Dn = An = reg = opword & 7;
     switch(mode)
     {
	  case 0 :
		{
		if (!exflag) return;  //10.1.96
		switch(opsize)
		{
			case BYTE : d_reg[Dn] = (d_reg[Dn] & 0xffffff00) | srcbyte;
				break;
			case WORD : d_reg[Dn] = (d_reg[Dn] & 0xffff0000) | srcword;
				break;
			case LONG : d_reg[Dn] = srclong;
				break;
		}
		break;
		}
	  case 1 :
		{
		if (!exflag) return;  //10.1.96
		switch(opsize)
		{
			case BYTE : a_reg[An] = (a_reg[An] & 0xffffff00) | srcbyte;
				break;
			case WORD : a_reg[Dn] = (a_reg[An] & 0xffff0000) | srcword;
				break;
			case LONG : a_reg[An] = srclong;
				break;
		}
		break;
		}
	  case 2 : adrind(WR, opsize, An, NONE); break;
	  case 3 : adrind(WR, opsize, An, POST); break;
	  case 4 : adrind(WR, opsize, An, PRE); break;
	  case 5 : readmem(WORD,pc);
		pc += 2;
		if (!exflag) return;  //10.1.96
		disp = memword;
		if (0 != (0x8000 & disp))
			disp |= 0xffff0000;
		dstaddr = a_reg[An] + disp;
		writemem(opsize,dstaddr);
		   break;
	  case 6 : readmem(WORD,pc);
		pc += 2;
		if (!exflag) return;  //10.1.96
		disp = memword & 0xff;
		if (0 != (0x80 & disp))
			disp |= 0xffffff00;
		Xn = (memword >> 12) & 7;
		if (0 == (memword & 0x8000))
			Xreg = d_reg[Xn];
		else
			Xreg = a_reg[Xn];
		if (0 == (memword & 0x800))
		{
			if (0 != (0x8000 & Xreg))
				Xreg |= 0xffff0000;
			else Xreg &= 0xffff;
		}
		dstaddr = a_reg[An] + Xreg + disp;
		writemem(opsize,dstaddr);
		break;
	  case 7 :
	  switch(reg)
	  {
	       case 0 : readmem(WORD,pc);
			pc += 2;
		if (!exflag) return;  //10.1.96
			dstaddr = memword;
			if (0 != (dstaddr & 0x8000))
				dstaddr |= 0xffff0000;
			writemem(opsize,dstaddr);
			break;
	       case 1 : readmem(LONG,pc);
			pc += 4;
		if (!exflag) return;  //10.1.96
			writemem(opsize,memlong);
			break;
	       case 2 : error = readmem(WORD,pc);
			disp = memword;
			if (0 != (0x8000 & disp))
				disp |= 0xffff0000;
			dstaddr = pc + disp;
			pc += 2;
		if (!exflag) return;  //10.1.96
			writemem(opsize,dstaddr);
			break;
	       case 3 : error = readmem(WORD,pc);
			if (!ingo)
			{
				printf("$%02X(PC,",memword & 0xff);
				(0 == (memword & 0x8000)) ? printf("D") : printf("A");
				printf("%1d.",(memword >> 12) & 7);
				(0 == (memword & 0x800)) ? printf("W)") : printf("L)");
			}
			pc += 2;
		if (!exflag) return;  //10.1.96
			break;
	       case 4 : printf("Error! Immediate data used as destination!\n");
			printf("Offending instruction at address $%08lX\n",pc);
			break;
	  } break;
     }
}

void unassemble()
{
	int i,endi,j;
	unsigned long int oldpc,diff,savedpc;
	unsigned char kyb;

	savedpc = pc;
	pc = unaspc;
	ingo = FALSE;
	if (',' == cmd[cptr])
		cptr++;
	else
	if ('\0' != cmd[cptr])
		pc = tohex(&cmd[cptr]) & 0xffffff;
	skipblank();
	if (',' == cmd[cptr])
		cptr++;
	skipblank();
	endi = 16;
	if ('\0' != cmd[cptr])
		endi = tohex(&cmd[cptr]) & 0xffff;
	for(i = 0; i < endi; i++)
	{
		oldpc = pc+2;
		fetch(NOEX);
     /*           diff = (pc - oldpc)/2;
		if (diff < 0) diff = 0;
		if (diff)
		{
			for(j = 0; diff > j; j++)
			{
				printf("%06lX  ",oldpc);
				readmem(WORD,oldpc);
				printf("%04X\n",memword);
				oldpc += 2;
			}
		}
       */
		if (kbhit())          //2.27.98
		{
			kyb = getch();
			if (kyb == 0x15)
				break;
		}
	}
	unaspc = pc;
	pc = savedpc;
}

void fetch(int ex)
{
     int op_ptr, error;

     mw = 0;
     isize = '\0';
     ops[0] = '\0';
     exflag = ex;
     if (!ingo) printf("%06lX ",pc);
     readmem(WORD,pc);
     inst = memword;
     pc += 2;
     if (!ingo) printf("%04X ",inst);
     op_ptr = op_search(inst);
     if(0xffff != op_ptr)
     {
	  icode = optab[op_ptr];
     // 3.1.98     if (!ingo) printf("%s",icode.opcode);
	  switch(icode.type)
	  {
	       case 0  : type0(icode.pattern); break;
	       case 99 : type99(); break;
	       case 111: type111(); break;
	       case 112: type112(); break;
	       case 2  : type2(); break;
	       case 3  : type3(); break;
	       case 4  : type4(inst & 7); break;
	       case 5  : type5((inst >> 8) & 0xf); break;
	       case 611: type611(); break;
	       case 612: type612(icode.pattern); break;
	       case 7  : type7(); break;
	       case 8  : type8(inst & 0x000f); break;
	       case 9  : type9(); break;
	       case 100: type100(icode.pattern); break;
	       case 101: type101(); break;
	       case 11 : type11(); break;
	       case 12 : type12((inst >> 8) & 0xf); break;
	       case 13 : type13(icode.pattern); break;
	       case 14 : type14(); break;
	       case 15 : type15(icode.pattern); break;
	       case 16 : type16(); break;
	       case 170: type170(); break;
	       case 171: type171(); break;
	       case 18 : type18(icode.pattern); break;
	       case 19 : type19(); break;
	       case 200: type200(); break;
	       case 201: type201((inst >> 12) & 0xf); break;
	       case 202: type202(icode.pattern); break;
	       case 21 : type21(icode.pattern); break;
	       case 220: type220(); break;
	       case 221: type221((inst >> 12) & 0xf); break;
	       case 23 : type23(icode.pattern); break;
	       case 24 : type24(icode.pattern); break;
	       case 25 : type25(icode.pattern); break;
	       case 26 : type26(); break;
	       case 27 : type27(icode.pattern); break;
	       case 28 : type28(inst & 7); break;
	       case 29 : type29(); break;
	       case 30 : type30((inst >> 9) & 7); break;
	       case 34 : type34((inst >> 8) & 0xf); break;
	       case 35 : type35(); break;
	  }
	  if (!ingo)
	  {
		mw = 4 - mw;
		while(mw > 0)
		{
			printf("     ");
			mw--;
		}
	  }
	if (!ingo) printf("%s",icode.opcode);
	if ((!ingo) && (isize != '\0')) printf(".%c",isize);
	if (!ingo) printf("\t");
	if ((!ingo) && (ops[0] != '\0')) printf("%s",ops);

     }
     else
	  if (!ingo) printf("                    ????  %c %c", (inst >> 8) & 0xff, inst & 0xff);
     if (!ingo) printf("\n");
}

int op_search(int iword)
{
     int op_ptr = 0, found = 0;
     int mword;

     while((0 != optab[op_ptr].mask) && !found)
     {
	  mword = optab[op_ptr].mask & iword;
	  if(mword == optab[op_ptr].pattern)
	       found = 1;
	  else
	       op_ptr++;
     }
     if(found)
	  return op_ptr;
     else
	  return 0xffff;
}

void op_size(int sval, int inst)
{
	switch(sval)
	{
		case 012 :
			switch(inst & 3)
			{
				case 0 : if (!ingo) isize = 'B'; // 3.1.98 printf(".B");
					 opsize = BYTE; break;
				case 1 : if (!ingo) isize = 'W'; //printf(".W");
					 opsize = WORD; break;
				case 2 : if (!ingo) isize = 'L'; //printf(".L");
					 opsize = LONG; break;
			} break;
		case 132 :
			switch(inst & 3)
			{
				case 1 : if (!ingo) isize = 'B'; // printf(".B");
					 opsize = BYTE; break;
				case 3 : if (!ingo) isize = 'W'; //printf(".W");
					 opsize = WORD; break;
				case 2 : if (!ingo) isize = 'L'; //printf(".L");
					 opsize = LONG; break;
			} break;
	}
}

void swaprm(int inst)
{
	pushpc;
	addr_mode(((inst >> 3) & 7) | ((inst << 3) & 0x38));
	poppc;
	write_dst(((inst >> 3) & 7) | ((inst << 3) & 0x38));
}

void fixN(int opsize, unsigned long int result)
{
	loadN(0);
	switch(opsize)
	{
		case BYTE : if (0 != (0x80 & result)) loadN(1);
			break;
		case WORD : if (0 != (0x8000 & result)) loadN(1);
			break;
		case LONG : if (0 != (0x80000000 & result)) loadN(1);
			break;
	}
}

void fixZ(int opsize, unsigned long int result)
{
	loadZ(1);
	switch(opsize)
	{
		case BYTE : if (0 == (0xff & result)) loadZ(0);
			break;
		case WORD : if (0 == (0xffff & result)) loadZ(0);
			break;
		case LONG : if (0 == result) loadZ(0);
			break;
	}
}

int bitval(int bitnum, unsigned long int number)
{
	return (number >> bitnum) & 1;
}

void readreg(unsigned long int regvalue)
{
	switch(opsize)
	{
		case BYTE : srcbyte = regvalue & 0xff;
			srcdata = srcbyte;
			break;
		case WORD : srcword = regvalue & 0xffff;
			srcdata = srcword;
			break;
		case LONG : srclong = regvalue;
			srcdata = srclong;
			break;
	}
}

void sflag(unsigned long int value)
{
	switch(opsize)
	{
		case BYTE: Sm = (value >> 7) & 1; break;
		case WORD: Sm = (value >> 15) & 1; break;
		case LONG: Sm = (value >> 31) & 1; break;
	}
}

void dflag(unsigned long int value)
{
	switch(opsize)
	{
		case BYTE: Dm = (value >> 7) & 1; break;
		case WORD: Dm = (value >> 15) & 1; break;
		case LONG: Dm = (value >> 31) & 1; break;
	}
}

void rflag(unsigned long int result)
{
	switch(opsize)
	{
		case BYTE: Rm = (result >> 7) & 1; break;
		case WORD: Rm = (result >> 15) & 1; break;
		case LONG: Rm = (result >> 31) & 1; break;
	}
}

void savedreg(int reg, unsigned long int regvalue)
{
	switch(opsize)
	{
		case BYTE: d_reg[reg] = (d_reg[reg] & 0xffffff00) | (regvalue & 0xff);
			break;
		case WORD: d_reg[reg] = (d_reg[reg] & 0xffff0000) | (regvalue & 0xffff);
			break;
		case LONG: d_reg[reg] = regvalue;
			break;
	}
}

void type0(int pattern)   /* ILLEGAL, NOP, RESET, RTE, RTR, RTS, TRAPV */
			  /* CC not affected */
{
	if (!exflag) return;  //10.1.96
	switch(pattern)
	{
		case 0x4e71 :   /* NOP */
			break;
		case 0x4e70 :   /* RESET */
			break;
		case 0x4afc :   /* ILLEGAL */
			printf("Illegal instruction at address %08lX.\n",pc-2);
			break;
		case 0x4e76 :   /* TRAPV */
			if (VF)
				printf("Overflow.\n");
			break;
		case 0x4e75 :   /* RTS */
			readmem(LONG,a_reg[7]);
			a_reg[7] += 4;
			pc = memlong;
			break;
		case 0x4e73 :   /* RTE */
			readmem(WORD,a_reg[7]);
			sr = memword;
			a_reg[7] += 2;
			readmem(LONG,a_reg[7]);
			a_reg[7] += 4;
			pc = memlong;
			break;
		case 0x4e77 :   /* RTR */
			readmem(WORD,a_reg[7]);
			sr = (sr & 0xff00) | (memword & 0xff);
			a_reg[7] += 2;
			readmem(LONG,a_reg[7]);
			a_reg[7] += 4;
			pc = memlong;
			break;
	}
}

void type111()   /* ABCD */
{
	int rx, ry, rm,c1,c2,CY;
	unsigned char xval, yval;

	opsize = BYTE;
	//if (!ingo) printf("\t");
	rx = (inst >> 9) & 7;
	ry = inst & 7;
	rm = (inst >> 3) & 1;
	if (rm == 0)
	{
		if (!ingo) sprintf(tops,"D%1d,D%1d",ry,rx);
		strcat(ops,tops);
		if (!exflag) return;  //10.1.96
		xval = d_reg[rx] & 0xff;
		yval = d_reg[ry] & 0xff;
		//xval += yval;  //ABCD fix 9.30.96
		c1 = daa(&xval,yval);
		if (XF)
			c2 = daa(&xval,1);
		else
			c2 = 0;
		CY = c1 | c2;
		d_reg[rx] = (d_reg[rx] & 0xffffff00) | xval;
	}
	else
	{
		if (!ingo) sprintf(tops,"-(A%1d),-(A%1d)",ry,rx);
		strcat(ops,tops);
		if (!exflag) return;  //10.1.96
		adrind(RD, BYTE, ry, PRE);
		yval = srcbyte;
		adrind(RD, BYTE, rx, PRE);
		xval = srcbyte;
		//xval += yval;  //ABCD fix 9.30.96
		c1 = daa(&xval,yval);
		if (XF)
			c2 = daa(&xval,1);
		else
			c2 = 0;
		CY = c1 | c2;
		srcbyte = xval;
		adrind(WR, BYTE, rx, NONE);
	}
	loadZ((xval || !ZF));
	loadC(CY);
	loadX(CY);
}

void type112()   /* SBCD */
{
	int rx, ry, rm,c1,c2,CY;
	unsigned char xval, yval;

	opsize = BYTE;
	//if (!ingo) printf("\t");
	ry = (inst >> 9) & 7;
	rx = inst & 7;
	rm = (inst >> 3) & 1;
	if (rm == 0)
	{
		if (!ingo) sprintf(tops,"D%1d,D%1d",rx,ry);
		strcat(ops,tops);
		if (!exflag) return;  //10.1.96
		xval = frombcd(d_reg[rx] & 0xff);
		yval = frombcd(d_reg[ry] & 0xff);
		yval -= xval;
		c1 = 0;
		if (0x80 & yval)
			c1 = 1;
		c2 = 0;
		if (XF)
			yval--;
		if (0x80 & yval)
		{
			yval = ~yval;
			yval++;
			c2 = 1;
		}
		yval = tobcd(yval);
		CY = c1 | c2;
		if (CY)
			yval = 0x9a - yval;
		d_reg[ry] = (d_reg[ry] & 0xffffff00) | yval;
	}
	else
	{
		if (!ingo) sprintf(tops,"-(A%1d),-(A%1d)",rx,ry);
		strcat(ops,tops);
		if (!exflag) return;  //10.1.96
		adrind(RD, BYTE, rx, PRE);
		xval = frombcd(srcbyte);
		adrind(RD, BYTE, ry, PRE);
		yval = frombcd(srcbyte);
		yval -= xval;
		c1 = 0;
		if (0x80 & yval)
			c1 = 1;
		c2 = 0;
		if (XF)
			yval--;
		if (0x80 & yval)
		{
			yval = ~yval;
			yval++;
			c2 = 1;
		}
		yval = tobcd(yval);
		CY = c1 | c2;
		if (CY)
			yval = 0x9a - yval;
		srcbyte = yval;
		adrind(WR, BYTE, ry, NONE);
	}
	loadZ((yval || !ZF));
	loadC(CY);
	loadX(CY);
}

void type2()   /* MOVE */
{
	op_size(132, inst >> 12);
	//if (!ingo) printf("\t");
	pushpc;
	addr_mode(inst);
	poppc;
	read_src(inst);
	if (!ingo) strcat(ops,",");
	swaprm(inst >> 6);
	if (!exflag) return;  //10.1.96
	fixN(opsize,result);
	fixZ(opsize,result);
	loadV(0);
	loadC(0);
}

void type3()   /* MOVEA */
	       /* CC not affected */
{
	op_size(132, inst >> 12);
	//if (!ingo) sprintf("\t");
	pushpc;
	addr_mode(inst);
	poppc;
	read_src(inst);
	if (opsize == WORD)
	{
		srclong = srcword;
		if (0 != (0x8000 & srclong))
			srclong |= 0xffff0000;
		opsize = LONG;
	}
	if (!ingo) strcat(ops,",");
	swaprm((inst >> 6) & 0x3f);
}

void type4(int Dn)   /* SWAP */
{
     //if (!ingo) printf("\t");
     addr_mode(Dn);
     if (!exflag) return;  //10.1.96
     temp = d_reg[Dn] >> 16;
     d_reg[Dn] = (d_reg[Dn] << 16) | temp;
     fixN(opsize,d_reg[Dn]);
     fixZ(opsize,d_reg[Dn]);
     loadV(0);
     loadC(0);
}

void condi(int cc)
{
	switch(cc)
	{
	case 2 : if (!ingo) strcat(icode.opcode,"HI"); COND = HI; break;
	case 3 : if (!ingo) strcat(icode.opcode,"LS"); COND = LS; break;
	case 4 : if (!ingo) strcat(icode.opcode,"CC"); COND = CC; break;
	case 5 : if (!ingo) strcat(icode.opcode,"CS"); COND = CS; break;
	case 6 : if (!ingo) strcat(icode.opcode,"NE"); COND = NE; break;
	case 7 : if (!ingo) strcat(icode.opcode,"EQ"); COND = EQ; break;
	case 8 : if (!ingo) strcat(icode.opcode,"VC"); COND = VC; break;
	case 9 : if (!ingo) strcat(icode.opcode,"VS"); COND = VS; break;
	case 10: if (!ingo) strcat(icode.opcode,"PL"); COND = PL; break;
	case 11: if (!ingo) strcat(icode.opcode,"MI"); COND = MI; break;
	case 12: if (!ingo) strcat(icode.opcode,"GE"); COND = GE; break;
	case 13: if (!ingo) strcat(icode.opcode,"LT"); COND = LT; break;
	case 14: if (!ingo) strcat(icode.opcode,"GT"); COND = GT; break;
	case 15: if (!ingo) strcat(icode.opcode,"LE"); COND = LE; break;
	}
}

void type5(int cc)   /* DBcc */
		     /* CC not affected */
{
	unsigned long int disp, adr;
	unsigned int count;
	int Dn, error;

	switch(cc)
	{
		case 0 : if (!ingo) strcat(icode.opcode,"T"); COND = 1; break;
		case 1 : if (!ingo) strcat(icode.opcode,"RA"); COND = 0; break;
		default: condi(cc);
	}
	Dn = inst & 7;
	if (!ingo) sprintf(tops,"D%1d,",Dn);
	strcat(ops,tops);
	readmem(WORD,pc);
	mw++;
	if (!ingo) printf("%04X ",memword);
	adr = pc;
	pc += 2;
	disp = memword;
	if (0 != (disp & 0x8000))
		disp = disp | 0xffff0000;
	adr = adr + disp;
	if (!ingo) sprintf(tops,"$%08lX",adr);
	strcat(ops,tops);
		if (!exflag) return;  //10.1.96
	if (!COND)
	{
		count = d_reg[Dn] & 0xffff;
		count -= 1;
		d_reg[Dn] = (d_reg[Dn] & 0xffff0000) | count;
		if (count != 0xffff)
			pc = adr;
	}
}

void type611()   /* ADDI, CMPI, SUBI */

{
	op_size(012, inst >> 6);
	//if (!ingo) printf("\t");
	pushpc;
	addr_mode(0x3c);
	poppc;
	read_src(0x3c);
	temp = srcdata;
	sflag(temp);
	if (!ingo) strcat(ops,",");
	pushpc;
	addr_mode(inst);
	poppc;
		if (!exflag) return;  //10.1.96
	if (0x0c00 != (inst & 0x0f00))
	{
		pushpc;
		pushAn();
	}
	read_src(inst);
	if (0x0c00 != (inst & 0x0f00))
	{
		popAn();
		poppc;
	}
	if (0x0c00 == (inst & 0x0f00))
	{
		dflag(srcdata);
		switch(opsize)
		{
			case BYTE : loadZ((srcbyte - temp));
				fixN(BYTE,srcbyte - temp);
				Rm = ((srcbyte - temp) >> 7) & 1;
				break;
			case WORD : loadZ((srcword - temp));
				fixN(WORD,srcword - temp);
				Rm = ((srcword - temp) >> 15) & 1;
				break;
			case LONG : loadZ((srclong - temp));
				fixN(LONG,srclong - temp);
				Rm = ((srclong - temp) >> 31) & 1;
				break;
		}
		//printf("aSm = %d, Dm = %d, Rm = %d\n",Sm,Dm,Rm);
		loadV((Sm == Rm) && (Sm != Dm));
		loadC((Sm && !Dm) || (Rm && !Dm) || (Sm && Rm));
	}
	else
	{
		switch(opsize)
		{
			case BYTE: temp2 = srcbyte; break;
			case WORD: temp2 = srcword; break;
			case LONG: temp2 = srclong; break;
		}
		dflag(temp2);
		if (0x0600 == (inst & 0x0f00))
			temp += temp2;
		else
			temp = temp2 - temp;
		switch(opsize)
		{
			case BYTE: srcbyte = temp & 0xff;
				Rm = (temp >> 7) & 1;    //11.26.98
				break;
			case WORD: srcword = temp & 0xffff;
				Rm = (temp >> 15) & 1;
				break;
			case LONG: srclong = temp;
				Rm = (temp >> 31) & 1;
				break;
		}
		write_dst(inst);
		if (0x0600 == (inst & 0x0f00))
		{
		//printf("bSm = %d, Dm = %d, Rm = %d\n",Sm,Dm,Rm);
			loadV((Sm == Dm ) && (Sm != Rm));
			loadC((Sm && Dm) || (!Rm && Dm) || (Sm && !Rm));
			loadX((Sm && Dm) || (!Rm && Dm) || (Sm && !Rm));
		}
		else
		{
		//printf("cSm = %d, Dm = %d, Rm = %d\n",Sm,Dm,Rm);
			loadV((Sm != Dm) && (Sm == Rm));
			loadC((Sm && !Dm) || (Rm && !Dm) || (Sm && Rm));
			loadX((Sm && !Dm) || (Rm && !Dm) || (Sm && Rm));
		}
		fixN(opsize,temp);
		fixZ(opsize,temp);
	}
}

void type612(int pattern)   /* ANDI, EORI, ORI */
{
	unsigned long int src,dst;

	op_size(012, inst >> 6);
	//if (!ingo) printf("\t");
	pushpc;
	addr_mode(0x3c);
	poppc;
	read_src(0x3c);
	switch(opsize)
	{
		case BYTE : src = srcbyte; break;
		case WORD : src = srcword; break;
		case LONG : src = srclong; break;
	}
	if (!ingo) strcat(ops,",");
	pushpc;
	addr_mode(inst);
	poppc;
		if (!exflag) return;  //10.1.96
	pushpc;
	pushAn();
	read_src(inst);
	popAn();
	poppc;
	switch(opsize)
	{
		case BYTE : dst = srcbyte; break;
		case WORD : dst = srcword; break;
		case LONG : dst = srclong; break;
	}
	switch(pattern)
	{
		case 0x0200: srcbyte = src & dst;
			loadN(0x80 & srcbyte);
			loadZ(srcbyte);
			break;
		case 0x0240: srcword = src & dst;
			loadN(0x8000 & srcword);
			loadZ(srcword);
			break;
		case 0x0280: srclong = src & dst;
			loadN(0x80000000 & srclong);
			loadZ(srclong);
			break;
		case 0x0a00: srcbyte = src ^ dst;
			loadN(0x80 & srcbyte);
			loadZ(srcbyte);
			break;
		case 0x0a40: srcword = src ^ dst;
			loadN(0x8000 & srcword);
			loadZ(srcword);
			break;
		case 0x0a80: srclong = src ^ dst;
			loadN(0x80000000 & srclong);
			loadZ(srclong);
			break;
		case 0x0000: srcbyte = src | dst;
			loadN(0x80 & srcbyte);
			loadZ(srcbyte);
			break;
		case 0x0040: srcword = src | dst;
			loadN(0x8000 & srcword);
			loadZ(srcword);
			break;
		case 0x0080: srclong = src | dst;
			loadN(0x80000000 & srclong);
			loadZ(srclong);
			break;
	}
	write_dst(inst);
	loadV(0);
	loadC(0);
}

void type7()   /* UNLK */
	       /* CC not affected */
{
	int An;

	An = inst & 7;
	if (!ingo) sprintf(tops,"A%1d",An);
	strcat(ops,tops);
		if (!exflag) return;  //10.1.96
	a_reg[7] = a_reg[An];
	readmem(LONG,a_reg[7]);
	a_reg[7] += 4;
	a_reg[An] = memlong;
}

void type8(int trapnum)    /* TRAP */
			   /* CC not affected */
{
	unsigned char cval;

	if (!ingo) sprintf(tops,"#%d",trapnum);
	strcat(ops,tops);
		if (!exflag) return;  //10.1.96
	if (!ingo) strcat(ops,"\n");
	switch(trapnum)
	{
	case 0: cval = getch();
		d_reg[1] = (d_reg[1] & 0xffffff00) | cval;
		break;
	case 1: cval = d_reg[1] & 0xff;
//		if (cval == 0x0d)
  //			printf("\n");
    //		if ((cval >= 32) && (cval <= 127))
			printf("%c",cval);
		break;
	case 2: printf("\n");
		break;
	case 3: do
		{
			readmem(BYTE,a_reg[3]);
			a_reg[3]++;
			cval = membyte;
			if (cval != 0)
			{
      //				if (cval == 0x0d)
	//				printf("\n");
	 //			if ((cval >= 32) || (cval <= 127))
					printf("%c",cval);
			}
		} while (cval != 0);
		break;
	case 4: printf("%02X",d_reg[2]);
		break;
	case 5: printf("%04X",d_reg[2]);
		break;
	case 6: printf("%08lX",d_reg[2]);
		break;
	case 9: printf("Program exit at address %08lX\n",pc-2);
		stopem = TRUE;
		break;
//	case 10:int21h();       //8.17.97
//		break;
	default:printf("TRAP  #%1X not implemented.\n",trapnum);
		break;
	}
}

void type9()   /* STOP */
{

	if (!ingo) sprintf(tops,"#$");
	strcat(ops,tops);
	readmem(WORD,pc);
	sr = memword;
	pc += 2;
	if (!ingo) sprintf(tops,"%04X",sr);
	strcat(ops,tops);
		if (!exflag) return;  //10.1.96
	printf("Program stopped at address %08lX.\n",pc-4);
	stopem = TRUE;
}

void type100(int pattern)  /* DIVS, DIVU, MULS, MULU */
{                          /* 81c0  80c0  c1c0  c0c0 */
	int Dn;
	unsigned int dval;
	int x,y;
	long int z;
	unsigned long int rem,ovchk;

	opsize = WORD;
	//if (!ingo) printf("\t");
	pushpc;
	addr_mode(inst);
	poppc;
	read_src(inst);
	Dn = (inst >> 9) & 7;
	if (!ingo) sprintf(tops,",D%1d",Dn);
	strcat(ops,tops);
		if (!exflag) return;  //10.1.96
	dval = d_reg[Dn] & 0xffff;
	switch(pattern)
	{
	case 0xc0c0:
		result = (unsigned long) srcword * (unsigned long) dval;
		d_reg[Dn] = result;
		fixN(LONG,result);
		loadZ(result);
		loadV(0);
		loadC(0);
		break;
	case 0xc1c0:
		x = srcword;
		y = dval;
		result = (long) x * (long) y;
		d_reg[Dn] = result;
		fixN(LONG,result);
		loadZ(result);
		loadV(0);
		loadC(0);
		break;
	case 0x80c0:
		if (0 == srcword)
			printf("\nDivision by zero at address %08lX.\n",pc);
		else
		{
			result = d_reg[Dn] / (unsigned long) srcword;
			ovchk = result >> 16;
			if (0 == ovchk)
			{
				rem = d_reg[Dn] % (unsigned long) srcword;
				rem <<= 16;
				d_reg[Dn] = rem | result;
				fixN(WORD,result);
				loadZ(result);
				loadV(0);
			}
			else
			{
				loadV(1);
			}
		}
		loadC(0);
		break;
	case 0x81c0:
		if (0 == srcword)
			printf("\nDivision by zero at address %08lX.\n",pc);
		else
		{
			z = d_reg[Dn];
			x = srcword;
			result = z / (long) x;
			ovchk = result >> 15;
			if ((0 == ovchk) || (0x1ffff == ovchk))
			{
				rem = z % (long) x;
				rem <<= 16;
				result &= 0xffff;
				d_reg[Dn] = rem | result;
				fixN(WORD,result);
				loadZ(result);
				loadV(0);
			}
			else
			{
				loadV(1);
			}
		}
		loadC(0);
		break;
	}
}
void type101()  /* CHK */
{
	int Dn;
	unsigned int dval;

	opsize = WORD;
	//if (!ingo) printf("\t");
	pushpc;
	addr_mode(inst);
	poppc;
	read_src(inst);
	Dn = (inst >> 9) & 7;
	if (!ingo) sprintf(tops,",D%1d",Dn);
	strcat(ops,tops);
		if (!exflag) return;  //10.1.96
	dval = d_reg[Dn] & 0xffff;
	if (dval & 0x8000)
	{
		loadN(1);
		printf("\nCHK exception (D%1d < 0) at address %08lX\n",Dn,pc-2);
	}
	else
	{
		if ((signed int) dval > (signed int) srcword)
		{
			loadN(0);
			printf("\nCHK exception (D%1d > EA) at address %08lX\n",Dn,pc-2);
		}
	}
}

void type11() /* ADDQ, SUBQ */
{
	unsigned long int d8;
	int An;

	op_size(012, inst >> 6);
	d8 = (inst >> 9) & 7;
	if (d8 == 0) d8 = 8;
	if (!ingo) sprintf(tops,"#%1d,",d8);
	strcat(ops,tops);
	pushpc;
	addr_mode(inst);
	poppc;
		if (!exflag) return;  //10.1.96
	Sm = 0;
	if (0x8 != (inst & 0x38))   /* not An */
	{
		pushpc;
		pushAn();
		read_src(inst);
		poppc;
		popAn();
		dflag(srcdata);
	       switch(opsize)
	       {
			case BYTE: if (inst & 0x100)
					result = srcbyte - d8;
				else
					result = srcbyte + d8;
				srcbyte = result & 0xff;
				Rm = (result >> 7) & 1;
				break;
			case WORD: if (inst & 0x0100)
					result = srcword - d8;
				else
					result = srcword + d8;
				srcword = result & 0xffff;
				Rm = (result >> 15) & 1;
				break;
			case LONG: if (inst & 0x0100)
					result = srclong - d8;
				else
					result = srclong + d8;
				srclong = result;
				Rm = (result >> 31) & 1;
				break;
	       }
	       write_dst(inst);
	       if (inst & 0x0100)   /* SUBQ */
	       {
			loadV((Sm != Dm) && (Sm == Rm));
			loadC((Sm && !Dm) || (Rm && !Dm) || (Sm && Rm));
			loadX((Sm && !Dm) || (Rm && !Dm) || (Sm && Rm));
	       }
	       else    /* ADDQ */
	       {
			loadV((Sm == Dm ) && (Sm != Rm));
			loadC((Sm && Dm) || (!Rm && Dm) || (Sm && !Rm));
			loadX((Sm && Dm) || (!Rm && Dm) || (Sm && !Rm));
	       }
	       fixN(opsize,result);
	       fixZ(opsize,result);
	}
	else   /*   An   */
	{
		An = inst & 7;
		if (inst & 0x0100)
			a_reg[An] -= d8;
		else
			a_reg[An] += d8;
	}
}

void type12(int cc)   /* Bcc (BSR too!)*/
		      /* CC not affected */
{
	unsigned long int disp, adr;
	int error;

	switch(cc)
	{
		case 0 : if (!ingo) strcat(icode.opcode,"RA"); COND = 1; break;
		case 1 : if (!ingo) strcat(icode.opcode,"SR"); COND = 1; break;
		default: condi(cc);
	}
	adr = pc;
	disp = inst & 0x00ff;
	if (0 != disp)
	{
		if (!ingo) isize = 'S';
		if (0 != (disp & 0x80))
			disp |= 0xffffff00;
	}
	else
	{
		readmem(WORD,pc);
		if (!ingo) printf("%04X ",memword);
		mw++;
		pc += 2;
		disp = memword;
		if (0 != (disp & 0x8000))
			disp |= 0xffff0000;
	}
	adr += disp;
	if (!ingo) sprintf(tops,"$%08lX",adr);
	strcat(ops,tops);
		if (!exflag) return;  //10.1.96
	if (COND)
	{
		if (cc == 1)
		{
			srclong = pc;
			a_reg[7] -= 4;
			writemem(LONG,a_reg[7]);
		}
		pc = adr;
	}
}

void type13(int pattern)   /* NBCD, TAS */
{
	int c1,c2,CY;
	unsigned char number;

	//if (!ingo) printf("\t");
	opsize = BYTE;
	pushpc;
	addr_mode(inst);
	poppc;
		if (!exflag) return;  //10.1.96
	pushpc;
	pushAn();
	read_src(inst);
	popAn();
	poppc;
	switch(pattern)
	{
		case 0x4800: number = srcbyte;
			//number = 0x99 - number;  //ABCD fix 9.30.96
			number = (((number >> 4) & 0xf) * 10) + (number & 0xf);
			number = 100 - number;
			if (XF)
				number--;
			if (number == 100)
			{
				CY = 1;
				number = 0;
			}
			else
				CY = 0;
			number = tobcd(number);
			srcbyte = number;
			write_dst(inst);
			loadZ((srcbyte || !ZF));
			loadC(CY);
			loadX(CY);
			break;
		case 0x4ac0: loadZ(srcbyte);
			loadN(0x80 & srcbyte);
			srcbyte |= 0x80;
			write_dst(inst);
			break;
	}
}

void type14()   /* PEA */
		/* CC not affected */
{
	//if (!ingo) printf("\t");
	pushpc;
	addr_mode(inst);
	poppc;
		if (!exflag) return;  //10.1.96
	read_src(inst);
	a_reg[7] -= 4;
	srclong = srcaddr;
	writemem(LONG,a_reg[7]);
}

void type15(int pattern)   /* JSR, JMP */
			   /* CC not affected */
{
	//if (!ingo) printf("\t");
	pushpc;
	addr_mode(inst);
	poppc;
		if (!exflag) return;  //10.1.96
	read_src(inst);
	if (pattern == 0x4e80)
	{
		srclong = pc;
		a_reg[7] -= 4;
		writemem(LONG,a_reg[7]);
	}
	pc = srcaddr;
}

void type16()   /* LINK */
		/* CC not affected */
{
	int An;
	unsigned long int d16;

	An = inst & 7;
	readmem(WORD,pc);
	d16 = memword;
	if (d16 & 0x8000)            // 9.11.96 -offset fix
		d16 |= 0xffff0000;
	pc += 2;
	if (!ingo) sprintf(tops,"A%1d,#$%04X",An,d16);
	strcat(ops,tops);
		if (!exflag) return;  //10.1.96
	a_reg[7] -= 4;
	srclong = a_reg[An];
	writemem(LONG,a_reg[7]);
	a_reg[An] = a_reg[7];
	a_reg[7] += d16;
}

void type170()   /* ASd, LSd */
{
	int Dx,Dy,rc,dir,k,bitout;
	unsigned long int msbit,dval,mschg;

	if(inst & 0x100)
	{
		if (!ingo) strcat(icode.opcode,"L");
		dir = 1;
	}
	else
	{
		if (!ingo) strcat(icode.opcode,"R");
		dir = 0;
	}
	if(0xc0 == (inst & 0xc0))   /* memory rotate 1 bit */
	{
		isize = 'W';
		opsize = WORD;
		pushpc;
		addr_mode(inst);
		poppc;
		if (!exflag) return;  //10.1.96
		pushpc;
		pushAn();
		read_src(inst);
		popAn();
		poppc;
		if (dir)   /* left */
		{
			mschg = 0x8000 & srcword;
			bitout = (srcword >> 15) & 1;
			srcword <<= 1;
			mschg = mschg ^ (0x8000 & srcword);
		}
		else   /* right */
		{
			mschg = 0;
			bitout = srcword & 1;
			msbit = 0x8000 & srcword;
			srcword >>= 1;
			if (0xe0c0 == (0xffc0 & inst))   /* ASR pattern */
				srcword |= msbit;
		}
		write_dst(inst);
		loadX(bitout);
		loadN(0x8000 & srcword);
		loadZ(srcword);
		if (0xe0c0 == (0xf0c0 & inst))   /* ASd pattern */
		{
			loadV(0 != mschg);
		}
		else
		{
			loadV(0);
		}
		loadC(bitout);
	}
	else
	{
		op_size(012, inst >> 6);
		//if (!ingo) printf("\t");
		rc = Dx = (inst >> 9) & 7;
		if(inst & 0x20)
		{
			if (!ingo) sprintf(tops,"D%1d",Dx);
			strcat(ops,tops);
			rc = d_reg[Dx] & 0x3f;
		}
		else
		{
			rc = (rc != 0) ? rc : 8;
			if (!ingo) sprintf(tops,"#%1d",rc);
			strcat(ops,tops);
		}
		Dy = inst & 7;
		if (!ingo) sprintf(tops,",D%1d",Dy);
		strcat(ops,tops);
		if (!exflag) return;  //10.1.96
		read_src(Dy);
		bitout = 0;
		if (dir)    /* left */
		{
			switch(opsize)
			{
				case BYTE: msbit = 0x80 & srcbyte;
					mschg = 0;
					for (k = 0; k < rc; k++)
					{
						bitout = (srcbyte >> 7) & 1;
						srcbyte <<= 1;
						if ((srcbyte & 0x80) != msbit)
							mschg = 1;
						loadN(srcbyte & 0x80);
						loadZ(srcbyte);
					}
					break;
				case WORD: msbit = 0x8000 & srcword;
					mschg = 0;
					for (k = 0; k < rc; k++)
					{
						bitout = (srcword >> 15) & 1;
						srcword <<= 1;
						if ((srcword & 0x8000) != msbit)
							mschg = 1;
						loadN(srcword & 0x8000);
						loadZ(srcword);
					}
					break;
				case LONG: msbit = 0x80000000 & srclong;
					mschg = 0;
					for (k = 0; k < rc; k++)
					{
						bitout = (srclong >> 31) & 1;
						srclong <<= 1;
						if ((srclong & 0x80000000) != msbit)
							mschg = 1;
						loadN(srclong & 0x80000000);
						loadZ(srclong);
					}
					break;
			}
			write_dst(Dy);
			if (0xe000 == (0xf018 & inst))  /* ASd pattern */
			{
				loadV(0 != mschg);
			}
			else
			{
				loadV(0);
			}
			loadC(bitout && (0 != rc));
			if (0 != rc)
			{
				loadX(bitout);
			}
		}
		else   /* right */
		{
			mschg = 0;
			switch(opsize)
			{
				case BYTE: msbit = srcbyte & 0x80;
					for (k = 0; k < rc; k++)
					{
						bitout = srcbyte & 1;
						srcbyte >>= 1;
						if (0xe000 == (0xf018 & inst))   /* ASR pattern */
							srcbyte |= msbit;
						loadN(srcbyte & 0x80);
						loadZ(srcbyte);
					}
					break;
				case WORD: msbit = srcword & 0x8000;
					for (k = 0; k < rc; k++)
					{
						bitout = srcword & 1;
						srcword >>= 1;
						if (0xe000 == (0xf018 & inst))   /* ASR pattern */
							srcword |= msbit;
						loadN(srcword & 0x8000);
						loadZ(srcword);
					}
					break;
				case LONG: msbit = srclong & 0x80000000;
					for (k = 0; k < rc; k++)
					{
						bitout = srclong & 1;
						srclong >>= 1;
						if (0xe000 == (0xf018 & inst))   /* ASR pattern */
							srclong |= msbit;
						loadN(srclong & 0x80000000);
						loadZ(srclong);
					}
					break;
			}
			write_dst(Dy);
			if (0xe000 == (inst & 0xf018))   /* ASR pattern */
			{
				loadV(0 != mschg);
			}
			else
			{
				loadV(0);
			}
			loadC(bitout && (0 != rc));
			if (0 != rc)
			{
				loadX(bitout);
			}
		}
	}
}

void type171()   /* ROd, ROXd */
{
	int Dx,Dy,rc,dir,k,bitout;
	unsigned long int msbit,dval,mschg;

	if(inst & 0x100)
	{
		if (!ingo) strcat(icode.opcode,"L");
		dir = 1;
	}
	else
	{
		if (!ingo) strcat(icode.opcode,"R");
		dir = 0;
	}
	if(0xc0 == (inst & 0xc0))   /* memory rotate 1 bit */
	{
		isize = 'W';
		opsize = WORD;
		pushpc;
		addr_mode(inst);
		poppc;
		if (!exflag) return;  //10.1.96
		pushpc;
		pushAn();
		read_src(inst);
		popAn();
		poppc;
		if (dir)   /* left */
		{
			bitout = (srcword >> 15) & 1;
			srcword <<= 1;
			if (0xe4c0 == (0xfec0 & inst))  /* ROXL */
			{
				if (XF) srcword |= 1;
			}
			else
				srcword |= bitout;
		}
		else   /* right */
		{
			bitout = srcword & 1;
			srcword >>= 1;
			if (0xe4c0 == (0xfec0 & inst))  /* ROXR */
			{
				if (XF) srcword |= 0x8000;
			}
			else
			{
				if (bitout) srcword |= 0x8000;
			}
		}
		write_dst(inst);
		loadN(0x8000 & srcword);
		loadZ(srcword);
		loadV(0);
		if (0xe4c0 == (0xfec0 & inst))   /* ROXd */
		{
			loadX(bitout);
		}
		if (0xe4c0 == (0xfec0 & inst))   /* ROXd */
		{
			if (0 == rc)
			{
				loadC(XF);
			}
			else
			{
				loadC(bitout);
			}
		}
		else
		{
			loadC(bitout && (rc != 0));
		}
	}
	else
	{
		op_size(012, inst >> 6);
		//if (!ingo) printf("\t");
		rc = Dx = (inst >> 9) & 7;
		if(inst & 0x20)
		{
			if (!ingo) sprintf(tops,"D%1d",Dx);
			strcat(ops,tops);
			rc = d_reg[Dx] & 0x3f;
		}
		else
		{
			rc = (rc != 0) ? rc : 8;
			if (!ingo) sprintf(tops,"#%1d",rc);
			strcat(ops,tops);
		}
		Dy = inst & 7;
		if (!ingo) sprintf(tops,",D%1d",Dy);
		strcat(ops,tops);
		if (!exflag) return;  //10.1.96
		read_src(Dy);
		bitout = 0;
		if (dir)    /* left */
		{
			switch(opsize)
			{
				case BYTE:
					for (k = 0; k < rc; k++)
					{
						bitout = (srcbyte >> 7) & 1;
						srcbyte <<= 1;
						if (0xe010 == (0xf018 & inst))  /* ROXd */
						{
							if (XF) srcbyte |= 1;
						}
						else
							srcbyte |= bitout;
						loadN(srcbyte & 0x80);
						loadZ(srcbyte);
					}
					break;
				case WORD:
					for (k = 0; k < rc; k++)
					{
						bitout = (srcword >> 15) & 1;
						srcword <<= 1;
						if (0xe010 == (0xf018 & inst))  /* ROXd */
						{
							if (XF) srcword |= 1;
						}
						else
							srcword |= bitout;
						loadN(srcword & 0x8000);
						loadZ(srcword);
					}
					break;
				case LONG:
					for (k = 0; k < rc; k++)
					{
						bitout = (srclong >> 31) & 1;
						srclong <<= 1;
						if (0xe010 == (0xf018 & inst))  /* ROXd */
						{
							if (XF) srclong |= 1;
						}
						else
							srclong |= bitout;
						loadN(srclong & 0x80000000);
						loadZ(srclong);
					}
					break;
			}
			write_dst(Dy);
		}
		else   /* right */
		{
			switch(opsize)
			{
				case BYTE:
					for (k = 0; k < rc; k++)
					{
						bitout = srcbyte & 1;
						srcbyte >>= 1;
						if (0xe010 == (0xf018 & inst))  /* ROXd */
						{
							if (XF) srcbyte |= 0x80;
						}
						else
						{
							if (bitout) srcbyte |= 0x80;
						}
						loadN(srcbyte & 0x80);
						loadZ(srcbyte);
					}
					break;
				case WORD:
					for (k = 0; k < rc; k++)
					{
						bitout = srcword & 1;
						srcword >>= 1;
						if (0xe010 == (0xf018 & inst))  /* ROXd */
						{
							if (XF) srcword |= 0x8000;
						}
						else
						{
							if (bitout) srcword |= 0x8000;
						}
						loadN(srcword & 0x8000);
						loadZ(srcword);
					}
					break;
				case LONG:
					for (k = 0; k < rc; k++)
					{
						bitout = srclong & 1;
						srclong >>= 1;
						if (0xe010 == (0xf018 & inst))  /* ROXd */
						{
							if (XF) srclong |= 0x80000000;
						}
						else
						{
							if (bitout) srclong |= 0x80000000;
						}
						loadN(srclong & 0x80000000);
						loadZ(srclong);
					}
					break;
			}
			write_dst(Dy);
		}
		loadV(0);
		if (0xe010 == (0xf018 & inst))   /* ROXd */
		{
			loadX(bitout);
		}
		if (0xe010 == (0xf018 & inst))   /* ROXd */
		{
			if (0 == rc)
			{
				loadC(XF);
			}
			else
			{
				loadC(bitout);
			}
		}
		else
		{
			loadC(bitout && (rc != 0));
		}
	}
}

void type18(int pattern)   /* BCHG, BCLR, BSET, BTST */
{
	int Dn, imm, bitnum;
	unsigned long int bitpat;

	if (inst & 0x38)
	{
		opsize = BYTE;
		isize = 'B';
	}
	else
	{
		opsize = LONG;
		isize = 'L';
	}
	if ((inst >> 8) & 1)
	{
		Dn = (inst >> 9) & 7;
		if (!ingo) sprintf(tops,"D%1d,",Dn);
		strcat(ops,tops);
		switch(opsize)
		{
		case BYTE : bitnum = d_reg[Dn] & 0x7; break;
		case LONG : bitnum = d_reg[Dn] & 0x1f; break;
		}
	}
	else
	{
		readmem(WORD,pc);
		imm = memword;
		pc += 2;
		switch(opsize)
		{
		case BYTE : bitnum = imm & 0x7;
			if (!ingo) sprintf(tops,"#%1d,",bitnum);
			strcat(ops,tops);
			break;
		case LONG : bitnum = imm & 0x1f;
			if (!ingo) sprintf(tops,"#%d,",bitnum);
			strcat(ops,tops);
			break;
		}

	}
	pushpc;
	addr_mode(inst);
	poppc;
		if (!exflag) return;  //10.1.96
	pushpc;
	pushAn();
	read_src(inst);
	popAn();
	poppc;
	switch(opsize)
	{
		case BYTE : loadZ(((srcbyte >> bitnum) & 1));
			break;
		case LONG : loadZ(((srclong >> bitnum) & 1));
			break;
	}
	bitpat = 1 << bitnum;
	if (opsize == BYTE)
	{
		switch(pattern)
		{
			case 0x0140 : srcbyte ^= bitpat; break;
			case 0x0180 : srcbyte &= ~bitpat; break;
			case 0x01c0 : srcbyte |= bitpat; break;
			case 0x0840 : srcbyte ^= bitpat; break;
			case 0x0880 : srcbyte &= ~bitpat; break;
			case 0x08c0 : srcbyte |= bitpat; break;
		}
	}
	else
	{
		switch(pattern)
		{
			case 0x0140 : srclong ^= bitpat; break;
			case 0x0180 : srclong &= ~bitpat; break;
			case 0x01c0 : srclong |= bitpat; break;
			case 0x0840 : srclong ^= bitpat; break;
			case 0x0880 : srclong &= ~bitpat; break;
			case 0x08c0 : srclong |= bitpat; break;
		}
	}
	write_dst(inst);
}

void type19()   /* ADDX, SUBX */
{
	int rx, ry;

	op_size(012, inst >> 6);
	//if (!ingo) printf("\t");
	rx = (inst >> 9) & 7;
	ry = inst & 7;
	if (0 == ((inst >> 3) & 1))
	{
		if (!ingo) sprintf(tops,"D%1d,D%1d",ry,rx);
		strcat(ops,tops);
		if (!exflag) return;  //10.1.96
		temp = d_reg[ry];
		sflag(temp);
		temp2 = d_reg[rx];
		dflag(temp2);
		if (0x9000 == (inst & 0xf000))
		{
			result = temp2 - temp;
			if (XF) result--;
		}
		else
		{
			result = temp + temp2;
			if (XF) result++;
		}
		rflag(result);
		savedreg(rx,result);
	}
	else
	{
		if (!ingo) sprintf(tops,"-(A%1d),-(A%1d)",ry,rx);
		strcat(ops,tops);
		if (!exflag) return;  //10.1.96
		adrind(RD, opsize, rx, PRE);
		dflag(srcdata);
		result = srcdata;
		adrind(RD, opsize, ry, PRE);
		sflag(srcdata);
		if (0x9000 == (inst & 0xf000))
		{
			result = srcdata - result;
			if (XF) result--;
		}
		else
		{
			result += srcdata;
			if (XF) result++;
		}
		rflag(result);
		switch(opsize)
		{
			case BYTE: srcbyte = result & 0xff; break;
			case WORD: srcword = result & 0xffff; break;
			case LONG: srclong = result; break;
		}
		adrind(WR, opsize, rx, NONE);
	}
	fixN(opsize,result);
	if (ZF)
	{
		fixZ(opsize,result);
	}
	loadV((Sm && Dm && !Rm) || (!Sm && !Dm && Rm));
	loadC((Sm && Dm ) || (!Rm && Dm) || (Sm && !Rm));
	loadX((Sm && Dm ) || (!Rm && Dm) || (Sm && !Rm));
}

void type200()   /* CMP */
{
	unsigned long int dval;
	int Dn;

	op_size(012, inst >> 6);
	//if (!ingo) printf("\t");
	pushpc;
	addr_mode(inst);
	poppc;
	read_src(inst);
	Dn = (inst >> 9) & 7;
	if (!ingo) sprintf(tops,",D%1d",Dn);
	strcat(ops,tops);
		if (!exflag) return;  //10.1.96
	sflag(srcdata);
	switch(opsize)
	{
		case BYTE : dval = d_reg[Dn] & 0xff;
			Dm = (dval >> 7) & 1;
			result = dval - srcbyte;
			Rm = (result >> 7) & 1;
			break;
		case WORD : dval = d_reg[Dn] & 0xffff;
			Dm = (dval >> 15) & 1;
			result = dval - srcword;
			Rm = (result >> 15) & 1;
			break;
		case LONG : dval = d_reg[Dn];
			Dm = (dval >> 31) & 1;
			result = dval - srclong;
			Rm = (result >> 31) & 1;
			break;
	}
	loadN(Rm);
	loadZ(result);
	loadV((Sm == Rm) && (Sm != Dm));
	loadC((Sm && !Dm) || (Rm && !Dm) || (Sm && Rm));
}

void type201(int pattern)   /* ADD, SUB */
{
	int Dn;
	unsigned long int aval,bval;

	op_size(012, inst >> 6);
	//if (!ingo) printf("\t");
	Dn = (inst >> 9) & 7;
	switch(opsize)
	{
		case BYTE : aval = d_reg[Dn] & 0xff; break;
		case WORD : aval = d_reg[Dn] & 0xffff; break;
		case LONG : aval = d_reg[Dn]; break;
	}
	if(inst & 0x100)   /*   Dn +/- <ea> -> <ea>   */
	{
		if (!ingo) sprintf(tops,"D%1d,",Dn);
		strcat(ops,tops);
		switch(opsize)
		{
			case BYTE : Sm = (aval >> 7) & 1; break;
			case WORD : Sm = (aval >> 15) & 1; break;
			case LONG : Sm = (aval >> 31) & 1; break;
		}
		pushpc;
		addr_mode(inst);
		poppc;
		if (!exflag) return;  //10.1.96
		pushpc;
		pushAn();
		read_src(inst);
		popAn();
		poppc;
		switch(opsize)
		{
			case BYTE : bval = srcbyte;
				Dm = (bval >> 7) & 1;   //2.14.98 was >
				break;
			case WORD : bval = srcword;
				Dm = (bval >> 15) & 1;
				break;
			case LONG : bval = srclong;
				Dm = (bval >> 31) & 1;
				break;
		}
		switch(pattern)
		{
			case 0xd : result = aval + bval; break;
			case 0x9 : result = bval - aval; break;
		}
		switch(opsize)
		{
			case BYTE : srcbyte = result & 0xff;
				Rm = (result >> 7) & 1;
				break;
			case WORD : srcword = result & 0xffff;
				Rm = (result >> 15) & 1;
				break;
			case LONG : srclong = result;
				Rm = (result >> 31) & 1;
				break;
		}
		write_dst(inst);
	}
	else   /*   <ea> +/- Dn -> Dn   */
	{
		pushpc;
		addr_mode(inst);
		poppc;
		read_src(inst);
		if (!ingo) sprintf(tops,",D%1d",Dn);
		strcat(ops,tops);
		if (!exflag) return;  //10.1.96
		sflag(srcdata);
		switch(opsize)
		{
			case BYTE :  aval = srcbyte; break;
			case WORD :  aval = srcword; break;
			case LONG :  aval = srclong; break;
		}
		switch(opsize)
		{
			case BYTE : bval = d_reg[Dn] & 0xff;
				Dm = (bval >> 7) & 1;
				break;
			case WORD : bval = d_reg[Dn] & 0xffff;
				Dm = (bval >> 15) & 1;
				break;
			case LONG : bval = d_reg[Dn];
				Dm = (bval >> 31) & 1;
				break;
		}
		switch(pattern)
		{
			case 0xd : result = aval + bval; break;
			case 0x9 : result = bval - aval; break;
		}
		switch(opsize)
		{
			case BYTE : d_reg[Dn] = (d_reg[Dn] & 0xffffff00) | (result & 0xff);
				Rm = (result >> 7) & 1;
				break;
			case WORD : d_reg[Dn] = (d_reg[Dn] & 0xffff0000) | (result & 0xffff);
				Rm = (result >> 15) & 1;
				break;
			case LONG : d_reg[Dn] = result;
				Rm = (result >> 31) & 1;
				break;
		}
	}
	fixN(opsize,result);
	fixZ(opsize,result);
	switch(pattern)
	{
		case 0xd: loadV((Sm == Dm ) && (Sm != Rm));
			loadC((Sm && Dm) || (!Rm && Dm) || (Sm && !Rm));
			loadX((Sm && Dm) || (!Rm && Dm) || (Sm && !Rm));
			break;
		case 0x9: loadV((Sm != Dm) && (Sm == Rm));
			loadC((Sm && !Dm) || (Rm && !Dm) || (Sm && Rm));
			loadX((Sm && !Dm) || (Rm && !Dm) || (Sm && Rm));
			break;
	}
}

void type202(int pattern)   /* AND, OR, EOR */
{
	int Dn;
	unsigned long int src,dst;

	op_size(012, inst >> 6);
	//if (!ingo) printf("\t");
	if(inst & 0x100)
	{
		Dn = (inst >> 9) & 7;
		if (!ingo) sprintf(tops,"D%1d,",Dn);
		strcat(ops,tops);
		src = d_reg[Dn];
		pushpc;
		addr_mode(inst);
		poppc;
		if (!exflag) return;  //10.1.96
		pushpc;
		pushAn();
		read_src(inst);
		popAn();
		poppc;
		switch(opsize)
		{
			case BYTE : dst = srcbyte; break;
			case WORD : dst = srcword; break;
			case LONG : dst = srclong; break;
		}
		switch(pattern)
		{
			case 0xb100 : result = src ^ dst;
				srcbyte = result & 0xff;
				break;
			case 0xb140 : result = src ^ dst;
				srcword = result & 0xffff;
				break;
			case 0xb180 : result = src ^ dst;
				srclong = result;
				break;
			case 0xc100 : result = src & dst;
				srcbyte = result & 0xff;
				break;
			case 0xc140 : result = src & dst;
				srcword = result & 0xffff;
				break;
			case 0xc180 : result = src & dst;
				srclong = result;
				break;
			case 0x8100 : result = src | dst;
				srcbyte = result & 0xff;
				break;
			case 0x8140 : result = src | dst;
				srcword = result & 0xffff;
				break;
			case 0x8180 : result = src | dst;
				srclong = result;
				break;
		}
		write_dst(inst);
	}
	else
	{
		pushpc;
		addr_mode(inst);
		poppc;
		Dn = (inst >> 9) & 7;
		if (!ingo) sprintf(tops,",D%1d",Dn);
		strcat(ops,tops);
		if (!exflag) return;  //10.1.96
		read_src(inst);
		switch(opsize)
		{
			case BYTE : src = srcbyte; break;
			case WORD : src = srcword; break;
			case LONG : src = srclong; break;
		}
		switch(pattern)
		{
		      /*  case 0xb100 : result = src ^ (d_reg[Dn] & 0xff);
				srcbyte = result & 0xff;
				break;
			case 0xb140 : result = src ^ (d_reg[Dn] & 0xffff);
				srcword = result & 0xffff;
				break;
			case 0xb180 : result = src ^ d_reg[Dn];
				srclong = result;
				break; */
			case 0xc000 : result = src & (d_reg[Dn] & 0xff);
				srcbyte = result & 0xff;
				break;
			case 0xc040 : result = src & (d_reg[Dn] & 0xffff);
				srcword = result & 0xffff;
				break;
			case 0xc080 : result = src & d_reg[Dn];
				srclong = result;
				break;
			case 0x8000 : result = src | (d_reg[Dn] & 0xff);
				srcbyte = result & 0xff;
				break;
			case 0x8040 : result = src | (d_reg[Dn] & 0xffff);
				srcword = result & 0xffff;
				break;
			case 0x8080 : result = src | d_reg[Dn];
				srclong = result;
				break;
		}
		write_dst(Dn);
	}
	fixN(opsize,result);
	fixZ(opsize,result);
}

void type21(int pattern)   /* CLR, NEG, NEGX, NOT, TST */
			/*    4200 4400 4000  4600 4a00 */
			/* nothing explicit for TST */
			/* it works by default */
{
	int Dm;

     op_size(012, inst >> 6);
     //if (!ingo) printf("\t");
     pushpc;
     addr_mode(inst);
     poppc;
		if (!exflag) return;  //10.1.96
     if (pattern != 0x4200)
     {
	if (pattern != 0x4a00)
	{
		pushpc;
		pushAn();
	}
	read_src(inst);
	if (pattern != 0x4a00)
	{
		popAn();
		poppc;
	}
     }
     switch(opsize)
     {
	case BYTE : if (pattern == 0x4200) srcbyte = 0;
		if ((pattern == 0x4400) || (pattern == 0x4000))
		{
			Dm = (srcbyte >> 7) & 1;
			srcbyte = 0 - srcbyte;
			if (pattern == 0x4000)
				srcbyte -= 1;
			Rm = (srcbyte >> 7) & 1;
		}
		if (pattern == 0x4600) srcbyte = ~srcbyte;
		if (pattern == 0x4000)
		{
			loadZ((srcbyte || !ZF));
		}
		else
			loadZ(srcbyte);
		fixN(BYTE,srcbyte);
		break;
	case WORD : if (pattern == 0x4200) srcword = 0;
		if ((pattern == 0x4400) || (pattern == 0x4000))
		{
			Dm = (srcword >> 15) & 1;
			srcword = 0 - srcword;
			if (pattern == 0x4000)
				srcword -= 1;
			Rm = (srcword >> 15) & 1;
		}
		if (pattern == 0x4600) srcword = ~srcword;
		if (pattern == 0x4000)
		{
			loadZ((srcword || !ZF));
		}
		else
			loadZ(srcword);
		fixN(WORD,srcword);
		break;
	case LONG : if (pattern == 0x4200) srclong = 0;
		if ((pattern == 0x4400) || (pattern == 0x4000))
		{
			Dm = (srclong >> 31) & 1;
			srclong = 0 - srclong;
			if (pattern == 0x4000)
				srclong -= 1;
			Rm = (srclong >> 31) & 1;
		}
		if (pattern == 0x4600) srclong = ~srclong;
		if (pattern == 0x4000)
		{
			loadZ((srclong || !ZF));
		}
		else
			loadZ(srclong);
		fixN(LONG,srclong);
		break;
     }
     if (pattern != 0x4a00)
	write_dst(inst);
     if ((pattern == 0x4400) || (pattern == 0x4000))
     {
	loadV(Dm && Rm);
	loadC(Dm || Rm);
	loadX(Dm || Rm);
     }
     else
     {
	loadV(0);
	loadC(0);
     }
}

void type220()   /* CMPA */
{
	int An;
	unsigned long int src;

	//if (!ingo) printf("\t");
	if (inst & 0x100)
		opsize = LONG;
	else
		opsize = WORD;
	pushpc;
	addr_mode(inst);
	poppc;
	read_src(inst);
	switch(opsize)
	{
		case WORD : src = srcword;
			if (src & 0x8000)
				src |= 0xffff0000;
			break;
		case LONG : src = srclong;
			break;
	}
	Sm = (src >> 31) & 1;
	An = (inst >> 9) & 7;
	if (!ingo) sprintf(tops,",A%1d",An);
	strcat(ops,tops);
		if (!exflag) return;  //10.1.96
	Dm = (a_reg[An] >> 31) & 1;
	result = a_reg[An] - src;
	Rm = (result >> 31) & 1;
	loadN(Rm);
	loadZ(result);
	loadV((Sm == Rm) && (Sm != Dm));
	loadC((Sm && !Dm) || (Rm && !Dm) || (Sm && Rm));
}

void type221(int pattern)   /* ADDA, SUBA */
{
	int An;
	unsigned long int src;

	//if (!ingo) printf("\t");
	if (inst & 0x100)
		opsize = LONG;
	else
		opsize = WORD;
	pushpc;
	addr_mode(inst);
	poppc;
	read_src(inst);
	switch(opsize)
	{
		case WORD : src = srcword;
			if (src & 0x8000)
				src |= 0xffff0000;
			break;
		case LONG : src = srclong;
			break;
	}
	An = (inst >> 9) & 7;
	if (!ingo) sprintf(tops,",A%1d",An);
	strcat(ops,tops);
		if (!exflag) return;  //10.1.96
	switch(pattern)
	{
		case 0xd: a_reg[An] += src; break;
		case 0x9: a_reg[An] -= src; break;
	}
}

void type23(int pattern)   /* ANDI, EORI, ORI to CCR */
{
	int d8;

	if (!ingo) sprintf(tops,"#");
	strcat(ops,tops);
	readmem(WORD,pc);
	d8 = memword;
	pc += 2;
	if (!ingo) sprintf(tops,"%02X,CCR",d8);
	strcat(ops,tops);
		if (!exflag) return;  //10.1.96
	switch(pattern)
	{
		case 0x023c: sr &= (d8 | 0xff00); break;
		case 0x0a3c: sr ^= (d8 & 0xff); break;
		case 0x003c: sr |= (d8 & 0xff); break;
	}
}

void type24(int pattern)   /* ANDI, EORI, ORI to SR */
{
	int d16;

	if (!ingo) sprintf(tops,"#");
	strcat(ops,tops);
	readmem(WORD,pc);
	d16 = memword;
	pc += 2;
	if (!ingo) sprintf(tops,"%04X,SR",d16);
	strcat(ops,tops);
		if (!exflag) return;  //10.1.96
	switch(pattern)
	{
		case 0x027c: sr &= d16; break;
		case 0x0a7c: sr ^= d16; break;
		case 0x007c: sr |= d16; break;
	}
}
void type99()   /* RTD */
{
	int d16;

	if (!ingo) sprintf(tops,"#");
	strcat(ops,tops);
	readmem(WORD,pc);
	d16 = memword;
	pc += 2;
	if (!ingo) sprintf(tops,"%04X",d16);
	strcat(ops,tops);
	if (!exflag) return;  //10.1.96
	readmem(LONG,a_reg[7]);
	a_reg[7] += 4;
	if (d16 & 0x8000)
		a_reg[7] += (0xffff0000 | d16);
	else
		a_reg[7] += d16;
	pc = memlong;
}

void type25(int pattern)   /* MOVEP */
		/* CC not affected */
{
	int Dx, Ay;
	unsigned char temp;
	unsigned long int disp,addr,temp2;

	//if (!ingo) printf("\t");
	Dx = (inst >> 9) & 7;
	Ay = inst & 7;
	readmem(WORD,pc);
	pc += 2;
	disp = memword;
	if (0x8000 & disp)
		disp |= 0xffff0000;
	addr = disp + a_reg[Ay];
	if (inst & 0x40)
		opsize = LONG;
	else
		opsize = WORD;
	if (inst & 0x80)    /* reg to mem */
	{
		if (!ingo) sprintf(tops,"D%1d,$%04X(A%1d)",Dx,memword,Ay);
		strcat(ops,tops);
		if (!exflag) return;  //10.1.96
		switch(opsize)
		{
			case WORD: srcbyte = (d_reg[Dx] >> 8) & 0xff;
				writemem(BYTE,addr);
				addr += 2;
				srcbyte = d_reg[Dx] & 0xff;
				writemem(BYTE,addr);
				break;
			case LONG: srcbyte = (d_reg[Dx] >> 24) & 0xff;
				writemem(BYTE,addr);
				addr += 2;
				srcbyte = (d_reg[Dx] >> 16) & 0xff;
				writemem(BYTE,addr);
				addr += 2;
				srcbyte = (d_reg[Dx] >> 8) & 0xff;
				writemem(BYTE,addr);
				addr += 2;
				srcbyte = d_reg[Dx] & 0xff;
				writemem(BYTE,addr);
				break;
		}
	}
	else   /* mem to reg */
	{
		if (!ingo) sprintf(tops,"$%04X(A%1d),D%1d",memword,Ay,Dx);
		strcat(ops,tops);
		if (!exflag) return;  //10.1.96
		switch(opsize)
		{
			case WORD: readmem(BYTE,addr);
				temp = membyte;
				addr += 2;
				readmem(BYTE,addr);
				d_reg[Dx] = (d_reg[Dx] & 0xffff0000) | (temp << 8) | membyte;
				break;
			case LONG: readmem(BYTE,addr);
				temp = membyte;
				addr += 2;
				readmem(BYTE,addr);
				temp <<= 8;
				temp |= membyte;
				addr += 2;
				readmem(BYTE,addr);
				temp <<= 8;
				temp |= membyte;
				addr += 2;
				readmem(BYTE,addr);
				d_reg[Dx] = (temp << 8) | membyte;
				break;
		}
	}
}

void type26()   /* CMPM */
{
	int Ax, Ay;
	unsigned long int src;

	op_size(012,inst >> 6);
	Ax = (inst >> 9) & 7;
	Ay = inst & 7;
	if (!ingo) sprintf(tops,"(A%1d)+,(A%1d)+",Ay,Ax);
	strcat(ops,tops);
		if (!exflag) return;  //10.1.96
	adrind(RD, opsize, Ax, POST);
	src = srcdata;
	sflag(src);
	adrind(RD, opsize, Ay, POST);
	dflag(srcdata);
	switch(opsize)
	{
		case BYTE : result = srcbyte - src;
	 //       printf("\nresult = %X\n",result);
	   //     printf("srcbyte = %X, src = %X\n",srcbyte,src);
			Rm = (result >> 7) & 1;
			break;
		case WORD : result = srcword - src;
			Rm = (result >> 15) & 1;
			break;
		case LONG : result = srclong - src;
			Rm = (result >> 31) & 1;
			break;
	}
	loadN(Rm);
	loadZ(result);
	loadV((Sm == Rm) && (Sm != Dm));
	loadC((Sm && !Dm) || (Rm && !Dm) || (Sm && Rm));
}

void type27(int pattern)   /* EXG */
			   /* CC not affected */
{
	int Rx, Ry;

	//if (!ingo) printf("\t");
	Rx = (inst >> 9) & 7;
	Ry = inst & 7;
	switch(pattern)
	{
		case 0xc188:
			{
			if (!ingo) sprintf(tops,"D%1d,A%1d",Rx,Ry);
			strcat(ops,tops);
		if (!exflag) return;  //10.1.96
			temp = d_reg[Rx];
			d_reg[Rx] = a_reg[Ry];
			a_reg[Ry] = temp;
			break;
			}
		case 0xc140:
			{
			if (!ingo) sprintf(tops,"D%1d,D%1d",Rx,Ry);
			strcat(ops,tops);
		if (!exflag) return;  //10.1.96
			temp = d_reg[Rx];
			d_reg[Rx] = d_reg[Ry];
			d_reg[Ry] = temp;
			break;
			}
		case 0xc148:
			{
			if (!ingo) sprintf(tops,"A%1d,A%1d",Rx,Ry);
			strcat(ops,tops);
		if (!exflag) return;  //10.1.96
			temp = a_reg[Rx];
			a_reg[Rx] = a_reg[Ry];
			a_reg[Ry] = temp;
			break;
			}
	}
}

void type28(int Dn)   /* EXT */
{
	if (!ingo) sprintf(tops,"D%1d",Dn);
	strcat(ops,tops);
		if (!exflag) return;  //10.1.96
	loadN(0);
	if (0 == (inst & 0x40))
	{
		if (0 != (d_reg[Dn] & 0x80))
		{
			d_reg[Dn] |= 0xff00;
			loadN(1);
		}
		else d_reg[Dn] &= 0xffff00ff;
		loadZ((d_reg[Dn] & 0xffff));
	}
	else
	{
		if (0 != (d_reg[Dn] & 0x8000))
		{
			d_reg[Dn] |= 0xffff0000;
			loadN(1);
		}
		else d_reg[Dn] &= 0xffff;
		loadZ(d_reg[Dn]);
	}
	loadV(0);
	loadC(0);
}

void type29()   /* LEA */
		/* CC not affected */
{
	int An;

	//if (!ingo) printf("\t");
	pushpc;
	addr_mode(inst);
	poppc;
	read_src(inst);
	An = (inst >> 9) & 7;
	if (!ingo) sprintf(tops,",A%1d",An);
	strcat(ops,tops);
		if (!exflag) return;  //10.1.96
	a_reg[An] = srcaddr;
}

void type30(int Dn)   /* MOVEQ */
{
	unsigned long int data;

	if (!ingo) strcat(ops,"#$");
	data = inst & 0xff;
	if (!ingo) sprintf(tops,"%02X",data);  //11.4.96
	strcat(ops,tops);
	if (!ingo) sprintf(tops,",D%1d",Dn);   //11.4.96
	strcat(ops,tops);
		if (!exflag) return;  //10.1.96
	loadZ(data);
	loadN(0);
	loadV(0);
	loadC(0);
	if (0x80 == (data & 0x80))
	{
		data |= 0xffffff00;
		loadN(1);
	}
	d_reg[Dn] = data;
}

void type34(int cc)   /* Scc */
		      /* CC not affected */
{
	switch(cc)
	{
		case 0 : strcat(icode.opcode,"T"); COND = 1; break;
		case 1 : strcat(icode.opcode,"F"); COND = 0; break;
		default: condi(cc);
	}
	//printf(".B\t");
	isize = 'B';
	opsize = BYTE;
	pushpc;
	addr_mode(inst);
	poppc;
		if (!exflag) return;  //10.1.96
	if (COND)
		srcbyte = 0xff;
	else
		srcbyte = 0;
	write_dst(inst);
}

int btst(int bitnum, unsigned char pattern)
{
	if (bitnum > 7)
		return 0;
	else
		return (pattern >> bitnum) & 1;
}

void rlist(char reg, unsigned char pattern)
{
	int i,j,k;

	i = 0;
	while (i < 8)
	{
		if (btst(i,pattern))
		{
			if (!first) printf("/");
			printf("%c%1d",reg,i);
			first = FALSE;
			j = i;
			while ((j < 8) && btst(j+1,pattern))
				j++;
			if (2 <= (j - i))
				printf("-%c%1d",reg,j);
			if (1 == (j - i))
				printf("/%c%1d",reg,j);
			i = j + 1;
		}
		else
			i++;
	}
}

unsigned char bitswap(unsigned char pattern)
{
	unsigned char p1,p2;
	int k;

	p1 = pattern;
	p2 = 0;
	for (k = 0; k < 8; k++)
	{
	       p2 <<= 1;
	       if (p1 & 1) p2 |= 1;
	       p1 >>= 1;
	}
	return p2;
}

void type35()   /* MOVEM */
		/* CC not affected */
{
	unsigned int rl, rlbak;
	int k,amode;
	unsigned long int ptr;
	unsigned char prepat;

	readmem(WORD,pc);
	pc += 2;
	rl = memword;
	rlbak = rl;
	if ((inst >> 6) & 1)
	{
		isize = 'L';
		opsize = LONG;
	}
	else
	{
		isize = 'W';
		opsize = WORD;
	}
	switch((inst >> 3) & 7)
	{
		case 2 : amode = NONE; break;
		case 3 : amode = POST; break;
		case 4 : amode = PRE; break;
		case 5 : amode = NONE; break;
		case 6 : amode = NONE; break;
		case 7 : amode = NONE; break;
	}
	if (0x400 & inst)  /* read A7-A0/D7-D0 from memory */
	{
		pushpc;
		addr_mode(inst);
		poppc;
		pushAn();
		read_src(inst);
		popAn();
		ptr = srcaddr;
		if (amode == PRE)
		{
			switch(opsize)
			{
				case WORD : ptr += 2; break;
				case LONG : ptr += 4; break;
			}
		}
		if (!ingo) strcat(ops,",");
		first = TRUE;
		if (!ingo) rlist('D',rl & 0xff);
		if (!ingo) rlist('A', (rl >> 8) & 0xff);
		if (!exflag) return;  //10.1.96
		for (k = 0; k < 8; k++)
		{
			if (rl & 1)
			{
			}
		}
		for (k = 0; k < 8; k++)
		{
			if (rl & 1)
			{
				switch(opsize)
				{
					case WORD : readmem(WORD,ptr);
						ptr += 2;
						if (memword & 0x8000)
							d_reg[k] = 0xffff0000 | memword;
						else
							d_reg[k] = memword;
						break;
					case LONG : readmem(LONG,ptr);
						ptr += 4;
						d_reg[k] = memlong;
						break;
				}
			}
			rl >>= 1;
		}
		for (k = 0; k < 8; k++)
		{
			if (rl & 1)
			{
				switch(opsize)
				{
					case WORD : readmem(WORD,ptr);
						ptr += 2;
						if (memword & 0x8000)
							a_reg[k] = 0xffff0000 | memword;
						else
							a_reg[k] = memword;
						break;
					case LONG : readmem(LONG,ptr);
						ptr += 4;
						a_reg[k] = memlong;
						break;
				}
			}
			rl >>= 1;
		}
		if (amode == POST)
			a_reg[inst & 7] = ptr;
	}
	else  /* write D0-D7/A0-A7 to memory */
	{
		first = TRUE;
		if (amode != PRE)
		{
			if (!ingo) rlist('D',rl & 0xff);
			if (!ingo) rlist('A', (rl >> 8) & 0xff);
		}
		else   /* PRE */
		{
			if (!ingo) rlist('D',bitswap((rl >> 8) & 0xff));
			if (!ingo) rlist('A',bitswap(rl & 0xff));
		}
		if (!ingo) strcat(ops,",");
		pushpc;
		addr_mode(inst);
		poppc;
		if (!exflag) return;  //10.1.96
		pushAn();
		read_src(inst);
		ptr = srcaddr;
		popAn();
		if (amode == PRE)
		{
			switch(opsize)
			{
				case WORD : ptr += 2; break;
				case LONG : ptr += 4; break;
			}
		}
		//if (amode == PRE)   //9.17.96 removed to fix -(An) mode
		//{
		//        prepat = bitswap((rl >> 8) & 0xff);
		//        rl = (bitswap(rl & 0xff) << 8) | prepat;
		//}
		for (k = 0; k <= 7; k++)
		{
			if (rl & 1)
			{
				switch(opsize)
				{
				case WORD: if (amode == PRE)
					{
						srcword = a_reg[7-k] & 0xffff;
						ptr -= 2;
					}
					else
						srcword = d_reg[k] & 0xffff;
					writemem(WORD,ptr);
					if (amode != PRE) ptr += 2;
					break;
				case LONG: if (amode == PRE)
					{
						srclong = a_reg[7-k];
						ptr -= 4;
					}
					else
						srclong = d_reg[k];
					writemem(LONG,ptr);
					if (amode != PRE) ptr += 4;
					break;
				}
			}
			rl >>= 1;
		}
		for (k = 0; k <= 7; k++)
		{
			if (rl & 1)
			{
				switch(opsize)
				{
				case WORD: if (amode == PRE)
					{
						srcword = d_reg[7-k] & 0xffff;
						ptr -= 2;
					}
					else
						srcword = a_reg[k] & 0xffff;
					writemem(WORD,ptr);
					if (amode != PRE) ptr += 2;
					break;
				case LONG: if (amode == PRE)
					{
						srclong = d_reg[7-k];
						ptr -= 4;
					}
					else
						srclong = a_reg[k];
					writemem(LONG,ptr);
					if (amode != PRE) ptr += 4;
					break;
				}
			}
			rl >>= 1;
		}
		if (amode == PRE)
			a_reg[inst & 7] = ptr;
	}
}

void trace()
{
	int num;
	unsigned char kyb;

	if (',' == cmd[cptr])
		cptr++;
	else
	if ('\0' != cmd[cptr])
		pc = tohex(&cmd[cptr]) & 0xffffff;
	skipblank();
	if (',' == cmd[cptr])
		cptr++;
	skipblank();
	num = 1;
	if ('\0' != cmd[cptr])
		num = tohex(&cmd[cptr]) & 0xffff;
	stopem = FALSE;
	while((num > 0) && !stopem)
	{
		fetch(EX);
		disp_reg();
		num--;
		if (kbhit())          //2.27.98
		{
			kyb = getch();
			if (kyb == 0x15)
				break;
		}
	}
}

void loadX(int exp)
{
 if(exp){sr|=0x10;}else{sr&=0xffef;}
}

void loadN(int exp)
{
 if(exp){sr|=0x08;}else{sr&=0xfff7;}
}

void loadZ(int exp)
{
 if(!exp){sr|=0x04;}else{sr&=0xfffb;}
}

void loadV(int exp)
{
 if(exp){sr|=0x02;}else{sr&=0xfffd;}
}

void loadC(int exp)
{
 if(exp){sr|=0x01;}else{sr&=0xfffe;}
}
