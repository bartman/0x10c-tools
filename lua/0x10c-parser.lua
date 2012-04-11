#!/usr/bin/lua

local lpeg = require 'lpeg'
local locale = lpeg.locale();
local P, R, S, C = lpeg.P, lpeg.R, lpeg.S
local C, Cc, Ct, Cg = lpeg.C, lpeg.Cc, lpeg.Ct, lpeg.Cg

package.path = './lua/?.lua;' .. package.path
local D = require 'dcpu16'


local debug_level = 0
local function dbg(l,...)
        if debug_level >= l then
                io.stderr:write("# "..table.concat({...},"\t").."\n")
        end
end
local function dbgf(l,fmt,...)
        if debug_level >= l then
                io.stderr:write("# "..string.format(fmt,...).."\n")
        end
end
local function die(...)
        io.stderr:write("ERROR: "..table.concat({...},"\t").."\n")
        os.exit(1)
end
local function lmap(func, array)
        local new_array = {}
        for i,v in ipairs(array) do
                new_array[i] = func(v)
        end
        return new_array
end
local function tmap(func, array)
        local new_array = {}
        for k,v in pairs(array) do
                new_array[k] = func(v)
        end
        return new_array
end
local function xx(num)
        return string.format("%02x", num)
end
local function xxxx(num)
        return string.format("%04x", num)
end

-- parser

local parser = {
        finalize = function(self)
                os.exit(0)
        end,
        dump = function(self, file)
                os.exit(0)
        end,
        output = function(self, file)
                os.exit(0)
        end,
}

local parser_actions = {
        comment  = function(...) print('# COMMENT', ...) end,
        literal  = function(...) print('# LITERAL', ...) end,
        comma    = function(...) print('# COMMA', ...) end,
        gop      = function(...) print('# GOP', ...) end,
        sop      = function(...) print('# SOP', ...) end,
        greg     = function(...) print('# GREG', ...) end,
        sreg     = function(...) print('# SREG', ...) end,
        mref     = function(...) print('# MREF', ...) end,
        variable = function(...) print('# VAR', ...) end,
        label    = function(...) print('# LABEL', ...) end
}

parser.matcher = ( D.comment       / parser_actions.comment
                + D.literal        / parser_actions.literal
                + P","           / parser_actions.comma
                + C( D.gop )       / parser_actions.gop
                + C( D.sop )       / parser_actions.sop
                + D.greg_c         / parser_actions.greg
                + C( D.sreg )      / parser_actions.sreg
                + D.mref_c         / parser_actions.mref
                + C( D.variable )  / parser_actions.variable
                + C( D.label )     / parser_actions.label
                + C( S'\n' ) / function(...) print('') end
                + D.whitespace
                )^0

local opargs = { }
local parser_oparg = C( D.oparg ) / function(...) 
        table.insert(opargs, ...)
end
local parser_gisn = C(D.gop) * D.w1 * parser_oparg * D.w0 * D.comma * D.w0 * parser_oparg

parser.matcher = ( D.comment       / function(...) print('') end
                 + parser_gisn   / function(...)
                         print('gisn '..(...)..' '..table.concat(opargs,' '))
                         opargs = {}
                 end
                 + D.xisn_c        / function(...) print('xisn') end
                 + D.label_c       / function(...) print('label') end
                 + C( S'\n' )    / function(...) print('') end
                 + D.whitespace
                 )^0





-- parse command line

local filename
local handler

local i = 1
while i<=#arg do
        local a = arg[i]
        i = i + 1
        if a == "-h" or a == "--help" then
                print '0x10c'
                print ''
                print ' -h --help                         - this help'
                print ' -v --verbose                      - be more verbose'
                print ' -p --parser <file>                - run parser on a file'
                print ' -o --output <file>                - write output here'
                os.exit(0)

        elseif a == "-v" or a == "--verbose" then
                debug_level = debug_level + 1

        elseif a == "-p" or a == "--parse" then
                handler = parser
                filename = arg[i]
                i = i + 1

        elseif a == "-o" or a == "--output" then
                output = arg[i]
                i = i + 1

        else
                die("unknown option " .. a)
        end
end

if not handler then die("need to specify an action; see --help") end
if not filename then die("need to specify a file") end


-- frontend

local fh = assert(io.open(filename))
local input = fh:read'*a'
fh:close()
local rc = lpeg.match(handler.matcher, input)
if (rc < input:len()) then
        print("ERROR: parser filed here...")
        print(input:sub(rc))
end

handler:dump()
handler:finalize()
handler:output(output)

