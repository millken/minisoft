require "inc/common"

loaddb()

--- 排除异常情况
if COOKIE.username and PEOPLE[COOKIE.username]==nil then
	HTTP.delcookie(COOKIE.username)
end

-- game main module
G = {}
G.username = COOKIE.username or ''  --我的用户名
G.me = PEOPLE[G.username] or {}  --我的资料
me = G.me

----------------------
-- useful function
----------------------
-- 在场的人
function whoat (roomname)
	local list = {}
	for k,v in pairs(PEOPLE) do
		if v.at == roomname then
			list[k] = v
		end
	end
	return list
end

-- 输出某人信息
function print_people (username)
	local u = PEOPLE[username]
	if u == nil then return end
	print(u.desc..'<br>\n' or '')
	print(u.name .. '(' .. username .. ')<br>\n')
end

-------------------------------------------------
--                   指令集
-------------------------------------------------
A = {}

-- 登陆
A.dologin = function(cmd)
	cmd0 = cmd[1]
	if PEOPLE[cmd0] ~= nil then
		HTTP.setcookie('username', cmd0, os.time()+COOKIE_EXPIRE)
		print ('Logined')
	else
		print ('user not existed!<br/>')
		A.login()
	end
end

-- 注册
A.new = function(cmd)
	local username = cmd[2]
	if username == nil then
		print('plz input username')
	else
		if PEOPLE[username] ~= nil then
			print ("username existed!")
			A.login()
		else
			PEOPLE[username] = {
				name=(cmd[3] or username),
				desc='',
				at=BORNROOM,
				cmd = {},
				isplayer=true
			}
			HTTP.setcookie('username', username, os.time()+COOKIE_EXPIRE)
			print('Register successfully! Now You can input [look] to check where you are.')
		end
	end
end

-- 退出游戏
A.quit = function ()
	HTTP.delcookie('username')
	G.me.state = nil
	A.login()
end
A.exit = A.quit
A.q = A.quit

-- 登录的字
A.login = function ()
	print ('login(new):')
end

-- 杀掉某帐号
A.kill = function (cmd)
	username = cmd[2]
	if username==nil or PEOPLE[username]==nil then
		print("User not exist")
	elseif username == 'all' then
		PEOPLE = nil
		A.quit()
		print("ok.")
	else
		PEOPLE[username] = nil
		print("ok.")
	end
end

-- 隐藏指令,进入游戏
A.enter = function ()
	print 'welcm<br>'
	if COOKIE.username == nil then
		A.login()
	else
		print('Hello! '..COOKIE.username..'<br>\n')
		A.look()
	end
end

-- 查看当前房间或人或东西
A.look = function (cmd)
	-- 查看人或东西
	if cmd and cmd[2] then
		local p = cmd[2] or ''
		if PEOPLE[p] then print_people(p) return end
		return
	end

	-- 查看当前房间
	if G.me.at == nil or ROOM[G.me.at]==nil then print("you are float in the zero pointer.") return end
	local room = ROOM[G.me.at]
	print("<b>".. room.name .."</b>")
	if room.desc then
		print("<div>"..room.desc.."</div>")
	end
	-- 在场的人
	for k,v in pairs(whoat(G.me.at)) do
		if (k ~= G.username) then
			print ("□ " .. v.name .. '(' .. k ..')<br>\n')
		end
	end
end
A.l = A.look

-- 添加新角色
A.newpeople = function (cmd)
	local sysname = cmd[2]
	if sysname == nil then print'plz give a name' return end
	if PEOPLE[sysname] then print'The name was exist' return end
	local name = cmd[3] or sysname
	local where = cmd[4] or ''
	PEOPLE[sysname] = {
		name = name,
		desc = '',
		at = where,
		cmd = {}
	}
	print'ok.'
end
A.newp = A.newpeople

-- 把某人送到指定地点
A.moveto = function (cmd)
	local username = cmd[2] or ''
	if PEOPLE[username] == nil then print'user not exist' return end
	local roomname = cmd[3] or ''
	if ROOM[roomname] == nil then print'room not exist' return end
	PEOPLE[username].at = roomname
	print'ok.'
end

-- 删除某人
A.delpeople = function (cmd)
	local username = cmd[2]
	if username==nil or	PEOPLE[username]==nil then print'no this guy' return end
	PEOPLE[username] = nil
	print'ok.'
end
A.delp = A.delpeople

-- 添加新房间
A.newroom = function (cmd)
	local sysname = cmd[2]
	local name = cmd[3]
	if sysname == nil then
		print "Need for sysname~"
		return
	end
	if ROOM[sysname] ~= nil then
		print "The room existed!"
		return 
	end
	ROOM[sysname] = {
		name=name,
		desc='这里是...',
		cmd = {}
	}
	print "ok."
end

-- 删除房间
A.delroom = function (cmd)
	local roomname = cmd[0]
	if roomname==nil or ROOM[roomname]==nil then
		print("No this room")
	else
		ROOM[roomname] = nil
		print('ok')
	end
end

