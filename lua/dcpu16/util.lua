-- severe error reporting

function die(...)
        io.stdout:flush()
        io.stderr:write("ERROR: ")
        io.stderr:write(...)
        io.stderr:write("\n")
        os.exit(1)
end

-- apply function to list or table

function lmap(func, array)
        local new_array = {}
        for i,v in ipairs(array) do
                new_array[i] = func(v)
        end
        return new_array
end
function tmap(func, array)
        local new_array = {}
        for k,v in pairs(array) do
                new_array[k] = func(v)
        end
        return new_array
end
function keys(array)
        local r = {}
        for k,v in pairs(array) do
                table.insert(r, k)
        end
        return r
end

-- debug output

debug_level = 0

function dbg(l,...)
    if debug_level >= l then
        io.stderr:write("# "..table.concat( lmap(function(n)
            return tostring(n)
        end, {...}), "\t").."\n")
    end
end

function dbgf(l,fmt,...)
    if debug_level >= l then
        io.stderr:write("# "..string.format(fmt,...).."\n")
    end
end

-- string conversion

function xx(num)
        return string.format("%02x", num)
end
function xxxx(num)
        return string.format("%04x", num)
end

-- test if table is empty (no data under valid keys)
function table_empty (self)
        for _, _ in pairs(self) do
                return false
        end
        return true
end

-- split path into directory, file and extension
function split_file_name(name)
        local dir, file = string.match(name, '(.+/)([^/]+)')
        local base, ext = string.match(file or name, '(.+)[.]([^/.]+)')
        return dir, base, ext
end

