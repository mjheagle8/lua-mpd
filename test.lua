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

print('state:')
state = mpd.state(conn)
print(state)
for k, v in pairs(state) do
    print(string.format('  %s: %s', tostring(k), tostring(v)))
end

print('now playing:')
np = mpd.now_playing(conn)
print(np)
for k, v in pairs(np) do
    print(string.format('  %s: %s', tostring(k), tostring(v)))
end

print('stats:')
stats = mpd.stats(conn)
print(stats)
for k, v in pairs(stats) do
    print(string.format('  %s: %s', tostring(k), tostring(v)))
end

print('playlist:')
pl = mpd.playlist(conn)
print(pl)
for k, v in pairs(pl) do
    print(string.format('  %s: %s - %s', tostring(k), tostring(v.artist), tostring(v.title)))
end

mpd.free_connection(conn)
print('done')
