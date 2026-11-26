#ASM
start1: ld bc, 1234
        ret
end1:
#ENDASM
    PRINT USR &start1
    LPRINT USR &start1
    PAUSE 3*50
    RAND USR 8191

#AUTOSTART=1