-- 添加新东西
A.newitem = function(cmd)
	local sysname = cmd[2]
	if sysname==nil or ITEM[sysname] then print'the name was exist' return end
	local name = cmd[3] or sysname
	local t = {
		name = name,
		desc = '',
		cmd = {}
	}
	ITEM[sysname] = t
	print'ok'
end

-- 删除东西
A.delitem = function(cmd)
	local sysname = cmd[2]
	if sysname==nil or ITEM[sysname]==nil then print'the item is not exist' return end
	ITEM[sysname] = nil
	print'ok'
end

-- 列出所有东西
A.listitem = function()
	print('There are ' .. table.count(ITEM) .. ' items<br>\n')
	for k,v in pairs(ITEM) do
		print('<b>' .. v.name .. '</b>' .. '(' .. k .. ') ')
	end
end

-- 给某人添加东西
A.additem = function(cmd)
	local username = cmd[2]
	if username==nil or PEOPLE[username]==nil then print'no username' return end
	local sysname = cmd[3]
	if sysname==nil or ITEM[sysname]==nil then print'no this item' return end
	local n = cmd[4]
	if n==nil then print'this count is nil' return end
	local sellval = cmd[5]
	if value==nil or type(sellval)~='number' then print'plz give it a supply-value' return end
	local buyval = cmd[6]
	if value==nil or type(buyval)~='number' then print'plz give it a buy-value' return end
	local user = PEOPLE[username]
	if user.item == nil then user.item = {} end
	if user.item[sysname] ~= nil then end
	local o = {
		
	}
end

-- 列出所有人
A.who = function ()
	print ("There are ".. table.count(PEOPLE) .." people:<br>")
	for k,v in pairs(PEOPLE) do
		print('<b>'..(v.name or 'Anonymous') .. '</b>')
		print('('.. k ..') ')
	end
end

-- 列出所有房间
A.listroom = function ()
	print ("There are "..table.count(ROOM)..' rooms:<br>\n')
	for k,v in pairs(ROOM) do
		print('<b>'.. (v.name or 'null') .. '</b>')
		print('('..k..') ')
	end
end

-- 列出所有指令
A.listcmd = function ()
	print("There are "..table.count(A)..' commands:<br>\n')
	local list = {}
	for k,_ in pairs(A) do
		table.insert(list, k)
	end
	print(table.concat(list, ', '))
end

-- 列出人,房间或指令
A.list = function (cmd)
	local p = cmd[2] or 'who'
	if p == 'people' then p = 'who' end
	if p == 'who' or p == 'w' or p == 'p' then
		A.who()
	elseif p == 'room' or p == 'r' then
		A.listroom()
	elseif p == 'item' or p == 'i' then
		A.listitem()
	elseif p == 'cmd' or p == 'c' then
		A.listcmd()
	else
		print("Command Help: list (who|room|item|cmd)")
	end
end

-- 注销帐号
A.suicide = function ()
	G.me.state = 'ready4suicide'
	print "Do you serious kill yourself? (y/n)"
end
A.sui = A.suicide

-- 确认注销帐号
A.ready4suicide = function (cmd)
	cmd0 = cmd[1]
	if cmd0=='y' or cmd0=='yes' then
		PEOPLE[G.username] = nil
		print "you dead!<br>"
		A.quit()
	elseif cmd0=='n' or cmd0=='no' then
		G.me.state = nil
		print "哦.."
	else
		A.suicide()
	end
end

G.a = A
-----------------------------------------------------------
--                      指令集结束↑
-----------------------------------------------------------

-- 执行指令
G.docmd = function (cmd)
	cmd = string.lower(cmd)
	local cmd = string.explode(cmd, ' ')

	-- 系统默认提供的指令缩写
	if cmd[1]=='w' then cmd[1]='west'
	elseif cmd[1]=='e' then cmd[1]='east'
	elseif cmd[1]=='n' then cmd[1]='north'
	elseif cmd[1]=='s' then cmd[1]='south'
	end

	local cmd0 = cmd[1]
	if cmd0 == nil or cmd0 == '' then
		print('???')
		return
	end
	G.cmd = cmd

	-- 最高指令
	if cmd0 == 'quit' or cmd0 == 'exit' then
		A.quit()
		return
	elseif cmd0=='who' then
		A.who()
		return
	end
	
	-- 未登录
	if COOKIE.username == nil then
		if cmd0 == 'new' then
			A.new(cmd)
		elseif A[cmd0] ~= nil then
			A.login()
		else
			A.dologin(cmd)
		end

	-- 异常情况, 有cookie但没这个人
	elseif G.me == nil then
		print "user data was not exist!<br/>"
		A.quit()

	-- 登陆后
	else
		-- 根据用户状态调执行2步,3步操作
		if A[G.me.state] ~= nil then
			A[G.me.state](cmd)
		-- 全局指令
		elseif A[cmd0] ~= nil then
				A[cmd0](cmd)
		-- 房间指令
		elseif ROOM[G.me.at] and ROOM[G.me.at].a[cmd0] then
			ROOM[G.me.at].a[cmd0](cmd)
		else
			print 'what?'
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
