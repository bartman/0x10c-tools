#!/usr/bin/lua

local lpeg = require 'lpeg'
local locale = lpeg.locale();
local P, R, S, C = lpeg.P, lpeg.R, lpeg.S
local C, Cc, Ct = lpeg.C, lpeg.Cc, lpeg.Ct

local whitespace = S' \t\v\n\f'
local w0 = whitespace^0

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

--local lex_variable = 

local op = (
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

local sop = (
  P"JSR"
)

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

local sreg = (
  P"POP" +
  P"PEEK" +
  P"PUSH" +
  P"SP" +
  P"PC" +
  P"O"
)

local mref = P"[" * w0 * (
                      ( greg * w0 * P"+" * w0 * numlit )
                    + ( numlit * w0 * P"+" * w0 * greg )
                    + ( greg )
                    + ( numlit )
                  ) * w0 * P"]"

local keywords = ( op + sop + greg + sreg )

local variable = (locale.alpha + P "_") * (locale.alnum + P "_")^0 - ( keywords )

local label = P":" * variable


-- lexers

local lex_comment  = comment       / function(...) print('COMMENT', ...) end
local lex_literal  = literal       / function(...) print('LITERAL', ...) end
local lex_comma    = P","          / function(...) print('COMMA', ...) end
local lex_op       = C( op )       / function(...) print('OP', ...) end
local lex_sop      = C( sop )      / function(...) print('OP', ...) end
local lex_greg     = C( greg )     / function(...) print('GREG', ...) end
local lex_sreg     = C( sreg )     / function(...) print('SREG', ...) end
local lex_mref     = C( mref )     / function(...) print('MREF', ...) end
local lex_variable = C( variable ) / function(...) print('VAR', ...) end
local lex_label    = C( label )    / function(...) print('LABEL', ...) end

local lex_actions = {
        comment  = function(...) print('COMMENT', ...) end,
        literal  = function(...) print('LITERAL', ...) end,
        comma    = function(...) print('COMMA', ...) end,
        op       = function(...) print('GOP', ...) end,
        sop      = function(...) print('SOP', ...) end,
        greg     = function(...) print('GREG', ...) end,
        sreg     = function(...) print('SREG', ...) end,
        mref     = function(...) print('MREF', ...) end,
        variable = function(...) print('VAR', ...) end,
        label    = function(...) print('LABEL', ...) end
}


-- final parser

function parser(actions)
        return ( comment       / actions.comment
               + literal       / actions.literal
               + P","          / actions.comma
               + C( op )       / actions.op
               + C( sop )      / actions.sop
               + C( greg )     / actions.greg
               + C( sreg )     / actions.sreg
               + C( mref )     / actions.mref
               + C( variable ) / actions.variable
               + C( label )    / actions.label
               + whitespace
               )^0
end

-- frontend
local filename = arg[1]
local fh = assert(io.open(filename))
local input = fh:read'*a'
fh:close()
local rc = lpeg.match(parser(lex_actions), input)
print(rc)
if (rc < input:len()) then
        print("ERROR: parser filed here...")
        print(input:sub(rc))
end

