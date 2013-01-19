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

local function printtable(t)
    if type(t) ~= type({}) then return end
    for k, v in pairs(t) do
        print(string.format('  %s: %s', tostring(k), tostring(v)))
    end
end

print('attempting mpd connection...')
conn = mpd.connect('localhost', 6600, 1000)
print(conn)

print('state:')
state = mpd.state(conn)
print(state)
printtable(state)

print('now playing:')
np = mpd.now_playing(conn)
print(np)
printtable(np)

print('stats:')
stats = mpd.stats(conn)
print(stats)
printtable(stats)

print('playlist:')
pl = mpd.playlist(conn)
print(pl)
if type(pl) == type({}) then
    for k, v in pairs(pl) do
        print(string.format('  %s: %s - %s', tostring(k), tostring(v.artist), tostring(v.title)))
    end
end

local v0 = state.volume
print(string.format('volume: %d%%', v0))
mpd.set_volume(conn, 95)
state = mpd.state(conn)
print(string.format('volume: %d%%', state.volume))
mpd.set_volume(conn, v0)
state = mpd.state(conn)
print(string.format('volume: %d%%', state.volume))

mpd.free_connection(conn)
print('done')
