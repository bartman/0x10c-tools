#!/usr/bin/lua

-- this module creates a cpu16 class, here called D

local D = {}

-- we use LPEG

local lpeg = require 'lpeg'
local locale = lpeg.locale();
local P, R, S, V = lpeg.P, lpeg.R, lpeg.S, lpeg.V
local C, Cc, Ct, Cg, Cf, Cmt = lpeg.C, lpeg.Cc, lpeg.Ct, lpeg.Cg, lpeg.Cf, lpeg.Cmt

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

-- Creates an LPeg pattern that matches a set of words.
-- @param words A table of words.
-- @param word_chars Optional string of additional characters considered to be
--   part of a word (default is `%w_`).
-- @param case_insensitive Optional boolean flag indicating whether the word
--   match is case-insensitive.
-- @usage local keyword = token(l.KEYWORD, word_match { 'foo', 'bar', 'baz' })
-- @usage local keyword = token(l.KEYWORD, word_match({ 'foo-bar', 'foo-baz',
--   'bar-foo', 'bar-baz', 'baz-foo', 'baz-bar' }, '-', true))
-- @name word_match
--
-- http://foicica.com/hg/scintillua/raw-file/36123c3a9216/lexers/lexer.lua
local function word_match(words, word_chars, case_insensitive)
        local word_list = {}
        for _, word in ipairs(words) do
                word_list[case_insensitive and word:lower() or word] = true
        end
        local chars = '%w_'
        -- escape 'magic' characters
        -- TODO: append chars to the end so ^_ can be passed for not including '_'s
        if word_chars then chars = chars..word_chars:gsub('([%^%]%-])', '%%%1') end
        return P(function(input, index)
                local s, e, word = input:find('^(['..chars..']+)', index)
                if word then
                        if case_insensitive then word = word:lower() end
                        return word_list[word] and e + 1 or nil
                end
        end)
end

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

local charlit = ( P'L'^-1 * P"'" * (P'\\' * P(1) + (1 - S"\\'"))^1 * P"'" )
              + ( P'L'^-1 * P'"' * (P'\\' * P(1) + (1 - S"\\\""))^1 * P'"' )

local stringlit =
  P'L'^-1 * P'"' * (P'\\' * P(1) + (1 - S'\\"'))^0 * P'"'

local literal = (numlit + charlit + stringlit)

local comment = semi * (1 - P'\n')^0

-- opcodes

local generic_opcode_names = { "SET", "ADD", "SUB", "MUL",
                               "DIV", "MOD", "SHL", "SHR",
                               "AND", "BOR", "XOR", "IFE",
                               "IFN", "IFG", "IFB", "IFB", }
local special_opcode_names = { "JSR" }

local gop = P(word_match(generic_opcode_names, '', true))
local sop = P(word_match(special_opcode_names, '', true))

-- registers

local generic_reg_names = { "A", "B", "C", "X", "Y", "Z", "I", "J" }
local special_reg_names = { "POP", "PEEK", "PUSH", "SP", "PC", "O" }

local greg = P(word_match(generic_reg_names, '', true))
local sreg = P(word_match(special_reg_names, '', true))

-- data definitions

local data_names = { "DAT" }

local data = P(word_match(data_names, '', true))

-- symbolic stuff: keywords, variables, lables, etc
local keywords = gop + sop + greg + sreg + data

local names = ((locale.alpha + P"_") * (locale.alnum + P"_")^0)

local macro_name  = names                    -- macro names can be anything
local variable_name = names - (greg + sreg)  -- labels cannot clash with registers

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
local _line_expnd = V'line_expnd';
local _expnd_args = V'expnd_args';
local _line_data  = V'line_data';
local _data_arg   = V'data_arg';
local _line_cmnt  = V'line_cmnt';
local _error      = V'error';
local _oparg      = V'oparg';
local _opargtb    = V'opargtb';
local _mref       = V'mref';
local _mrefarg    = V'mrefarg';
local _var        = V'var';
local _reg        = V'reg';
local _greg       = V'greg';
local _sreg       = V'sreg';
local _num        = V'num';
local _str        = V'str';
local _char       = V'char';
local _data       = V'data';

