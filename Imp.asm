
    ORG    $0200
main
	LEA		main,A2
	LEA		EndProgram,A3
	move.l	#790,D1
StartLoop
	move.l	(A2)+,(A3)+
	subq	#$1,D1
	tst.l	D1
	bne		StartLoop
	nop
EndProgram


    END
