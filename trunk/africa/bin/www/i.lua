require "inc/common"

loaddb()


-- game main module
G = {}
G.username = COOKIE.username or ''  --我的用户名
G.user = TP[G.username]  --我的资料

-- command table
A = {}

-- 退出游戏
A.quit = function ()
	HTTP.delcookie('username')
	G.user.state = nil
	A.login()
end
A.exit = A.quit

A.login = function ()
	echo ('login(new):')
end

-- 隐藏指令
A.enter = function ()
	echo 'welcm<br>'
	if COOKIE.username == nil then
		A.login()
	else
		echo('you are '..COOKIE.username)
	end
end

-- 查看当前房间
A.look = function (cmd)
	echo('this is ' .. (cmd[2] or 'here'))
end
A.l = A.look

-- 添加新房间
A.newroom = function (cmd)
	local sysname = cmd[2]
	local name = cmd[3]
	if sysname == nil then
		echo "Need for sysname~"
		return
	end
	if TR[sysname] ~= nil then
		echo "The room existed!"
		return 
	end
	TR[sysname] = {name=name}
	echo "ok"
end

-- 注销帐号
A.suicide = function ()
	G.user.state = 'ready4suicide'
	echo "Do you serious kill yourself? (y/n)"
end
A.sui = A.suicide

-- 确认注销帐号
A.ready4suicide = function (cmd)
	cmd0 = cmd[1]
	if cmd0=='y' or cmd0=='yes' then
		TP[G.username] = nil
		echo "you dead!<br>"
		A.quit()
	elseif cmd0=='n' or cmd0=='no' then
		G.user.state = nil
		echo "哦.."
	else
		A.suicide()
	end
end

G.a = A
G.docmd = function (cmd)
	cmd = string.lower(cmd)
	local cmd = string.explode(cmd, ' ')
	local cmd0 = cmd[1]
	if cmd0 == nil or cmd0 == '' then
		echo('???')
		return
	end

	if cmd0 == 'quit' or cmd0 == 'exit' then
		A.quit()
		return
	end
	
	-- 未登录
	if COOKIE.username == nil then
		if cmd0 == 'new' then
			local username = cmd[2]
			if username == nil then
				echo('plz input username')
			else
				if TP[username] ~= nil then
					echo ("username existed!")
					A.login()
				else
					TP[username] = {name='newbie',desc='a cute shy cool boy.'}
					HTTP.setcookie('username', username, os.time()+COOKIE_EXPIRE)
					echo('Register successfully! Now You can input [look] to check where you are.')
				end
			end
		elseif A[cmd0] ~= nil then
			A.login()
		else
			if TP[cmd0] ~= nil then
				HTTP.setcookie('username', cmd0, os.time()+COOKIE_EXPIRE)
				echo ('Logined')
			else
				echo ('user not existed!<br/>')
				A.login()
			end
		end

	-- 异常情况, 有cookie但没这个人
	elseif G.user == nil then
		echo "user data was not exist!<br/>"
		A.quit()

	-- 登陆后再根据用户状态调执行2级,3级指令
	else
		if A[G.user.state] ~= nil then
			A[G.user.state](cmd)
		else
			if A[cmd0] == nil then
				echo('what?')
			else
				A[cmd0](cmd)
			end
		end
	end

end

-- 接收指令
if POSTS ~= '' then 
	G.docmd (POSTS)
	savedb()
end

-- 响应
HTTP.header()
