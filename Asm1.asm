        ORG     0000H
        LJMP    MAIN

        ORG     0100H
MAIN:   MOV     R0, #30H
        MOV     R7, #10H
        CLR     A

LOOP:   MOV     @R0, A
        INC     A
        INC     R0
        DJNZ    R7, LOOP

        SJMP    $
        END
