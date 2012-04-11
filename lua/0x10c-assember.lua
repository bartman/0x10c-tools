#!/usr/bin/lua

local lpeg = require 'lpeg'
local locale = lpeg.locale();
local P, R, S, C = lpeg.P, lpeg.R, lpeg.S
local C, Cc, Ct, Cg = lpeg.C, lpeg.Cc, lpeg.Ct, lpeg.Cg

package.path = './lua/?.lua;' .. package.path
local D = require 'dcpu16'


local debug_level = 0
local function dbg(l,...)
        if debug_level >= l then
                io.stderr:write("# "..table.concat({...},"\t").."\n")
        end
end
local function dbgf(l,fmt,...)
        if debug_level >= l then
                io.stderr:write("# "..string.format(fmt,...).."\n")
        end
end
local function die(...)
        io.stderr:write("ERROR: "..table.concat({...},"\t").."\n")
        os.exit(1)
end
local function lmap(func, array)
        local new_array = {}
        for i,v in ipairs(array) do
                new_array[i] = func(v)
        end
        return new_array
end
local function tmap(func, array)
        local new_array = {}
        for k,v in pairs(array) do
                new_array[k] = func(v)
        end
        return new_array
end
local function xx(num)
        return string.format("%02x", num)
end
local function xxxx(num)
        return string.format("%04x", num)
end

-- assembler

