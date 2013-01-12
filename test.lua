#!/usr/bin/env lua

require 'mpd'
if mpd ~= nil then
    print('functions:')
    for k, v in pairs(mpd) do
        print(string.format('  %s: %s', tostring(k), tostring(v)))
    end
else
    os.exit(1)
end

print('attempting mpd connection...')
conn = mpd.connect('localhost', 6600, 1000)
print(conn)

print('toggle:')
print(mpd.toggle(conn))

mpd.free_connection(conn)
print('done')
