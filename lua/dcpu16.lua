#!/usr/bin/lua

-- this module creates a cpu16 class, here called D

local D = {}

-- we use LPEG

local lpeg = require 'lpeg'
local locale = lpeg.locale();
local P, R, S, V = lpeg.P, lpeg.R, lpeg.S, lpeg.V
local C, Cc, Ct, Cg, Cf = lpeg.C, lpeg.Cc, lpeg.Ct, lpeg.Cg, lpeg.Cf

------------------------------------------------------------------------
-- grammar variables

local whitespace = S' \t\v'
local w0 = whitespace^0
local w1 = whitespace^1
local comma = w0 * P"," * w0
local plus = w0 * P"+" * w0
local colon = w0 * P":" * w0
local semi = w0 * P";" * w0
local hash = w0 * P"#" * w0
local eol = S'\r\n\f'
local digit = R'09'
local hex = R('af', 'AF', '09')

local hexnum = P'0' * S'xX' * hex^1
local octnum = P'0' * digit^1
local decnum = digit^1

local numlit = hexnum + octnum + decnum

local charlit =
  P'L'^-1 * P"'" * (P'\\' * P(1) + (1 - S"\\'"))^1 * P"'"

local stringlit =
  P'L'^-1 * P'"' * (P'\\' * P(1) + (1 - S'\\"'))^0 * P'"'

local literal = (numlit + charlit + stringlit)

local comment = semi * (1 - P'\n')^0

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

local sop = (
  P"JSR"
)

local op = gop + sop

-- registers

local greg = ( -- "generic register"
  P"A" +
  P"B" +
  P"C" +
  P"X" +
  P"Y" +
  P"Z" +
  P"I" +
  P"J"
)

local sreg = ( -- "special register"
  P"POP" +
  P"PEEK" +
  P"PUSH" +
  P"SP" +
  P"PC" +
  P"O"
)

local reg = greg + sreg

-- symbolic stuff: keywords, variables, lables, etc
local keywords = gop + sop + reg

local variable = (locale.alpha + P"_") * (locale.alnum + P"_")^0 -- - keywords

------------------------------------------------------------------------
-- grammar variables

local _program    = V'program';
local _block      = V'block';
local _directive  = V'directive';
local _dir_macro  = V'dir_macro';
local _line       = V'line';
local _line_label = V'line_label';
local _line_gisn  = V'line_gisn';
local _line_sisn  = V'line_sisn';
local _line_cmnt  = V'line_cmnt';
local _oparg      = V'oparg';
local _mref       = V'mref';
local _mrefarg    = V'mrefarg';
local _var        = V'var';
local _reg        = V'reg';
local _greg       = V'greg';
local _sreg       = V'sreg';
local _num        = V'num';

------------------------------------------------------------------------
-- helper functions

-- creates a named capture, result of which will be a list { id, matched_item }
local function token(id, patt)
        return Ct(Cc(id) * C(patt))
end

-- test if table is empty (no data under valid keys)
local function table_empty (self)
        for _, _ in pairs(self) do
                return false
        end
        return true
end


function D.new()

        -- the new class and private variables
        local t = {}
        local line = 1

        ------------------------------------------------------------------------
        -- grammar construction functions

        local function build_table(...)
                local t = { }
                local bad = {}
                --io.stderr:write(DataDumper({...},'>> ').."\n")
                for i,v in ipairs({...}) do
                        if type(v) == 'table' then
                                -- t['_'..v[1]] = v[2]
                                t[v[1]] = v[#v]
                        elseif v ~= nil and v ~= '' then
                                table.insert(bad, v)
                        end
                end
                if #bad > 0 then t['error'] = bad end
                return t
        end

        -- folding is the process of taking multiple blocks and creating a list out of them
        local function fold_line(a,b)
                if type(b) ~= 'table' or table_empty(b) then
                        -- no code generated
                        return a
                end

                b['line'] = line

                if type(a) == 'table' then
                        -- this is every line
                        table.insert(a, b)
                        return a
                else
                        -- this is the start token
                        return { b }
                end
        end

        -- keep track of line numbers
        local function inc_line(...)
                line = line + 1
        end
        local eol_inc_line = eol/inc_line

        ------------------------------------------------------------------------
        -- the full grammar

        local grammar = P{'program',
                program     = Cf(Cc('start') -- need a starting token for folding
                                  * _block
                                  * (eol_inc_line * _block)^0
                                  * -1,
                                 fold_line);
                block       = _directive + _line;
                directive   = _dir_macro;
                dir_macro   = hash * P'macro' * w0 * token('macro', variable) * w0 * P'(' * variable * P')'
                               * (w0 + eol_inc_line) * P'{'
                                   * _block * (eol_inc_line * _block)^0
                               * (w0 + eol_inc_line) * P'}';
                line        = ( _line_label^-1
                               * (w0 * (_line_gisn + _line_sisn))^-1
                               * (w0 * _line_cmnt)^-1
                               ) / build_table;
                line_label  = colon * token('label', variable);
                line_gisn   = token('op', gop) * w1 *
                              token('a', _oparg) * comma *
                              token('b', _oparg);
                line_sisn   = token('op', sop) * w1 *
                              token('a', _oparg);
                line_cmnt   = token('comment', semi * (1 - eol)^0);
                --
                oparg       = ( _num + _reg + _mref + _var ) / build_table;
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
                var         = token('var', variable);
        }

        -- reset line
        function t.reset(self, program)
                line = 1
        end

        function t.parse(self, program)
                return lpeg.match(grammar, program)
        end

        function t.newparse(self, program)
                self:reset()
                return self:parse(program)
        end

        return t
end

------------------------------------------------------------------------
-- class definition

return D