local assembler = {
        pc = 0,            -- current program counter
        instructions = {}, -- { { ofs=num, isn={}, words={}, final=bool } ... }
        vars = {},         -- { name = ofs, ... }

        append = function(self,isn)
                isn.ofs = self.pc
                table.insert(self.instructions, isn)
                self.pc = self.pc + #isn.words
        end,

        finalize = function(self)

                dbg(1, 'finalize...')

                self.instructions = tmap(function(isn)
                        if isn.final then return isn end

                        dbg(1, 'not final: '..table.concat(isn.isn,','))

                        local new = isn.opcode.assemble(isn.opcode, isn.isn[1], isn.isn[2], isn.isn[3])

                        isn.words = new.words

                        return isn
                end, self.instructions)
                
        end,

        dump = function(self, file)
                dbg(1, 'dump...')

                for k,isn in pairs(self.instructions) do
                        for var,vofs in pairs(self.vars) do
                                if isn.ofs == vofs then
                                        dbgf(1, "       :%s", var)
                                end
                        end
                        dbgf(1, "%04x: %4s %-20s ; %s",
                            isn.ofs,
                            isn.isn[1],
                            table.concat({isn.isn[2],isn.isn[3]},', '),
                            table.concat(lmap(function(num) return xxxx(num) end, isn.words), ' ')
                        )
                end
        end,

        output = function(self, filename)
                dbg(1, 'output...')

                local f

                if filename == '-' then
                        f = io.stdout
                else
                        f = assert(io.open(filename, "w+b"))
                end

                local words = {}

                for k,isn in pairs(self.instructions) do
                        for i = 1, #isn.words do
                                words[#words+1] = isn.words[i]
                        end
                end

                local o=0
                while o < #words do
                        f:write(xxxx(o)..':')
                        for i = 1,8 do
                                if words[o+i] then
                                        f:write(' '..xxxx(words[o+i]))
                                end
                        end
                        f:write("\n")
                        o = o + 8
                end

                if filename ~= '-' then
                        f:close()
                end
        end,
}

local generic_registers = {
        A={ num=0 },
        B={ num=1 },
        C={ num=2 },
        X={ num=3 },
        Y={ num=4 },
        Z={ num=5 },
        I={ num=6 },
        J={ num=7 },
}

local special_registers = {
       POP ={ num=0x18 },
       PEEK={ num=0x19 },
       PUSH={ num=0x1a },
       SP  ={ num=0x1b },
       PC  ={ num=0x1c },
       O   ={ num=0x1d },
}

function assemble_isn_arg(arg, isn, mult)

        local num = 0

        local rc = lpeg.match(
        ( D.greg / function()
                local gr = generic_registers[arg]
                dbg(3,"", "greg", arg)
                if not gr then
                        die("don't know how to encode reg '"..arg.."'")
                end
                num = gr.num
        end
        + D.sreg / function()
                local sr = special_registers[arg]
                dbg(3,"", "sreg", arg)
                if not sr then
                        die("don't know how to encode reg '"..arg.."'")
                end
                num = sr.num
        end
        + D.numlit / function()
                dbg(3,"", "numlit", arg)
                local n = tonumber(arg)
                if n >= 0 and n < 0x20 then
                        num = n + 0x20
                else
                        num = 0x1f
                        isn.words[#isn.words + 1] = n
                end
        end
        + D.variable / function()
                dbg(3,"", "variable", arg)

                local ofs = assembler.vars[arg]
                if not ofs then
                        -- record unresolved address
                        isn.final = false
                        ofs = 0 -- don't know yet where it is
                end

                num = 0x1f
                isn.words[#isn.words + 1] = ofs
        end
        + D.mref / function()
                local t = lpeg.match(D.mref_ct, arg)
                if not t then
                        die("don't know how to encode mref '"..arg.."'")
                end

                local gr = nil
                if t.reg then
                        gr = generic_registers[t.reg]
                        if not gr then
                                die("don't know how to encode reg '"..t.reg.."' of '"..arg.."'")
                        end
                end

                if t.reg and t.numlit then

                        num = 0x10 + gr.num
                        isn.words[#isn.words + 1] = t.numlit

                elseif t.reg then

                        num = 0x08 + gr.num

                elseif t.numlit then

                        num = 0x1e
                        isn.words[#isn.words + 1] = t.numlit

                else
                        die("don't know how to encode mref '"..arg.."'")
                end
        end)
        , arg)

        if not rc then
                die("failed to parse argument '"..arg.."'")
        end

        -- this is really op |= num << shift, but lua lacks bit ops
        isn.words[1] = isn.words[1] + (num * mult)
end


function assemble_gisn(opcode, isn, a, b)
        dbg(2,"", isn..'('..xx(opcode.num)..')', a, b)
        
        local isn = {
                opcode = opcode,
                isn = { isn, a, b },
                words = { opcode.num },
                final = true
        }

        assemble_isn_arg(a, isn, 16)    -- 16 to shift by 4 bits
        assemble_isn_arg(b, isn, 1024)  -- 1024 to shift by 10 bits

        dbg(1,">> ".. table.concat(lmap(function(n)
                return string.format("0x%04x", n)
        end, isn.words), ' '))

        return isn
end

local generic_opcodes = {
        SET={ num=0x1, assemble=assemble_gisn },
        SET={ num=0x1, assemble=assemble_gisn },
        ADD={ num=0x2, assemble=assemble_gisn },
        SUB={ num=0x3, assemble=assemble_gisn },
        MUL={ num=0x4, assemble=assemble_gisn },
        DIV={ num=0x5, assemble=assemble_gisn },
        MOD={ num=0x6, assemble=assemble_gisn },
        SHL={ num=0x7, assemble=assemble_gisn },
        SHR={ num=0x8, assemble=assemble_gisn },
        AND={ num=0x9, assemble=assemble_gisn },
        BOR={ num=0xa, assemble=assemble_gisn },
        XOR={ num=0xb, assemble=assemble_gisn },
        IFE={ num=0xc, assemble=assemble_gisn },
        IFN={ num=0xd, assemble=assemble_gisn },
        IFG={ num=0xe, assemble=assemble_gisn },
        IFB={ num=0xf, assemble=assemble_gisn },
}

function assemble_xisn(opcode, isn, a)
        dbg(2,"", isn..'('..xx(opcode.num)..')', a, b)
        
        local isn = {
                opcode = opcode,
                isn = { isn, a },
                words = { opcode.num * 16 },
                final = true
        }

        assemble_isn_arg(a, isn, 1024)    -- 1024 to shift by 10 bits

        dbg(1,">> ".. table.concat(lmap(function(n)
                return string.format("0x%04x", n)
        end, isn.words), ' '))

        return isn
end

local extension_opcodes = {
        JSR={ num=0x1, assemble=assemble_xisn },
}


local assembler_actions = {
        comment = function(...) end,
        gisn = function(...)
                local args = {...}

                dbg(1,"GISN",...)

                if #args ~= 3 then
                        die("expecting 2 arguments for '"..args[1].."' in '"..(...).."'")
                end

                local opcode = generic_opcodes[args[1]]
                if not opcode then
                        die("don't know opcode '"..args[1].."' in '"..(...).."'")
                end

                local isn = opcode.assemble(opcode, args[1], args[2], args[3])
                assembler:append(isn)
                dbg(1)
        end,
        xisn = function(...)
                local args = {...}

                dbg(1,"XISN",...)

                if #args ~= 2 then
                        die("expecting 1 argument for '"..args[1].."' in '"..(...).."'")
                end

                local opcode = extension_opcodes[args[1]]
                if not opcode then
                        die("don't know opcode '"..args[1].."' in '"..(...).."'")
                end

                local isn = opcode.assemble(opcode, args[1], args[2])
                assembler:append(isn)
                dbg(1)
        end,
        label = function(...)
                local args = {...}

                dbg(1,"LABEL",...)

                if #args ~= 1 then
                        die("expecting 0 additional arguments in '"..(...).."'")
                end

                local label = args[1]

                if assembler.vars[label] then
                        die("duplicate label found '"..label.."'")
                end

                assembler.vars[label] = assembler.pc
                dbg(1)
        end
}

assembler.matcher = ( D.comment       / assembler_actions.comment
                    + D.gisn_c        / assembler_actions.gisn
                    + D.xisn_c        / assembler_actions.xisn
                    + D.label_c       / assembler_actions.label
                    + D.whitespace
                    )^0

-- parse command line

local filename
local handler
local output='-'

local i = 1
while i<=#arg do
        local a = arg[i]
        i = i + 1
        if a == "-h" or a == "--help" then
                print '0x10c'
                print ''
                print ' -h --help                         - this help'
                print ' -v --verbose                      - be more verbose'
                print ' -a --assemble <file>              - run assembler a file'
                print ' -o --output <file>                - write output here'
                os.exit(0)

        elseif a == "-v" or a == "--verbose" then
                debug_level = debug_level + 1

        elseif a == "-a" or a == "--assemble" then
                handler = assembler
                filename = arg[i]
                i = i + 1

        elseif a == "-o" or a == "--output" then
                output = arg[i]
                i = i + 1

        else
                die("unknown option " .. a)
        end
end

if not handler then die("need to specify an action; see --help") end
if not filename then die("need to specify a file") end


-- frontend

local fh = assert(io.open(filename))
local input = fh:read'*a'
fh:close()
local rc = lpeg.match(handler.matcher, input)
if (rc < input:len()) then
        print("ERROR: parser filed here...")
        print(input:sub(rc))
end

handler:dump()
handler:finalize()
handler:output(output)

