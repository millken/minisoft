-- table序列化为可打印格式
function t2s(t)
	local mark={}
	local assign={}
	
	local function ser_table(tbl,parent, indent)
		local ts = ''
		for i = 1, indent do ts = ts .. ' ' end
		mark[tbl]=parent
		local tmp={}
		for k,v in pairs(tbl) do
			local key= type(k)=="number" and "["..k.."]" or k
			if type(v)=="table" then
				local dotkey= parent..(type(k)=="number" and key or "."..key)
				if mark[v] then
					table.insert(assign, ts..dotkey.."="..mark[v])
				else
					table.insert(tmp, ts..key.."="..ser_table(v,dotkey,indent+1))
				end
			else
				table.insert(tmp, ts..key.."="..string.format('%q',v))
			end
		end
		return "{\n"..table.concat(tmp,",\n").."\n}\n"
	end

	return ser_table(t,"ret", 0)..table.concat(assign," ")
end

-- http cgi协议类
function gethttp ()
	local t = {}
	local response = ''
	local cookie = {}

	t.header = function ()
		io.write ("Content-type: text/html\n")
		if next(cookie) ~= nil then
			local cookies = ''
			for _,c in pairs(cookie) do
				cookies = cookies .. "Set-Cookie:" .. c.key .. "=" .. c.value
				if c.expire ~= nil then
					cookies = cookies .. ';expires=' .. os.date('%A,%d-%B-%Y %X GMT', c.expire)
				end
				cookies = cookies .. '\n'
			end
			io.write(cookies)
		end
		io.write ('\n')
		if response ~= '' then
			io.write (response)
		end
		os.exit()
	end

	t.setcookie = function (key, value, expire)
		table.insert(cookie, {key=key, value=value, expire=expire})
	end

	t.delcookie = function (key)
		t.setcookie(key, 0, 0)
	end

	t.echo = function (s)
		if s == nil then
			s = '#NIL'
		elseif type(s) == 'table' then 
			s = t2s(s)
		elseif type(s) == 'function' then
			s = '#FUNCTION'
		else
			s = s
		end
		response = response .. s
	end

	return t
end
HTTP = gethttp()
echo = HTTP.echo
error = HTTP.echo

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

-- 字符串解析为key-value数组
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
	f = io.open(filename, 'a')
	if f ~= nil then 
		io.close(f) 
	else
		return true
	end
	return not os.rename(filename, filename) 
end

-- table序列化 
function serialize(t)
	local mark={}
	local assign={}
	
	local function ser_table(tbl,parent, indent)
		local ts = ''
		for i = 1, indent do ts = ts .. ' ' end
		mark[tbl]=parent
		local tmp={}
		for k,v in pairs(tbl) do
			local key= type(k)=="number" and "["..k.."]" or "['"..k.."']"
			if type(v)=="table" then
				local dotkey= parent..(type(k)=="number" and key or "."..key)
				if mark[v] then
					table.insert(assign, ts..dotkey.."="..mark[v])
				else
					table.insert(tmp, ts..key.."="..ser_table(v,dotkey,indent+1))
				end
			elseif type(v)=='boolean' then
				table.insert(tmp, ts..key.."=".. (v==true and 'true' or 'false'))
			else
				table.insert(tmp, ts..key.."="..string.format('%q',v))
			end
		end
		return "{\n"..table.concat(tmp,",\n").."\n}\n"
	end

	return "do local ret="..ser_table(t,"ret", 0)..table.concat(assign," ").."return ret end"
end

-- 锁
g_lock = {}

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
	local t = dofile(path)
	g_lock[tname] = io.open(path, "r+")
	return t
end

-- 保存数据库表
function savetable (tname, o)
	local path = DBDIR .. tname .. '.' .. DBFILE_EXT
	local start = os.clock()
	-- 解锁
	if g_lock[tname] then
		io.close(g_lock[tname])
	end
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

-------------------------
-- 加载所有数据库
TP = {} --people
TR = {} --room
TI = {} --item
-------------------------
function loaddb ()
	TP = loadtable('people') or {}
	TR = loadtable('room') or {}
	TI = loadtable('item') or {}
	return TP, TR, TI
end

function savedb ()
	savetable('people', TP)
	savetable('room', TR)
	savetable('item', TI)
end

-- 配置
DBDIR = 'data/'
DBFILE_EXT = 't'
LOCKMAXTIME = 2  --尝试打开被锁文件的超时时间(秒)

_thisroom = {}
_thisplayer = {}

COOKIE = string.kvlist(os.getenv('HTTP_COOKIE'), ';')
COOKIE_EXPIRE = 3600 --cookie过期时间

POSTS = ''
POST = {}
local method = os.getenv('REQUEST_METHOD')
if method == 'POST' then
	local cmd = ''
	local len = 0 + (os.getenv('CONTENT_LENGTH') or 0)
	if len > 0 then
		cmd = io.read(len)
	end
	POSTS = cmd
	POST = string.kvlist(POSTS, '&')
end

GETS = os.getenv('QUERY_STRING') or ''
GET = string.kvlist(GETS, '&')
