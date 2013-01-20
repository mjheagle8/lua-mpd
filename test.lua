#!/usr/bin/env lua
-- unit test functions
-- for luampd
-- by mjheagle
-------

-- import module
require 'mpd'
if mpd ~= nil then
    print('functions:')
    for k, v in pairs(mpd) do
        print(string.format('  %s: %s', tostring(k), tostring(v)))
    end
else
    print('import failed!')
    os.exit(1)
end

-- helper function to print tables
local function printtable(t)
    if type(t) ~= type({}) then return end
    for k, v in pairs(t) do
        print(string.format('  %s: %s', tostring(k), tostring(v)))
    end
end

-- helper function to print relative times
local function reltime(t)
    local days = math.floor(t/(24*60*60))
    t = t - days*24*60*60
    local hours = math.floor(t/(60*60))
    t = t - hours*60*60
    local min = math.floor(t/60)
    t = t - min*60
    local sec = t

    return string.format('%d days, %d:%02d:%02d', days, hours, min, sec)
end

-- connect to mpd
print('attempting mpd connection...')
conn = mpd.connect('localhost', 6600, 1000)
print(conn)
if type(conn) ~= 'userdata' then
    print('connecting to mpd failed!')
    os.exit(2)
end

-- test statistics function
-- compare data gathered against 'mpc stats' output
function test_stats(conn, verbose)
    print('stats:')
    local stats = mpd.stats(conn)

    -- gather command output
    local cmd = io.popen('mpc stats')
    local mpclines = cmd:read('*a')
    cmd:close()
    if verbose == true then print(mpclines) end

    -- generate (hopefully) matching output
    local liblines = string.format(
[[Artists: %6d
Albums: %7d
Songs: %8d

Play Time:    %s
Uptime:       %s
DB Updated:   %s
DB Play Time: %s
]],
    stats.artists, stats.albums, stats.songs, reltime(stats.play_time), reltime(stats.uptime), stats.db_update_time, reltime(stats.db_play_time))
    if verbose == true then print(liblines) end

    if liblines == mpclines then
        print('  passed')
    else
        print('  failed')
    end
end

-- test volume function
-- requires mpd.state
-- change volume, check it worked, reset volume, check again
function test_volume(conn, verbose)
    print('volume:')
    pass = true
    local testvol = 95

    local v0 = state.volume
    if verbose == true then print(string.format('  volume: %d%%', v0)) end

    mpd.set_volume(conn, testvol)
    state = mpd.state(conn)
    if verbose == true then print(string.format('  volume: %d%%', state.volume)) end
    if state.volume ~= testvol then pass = false end

    mpd.set_volume(conn, v0)
    state = mpd.state(conn)
    if verbose == true then print(string.format('  volume: %d%%', state.volume)) end
    if state.volume ~= v0 then pass = false end

    if pass then
        print('  passed')
    else
        print('  failed')
    end
end

-- test state function
-- compare data gathered against 'mpc status' output
-- returns state for global use
function test_state(conn, verbose)
    print('state:')
    local state = mpd.state(conn)

    -- gather command output
    local cmd = io.popen('mpc status')
    local mpclines = cmd:read('*a')
    cmd:close()
    if verbose == true then print(mpclines) end

    -- parse mpc output
    -- remove first line
    mpclines = mpclines:sub(2+#(mpclines:match('[^\n]+')))
    -- match fields in 2nd line
    local mpcstate, plpos, plsize, songpos, songsize =
        mpclines:match('%[(%w+)%]%s+#(%d+)/(%d+)%s+([%d:]+)/([%d:]+)')
    -- remove second line
    mpclines = mpclines:sub(2+#(mpclines:match('[^\n]+')))
    -- match fields in 3rd line
    local vol, rpt, random, single, consume =
        mpclines:match('volume:(%d+)%%%s+repeat: (%w+)%s+random: (%w+)%s+single: (%w+)%s+consume: (%w+)')

    -- helper function to map on/off to 1/0
    local str2bool = function (a) if a == 'on' then return true else return false end end

    -- helper function to map mm:ss to integer
    local time2int = function (a)
        local m, s = a:match('(%d+):(%d+)')
        return 60*m + s
    end

    -- print fields parsed
    if verbose == true then
        print(string.format('  %s', mpcstate))
        print(string.format('  %s', plpos))
        print(string.format('  %s', plsize))
        print(string.format('  %s (%d)', songpos, time2int(songpos)))
        print(string.format('  %s (%d)', songsize, time2int(songsize)))
        print(string.format('  %s', vol))
        print(string.format('  %s (%s)', rpt, str2bool(rpt)))
        print(string.format('  %s (%s)', random, str2bool(random)))
        print(string.format('  %s (%s)', single, str2bool(single)))
        print(string.format('  %s (%s)', consume, str2bool(consume)))
    end

    -- check parsed fields
    local passed =
        state.state == mpcstate and
        state.song_id+1 == tonumber(plpos) and
        state.queue_length == tonumber(plsize) and
        state.elapsed_time == time2int(songpos) and
        state.total_time == time2int(songsize) and
        state.volume == tonumber(vol) and
        state['repeat'] == str2bool(rpt) and
        state.random == str2bool(random) and
        state.single == str2bool(single) and
        state.consume == str2bool(consume)

    if passed == true then
        print('  passed')
    else
        print('  failed')
    end

    return state
end

-- test now playing function
-- compare data gathered against formatted 'mpc status' output
function test_nowplaying(conn, verbose)
    print('now playing:')
    local np = mpd.now_playing(conn)

    -- get mpc output
    local cmd = io.popen("mpc status --format='%title%::%album%::%artist%::%track%::%genre%::%date%'")
    local mpc = cmd:read('*l')
    cmd:close()
    if verbose == true then print(mpc) end

    -- format now playing to match mpc output
    local npstr = string.format('%s::%s::%s::%s::%s::%s', np.title, np.album, np.artist, np.track, np.genre, np.date)
    if verbose == true then print(npstr) end

    -- check for matching output
    if npstr == mpc then
        print('  passed')
    else
        print('  failed')
    end
end

-- test playlist function
-- compare data gathered against 'mpc playlist' output
function test_playlist(conn, verbose)
    print('playlist:')
    local pl = mpd.playlist(conn)
    local passed = true

    -- check for valid return type
    if type(pl) ~= type({}) then
        passed = false
    else

    -- get mpc output and iterate through it
        local cmd = io.popen('mpc playlist --format="%file%"')
        local counter = 0
        while true do
            local line = cmd:read('*l')
            if line == nil then break end
            counter = counter + 1
            if line ~= pl[counter].uri then
                passed = false
                break
            end
        end
        cmd:close()
    end

    if passed == true then
        print('  passed')
    else
        print('  failed')
    end
end

-- run test functions
local verbose = false
test_stats(conn, verbose)
state = test_state(conn, verbose)
test_volume(conn, verbose)
test_nowplaying(conn, verbose)
test_playlist(conn, verbose)

mpd.free_connection(conn)
print('done')
