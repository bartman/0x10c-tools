#!/usr/bin/lua

package.path = './lua/?.lua;' .. package.path
local DD = require 'dcpu16.defs'
local DP = require 'dcpu16.parser'
local DA = require 'dcpu16.assembler'
require 'dcpu16.util'

require 'dumper'
local function dump(...)
    print(DataDumper(...))
end


-- parse a file into a parse tree

local function parse_file(file)
        local d = DP.new()

        local f = assert(io.open(file))
        local program = f:read'*all'
        f:close()

        local res, suc, msg = d:newparse(program)

        if msg then print("\n"..msg.."\n") end
        if not suc then os.exit(1) end

        return res
end

local function assemble(prog)
        local a = DA.new()

        a:append(prog)

        a:dump()

        return a
end


------------------------------------------------------------------------

if #arg ~= 1 then
    die'provide a single file'
end

local prog = parse_file(arg[1])
local ret = assemble(prog)

