
function gethttp ()
	local t = {}
	local _response = ''
	local _cookies = ''

	t.header = function ()
		io.write ("Content-type: text/html\n")
		if _cookies ~= '' then
			io.write (_cookies)
		end
		io.write ('\n')
		if _response ~= '' then
			io.write (_response)
		end
		os.exit()
	end

	t.setcookie = function (key, value, expire)
		local s = "Set-Cookie:" .. key .. "=" .. value
		if expire ~= nil then
			s = s .. ';expires=' .. os.date('%A,%d-%B-%Y %X GMT', expire)
		end
		s = s .. '\n'
		_cookies = _cookies .. s
	end

	t.delcookie = function (key)
		t.setcookie(key, 0, 1)
	end

	t.echo = function (s)
		s = s or ''
		_response = _response .. s
	end

	return t
end
http = gethttp()

-- 字符串转化为表
string.explode = function (s, tok)
	local t = {}
	local start = 1
	local len = string.len(s)
	local i, j = 1, 1
	local elem = ''
	while true do
		i, j = string.find(s, tok, start)
		if i == nil or j==nil then 
			elem = s:sub(start, len)
			elem = string.gsub(elem, ' ', '')
			if string.len(elem) > 0 then
				table.insert(t, elem)
			end
			break
		end
		elem = s:sub(start, i-1)
		elem = string.gsub(elem, ' ', '')
		if string.len(elem) > 0 then
			table.insert(t, elem)
		end
		start = j + 1
	end
	return t
end

-- 字符串转化为key-value数组
string.kvlist = function (s, tok)
	if s == nil then s = '' end
	local list = string.explode(s, tok)
	local t = {}
	for _, i in pairs(list) do
		local exp = string.explode(i, '=')
		local k, v = exp[1], exp[2]
		if v == nil then v = '' end	
		if k ~= nil then t[k] = v end
	end
	return t
end

-- 解释并执行string
function dostring (s)
	local f = assert(loadstring(s))
	return f()
end

-- 文件是否被锁
function islock (filename)
	return not os.rename(filename, filename)
end

-- table序列化 
function serialize(t)
	local mark={}
	local assign={}
	
	local function ser_table(tbl,parent)
		mark[tbl]=parent
		local tmp={}
		for k,v in pairs(tbl) do
			local key= type(k)=="number" and "["..k.."]" or k
			if type(v)=="table" then
				local dotkey= parent..(type(k)=="number" and key or "."..key)
				if mark[v] then
					table.insert(assign,dotkey.."="..mark[v])
				else
					table.insert(tmp, key.."="..ser_table(v,dotkey))
				end
			else
				table.insert(tmp, key.."="..string.format('%q',v))
			end
		end
		return "{"..table.concat(tmp,",").."}"
	end
 
	return "do local ret="..ser_table(t,"ret")..table.concat(assign," ").." return ret end"
end

-- table序列化2
function t2s (t, indent)
	local tab = ''
	for i = 1,indent do
		tab = tab .. '&nbsp;&nbsp;&nbsp;&nbsp;'
	end

	if type(t) == 'number' then
		return tab .. t
	elseif type(t) == 'string' then
		return tab .. string.format("%q", t)
	elseif type(t) == 'table' then
		local s = ''
		for k,v in pairs(t) do
			s = s .. tab .. "['" .. k .. "'] = " .. '{<br/>' .. t2s(v, indent+1) .. '<br/>' .. tab .. '},<br/>'
		end
		if string.len(s) <= 0 then
			return "{}"
		else
			return s
		end
	end

	return nil
end

-- 打印表格
function print_r(t)
	print(assert(t2s(t, 0)))
end

-- 加载数据库表
function loadtable (tname)
	local path = DBDIR .. tname .. '.' .. DBFILE_EXT
	local start = os.clock()
	while islock(path) do
		if os.clock() - start > LOCKMAXTIME then
			error("</b>Error</b>: File was locked when opening(".. path .. ").<br/>\n")
			return nil
		end
	end
	-- 继续保持锁
	g_lock = io.open(path, "r+")
	return dofile(path)
end

-- 保存数据库表
function savetable (tname, o)
	local path = DBDIR .. tname .. '.' .. DBFILE_EXT
	local start = os.clock()
	-- 解锁
	io.close(g_lock)
	while islock(path) do
		if os.clock() - start > LOCKMAXTIME then
			error("</b>Error</b>: File was locked when writting(".. path .. ").<br/>\n")
			return nil
		end
	end
	local code = serialize(o)
	local f = io.open(path, 'w')
	f:write(code)
	io.close(f)
end
