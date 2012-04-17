#!/usr/bin/lua

local lpeg = require 'lpeg'
local locale = lpeg.locale();
local P, R, S, V = lpeg.P, lpeg.R, lpeg.S, lpeg.V
local C, Cc, Ct, Cg, Cf = lpeg.C, lpeg.Cc, lpeg.Ct, lpeg.Cg, lpeg.Cf

module(..., package.seeall)

local whitespace = S' \t\v\n\f'
local w0 = whitespace^0
local w1 = whitespace^1
local comma = w0 * P"," * w0
local plus = w0 * P"+" * w0
local colon = w0 * P":" * w0
local semi = w0 * P";" * w0
local eol = S '\r\n\f'
local digit = R'09'
local letter = R('az', 'AZ') + P'_'
local alphanum = letter + digit
local hex = R('af', 'AF', '09')
local exp = S'eE' * S'+-'^-1 * digit^1

--local is = S'uUlL'^0
hexnum = P'0' * S'xX' * hex^1 -- * is^-1
octnum = P'0' * digit^1 -- * is^-1
decnum = digit^1 -- * is^-1

--[[
local fs = S'fFlL'
local floatnum = digit^1 * exp * fs^-1 +
                 digit^0 * P'.' * digit^1 * exp^-1 * fs^-1 +
                 digit^1 * P'.' * digit^0 * exp^-1 * fs^-1
]]--
numlit = hexnum + octnum + decnum -- + floatnum

--numlit_cg = Cg(numlit, "numlit")

charlit =
  P'L'^-1 * P"'" * (P'\\' * P(1) + (1 - S"\\'"))^1 * P"'"

stringlit =
  P'L'^-1 * P'"' * (P'\\' * P(1) + (1 - S'\\"'))^0 * P'"'

literal = (numlit + charlit + stringlit)

comment = P';' * (1 - P'\n')^0

-- opcodes

gop = (
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

jsr_op = P"JSR"

sop = (
  jsr_op
)

-- "generic register"
greg = (
  P"A" +
  P"B" +
  P"C" +
  P"X" +
  P"Y" +
  P"Z" +
  P"I" +
  P"J"
)
--greg_c = C( greg )

-- "special register"
sreg = (
  P"POP" +
  P"PEEK" +
  P"PUSH" +
  P"SP" +
  P"PC" +
  P"O"
)
--sreg_c = C( sreg )

reg = greg + sreg
--reg_cg = Cg(reg, "reg")

-- "memory reference"
mref = P"[" * w0 * (
                      ( reg * w0 * P"+" * w0 * numlit )
                    + ( numlit * w0 * P"+" * w0 * reg )
                    + ( reg )
                    + ( numlit )
                  ) * w0 * P"]"
--mref_c = C( mref )

--[[
mref_ct = Ct( P"[" * w0 * (
                      ( reg_cg * w0 * P"+" * w0 * numlit_cg )
                    + ( numlit_cg * w0 * P"+" * w0 * reg_cg )
                    + ( reg_cg )
                    + ( numlit_cg )
                  ) * w0 * P"]" )
]]--

-- symbolic stuff: keywords, variables, lables, etc
keywords = gop + sop + reg

--variable = (locale.alpha + P"_") * (locale.alnum + P"_")^0 - ( keywords )
variable = ( (R'AZ' + R'az' + P"_") * (R'AZ' + R'az' + R'09' + P"_")^0 ) - keywords

label = P":" * variable

--label_c = P":" * C( variable )

-- instruction opcodes take arguments

oparg = reg + numlit + variable + mref
--oparg_c = C( oparg )

-- "generic instruction"
--gisn_c = C(gop) * w1 * C(oparg) * w0 * comma * w0 * oparg_c

-- "extended instruction"
jsr_arg = numlit + variable
jsr_isn_c = C(jsr_op) * w1 * C(jsr_arg)
xisn_c = jsr_isn_c

--isn_c = gisn_c + xisn_c

-- the full grammar

local _comment = V'comment'
local _lisn = V'lisn'
local _isn = V'isn'
local _gisn = V'gisn'
local _sisn = V'sisn'
local _label = V'label'
local _var = V'var'
local _gop = V'gop'
local _sop = V'sop'
local _oparg = V'oparg'
local _reg = V'reg'
local _greg = V'greg'
local _sreg = V'sreg'
local _num = V'num'
local _mrefp = V'mrefp'
local _mref = V'mref'

grammar = lpeg.P{ 'line',
        line    = w0 * _lisn^-1 * w0 * _comment^-1;
        lisn    = Ct(_label * w1 * _isn) + _isn;
        comment = C(semi * (1 - eol)^0);
        isn     = _gisn + _sisn;
        gisn    = Ct(_gop * w1 * _oparg * comma * _oparg);
        sisn    = Ct(_sop * w1 * _oparg);
        label   = C(colon * variable);
        var     = C(variable);
        gop     = C(gop);
        sop     = C(sop);
        oparg   = _reg + _num + _mref;
        reg     = _greg + _sreg;
        greg    = C(greg);
        sreg    = C(sreg);
        mrefp   = (_greg * plus * _num) + (_num * plus * _greg) + _greg + _num;
        mref    = Ct(P'['*w0 * _mrefp * w0*P']');
        num     = numlit / tonumber;
}

function parse(program)
        return lpeg.match(grammar, program)
end

