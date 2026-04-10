        ORG     0000H

        LJMP    MAIN

        ORG     0100H
MAIN:   MOV     P1, #00H        ; LED ??
        MOV     P2, #0FFH       ; P2 ???,??? 1
        MOV     30H, #01H       ; ?????
        MOV     31H, #80H       ; ?????
        MOV     32H, #55H       ; ??????

LOOP:   MOV     A, P2           ; ???
        CPL     A               ; ?????,?????? 1
        ANL     A, #03H         ; ?? P2.0 / P2.1

        CJNE    A, #01H, CHECK_R
        ACALL   LEFT_STEP
        SJMP    LOOP

CHECK_R:
        CJNE    A, #02H, CHECK_F
        ACALL   RIGHT_STEP
        SJMP    LOOP

CHECK_F:
        CJNE    A, #03H, IDLE
        ACALL   FLASH_STEP
        SJMP    LOOP

IDLE:   MOV     P1, #00H
        SJMP    LOOP

LEFT_STEP:
        MOV     A, 30H
        MOV     P1, A
        ACALL   DELAY
        RL      A
        MOV     30H, A
        RET

RIGHT_STEP:
        MOV     A, 31H
        MOV     P1, A
        ACALL   DELAY
        RR      A
        MOV     31H, A
        RET

FLASH_STEP:
        MOV     A, 32H
        MOV     P1, A
        ACALL   DELAY
        CPL     A
        MOV     32H, A
        RET

DELAY:  MOV     R7, #20H
D1:     MOV     R6, #0FFH
D2:     DJNZ    R6, D2
        DJNZ    R7, D1
        RET

        END
