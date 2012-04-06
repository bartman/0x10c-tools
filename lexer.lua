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

local scomment = P';' * (1 - P'\n')^0

local lex_comment = scomment
              / function(...) print('COMMENT', ...) end

local lex_literal = (numlit + charlit + stringlit)
              / function(...) print('LITERAL', ...) end

local lex_comma = C( P"," ) / function(...) print('COMMA', ...) end

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

local lex_op = C( op ) / function(...) print('OP', ...) end

local sop = (
  P"JSR"
)

local lex_sop = C( sop ) / function(...) print('OP', ...) end

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

local lex_greg = C( greg ) / function(...) print('GREG', ...) end

local lex_sreg = C( sreg ) / function(...) print('SREG', ...) end

local lex_mref = C( P"[" * w0 * (
                      ( greg * w0 * P"+" * w0 * numlit )
                    + ( numlit * w0 * P"+" * w0 * greg )
                    + ( greg )
                    + ( numlit )
                  ) * w0 * P"]" ) / function(...) print('MREF', ...) end

local keywords = ( op + sop + greg + sreg )

local variable = (locale.alpha + P "_") * (locale.alnum + P "_")^0 - ( keywords )

local lex_variable = C( variable ) / function(...) print('VAR', ...) end


local label = P":" * variable

local lex_label = C( label ) / function(...) print('LABEL', ...) end


--[[

local keyword = C(
  P"auto" + 
  P"_Bool" +
  P"break" +
  P"case" +
  P"char" +
  P"_Complex" +
  P"const" +
  P"continue" +
  P"default" +
  P"do" +
  P"double" +
  P"else" +
  P"enum" +
  P"extern" +
  P"float" +
  P"for" +
  P"goto" +
  P"if" +
  P"_Imaginary" +
  P"inline" +
  P"int" +
  P"long" +
  P"register" +
  P"restrict" +
  P"return" +
  P"short" +
  P"signed" +
  P"sizeof" +
  P"static" +
  P"struct" +
  P"switch" +
  P"typedef" +
  P"union" +
  P"unsigned" +
  P"void" +
  P"volatile" +
  P"while"
) / function(...) print('KEYWORD', ...) end

local identifier = (letter * alphanum^0 - keyword * (-alphanum))
                 / function(...) print('ID',...) end

local op = C(
  P"..." +
  P">>=" +
  P"<<=" +
  P"+=" +
  P"-=" +
  P"*=" +
  P"/=" +
  P"%=" +
  P"&=" +
  P"^=" +
  P"|=" +
  P">>" +
  P"<<" +
  P"++" +
  P"--" +
  P"->" +
  P"&&" +
  P"||" +
  P"<=" +
  P">=" +
  P"==" +
  P"!=" +
  P";" +
  P"{" + P"<%" +
  P"}" + P"%>" +
  P"," +
  P":" +
  P"=" +
  P"(" +
  P")" +
  P"[" + P"<:" +
  P"]" + P":>" +
  P"." +
  P"&" +
  P"!" +
  P"~" +
  P"-" +
  P"+" +
  P"*" +
  P"/" +
  P"%" +
  P"<" +
  P">" +
  P"^" +
  P"|" +
  P"?"
) / function(...) print('OP', ...) end

local tokens = (comment + identifier + keyword +
                literal + op + whitespace)^0

]]--

local tokens = ( lex_comment
               + lex_op
               + lex_sop
               + lex_mref
               + lex_greg
               + lex_sreg
               + lex_literal
               + lex_label
               + lex_variable
               + lex_comma
               + whitespace
               )^0

-- frontend
local filename = arg[1]
local fh = assert(io.open(filename))
local input = fh:read'*a'
fh:close()
print(lpeg.match(tokens, input))

--[[

~~ ThomasHarningJr : Suggestion for optimization of the 'op' matcher in the C preprocessor... This should be faster due to the use of sets instead of making tons of 'basic' string comparisons. Not sure 'how' much faster...

local shiftOps = P">>" + P"<<"
local digraphs = P"<%" + P"%>" + P"<:" + P":>" -- {, }, [, ]
local op = C(
-- First match the multi-char items
  P"..." +
  ((shiftOps + S("+-*/%&^|<>=!")) * P"=") +
  shiftOps +
  P"++" +
  P"--" +
  P"&&" +
  P"||" +
  P"->" +
  digraphs +
  S(";{},:=()[].&!~-+*/%<>^|?")
) / function(...) print('OP', ...) end

]]--

