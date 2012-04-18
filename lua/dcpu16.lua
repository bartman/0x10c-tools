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
local hex = R('af', 'AF', '09')

hexnum = P'0' * S'xX' * hex^1
octnum = P'0' * digit^1
decnum = digit^1

numlit = hexnum + octnum + decnum

charlit =
  P'L'^-1 * P"'" * (P'\\' * P(1) + (1 - S"\\'"))^1 * P"'"

stringlit =
  P'L'^-1 * P'"' * (P'\\' * P(1) + (1 - S'\\"'))^0 * P'"'

literal = (numlit + charlit + stringlit)

comment = semi * (1 - P'\n')^0

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

sop = (
  P"JSR"
)

op = gop + sop

-- registers

greg = ( -- "generic register"
  P"A" +
  P"B" +
  P"C" +
  P"X" +
  P"Y" +
  P"Z" +
  P"I" +
  P"J"
)

sreg = ( -- "special register"
  P"POP" +
  P"PEEK" +
  P"PUSH" +
  P"SP" +
  P"PC" +
  P"O"
)

reg = greg + sreg

-- symbolic stuff: keywords, variables, lables, etc
keywords = gop + sop + reg

variable = (locale.alpha + P"_") * (locale.alnum + P"_")^0 -- - keywords

-- grammar construction functions

local function token(id, patt)
        return Ct(Cc(id) * C(patt))
end

local function build_table(...)
        local t = { }
        local bad = {}
        -- io.stderr:write(DataDumper({...},'>> ').."\n")
        for i,v in ipairs({...}) do
                if type(v) == 'table' then
                        -- t['_'..v[1]] = v[2]
                        t[v[1]] = v[#v]
                else
                        table.insert(bad, v)
                end
        end
        if #bad > 0 then t['error'] = bad end
        return t
end

-- grammar variables

local _line       = V'line';
local _line_label = V'line_label';
local _line_gisn  = V'line_gisn';
local _line_sisn  = V'line_sisn';
local _line_cmnt  = V'line_cmnt';
local _oparg      = V'oparg';
local _mref       = V'mref';
local _mrefarg    = V'mrefarg';
local _reg        = V'reg';
local _greg       = V'greg';
local _sreg       = V'sreg';
local _num        = V'num';

-- the full grammar

local grammar = P{'line',
        line        = ( _line_label^-1
                      * w0
                      * (_line_gisn + _line_sisn)^-1
                      * w0
                      * _line_cmnt^-1
                      * -1 ) / build_table;
        line_label  = colon * token('label', variable);
        line_gisn   = token('op', gop) * w1 * 
                      token('a', _oparg) * comma * 
                      token('b', _oparg);
        line_sisn   = token('op', sop) * w1 * 
                      token('a', _oparg);
        line_cmnt   = token('comment', semi * (1 - eol)^0);
        --
        oparg       = ( _num + _reg + _mref ) / build_table;
        --
        mref        = token('mref', P'[' * w0 * _mrefarg * w0 * P']' );
        mrefarg     = ( ( _greg * plus * _num )
                      + ( _num * plus * _greg )
                      + _num + _greg ) / build_table;
        --
        reg         = _greg + _sreg;
        greg        = token('greg', greg);
        sreg        = token('sreg', sreg);
        num         = token('num', numlit);
}

function parse(program)
        return lpeg.match(grammar, program)
end

