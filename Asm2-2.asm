        ORG     0000H
        LJMP    MAIN

        ORG     0100H
MAIN:   MOV     R0, #22H
        MOV     A, R0
        PUSH    ACC
        MOV     R7, #03H
        CLR     A

LP1:    MOV     @R0, A
        DEC     R0
        DJNZ    R7, LP1

        POP     ACC
        MOV     R0, A
        MOV     R7, #16

LP2:    PUSH    ACC
        CLR     C

        MOV     A, R4
        RLC     A
        MOV     R4, A

        MOV     A, R3
        RLC     A
        MOV     R3, A

        MOV     B, #03H

LP3:    MOV     A, @R0
        ADDC    A, @R0
        DA      A
        MOV     @R0, A
        DEC     R0
        DJNZ    B, LP3

        POP     ACC
        MOV     R0, A
        DJNZ    R7, LP2

        SJMP    $
        END
