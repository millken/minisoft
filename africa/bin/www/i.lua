require "inc/config"

local cmd = ''
local g_state = 0
local user = nil



-- 接收指令
local method = os.getenv('REQUEST_METHOD')
if method == 'POST' then
	local len = 0+os.getenv('CONTENT_LENGTH') or 0
	if len > 0 then
		cmd = io.read(len)
	end
end

if cmd == 'quit' or cmd == 'exit' then
	http.delcookie('user')
	http.echo "login:"
	http.header()
end

if _cookie.user ~= nil then
	g_state = 1
	username = _cookie.user
end

if g_state == 0 then
	if cmd == '' then
		http.echo "login:"
	else
		http.setcookie('user', cmd)
		http.echo (cmd .. ' ok')
	end
else
	http.echo (username)
end

-- 响应
http.header()
