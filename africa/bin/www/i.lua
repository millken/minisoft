require "inc/common"

-- lib
function look (username)
	echo('A room.')
end

-- main module
G = {}
A = {}

A.quit = function ()
	HTTP.delcookie('username')
	A.login()
end
A.exit = A.quit

A.login = function ()
	echo ('login(new):')
end

A.enter = function ()
	echo 'welcm<br>'
	if COOKIE.username == nil then
		A.login()
	else
		echo('you are '..COOKIE.username)
	end
end

A.look = function (cmd)
	echo('this is ' .. (cmd[2] or 'here'))
end

G.a = A
G.docmd = function (cmd)
	local cmd = string.explode(cmd, ' ')
	local cmd0 = cmd[1]
	local action = G.a[cmd0]
	if cmd0 == nil or cmd0 == '' then
		echo('???')
		return
	end

	if cmd0 == 'quit' or cmd0 == 'exit' then
		A.quit()
		return
	end

	if COOKIE.username == nil then
		if cmd0 == 'new' then
			local username = cmd[2]
			if username == nil then
				echo('plz input username')
			else
				HTTP.setcookie('username', username, os.time()+COOKIE_EXPIRE)
				echo('reg ok')
			end
		elseif A[cmd0] ~= nil then
			A.login()
		else
			HTTP.setcookie('username', cmd0, os.time()+COOKIE_EXPIRE)
			echo ('logined')
		end
	else
		if action == nil then
			echo('??')
		else
			action(cmd)
		end
	end

end

-- 接收指令
if POSTS ~= '' then 
	G.docmd (POSTS)
end

-- 响应
HTTP.header()
