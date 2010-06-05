require "inc/config"

http_header_begin()

http_set_cookie('username', 'cat', os.time()+3560)

http_header_end()

print_r(_cookie)

