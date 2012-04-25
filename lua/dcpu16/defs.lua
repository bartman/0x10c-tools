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
        POP ={ num=0x18, a=1,             },    -- [SP++]
        PUSH={ num=0x18,      b=1         },    -- [--SP]
        PEEK={ num=0x19, a=1, b=1         },    -- [SP]
        PICK={ num=0x1a, a=1, b=1         },    -- [SP + next word]
        SP  ={ num=0x1b, a=1, b=1, mref=1 },
        PC  ={ num=0x1c, a=1, b=1         },
        EX  ={ num=0x1d, a=1, b=1         },
}

function DD.special_regs_marked_with(self, tag)
        return tmap(
            function(r)
                    if r[tag] then
                            return r
                    end
            end, self.special_registers)
end

DD.generic_opcodes = {
        SET={ num=0x01, cycles=1 },
        ADD={ num=0x02, cycles=2 },
        SUB={ num=0x03, cycles=2 },
        MUL={ num=0x04, cycles=2 },
        MLI={ num=0x05, cycles=2 },
        DIV={ num=0x06, cycles=3 },
        DVI={ num=0x07, cycles=3 },
        MOD={ num=0x08, cycles=3 },
        AND={ num=0x09, cycles=1 },
        BOR={ num=0x0a, cycles=1 },
        XOR={ num=0x0b, cycles=1 },
        SHR={ num=0x0c, cycles=2 },
        ASR={ num=0x0d, cycles=2 },
        SHL={ num=0x0e, cycles=2 },
        --        0x0f
        IFB={ num=0x10, cycles=2 },
        IFC={ num=0x11, cycles=2 },
        IFE={ num=0x12, cycles=2 },
        IFN={ num=0x13, cycles=2 },
        IFG={ num=0x14, cycles=2 },
        IFA={ num=0x15, cycles=2 },
        IFL={ num=0x16, cycles=2 },
        IFU={ num=0x17, cycles=2 },
}

DD.special_opcodes = {
        JSR={ num=0x01, cycles=3 },

        INT={ num=0x08, cycles=4 },
        INT={ num=0x09, cycles=1 },
        INT={ num=0x0a, cycles=1 },

        HWN={ num=0x10, cycles=2 },
        HWQ={ num=0x11, cycles=4 },
        HWQ={ num=0x12, cycles=4 },
}


return DD
