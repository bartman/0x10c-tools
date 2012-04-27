#!/usr/bin/lua

-- this module creates a cpu16.defs class, here called DD

local DD = {}

DD.generic_registers = {
        A={ num=0x00, mref_solo=0x08, mref_ofs=0x10, },
        B={ num=0x01, mref_solo=0x09, mref_ofs=0x11, },
        C={ num=0x02, mref_solo=0x0a, mref_ofs=0x12, },
        X={ num=0x03, mref_solo=0x0b, mref_ofs=0x13, },
        Y={ num=0x04, mref_solo=0x0c, mref_ofs=0x14, },
        Z={ num=0x05, mref_solo=0x0d, mref_ofs=0x15, },
        I={ num=0x06, mref_solo=0x0e, mref_ofs=0x16, },
        J={ num=0x07, mref_solo=0x0f, mref_ofs=0x17, },
}

DD.special_registers = {
        POP ={ num=0x18, a=1,                                            },    -- [SP++]
        PUSH={ num=0x18,      b=1                                        },    -- [--SP]
        PEEK={ num=0x19, a=1, b=1                                        },    -- [SP]
        --PICK={ num=0x1a, a=1, b=1                                        },    -- [SP + next word]
        SP  ={ num=0x1b, a=1, b=1, mref=1, mref_solo=0x19, mref_ofs=0x1a },
        PC  ={ num=0x1c, a=1, b=1                                        },
        EX  ={ num=0x1d, a=1, b=1                                        },
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
        MDI={ num=0x09, cycles=3 },
        AND={ num=0x0a, cycles=1 },
        BOR={ num=0x0b, cycles=1 },
        XOR={ num=0x0c, cycles=1 },
        SHR={ num=0x0d, cycles=1 },
        ASR={ num=0x0e, cycles=1 },
        SHL={ num=0x0f, cycles=1 },
        IFB={ num=0x10, cycles=2 },
        IFC={ num=0x11, cycles=2 },
        IFE={ num=0x12, cycles=2 },
        IFN={ num=0x13, cycles=2 },
        IFG={ num=0x14, cycles=2 },
        IFA={ num=0x15, cycles=2 },
        IFL={ num=0x16, cycles=2 },
        IFU={ num=0x17, cycles=2 },
        --        0x18
        --        0x19
        ADX={ num=0x1a, cycles=3 },
        SBX={ num=0x1b, cycles=3 },
        --        0x1c
        --        0x1d
        STI={ num=0x1e, cycles=2 },
        STD={ num=0x1e, cycles=2 },
}

DD.special_opcodes = {
        JSR={ num=0x01, cycles=3 },

        HCF={ num=0x07, cycles=9 },

        INT={ num=0x08, cycles=4 },
        IAG={ num=0x09, cycles=1 },
        IAS={ num=0x0a, cycles=1 },

        RFI={ num=0x09, cycles=3 },
        IAQ={ num=0x0a, cycles=2 },

        HWN={ num=0x10, cycles=2 },
        HWQ={ num=0x11, cycles=4 },
        HWI={ num=0x12, cycles=4 },
}


return DD