------------------------------------------------------------------------
-- create a new instance of the parser, in a private closure

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
                --io.stderr:write("--build_table--\n")
                for i,v in ipairs(s) do
                        --io.stderr:write(DataDumper({i=i, v=v},'    bt>> ').."\n")
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
                --io.stderr:write(DataDumper(t,'    bt>> ').."\n")
                return t
        end

        -- folding is the process of taking multiple blocks and creating a list out of them
        local function fold_line(a,b)
                --io.stderr:write("--fold_line--\n")
                --io.stderr:write(DataDumper(a,'    fl>> a=').."\n")
                --io.stderr:write(DataDumper(b,'    fl>> b=').."\n")

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
                        -- macro expansion, passthrough after first entry
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
                --io.stderr:write("--fold_macro--\n")
                --io.stderr:write(DataDumper(a,'    fm>> a=').."\n")
                --io.stderr:write(DataDumper(b,'    fm>> b=').."\n")

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

                --io.stderr:write(DataDumper(m.vars,'    fm>> vars=').."\n")
                return m
        end

        local function expand_macro_line(s, vars)
                local lookup_table = {}
                local function _copy(obj)
                        if type(obj) ~= "table" then
                                return obj
                        end
                        if obj.var and vars[obj.var] then
                                obj = vars[obj.var]
                        end
                        if lookup_table[obj] then
                                return lookup_table[obj]
                        end
                        local new_table = {}
                        lookup_table[obj] = new_table
                        for index, value in pairs(obj) do
                                new_table[_copy(index)] = _copy(value)
                        end
                        return new_table
                end
                return _copy(s)
        end

        -- this function handles expansion of a macro
        local function fold_expansion(a,b)
                --io.stderr:write("--fold_expansion--\n")
                --io.stderr:write(DataDumper(a,'    fe>> a=').."\n")
                --io.stderr:write(DataDumper(b,'    fe>> b=').."\n")

                local x
                if type(a) == 'table' and a[1] == 'macro_ex_start' then
                        x = { line=ct_line, macro=a[2], vars={} }
                else
                        x = a
                end

                if type(b) == 'table' then
                        if b.num or b.greg or b.sreg or b.mref or b.var then
                                table.insert(x.vars, b)

                        elseif b[1] == 'macro_ex_end' then
                                --io.stderr:write("lookup!\n")

                                local m = macros[x.macro]

                                if not m then
                                        return set_error("no macro by name '"..x.macro.."' found.")
                                end

                                if #m.vars ~= #x.vars then
                                        return set_error("macro '"..m.macro.."' expects "..tostring(#m.vars)..
                                                        " arguments, but "..tostring(#x.vars).." were given.")
                                end

                                local vars={}
                                for i,v in ipairs(m.vars) do
                                        vars[v] = x.vars[i]
                                end

                                --io.stderr:write("found!\n")
                                --io.stderr:write(DataDumper(vars,'    fe>> vars=').."\n")
                                --io.stderr:write(DataDumper(m.code,'    fe>> code=').."\n")

                                local r = { 'expanded_block' }

                                for i,v in ipairs(m.code) do

                                        local w = expand_macro_line(v, vars)

                                        table.insert(r, w)
                                end

                                return r
                        end
                end

                return x
        end

        -- used for debugging only
        local function dbg(pat)
                return Cmt(pat,function(s,i,cap)
                        local txt = DataDumper(cap, '')
                        io.stderr:write("### dbg: ["..tostring(mt_line).."] '"..txt.."'\n")
                        return i,cap
                end)
        end

        -- matched something unexpected
        local function mark_error(...)
                io.stderr:write("--mark_error--\n")
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

        -- mark reaching end
        local end_reached = false
        local function mark_end(...)
                io.stderr:write("--mark_end--\n")
                end_reached = true
                return ...
        end

        ------------------------------------------------------------------------
        -- the full grammar

        local grammar = P{'program',
                program     = Cf(Cc('start_program') -- need a starting token for folding
                                  * _block
                                  * (eol_inc_line * _block)^0,
                                  fold_line)
                               --* #(P(-1)/mark_end)
                               * -1;
                block       = _directive + _line + _error^0;
                --
                directive   = _dir_macro; -- / build_table;
                dir_macro   = Cf(hash * P'macro' * w1 * token('macro_start', macro_name) * w0 * P'(' * _macro_args * P')'
                               * wcr0 * P'{' * wcr0
                                        * _line
                                        * (eol_inc_line * _line)^0
                               * wcr0 * token('macro_end', P'}'), fold_macro);
                macro_args  = w0 * _var/build_table * (comma * _var/build_table)^0 * w0;
                --
                line        = w0 * ( _line_label^-1
                               * (w0 * (_line_gisn + _line_sisn + _line_data + _line_expnd))^-1
                               * (w0 * _line_cmnt)^-1) / build_table
                               * w0;
                line_label  = colon * token('label', variable_name);
                line_gisn   = token('op', gop) * w1 *
                              token('a', _opargtb) * comma *
                              token('b', _opargtb);
                line_sisn   = token('op', sop) * w1 *
                              token('a', _opargtb);
                line_cmnt   = token('comment', semi * (1 - eol)^0);
                --
                line_expnd  = Cf(token('macro_ex_start', macro_name) * w0 * P'(' * _expnd_args * token('macro_ex_end',P')'), fold_expansion);
                expnd_args  = w0 * _oparg/build_table * (comma * _oparg/build_table)^0 * w0;
                --
                line_data   = _data * w1 * _data_arg * (comma * _data_arg)^0 * w0;
                data_arg    = _str + _num;
                --
                oparg       = _reg + _mref + _num + _var;
                opargtb     = ( _oparg ) / build_table;
                --
                mref        = token('mref', P'[' * w0 * _mrefarg * w0 * P']' );
                mrefarg     = ( ( _greg * plus * _num )
                              + ( _num * plus * _greg )
                              + _greg + _num ) / build_table;
                --
                reg         = _greg + _sreg;
                greg        = token('greg', greg);
                sreg        = token('sreg', sreg);
                num         = token('num', numlit) + _var + _char;
                str         = token('str', stringlit);
                char        = token('char', charlit);
                var         = token('var', variable_name);
                --
                data        = token('data', data);
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

        function t.parse(self, program, debug)
                local r = lpeg.match(grammar, program)
                if error_line then
                        local msg = "ERROR: parsing line "..(tostring(error_line))..": "..error_msg
                        return r, false, msg
                end
                if r == 'start_program' then
                        local msg = "WARNING: parsing generated no code"
                        return nil, true, msg
                end
                if not r then
                        local msg = "ERROR: parsing stopped at line "..(tostring(mt_line))
                        return r, false, msg
                end

                if debug then
                        dump(r, '"'..program..'"  =>  ')
                end

                return r, true
        end

        function t.newparse(self, program, debug)
                self:reset()
                return self:parse(program, debug)
        end

        return t
end

------------------------------------------------------------------------
-- class definition

return D
