#!/usr/bin/lua

package.path = './lua/?.lua;' .. package.path
local lpeg = require 'lpeg'
local D = require 'dcpu16'

require 'dumper'
function dump(...)
        print(DataDumper(...), "\n-----------------------------------------")
end

--

local d = D.new()
function parse(program)
        dump(d:newparse(program), '"'..program..'"  =>  ')
end
function test()
        --[[
        parse ('SET A, B')
        parse ('SET A, 1')
        parse ('SET A, [A]')
        parse ('SET A, [1]')
        parse ('SET A, [A+1]')
        parse ('SET A, [1+A]')
        parse ('JSR A')
        parse (':x JSR A')
        parse (':X JSR POP')
        parse (':x_0 JSR POP')
        parse (':Z_0 JSR POP')
        parse ('; xxx')
        parse ('SET A, 1; 123')
        parse (' SET A, 1 ;123')
        parse ('  SET A, 1 ;123')
        parse ('   SET A, 1 ;123')
        parse ('    SET A, 0x30              ; 7c01 0030')
        parse ('    SET A, 0x30              ; 7c01 0030   ')
        parse ('; cmnt')
        parse ('SET A, B')
        parse ('SET A, 1 ; cmnt')
        parse ('JSR A')
        parse (':foo')
        parse (':foo SET A, B')
        parse (':foo_x')
        parse (':foo_x SET A, B')
        parse (':foo SET A, B')
        parse ('SET A, [A]')
        parse ('SET A, [1+A]')
        parse ('SET A, variable')
        parse (':xxx SET A, variable ; comment')
        parse ("SET A, B\n")
        parse ("SET A, B\nxxx")
        parse ("SET A, B\nSET B, A")
        parse ("SET A, B\n\nSET B, A")
        parse ("SET x,POP")
        ]]--
        parse ("#macro pop(x){\nSET x,POP\n}")
        parse ("#macro pop2(x,y){\nSET x,POP\nSET y,POP\n}")
        parse ("#macro pop(x){\n SET x,POP\n}\nSET A, 1")
        parse ("#macro pop(x){\n SET x,POP\n}\npop(A)")
        parse ("#macro pop2(x,y){\nSET x,POP\nSET y,POP\n}\npop2(A,B)")


        --[[
        local f = assert(io.open('examples/sample.dasm', 'r'))
        while true do 
                local line = f:read()
                if line == nil then break end
                parse(line)
        end
        f:close()
        ]]--
end

if #arg == 1 then
        local f = assert(io.open(arg[1]))
        while true do
                local line = f:read'*line'
                if not line then break end
                parse (line)
        end
        f:close()
elseif #arg == 0 then
        test()
else
        print 'provide a single file'
        os.exit(0)
end
