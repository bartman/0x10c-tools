#!/usr/bin/lua

-- this module creates a cpu16 class, here called D

local D = {}

-- we use LPEG

local lpeg = require 'lpeg'
local locale = lpeg.locale();
local P, R, S, V = lpeg.P, lpeg.R, lpeg.S, lpeg.V
local C, Cc, Ct, Cg, Cf, Cmt = lpeg.C, lpeg.Cc, lpeg.Ct, lpeg.Cg, lpeg.Cf, lpeg.Cmt

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
local _macro_args = V'macro_args';
local _line       = V'line';
local _line_label = V'line_label';
local _line_gisn  = V'line_gisn';
local _line_sisn  = V'line_sisn';
local _line_macro = V'line_macro';
local _line_cmnt  = V'line_cmnt';
local _error      = V'error';
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
        local ct_line = 1
        local mt_line = 1
        local error_line = nil
        local error_msg = nil
        local macros = {}

        local function set_error(msg)
                if not error_line then
                        error_line = ct_line
                        error_msg = msg
                end
                return msg
        end

        ------------------------------------------------------------------------
        -- grammar construction functions

        local function build_table(...)
                local s = {...}
                local t = { }
                local bad = { }
                --io.stderr:write(DataDumper({...},'bt>> ').."\n")
                io.stderr:write("--build_table--\n")
                for i,v in ipairs(s) do
                        io.stderr:write(DataDumper({i=i, v=v},'    bt>> ').."\n")
                        if type(v) == 'table' and v[1] then
                                if v[1] == 'expanded_block' then
                                        -- expansion of a macro, no need to do anything else
                                        return v
                                else
                                        t[v[1]] = v[#v]
                                end
                        elseif v ~= nil and v ~= '' then
                                table.insert(bad, v)
                        end
                end
                if #bad > 0 then t['error'] = bad end
                        io.stderr:write(DataDumper(t,'    bt>> ').."\n")
                return t
        end

        -- folding is the process of taking multiple blocks and creating a list out of them
        local function fold_line(a,b)
                io.stderr:write("--fold_line--\n")
                io.stderr:write(DataDumper(a,'    fl>> a=').."\n")
                io.stderr:write(DataDumper(b,'    fl>> b=').."\n")

                if type(b) ~= 'table' or table_empty(b) then
                        -- no code generated
                        return a
                end

                if not b.line then b.line = ct_line end

                if type(a) ~= 'table' then
                        -- this is the start token
                        a = {}
                end

                if b[1] == 'expanded_block' then
                        -- macro expansion
                        for i,v in ipairs(b) do
                                if i>1 then
                                        table.insert(a, v)

                                end
                        end
                        return a
                end

                -- this is every line
                table.insert(a, b)
                return a
        end

        -- collect all the bits of a macro definition
        local function fold_macro(a,b)
                io.stderr:write("--fold_macro--\n")
                io.stderr:write(DataDumper(a,'    fm>> a=').."\n")
                io.stderr:write(DataDumper(b,'    fm>> b=').."\n")

                local m
                if type(a) == 'table' and a[1] == 'macro_start' then
                        m = { line=ct_line, macro=a[2], vars={}, code={} }
                else
                        m = a
                end

                if type(b) == 'table' then
                        if b.var then
                                table.insert(m.vars, b.var)
                        elseif b.op then
                                b.line = ct_line
                                table.insert(m.code, b)
                        elseif b[1] == 'macro_end' then
                                macros[m.macro] = m
                                return nil
                        end
                end

                return m
        end

        -- this 
        local function fold_expansion(a,b)
                io.stderr:write("--fold_expansion--\n")
                io.stderr:write(DataDumper(a,'    fe>> a=').."\n")
                io.stderr:write(DataDumper(b,'    fe>> b=').."\n")

                local m
                if type(a) == 'table' and a[1] == 'macro_ex_start' then
                        m = { line=ct_line, macro=a[2], vars={} }
                else
                        m = a
                end

                if type(b) == 'table' then
                        if b.var then
                                table.insert(m.vars, b.var)
                        elseif b[1] == 'macro_ex_end' then
                                io.stderr:write("lookup!\n")

                                local M = macros[m.macro]

                                if not M then
                                        return set_error("no macro by name '"..m.macro.."' found.")
                                end

                                io.stderr:write("found!\n")
                                io.stderr:write(DataDumper(M.code,'    fe>> code=').."\n")

                                local r = { 'expanded_block' }

                                for i,v in ipairs(M.code) do

                                        table.insert(r, v)
                                end

                                return r
                        end
                end

                return m
        end

        -- matched something unexpected
        local function mark_error(...)
                return set_error("error at word '"..(tostring(...)).."'")
        end

        -- keep track of line numbers
        local function inc_ct_line(...)
                ct_line = ct_line + 1
        end
        local eol_inc_line = Cmt(eol,
        function(s,i,a,b,c,d,e,f)
                mt_line = mt_line + 1
                return i,a,b,c,d,e,f
        end)/inc_ct_line
        local wcr0 = w0 * eol_inc_line^0 * w0

        ------------------------------------------------------------------------
        -- the full grammar

local function dbg(pat)
        return Cmt(pat,function(s,i,cap)
                local txt = DataDumper(cap, '')
                io.stderr:write("### dbg: ["..tostring(mt_line).."] '"..txt.."'\n")
                return i,cap
        end)
end

        local grammar = P{'program',
                program     = Cf(Cc('start') -- need a starting token for folding
                                  * _block
                                  * (eol_inc_line * _block)^0
                                  * -1,
                                 fold_line);
                block       = _directive + _line + _error^0;
                --
                directive   = _dir_macro; -- / build_table;
                dir_macro   = Cf(hash * P'macro' * w1 * token('macro_start', variable) * w0 * P'(' * _macro_args * P')'
                               * wcr0 * P'{' * wcr0
                                        * dbg(_line)
                                        * (eol_inc_line * dbg(_line))^0
                               * wcr0 * token('macro_end', P'}'), fold_macro);
                macro_args  = w0 * _var/build_table * (comma * _var/build_table)^0 * w0;
                --
                line        = ( _line_label^-1
                               * (w0 * (_line_gisn + _line_sisn + _line_macro))^-1
                               * (w0 * _line_cmnt)^-1) / build_table;
                line_label  = colon * token('label', variable);
                line_gisn   = token('op', gop) * w1 *
                              token('a', _oparg) * comma *
                              token('b', _oparg);
                line_sisn   = token('op', sop) * w1 *
                              token('a', _oparg);
                line_cmnt   = token('comment', semi * (1 - eol)^0);
                --
                line_macro  = Cf(token('macro_ex_start', variable) * P'(' * _macro_args * token('macro_ex_end',P')'), fold_expansion);
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
                --
                error       = (P(1) - w1 - eol)^1/mark_error;
        }

        -- reset line
        function t.reset(self, program)
                ct_line = 1
                mt_line = 1
                error_line = nil
                error_msg = nil
        end

        function t.parse(self, program)
                local r = lpeg.match(grammar, program)
                if error_line then
                        io.stderr:write("ERROR: parsing line "..(tostring(error_line))..": "..error_msg.."\n")
                        return r, false
                end
                if not r then
                        io.stderr:write("ERROR: parsing stopped at line "..(tostring(mt_line)).."\n")
                end
                return r, true
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
