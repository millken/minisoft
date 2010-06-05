local roomlist = {}

local room = {}
room.sysname = 'alexander'
room.name = '亚历山大'
room.description = [[
亚历山大港（阿拉伯语：الإسكندرية，又译为亚历山卓、埃尔伊斯坎达里亚）是埃及在地中海岸的一个港口，也是埃及最重要的海港，埃及的第二大城市和亚历山大港省的省会。其地理位置是北纬31°12'，东经29°15'，距离开罗西北208千米。尼罗河多支的、现已干枯的入海口位于亚历山大港东19千米处，古城卡诺珀斯的遗迹就在那里。亚历山大港现有约334万居民。 其港口性质为湾颈河口S、设有自由工业区。
]]
room.a = {}
room.a.east = [[
	print (_thisroom.name)
]]
roomlist[room.sysname] = room


local room = {}
room.sysname = 'sudan'
room.name = '苏丹'
room.description = [[
苏丹共和国位于非洲东北部，红海西岸，是非洲面积最大的国家。北邻埃及，西接利比亚、乍得、中非共和国，南毗刚果（金）、乌干达、肯尼亚，东壤埃塞俄比亚、厄立特里亚。东北濒临红海，海岸线长约720公里。
]]
room.a = {}
room.a['east'] = [[
	print (_thisroom.name)
]]
roomlist[room.sysname] = room


return roomlist
