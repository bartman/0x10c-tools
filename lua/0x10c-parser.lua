#!/usr/bin/lua

package.path = './lua/?.lua;' .. package.path
local lpeg = require 'lpeg'
local D = require 'dcpu16'

require 'dumper'
function dump(...)
        print(DataDumper(...), "\n-----------------------------------------")
end

--

function test()
        local d = D.new()
        function parse_and_dump(program)
                local r = d:newparse(program)
                dump(r, '"'..program..'"  =>  ')
        end
        --[[
        parse_and_dump ('SET A, B')
        parse_and_dump ('SET A, 1')
        parse_and_dump ('SET A, [A]')
        parse_and_dump ('SET A, [1]')
        parse_and_dump ('SET A, [A+1]')
        parse_and_dump ('SET A, [1+A]')
        parse_and_dump ('JSR A')
        parse_and_dump (':x JSR A')
        parse_and_dump (':X JSR POP')
        parse_and_dump (':x_0 JSR POP')
        parse_and_dump (':Z_0 JSR POP')
        parse_and_dump ('; xxx')
        parse_and_dump ('SET A, 1; 123')
        parse_and_dump (' SET A, 1 ;123')
        parse_and_dump ('  SET A, 1 ;123')
        parse_and_dump ('   SET A, 1 ;123')
        parse_and_dump ('    SET A, 0x30              ; 7c01 0030')
        parse_and_dump ('    SET A, 0x30              ; 7c01 0030   ')
        parse_and_dump ('; cmnt')
        parse_and_dump ('SET A, B')
        parse_and_dump ('SET A, 1 ; cmnt')
        parse_and_dump ('JSR A')
        parse_and_dump (':foo')
        parse_and_dump (':foo SET A, B')
        parse_and_dump (':foo_x')
        parse_and_dump (':foo_x SET A, B')
        parse_and_dump (':foo SET A, B')
        parse_and_dump ('SET A, [A]')
        parse_and_dump ('SET A, [1+A]')
        parse_and_dump ('SET A, variable')
        parse_and_dump (':xxx SET A, variable ; comment')
        parse_and_dump ("SET A, B\n")
        parse_and_dump ("SET A, B\nxxx")
        parse_and_dump ("SET A, B\nSET B, A")
        parse_and_dump ("SET A, B\n\nSET B, A")
        parse_and_dump ("SET x,POP")
        ]]--
        parse_and_dump ("#macro pop(x){\nSET x,POP\n}")
        parse_and_dump ("#macro pop2(x,y){\nSET x,POP\nSET y,POP\n}")
        parse_and_dump ("#macro pop(x){\n SET x,POP\n}\nSET A, 1")
        parse_and_dump ("#macro pop(x){\n SET x,POP\n}\npop(A)")
        parse_and_dump ("#macro pop2(x,y){\nSET x,POP\nSET y,POP\n}\npop2(A,B)")


        --[[
        local f = assert(io.open('examples/sample.dasm', 'r'))
        while true do 
                local line = f:read()
                if line == nil then break end
                parse_and_dump(line)
        end
        f:close()
        ]]--
end

if #arg == 1 then
        local d = D.new()
        local f = assert(io.open(arg[1]))
        local program = f:read'*all'
        local res = d:newparse(program)
        dump(res, '"'..program..'"  =>  ')
        f:close()
elseif #arg == 0 then
        test()
else
        print 'provide a single file'
        os.exit(0)
end
