#!/usr/bin/lua

package.path = './lua/?.lua;' .. package.path
local lpeg = require 'lpeg'
local DP = require 'dcpu16.parser'
require 'dcpu16.util'

require 'dumper'
function dump(...)
        print(DataDumper(...))
end

if #arg == 1 then
        local d = DP.new()
        local f = assert(io.open(arg[1]))
        local program = f:read'*all'
        local res, suc, msg = d:newparse(program)
        dump(res, '"'..program..'"  =>  ')
        f:close()
        if msg then print("\n"..msg.."\n") end
        if not suc then os.exit(1) end
else
        die'provide a single file'
end
