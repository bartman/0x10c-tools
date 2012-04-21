#!/usr/bin/lua

package.path = './lua/?.lua;' .. package.path
local lpeg = require 'lpeg'
local D = require 'dcpu16'

require 'dumper'
function dump(...)
    print(DataDumper(...))
end

function die(...)
    io.stdout:flush()
    io.stderr:write(...)
    os.exit(1)
end

function parse(d, file)

    local f = assert(io.open(file))
    local program = f:read'*all'
    local res, suc, msg = d:newparse(program)
    f:close()

    if msg then print("\n"..msg.."\n") end
    if not suc then os.exit(1) end

    return res
end

function assemble(d, prog)
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

    local function assemble_isn_arg(arg, isn, mult)

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


    local function assemble_gisn(opcode, isn, a, b)
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

    local function assemble_xisn(opcode, isn, a)
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

    local pc = 0
    local memory = {}

    local function handle_label(block)
    end
    local function handle_data(block)
    end
    local function handle_op(block)
    end

    for i,v in ipairs(prog) do
        dump(v, "")
        if v.label then
            handle_label(v)
        end
        if v.data then
            handle_data(v)
        end
        if v.op then
            handle_op(v)
        end
    end

    return true
end




if #arg ~= 1 then
    die'provide a single file'
end

local d = D.new()
local prog = parse(d, arg[1])
local ret = assemble(d, prog)













--[[

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
]]--
