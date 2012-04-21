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

--

function test()
        local d = D.new()
        function test_parser(program, expectation)
                local r,s,msg = d:newparse(program)
                if type(expectation) == 'boolean' then
                        if expectation and s then
                                return
                        elseif not expectation and not s then
                                return
                        end
                else
                        die("unhandled expectation type: "..type(expectation).."\n")
                end
                print("-----------------------------------------\n"
                      .. "FAILED TEST CASE\n")
                if msg then print("    "..msg.."\n") end
                dump(r, '"'..program..'"  =>  ')
                print''
        end
        function good(program)
                test_parser(program,true)
        end
        function fail(program)
                test_parser(program,false)
        end
        good ('SET A, B')
        good ('SET A, B')
        good ('SET A, 1')
        good ('SET A, [A]')
        good ('SET A, [1]')
        good ('SET A, [A+1]')
        good ('SET A, [1+A]')
        good ('JSR A')
        good (':x JSR A')
        good (':X JSR POP')
        good (':x_0 JSR POP')
        good (':Z_0 JSR POP')
        good ('; xxx')
        good ('SET A, 1; 123')
        good (' SET A, 1 ;123')
        good ('  SET A, 1 ;123')
        good ('   SET A, 1 ;123')
        good ('    SET A, 0x30              ; 7c01 0030')
        good ('    SET A, 0x30              ; 7c01 0030   ')
        good ('; cmnt')
        good ('SET A, 1 ; cmnt')
        good (':foo')
        good (':foo SET A, B')
        good (':foo_x')
        good (':foo_x SET A, B')
        good (':foo SET A, B')
        good ('SET A, [A]')
        good ('SET A, [1+A]')
        good ('SET A, variable')
        good (':xxx SET A, variable ; comment')
        good ("SET A, B\n")
        fail ("SET A, B\nxxx")
        good ("SET A, B\nSET B, A")
        good ("SET A, B\n\nSET B, A")
        good ("SET x,POP")
        good ("#macro pop(x){\nSET x,POP\n}")
        good ("#macro pop2(x,y){\nSET x,POP\nSET y,POP\n}")
        good ("#macro pop(x){\n SET x,POP\n}\nSET A, 1")
        good ("#macro pop(x){\n SET x,POP\n}\npop(A)")
        good ("#macro pop2(x,y){\nSET x,POP\nSET y,POP\n}\npop2(A,B)")
        good ("   jsr setup_drawBackBuffer     ")


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
        local res, suc, msg = d:newparse(program)
        dump(res, '"'..program..'"  =>  ')
        f:close()
        if msg then print("\n"..msg.."\n") end
        if not suc then os.exit(1) end
elseif #arg == 0 then
        test()
else
        print 'provide a single file'
        os.exit(0)
end
