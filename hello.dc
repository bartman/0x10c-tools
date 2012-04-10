; http://img.wonderhowto.com/images/gfx/gallery/l634691801349533933.jpg

; Assembler test for DCPU
; by Markus Persson

:start
        set i, 0
        set j, 0
        set b, 0xf100
:nextchar
        set a, (data+i)
        ife a, 0
            set PC, end
        ifg a, 0xff
            set PC, setcolor
        bor a, b
        set (0x8000+j), a
        add i, 1
        add j, 1
        set PC, nextchar

:setcolor
        set b, a
        and b, 0xff
        shl b, 8
        ifg a, 0x1ff
            add b, 0x80
        add i, 1
        set PC, nextchar


:data
        dat 0x170, "Hello ", 0x2e1, "world", 0x170, ", how are you?", 0

:end
        set PC, end
