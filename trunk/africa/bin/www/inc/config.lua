require 'inc/common'

io.stderr = io.stdout

DBDIR = 'data/'
DBFILE_EXT = 'lua'
-- 尝试打开被锁文件超时时间
LOCKMAXTIME = 2

_thisroom = {}
_thisplayer = {}
_cookie = string.kvlist(os.getenv('HTTP_COOKIE'), ';')
