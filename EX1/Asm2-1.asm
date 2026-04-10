        ORG     0000H
        LJMP    MAIN

        ORG     0100H
MAIN:   MOV     R0, #20H
        MOV     R7, #03H
        CLR     A
        MOV     R4, A

LP1:    MOV     A, R4
        MOV     B, #0AH
        MUL     AB
        ADD     A, @R0
        INC     R0
        MOV     R4, A
        DJNZ    R7, LP1

        SJMP    $
        END
