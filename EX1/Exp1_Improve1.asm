        ORG     0000H
        LJMP    MAIN

        ORG     0100H
MAIN:   MOV     DPTR, #SQR
        MOV     A, 20H
        MOVC    A, @A+DPTR
        MOV     21H, A
        SJMP    $

SQR:    DB      00H,01H,04H,09H,16H
        DB      25H,36H,49H,64H,81H
        END
