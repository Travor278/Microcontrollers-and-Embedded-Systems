        ORG     0000H
        LJMP    MAIN

        ORG     0100H
MAIN:   MOV     DPTR, #ASCTAB
        MOV     A, R7
        ANL     A, #0FH
        MOVC    A, @A+DPTR
        MOV     R5, A

        MOV     A, R7
        ANL     A, #0F0H
        SWAP    A
        MOVC    A, @A+DPTR
        MOV     R6, A

        SJMP    $

ASCTAB: DB      30H,31H,32H,33H,34H
        DB      35H,36H,37H,38H,39H
        DB      41H,42H,43H,44H,45H,46H
        END
