        ORG     0000H
        LJMP    MAIN

        ORG     0100H
MAIN:   MOV     DPTR, #SQR
        MOV     A, R7
        MOVC    A, @A+DPTR
        MOV     R6, A
        SJMP    $

SQR:    DB      00H,01H,04H,09H,16H
        DB      25H,36H,49H,64H,81H
        END
