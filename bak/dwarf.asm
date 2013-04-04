           ORG         $0200

label       TRAP        #10
	        bclr	    #$0,D0
	        move.l	    D0,A1
	        move.l	    #$FFFF,(A1)
	        bra	        label

           END         