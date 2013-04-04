
void save_context(int cont_num)
{
     int reg;

     for(reg = 0; reg <= 7; reg++)
     {
	 contexts[cont_num].data_reg[reg] = d_reg[reg];
	 contexts[cont_num].address_reg[reg] = a_reg[reg];
     }
   
	 contexts[].pcounter = pc;
	 contexts[].sreg = sr;
	 contexts[].wXflag = XF;
	 contexts[].wNflag = NF;
	 contexts[].wNflag = ZF;
	 contexts[].wZflag = VF;
	 contexts[].wCflag = CF;
}

void load_context(int cont_num)
{
     int reg;

     for(reg = 0; reg <= 7; reg++)
     {
		d_reg[reg] = contexts[cont_num].data_reg[reg];
		a_reg[reg] = contexts[cont_num].address_reg[reg];
     }
   
	 pc = contexts[].pcounter;
	 sr = contexts[].sregsr;
	 XF = contexts[].wXflag;
	 NF = contexts[].wNflag;
	 ZF = contexts[].wNflag;
	 VF = contexts[].wZflag;
	 CF = contexts[].wCflag;
}