#!/usr/bin/lua

-- this module creates a cpu16.assembler class, here called DA

local DA = {}

local DD = require 'dcpu16.defs'
require 'dcpu16.util'

------------------------------------------------------------------------
-- private assembler state object

local DAS = {}
function DAS.new()

        local t = { }

        -- initializer

        function t.init(self)
                self.pc = 0;
                self.memory = {};
                self.labels = {};
                self.programs = {};
                return self
        end
        
        function t.reset_labels(self)
                self.labels = {};
                return self
        end
        
        -- memory allocation and fill

        function t.mem_alloc(self, count)
                local ofs = self.pc
                count = count or 1
                self.pc = self.pc + count
                return ofs
        end

        function t.mem_append(self, ...)
                local ofs = self.pc
                local count = 0
                for i,v in ipairs({...}) do
                        self.memory[self.pc] = v
                        self.pc = self.pc + 1
                        count = count + 1
                end
                return ofs, count
        end

        function t.mem_section(self, ofs, len)
                local s = {}
                for i=0,(len-1) do
                        table.insert(s, self.memory[ofs + i])
                end
                return s
        end

        return t:init()
end


------------------------------------------------------------------------
-- create a new instance of the assembler, in a private closure

function DA.new()

        -- the new class and private variables
        local t = {}
        local s = DAS.new()

        -- update isn object with compiled words
        local function assemble_isn_arg(arg, isn, AorB, mult)

                local num = 0

                dbgf(1,"  >> arg=%s", DataDumper(arg,""))

                if arg.greg then
                        dbg(3,"", "greg", arg.greg)

                        local gr = DD.generic_registers[arg.greg]
                        if not gr then
                                die("don't know how to encode reg '"..(arg.greg).."'")
                        end
                        num = gr.num

                elseif arg.sreg then
                        dbg(3,"", "sreg", arg.sreg)

                        local sr = DD.special_registers[arg.sreg]
                        if not sr then
                                die("don't know how to encode reg '"..(arg.sreg).."'")
                        end
                        if not sr[AorB] then
                                die("special register '"..(arg.sreg).."' not allowed as '"..(AorB).."'")
                        end
                        num = sr.num

                elseif arg.num ~= nil then
                        dbg(3,"", "num", arg.num)

                        local n = tonumber(arg.num)
                        if AorB == 'a' and n >= -1 and n <= 0x1e then
                                num = n + 0x21
                        else
                                num = 0x1f
                                s:mem_append(n)
                                isn.length = isn.length + 1
                        end

                elseif arg.var then
                        dbg(3,"", "var", arg.var)

                        local label_used_at = s:mem_alloc(1)
                        isn.length = isn.length + 1

                        local label_offset = s.labels[arg.var]
                        if label_offset ~= nil then
                                -- label location already known
                                s.memory[label_used_at] = label_offset
                        else
                                -- label location not yet known, do it later
                                isn.finalize = function(self)
                                        label_offset = s.labels[arg.var]
                                        if label_offset ~= nil then
                                                s.memory[label_used_at] = label_offset
                                        else
                                                die(string.format("could not resolve variable/label '%s'", arg.var))
                                        end
                                end
                        end

                        num = 0x1f

                elseif arg.mref then
                        local reg = nil
                        if arg.mref.greg then
                                reg = DD.generic_registers[arg.mref.greg]
                                if not reg then
                                        die("don't know how to encode reg '"..(arg.mref.greg).."' of "..DataDumper(arg,""))
                                end
                        elseif arg.mref.sreg then
                                reg = DD.special_registers[arg.mref.sreg]
                                if not reg then
                                        die("don't know how to encode reg '"..(arg.mref.sreg).."' of "..DataDumper(arg,""))
                                end
                                if not reg.mref then
                                        die("cannot use register '"..(arg.mref.sreg).."' in memory reference,  "..DataDumper(arg,""))
                                end
                        end

                        if reg and arg.mref.num then

                                num = reg.mref_ofs
                                s:mem_append(arg.mref.num)
                                isn.length = isn.length + 1

                        elseif reg then

                                num = reg.mref_solo

                        elseif arg.mref.num then

                                num = 0x1e
                                s:mem_append(arg.mref.num)
                                isn.length = isn.length + 1

                        else
                                die("don't know how to encode mref "..DataDumper(arg.mref,""))
                        end
                else
                        die(DataDumper(arg, "didn't know how to handle arg: "))
                end

                -- this is really op |= num << shift, but lua lacks bit ops
                s.memory[isn.offset] = s.memory[isn.offset] + (num * mult)
        end

        local function assemble_gisn(num, isn)
                -- encoding of a generic instruction is aaaaaabbbbbooooo (6a/5b/5o)
                dbg(2,"", (isn.op)..'('..xx(num)..')', a, b)

                isn.offset, isn.length = s:mem_append(num)
                isn.finalize = nil

                assemble_isn_arg(isn.a, isn, 'a', 1024)  -- 1024 to shift by 10 bits
                assemble_isn_arg(isn.b, isn, 'b', 32)    -- 32 to shift by 5 bits

                dbg(1,">> ".. table.concat(lmap(function(n)
                        return string.format("0x%04x", n)
                end, s:mem_section(isn.offset, isn.length)), ' '), "\n")
        end

        local function assemble_sisn(num, isn)
                -- encoding of a special instruction is aaaaaaooooo00000 (6a/5o)
                dbg(2,"", (isn.op)..'('..xx(num)..')', a, b)

                isn.offset, isn.length = s:mem_append(num*32) -- shift up by 5 bits
                isn.finalize = nil

                assemble_isn_arg(isn.a, isn, 'a', 1024)    -- 1024 to shift by 10 bits

                dbg(1,">> ".. table.concat(lmap(function(n)
                        return string.format("0x%04x", n)
                end, s:mem_section(isn.offset, isn.length)), ' '))
        end

        -- top level handers

        local function handle_label(block)
                dbgf(1, "pc=0x%04x label '%s'", s.pc, block.label)
                if s.labels[block.label] ~= nil then
                        die(string.format("label '%s' already exists at 0x%04x", block.label, s.labels[block.label]))
                end
                s.labels[block.label] = s.pc
        end
        local function handle_data(block)
                dbgf(1, "pc=0x%04x data '%s'", s.pc, DataDumper(block.data, ""))
                for i,datum in ipairs(block.data) do
                        dbgf(1, "  > %s", DataDumper(datum, ""))
                        if datum.num then
                                s:mem_append(datum.num)
                        elseif datum.str then
                                local bytes = { string.byte(datum.str,1,datum.str:len()) }
                                for i,byte in ipairs(bytes) do
                                        s:mem_append(byte)
                                end
                        else
                                die(DataDumper(datum, "don't know how to handle data: "))
                        end
                end
        end
        local function handle_op(block)
                dbgf(1, "pc=0x%04x op '%s'", s.pc, DataDumper({block.op,block.b,block.a}, ""))

                local opcode = string.upper(block.op)
                local op = DD.generic_opcodes[opcode]
                if op ~= nil then
                        assemble_gisn(op.num, block)
                else
                        op = DD.special_opcodes[opcode]
                        if op ~= nil then
                                assemble_sisn(op.num, block)
                        else
                                die("unknown opcode: "..opcode)
                        end
                end

        end

        function t.reset(self, prog)
                s:init()
                return self
        end

        function t.append(self, prog)
                s:reset_labels()
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
                table.insert(s.programs, prog)
                return self
        end

        function t.dump(self, _out)
                local out = _out or io.stdout

                dbgf(1, "pc=0x%04x end", s.pc)
                dbgf(1, "#memory=%u", #s.memory)

                local o=0
                while o <= #s.memory do
                        out:write(xxxx(o)..':')
                        for i = 0,7 do
                                if s.memory[o+i] then
                                        out:write(' '..xxxx(s.memory[o+i]))
                                end
                        end
                        out:write("\n")
                        o = o + 8
                end
        end
        
        function t.write(self, _out, _endian)
                local out = _out or io.stdout
                local endian = string.upper(_endian or 'l')

                dbgf(1, "pc=0x%04x end", s.pc)
                dbgf(1, "#memory=%u", #s.memory)

                local encode
                if endian == 'L' then -- little endian
                        encode = function(word)
                                return string.char(
                                        word % 0x100,
                                        (word / 0x100) % 0x100)
                        end
                else                  -- big endian
                        encode = function(word)
                                return string.char(
                                        (word / 0x100) % 0x100,
                                        word % 0x100)
                        end
                end

                for addr = 0,#s.memory do
                        local word = s.memory[addr] or 0

                        out:write(encode(word))
                end
        end

        function t.write_intermediate(self, _out)
                local out = _out or io.stdout

                dbgf(1, "pc=0x%04x end", s.pc)
                dbgf(1, "#memory=%u", #s.memory)

                for i,prog in ipairs(s.programs) do
                        for j,isn in ipairs(prog) do
                                if isn.offset ~= nil and isn.length ~= nil then
                                        out:write(string.format("%s\t%u\t0x%04x\t", isn.program, isn.line, isn.offset))
                                        for k=0,(isn.length-1) do
                                                local addr = isn.offset + k
                                                local word = s.memory[addr] or 0
                                                out:write(string.format("0x%04x", word).." ")
                                        end
                                        out:write("\n")
                                end
                        end
                end
        end

        return t
end

return DA
