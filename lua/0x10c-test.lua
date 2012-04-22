#!/usr/bin/lua

package.path = './lua/?.lua;' .. package.path
local lpeg = require 'lpeg'
local DP = require 'dcpu16.parser'
require 'dcpu16.util'

require 'dumper'
function dump(...)
        print(DataDumper(...))
end

--

local d = DP.new()

local total = 0
local failed = 0
local debug = false

-- ------------------------------------------------------------------------
-- helpers for running tests

function test_parser(program, expectation)
    local r,s,msg = d:newparse(program, debug)
    if type(expectation) == 'boolean' then
        if expectation and s then
            total = total + 1
            return
        elseif not expectation and not s then
            total = total + 1
            return
        end
    else
        die("unhandled expectation type: "..type(expectation))
    end

    failed = failed + 1

    print("-----------------------------------------\n"
    .. "FAILED TEST CASE\n")
    if msg then print("    "..msg.."\n") end
    dump(r, '"'..program..'"  =>  ')
    print''
    if not s then os.exit(1) end
end

-- ------------------------------------------------------------------------
-- parse options

local long_options = {
    help='h',
    debug='d',
    parse='p',
}

local option_handlers = {
    h=function(arg,i)
        print("test [options...]\n"
            .."\n"
            .." -h --help          - print this help\n"
            .." -d --debug         - turn on debug\n"
            .." -p --parse <code>  - run parser on some code\n"
            )
        return 0
    end,
    d=function(arg,i)
        debug = true
        return 0
    end,
    p=function(arg,i)
        test_parser(arg[i],true)
        os.exit(0)
    end
}

for i = 1, #arg do
    local a=arg[i]
    i = i + 1

    if a:sub(1,1) == '-' then

        local op
        if a:sub(2,2) == '-' then
            op = long_options[a:sub(3)]
        elseif a:sub(2,2) and a:sub(3,3):len() == 0 then
            op = a:sub(2)
        else
            die("invalid option: "..a)
        end

        local h = option_handlers[op]
        if not h then
            die("unknonw option: "..a)
        end

        local consume = h(arg,i)
        i = i + consume
    end
end

-- ------------------------------------------------------------------------
-- default test cases

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
