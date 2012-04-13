#!/usr/bin/lua

package.path = './lua/?.lua;' .. package.path
local lpeg = require 'lpeg'
local D = require 'dcpu16'

require 'dumper'
function dump(...)
        print(DataDumper(...), "\n---")
end

--

function test()
        function parse(program)
                dump(lpeg.match(D.grammar, program), program..'  =>  ')
        end
        parse ('SET A, B')
        parse ('SET A, 1')
        parse ('SET A, [A]')
        parse ('SET A, [1]')
        parse ('SET A, [A+1]')
        parse ('SET A, [1+A]')
        parse ('JSR A')
        parse (':x JSR A')
        parse (':x JSR POP')
        parse ('; xxx')
        parse ('SET A, 1; 123')
        parse ('SET A, 1 ;123')
end

test()
