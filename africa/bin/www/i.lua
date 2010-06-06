require "inc/config"



http_header_begin()

-- http_set_cookie('username', 'cat', os.time()+3560)

http_header_end()



local method = os.getenv('REQUEST_METHOD')
if method == 'POST' then
	local len = 0+os.getenv('CONTENT_LENGTH') or 0
	local data = ''
	if len > 0 then
		data = io.read(len)
	end
	print(data)
end
