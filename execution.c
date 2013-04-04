

void execute_single()
{
	int num;

//pc = tohex(&cmd[cptr]) & 0xffffff;

//	if ('\0' != cmd[cptr])
	num = 1;
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
