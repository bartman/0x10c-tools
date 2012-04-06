#!/usr/bin/lua

local die = function(...)
        print(...)
        os.exit(1)
end

local lpeg = require 'lpeg'
local locale = lpeg.locale();
local P, R, S, C = lpeg.P, lpeg.R, lpeg.S
local C, Cc, Ct = lpeg.C, lpeg.Cc, lpeg.Ct

local whitespace = S' \t\v\n\f'
local w0 = whitespace^0
local w1 = whitespace^1

local comma = P","

local digit = R('09')

local digit = R'09'
local letter = R('az', 'AZ') + P'_'
local alphanum = letter + digit
local hex = R('af', 'AF', '09')
local exp = S'eE' * S'+-'^-1 * digit^1
local fs = S'fFlL'
local is = S'uUlL'^0

local hexnum = P'0' * S'xX' * hex^1 * is^-1
local octnum = P'0' * digit^1 * is^-1
local decnum = digit^1 * is^-1
local floatnum = digit^1 * exp * fs^-1 +
                 digit^0 * P'.' * digit^1 * exp^-1 * fs^-1 +
                 digit^1 * P'.' * digit^0 * exp^-1 * fs^-1
local numlit = hexnum + octnum + floatnum + decnum

local charlit =
  P'L'^-1 * P"'" * (P'\\' * P(1) + (1 - S"\\'"))^1 * P"'"

local stringlit =
  P'L'^-1 * P'"' * (P'\\' * P(1) + (1 - S'\\"'))^0 * P'"'

local literal = (numlit + charlit + stringlit)

local comment = P';' * (1 - P'\n')^0

-- opcodes

local gop = (
  P"SET" +
  P"ADD" +
  P"SUB" +
  P"MUL" +
  P"DIV" +
  P"MOD" +
  P"SHL" +
  P"SHR" +
  P"AND" +
  P"BOR" +
  P"XOR" +
  P"IFE" +
  P"IFN" +
  P"IFG" +
  P"IFB" +
  P"IFB"
)

local jsr_op = P"JSR"

local sop = (
  jsr_op
)

-- "generic register"
local greg = (
  P"A" +
  P"B" +
  P"C" +
  P"X" +
  P"Y" +
  P"Z" +
  P"I" +
  P"J"
)

-- "special register"
local sreg = (
  P"POP" +
  P"PEEK" +
  P"PUSH" +
  P"SP" +
  P"PC" +
  P"O"
)

local reg = greg + sreg

-- "memory reference"
local mref = P"[" * w0 * (
                      ( reg * w0 * P"+" * w0 * numlit )
                    + ( numlit * w0 * P"+" * w0 * reg )
                    + ( reg )
                    + ( numlit )
                  ) * w0 * P"]"

-- symbolic stuff: keywords, variables, lables, etc
local keywords = gop + sop + reg

local variable = (locale.alpha + P "_") * (locale.alnum + P "_")^0 - ( keywords )

local label = P":" * variable

-- instruction opcodes take arguments

local oparg = reg + numlit + variable + mref

-- "generic instruction"
local gisn = gop * w1 * oparg * w0 * comma * w0 * oparg

-- "extended instruction"
local jsr_arg = numlit + variable
local jsr_isn = jsr_op * w1 * jsr_arg
local xisn = jsr_isn

local isn = gisn + xisn

-- lexer

local lex_actions = {
        comment  = function(...) print('COMMENT', ...) end,
        literal  = function(...) print('LITERAL', ...) end,
        comma    = function(...) print('COMMA', ...) end,
        gop      = function(...) print('GOP', ...) end,
        sop      = function(...) print('SOP', ...) end,
        greg     = function(...) print('GREG', ...) end,
        sreg     = function(...) print('SREG', ...) end,
        mref     = function(...) print('MREF', ...) end,
        variable = function(...) print('VAR', ...) end,
        label    = function(...) print('LABEL', ...) end
}

function get_lexer()
        return ( comment       / lex_actions.comment
               + literal       / lex_actions.literal
               + P","          / lex_actions.comma
               + C( gop )      / lex_actions.gop
               + C( sop )      / lex_actions.sop
               + C( greg )     / lex_actions.greg
               + C( sreg )     / lex_actions.sreg
               + C( mref )     / lex_actions.mref
               + C( variable ) / lex_actions.variable
               + C( label )    / lex_actions.label
               + whitespace
        )^0
end

-- compiler

function compile_gisn(opcode, a, b)
end

local generic_opcodes = {
        SET={ opcode=0x1, compile=compile_gisn },
        SET={ opcode=0x1, compile=compile_gisn },
        ADD={ opcode=0x2, compile=compile_gisn },
        SUB={ opcode=0x3, compile=compile_gisn },
        MUL={ opcode=0x4, compile=compile_gisn },
        DIV={ opcode=0x5, compile=compile_gisn },
        MOD={ opcode=0x6, compile=compile_gisn },
        SHL={ opcode=0x7, compile=compile_gisn },
        SHR={ opcode=0x8, compile=compile_gisn },
        AND={ opcode=0x9, compile=compile_gisn },
        BOR={ opcode=0xa, compile=compile_gisn },
        XOR={ opcode=0xb, compile=compile_gisn },
        IFE={ opcode=0xc, compile=compile_gisn },
        IFN={ opcode=0xd, compile=compile_gisn },
        IFG={ opcode=0xe, compile=compile_gisn },
        IFB={ opcode=0xf, compile=compile_gisn },
}

function compile_xisn(opcode, a)
end

local extension_opcodes = {
        JSR={ opcode=0x1, compile=compile_xisn },
}


local compiler_actions = {
        comment = function(...) end,
        gisn = function(...)
                print('GISN', ...)
        end,
        xisn = function(...)
                print('XISN', ...)
        end,
        label = function(...)
                print('LABEL', ...)
        end
}

function get_compiler()
        return ( comment       / compiler_actions.comment
               + gisn          / compiler_actions.gisn
               + xisn          / compiler_actions.xisn
               + label         / compiler_actions.label
               + whitespace
        )^0
end


-- parse command line

local filename
local matcher

for i=1,table.getn(arg) do
        local a = arg[i]
        if a:sub(0,1) ~= '-' then
                filename = a
                break

        elseif a == "-h" or a == "--help" then
                print '0x10c'
                print ''
                print ' -h --help                         - this help'
                print ' -l --lex <file>                   - run lexer on a file'
                print ' -c --compile <file>               - compile file'
                os.exit(0)

        elseif a == "-l" or a == "--lex" then
                matcher = get_lexer()

        elseif a == "-c" or a == "--compiler" then
                matcher = get_compiler()

        else
                die("unknown option " .. a)
        end
end

if not matcher then die("need to specify an action; see --help") end
if not filename then die("need to specify a file") end


-- frontend

local fh = assert(io.open(filename))
local input = fh:read'*a'
fh:close()
local rc = lpeg.match(matcher, input)
print()
if (rc < input:len()) then
        print("ERROR: parser filed here...")
        print(input:sub(rc))
end

