        ORG     0000H
        LJMP    MAIN

        ORG     0100H
MAIN:   MOV     A, #01H        ; ??????? LED

LOOP:   MOV     P1, A          ; ??? P1 ?
        ACALL   DELAY
        RL      A              ; ?????
        SJMP    LOOP

DELAY:  MOV     R7, #0FFH
D1:     MOV     R6, #0FFH
D2:     DJNZ    R6, D2
        DJNZ    R7, D1
        RET

        END
