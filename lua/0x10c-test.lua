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

local d = D.new()

local total = 0
local failed = 0

function test_parser(program, expectation)
    local r,s,msg = d:newparse(program)
    if type(expectation) == 'boolean' then
        if expectation and s then
            total = total + 1
            return
        elseif not expectation and not s then
            total = total + 1
            return
        end
    else
        die("unhandled expectation type: "..type(expectation).."\n")
    end

    failed = failed + 1

    print("-----------------------------------------\n"
    .. "FAILED TEST CASE\n")
    if msg then print("    "..msg.."\n") end
    dump(r, '"'..program..'"  =>  ')
    print''
    if not s then os.exit(1) end
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
good (':xx JSR A')
good (':XX JSR POP')
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
good ("SET xx,POP")
good (";\n#macro pop(dd){\nSET A,dd\n}\n;\n")
good ("#macro pop(xx){\nSET xx,POP\n}")
good ("#macro pop2(xx,yy){\nSET xx,POP\nSET yy,POP\n}")
good ("#macro pop(xx){\n SET xx,POP\n}\nSET A, 1")
good ("#macro pop(xx){\n SET xx,POP\n}\npop(A)")
good ("#macro pop2(xx,yy){\nSET xx,POP\nSET yy,POP\n}\npop2(A,B)")
good ("   jsr setup_drawBackBuffer     ")
good (" ifg A, 0")
good (" ifg [buttonPushTimer], 0")
good ("set A, 1")
good ("set A, '='")
good ('set A, "="')

if failed > 0 then
    print(tostring(failed) .. "/" .. tostring(total) .. " test cases failed.")
    return 1
end

print("ALL GOOD")
return 0
