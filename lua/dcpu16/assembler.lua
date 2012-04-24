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
                                s:mem_append(n)
                        end

                elseif arg.var then
                        dbg(3,"", "var", arg.var)

                        local label_used_at = s:mem_alloc(1)

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
                        local gr = nil
                        if arg.mref.greg then
                                gr = DD.generic_registers[arg.mref.greg]
                                if not gr then
                                        die("don't know how to encode reg '"..(arg.mref.greg).."' of "..DataDumper(arg,""))
                                end
                        end

                        if gr and arg.mref.num then

                                num = 0x10 + gr.num
                                s:mem_append(arg.mref.num)

                        elseif gr then

                                num = 0x08 + gr.num

                        elseif arg.mref.num then

                                num = 0x1e
                                s:mem_append(arg.mref.num)

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
                dbg(2,"", (isn.op)..'('..xx(num)..')', a, b)

                isn.offset, isn.length = s:mem_append(num)
                isn.finalize = nil

                assemble_isn_arg(isn.a, isn, 16)    -- 16 to shift by 4 bits
                assemble_isn_arg(isn.b, isn, 1024)  -- 1024 to shift by 10 bits

                dbg(1,">> ".. table.concat(lmap(function(n)
                        return string.format("0x%04x", n)
                end, s:mem_section(isn.offset, isn.length)), ' '), "\n")
        end

        local function assemble_sisn(num, isn)
                dbg(2,"", (isn.op)..'('..xx(num)..')', a, b)

                isn.offset, isn.length = s:mem_append(num)
                isn.finalize = nil

                assemble_isn_arg(isn.a, isn, 1024)    -- 1024 to shift by 10 bits

                dbg(1,">> ".. table.concat(lmap(function(n)
                        return string.format("0x%04x", n)
                end, s:mem_section(isn.offset, isn.length)), ' '))
        end

        -- top level handers

        local function handle_label(block)
                dbgf(1, "pc=0x%04x label '%s'\n", s.pc, block.label)
                if s.labels[block.label] ~= nil then
                        die(string.format("label '%s' already exists at 0x%04x", block.label, s.labels[block.label]))
                end
                s.labels[block.label] = s.pc
        end
        local function handle_data(block)
                dbgf(1, "pc=0x%04x data '%s'\n", s.pc, DataDumper(block.data, ""))
                for i,datum in ipairs(block.data) do
                        dbgf(1, "  > %s\n", DataDumper(datum, ""))
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
                dbgf(1, "pc=0x%04x op '%s'\n", s.pc, DataDumper({block.op,block.a,block.b}, ""))

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
                return self
        end

        function t.dump(self)
                local out = io.stdout

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

        return t
end

return DA
