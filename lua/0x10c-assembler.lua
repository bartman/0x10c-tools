#!/usr/bin/lua

package.path = './lua/?.lua;' .. package.path
local DP = require 'dcpu16.parser'
local DD = require 'dcpu16.defs'
require 'dcpu16.util'

require 'dumper'
local function dump(...)
    print(DataDumper(...))
end


-- parse a file into a parse tree

local function parse(d, file)

    local f = assert(io.open(file))
    local program = f:read'*all'
    f:close()

    local res, suc, msg = d:newparse(program)

    if msg then print("\n"..msg.."\n") end
    if not suc then os.exit(1) end

    return res
end

function assemble(d, prog)
    -- collecting things about the assembly
    local pc = 0
    local memory = {}
    local labels = {}

    -- memory allocation and fill

    local function mem_alloc(count)
        local ofs = pc
        count = count or 1
        pc = pc + count
        return ofs
    end

    local function mem_append(...)
        local ofs = pc
        local count = 0
        for i,v in ipairs({...}) do
            memory[pc] = v
            pc = pc + 1
            count = count + 1
        end
        return ofs, count
    end

    local function mem_section(ofs, len)
        local s = {}
        for i=0,(len-1) do
            table.insert(s, memory[ofs + i])
        end
        return s
    end

    -- update isn object with compiled words
    local function assemble_isn_arg(arg, isn, mult)

        local num = 0

        dbgf(1,"  >> arg=%s\n", DataDumper(arg,""))

        if arg.greg then
            local gr = DD.generic_registers[arg.greg]
            dbg(3,"", "greg", arg.greg)
            if not gr then
                die("don't know how to encode reg '"..(arg.greg).."'")
            end
            num = gr.num

        elseif arg.sreg then
            local sr = DD.special_registers[arg.sreg]
            dbg(3,"", "sreg", arg.sreg)
            if not sr then
                die("don't know how to encode reg '"..(arg.sreg).."'")
            end
            num = sr.num

        elseif arg.num ~= nil then
            dbg(3,"", "num", arg.num)
            local n = tonumber(arg.num)
            if n >= 0 and n < 0x20 then
                num = n + 0x20
            else
                num = 0x1f
                mem_append(n)
            end

        elseif arg.var then
            dbg(3,"", "var", arg.var)

            local label_used_at = mem_alloc(1)

            local label_offset = labels[arg.var]
            if label_offset ~= nil then
                -- label location already known
                memory[label_used_at] = label_offset
            else
                -- label location not yet known, do it later
                isn.finalize = function(self)
                    label_offset = labels[arg.var]
                    if label_offset ~= nil then
                        memory[label_used_at] = label_offset
                    else
                        die(string.format("could not resolve variable/label '%s'", arg.var))
                    end
                end
            end

            num = 0x1f

        elseif arg.mref then
            local gr = nil
            if arg.mref.greg then
                gr = DD.generic_registers[arg.mref.greg]
                if not gr then
                    die("don't know how to encode reg '"..(arg.mref.greg).."' of "..DataDumper(arg,""))
                end
            end

            if gr and arg.mref.num then

                num = 0x10 + gr.num
                mem_append(arg.mref.num)

            elseif gr then

                num = 0x08 + gr.num

            elseif arg.mref.num then

                num = 0x1e
                mem_append(arg.mref.num)

            else
                die("don't know how to encode mref "..DataDumper(arg.mref,""))
            end
        else
            die(DataDumper(arg, "didn't know how to handle arg: "))
        end

        -- this is really op |= num << shift, but lua lacks bit ops
        memory[isn.offset] = memory[isn.offset] + (num * mult)
    end


    local function assemble_gisn(num, isn)
        dbg(2,"", (isn.op)..'('..xx(num)..')', a, b)

        isn.offset, isn.length = mem_append(num)
        isn.finalize = nil

        assemble_isn_arg(isn.a, isn, 16)    -- 16 to shift by 4 bits
        assemble_isn_arg(isn.b, isn, 1024)  -- 1024 to shift by 10 bits

        dbg(1,">> ".. table.concat(lmap(function(n)
            return string.format("0x%04x", n)
        end, mem_section(isn.offset, isn.length)), ' '), "\n")
    end

    local function assemble_xisn(num, isn)
        dbg(2,"", (isn.op)..'('..xx(num)..')', a, b)

        isn.offset, isn.length = mem_append(num)
        isn.finalize = nil

        assemble_isn_arg(isn.a, isn, 1024)    -- 1024 to shift by 10 bits

        dbg(1,">> ".. table.concat(lmap(function(n)
            return string.format("0x%04x", n)
        end, mem_section(isn.offset, isn.length)), ' '))
    end

    -- top level handers

    local function handle_label(block)
        dbgf(1, "pc=0x%04x label '%s'\n", pc, block.label)
        if labels[block.label] ~= nil then
            die(string.format("label '%s' already exists at 0x%04x", block.label, labels[block.label]))
        end
        labels[block.label] = pc
    end
    local function handle_data(block)
        dbgf(1, "pc=0x%04x data '%s'\n", pc, DataDumper(block.data, ""))
        for i,datum in ipairs(block.data) do
            dbgf(1, "  > %s\n", DataDumper(datum, ""))
            if datum.num then
                mem_append(datum.num)
            elseif datum.str then
                local bytes = { string.byte(datum.str,1,datum.str:len()) }
                for i,byte in ipairs(bytes) do
                    mem_append(byte)
                end
            else
                die(DataDumper(datum, "don't know how to handle data: "))
            end
        end
    end
    local function handle_op(block)
        dbgf(1, "pc=0x%04x op '%s'\n", pc, DataDumper({block.op,block.a,block.b}, ""))

        local op = DD.generic_opcodes[block.op]
        if op ~= nil then
                assemble_gisn(op.num, block)
        else
            op = DD.extension_opcodes[block.op]
            if op ~= nil then
                    assemble_xisn(op.num, block)
            else
                    die("unknown opcode: "..block.op)
            end
        end

    end

    for i,block in ipairs(prog) do
        dbg(2, DataDumper(block, ""))
        if block.label then
            handle_label(block)
        end
        if block.data then
            handle_data(block)
        end
        if block.op then
            handle_op(block)
        end
    end

    for i,block in ipairs(prog) do
        if block.finalize then
            block:finalize()
        end
    end


    local out = io.stdout

    dbgf(1, "pc=0x%04x end", pc)
    dbgf(1, "#memory=%u", #memory)

    local o=0
    while o <= #memory do
        out:write(xxxx(o)..':')
        for i = 0,7 do
            if memory[o+i] then
                out:write(' '..xxxx(memory[o+i]))
            end
        end
        out:write("\n")
        o = o + 8
    end

    return true
end




if #arg ~= 1 then
    die'provide a single file'
end

local d = DP.new()
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
