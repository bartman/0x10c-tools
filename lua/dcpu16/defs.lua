#!/usr/bin/lua

-- this module creates a cpu16.defs class, here called DD

local DD = {}

DD.generic_registers = {
        A={ num=0 },
        B={ num=1 },
        C={ num=2 },
        X={ num=3 },
        Y={ num=4 },
        Z={ num=5 },
        I={ num=6 },
        J={ num=7 },
}

DD.special_registers = {
        POP ={ num=0x18 },
        PEEK={ num=0x19 },
        PUSH={ num=0x1a },
        SP  ={ num=0x1b },
        PC  ={ num=0x1c },
        O   ={ num=0x1d },
}

DD.generic_opcodes = {
        SET={ num=0x1 },
        SET={ num=0x1 },
        ADD={ num=0x2 },
        SUB={ num=0x3 },
        MUL={ num=0x4 },
        DIV={ num=0x5 },
        MOD={ num=0x6 },
        SHL={ num=0x7 },
        SHR={ num=0x8 },
        AND={ num=0x9 },
        BOR={ num=0xa },
        XOR={ num=0xb },
        IFE={ num=0xc },
        IFN={ num=0xd },
        IFG={ num=0xe },
        IFB={ num=0xf },
}

DD.extension_opcodes = {
        JSR={ num=0x1 },
}


return DD
