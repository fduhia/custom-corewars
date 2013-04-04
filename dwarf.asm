           ORG         $0200

START

HERE        TRAP        #10
	        bclr	    #$0,D0
	        move.l	    D0,A1
	        move.l	    #$FFFF,(A1)
	        JSR	        HERE

           END         START