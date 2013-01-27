/*
 * lua media functions
 * by mjheagle
 */

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <string.h>
#include <mpd/client.h>

#if LUA_VERSION_NUM < 502
#  define luaL_newlib(L,l) (lua_newtable(L), luaL_register(L,NULL,l))
#endif

/*
 * macros
 */
/* get mpd connection from arguments */
#define get_mpd_conn(L) \
        struct mpd_connection *conn = NULL; \
        if (lua_islightuserdata(L, 1) == 1) \
                conn = (struct mpd_connection *)lua_topointer(L, 1); \
        else \
                return 0;
/* push a value into a table */
#define lua_table_push(L, key, value, pushfunc) \
        { \
                lua_pushstring(L, key); \
                pushfunc(L, value); \
                lua_settable(L, -3); \
        }
#define lua_table_push_int(L, key, value)    {lua_table_push(L, key, value, lua_pushinteger)}
#define lua_table_push_str(L, key, value)    {lua_table_push(L, key, value, lua_pushstring)}
#define lua_table_push_float(L, key, value)  {lua_table_push(L, key, value, lua_pushnumber)}
#define lua_table_push_bool(L, key, value)   {lua_table_push(L, key, value, lua_pushboolean)}

/*
 * create a connection to mpd
 * lua function
 * arguments are the mpd host (string), port (int), and timeout (int (ms))
 * return is a pointer to the mpd connection
 */
int mpd_connect(lua_State *L)
{
        /* get arguments to function */
        if (!lua_isstring(L, 1))
                goto errorout;
        const char *host = lua_tostring(L, 1);
        if (!lua_isnumber(L, 2))
                goto errorout;
        const int port = lua_tonumber(L, 2);
        if (!lua_isnumber(L, 3))
                goto errorout;
        const int timeout = lua_tonumber(L, 3);

        /* establish connection */
        struct mpd_connection *conn = mpd_connection_new(host, port, timeout);
        enum mpd_error err = mpd_connection_get_error((const struct mpd_connection *)conn);
        if (err != MPD_ERROR_SUCCESS)
        {
                lua_pushstring(L, "mpd connection failed");
                lua_pushstring(L, mpd_connection_get_error_message(conn));
                return 2;
        }

        /* push connection */
        lua_pushlightuserdata(L, conn);

        return 1;

errorout:
        lua_pushstring(L, "invalid arguments. syntax: host, port, timeout");
        return 1;
}

/*
 * run a mpd command
 * internal function
 * arguments are lua_state and the function to execute
 * will push error message onto stack if necessary
 * and return the integer to be returned to lua
 */
int mpd_cmd(lua_State *L, bool (*func)())
{
        get_mpd_conn(L);

        /* run command */
        bool ret = (int)func(conn);

        /* determine whether to print error message in lua */
        if (ret)
                return 0;
        else
        {
                lua_pushstring(L, "error running mpd command");
                return 1;
        }
}

/*
 * stop mpd
 * lua function
 * argument is the connection
 * will return error message, if one exists
 */
int mpd_stop(lua_State *L)
{
        return mpd_cmd(L, &mpd_run_stop);
}

/*
 * begin playing
 * lua function
 * argument is the connection
 * will return error message, if one exists
 */
int mpd_play(lua_State *L)
{
        return mpd_cmd(L, &mpd_run_play);
}

/*
 * toggle mpd status
 * lua function
 * argument is the connection
 * will return error message, if one exists
 */
int mpd_toggle(lua_State *L)
{
        return mpd_cmd(L, &mpd_run_toggle_pause);
}

/*
 * move to next track
 * lua function
 * argument is the connection
 * will return error message, if one exists
 */
int mpd_next(lua_State *L)
{
        return mpd_cmd(L, &mpd_run_next);
}

/*
 * move to previous track
 * lua function
 * argument is the connection
 * will return error message, if one exists
 */
int mpd_prev(lua_State *L)
{
        return mpd_cmd(L, &mpd_run_previous);
}

/*
 * free mpd connection
 * lua function
 * argument is the connection
 * no returns
 */
int mpd_free_connection(lua_State *L)
{
        get_mpd_conn(L);

        mpd_connection_free(conn);

        return 0;
}

/*
 * get mpd state
 * lua function
 * argument is the connection
 * return is a table with status values
 */
int mpd_state(lua_State *L)
{
        get_mpd_conn(L);

        /* get status from mpd */
        struct mpd_status *status = NULL;
        status = mpd_run_status(conn);

        /* create table for results */
        lua_newtable(L);

        /* get state */
        enum mpd_state state = mpd_status_get_state(status);
        if (state == MPD_STATE_STOP)
                lua_table_push_str(L, "state", "stopped")
        else if (state == MPD_STATE_PLAY)
                lua_table_push_str(L, "state", "playing")
        else if (state == MPD_STATE_PAUSE)
                lua_table_push_str(L, "state", "paused")
        else
        {
                lua_table_push_str(L, "state", "unknown")
                return 1;
        }

        /* simple values */
        lua_table_push_int(L, "volume",         mpd_status_get_volume(status));
        lua_table_push_bool(L, "random",        mpd_status_get_random(status));
        lua_table_push_bool(L, "repeat",        mpd_status_get_repeat(status));
        lua_table_push_bool(L, "single",        mpd_status_get_single(status));
        lua_table_push_bool(L, "consume",       mpd_status_get_consume(status));
        lua_table_push_int(L, "queue_length",   mpd_status_get_queue_length(status));
        lua_table_push_int(L, "queue_version",  mpd_status_get_queue_version(status));
        lua_table_push_int(L, "crossfade",      mpd_status_get_crossfade(status));
        lua_table_push_float(L, "mixrampdb",    mpd_status_get_mixrampdb(status));
        lua_table_push_float(L, "mixrampdelay", mpd_status_get_mixrampdelay(status));
        lua_table_push_int(L, "song_pos",       mpd_status_get_song_pos(status));
        lua_table_push_int(L, "song_id",        mpd_status_get_song_id(status));
        lua_table_push_int(L, "next_song_pos",  mpd_status_get_next_song_pos(status));
        lua_table_push_int(L, "next_song_id",   mpd_status_get_next_song_id(status));
        lua_table_push_int(L, "elapsed_time",   mpd_status_get_elapsed_time(status));
        lua_table_push_int(L, "elapsed_ms",     mpd_status_get_elapsed_ms(status));
        lua_table_push_int(L, "total_time",     mpd_status_get_total_time(status));
        lua_table_push_int(L, "kbit_rate",      mpd_status_get_kbit_rate(status));
        lua_table_push_int(L, "update_id",      mpd_status_get_update_id(status));

        /* free status struct */
        mpd_status_free(status);

        return 1;
}

/*
 * get tags from a song struct and push them in a table onto the stack
 * internal function
 * argument is the lua_State and a pointer to the song
 * no returns
 */
void mpd_parse_song(lua_State *L, struct mpd_song *song)
{
        /* create table for results */
        lua_newtable(L);

        /* iterate through tags and add them to the table */
        lua_table_push_str(L, "title",          mpd_song_get_tag(song, MPD_TAG_TITLE, 0));
        lua_table_push_str(L, "artist",         mpd_song_get_tag(song, MPD_TAG_ARTIST, 0));
        lua_table_push_str(L, "album_artist",   mpd_song_get_tag(song, MPD_TAG_ALBUM_ARTIST, 0));
        lua_table_push_str(L, "album",          mpd_song_get_tag(song, MPD_TAG_ALBUM, 0));
        lua_table_push_str(L, "track",          mpd_song_get_tag(song, MPD_TAG_TRACK, 0));
        lua_table_push_str(L, "name",           mpd_song_get_tag(song, MPD_TAG_NAME, 0));
        lua_table_push_str(L, "genre",          mpd_song_get_tag(song, MPD_TAG_GENRE, 0));
        lua_table_push_str(L, "date",           mpd_song_get_tag(song, MPD_TAG_DATE, 0));
        lua_table_push_str(L, "composer",       mpd_song_get_tag(song, MPD_TAG_COMPOSER, 0));
        lua_table_push_str(L, "performer",      mpd_song_get_tag(song, MPD_TAG_PERFORMER, 0));
        lua_table_push_str(L, "comment",        mpd_song_get_tag(song, MPD_TAG_COMMENT, 0));
        lua_table_push_str(L, "disc",           mpd_song_get_tag(song, MPD_TAG_DISC, 0));

        /* add non-tag fields */
        lua_table_push_str(L, "uri",            mpd_song_get_uri(song));
        lua_table_push_int(L, "duration",       mpd_song_get_duration(song));
        lua_table_push_int(L, "start",          mpd_song_get_start(song));
        lua_table_push_int(L, "end",            mpd_song_get_end(song));
        lua_table_push_int(L, "last_modified",  mpd_song_get_last_modified(song));
        lua_table_push_int(L, "pos",            mpd_song_get_pos(song));
        lua_table_push_int(L, "id",             mpd_song_get_id(song));
}

/*
 * get now playing track data
 * lua function
 * argument is connection
 * return is a table with tag data
 */
int mpd_now_playing(lua_State *L)
{
        get_mpd_conn(L);

        /* check that the state is playing or paused */
        struct mpd_status *status = NULL;
        status = mpd_run_status(conn);
        enum mpd_state state = mpd_status_get_state(status);
        bool cont = (state == MPD_STATE_PLAY) || (state == MPD_STATE_PAUSE);
        mpd_status_free(status);
        if (!cont)
                return 0;

        /* get current song from mpd */
        struct mpd_song *song = mpd_run_current_song(conn);

        /* parse song struct */
        mpd_parse_song(L, song);

        /* free song struct */
        mpd_song_free(song);

        return 1;
}

/*
 * receive a list of songs from mpd
 * internal function
 * arguments are lua_State and mpd connection
 * will push table of song results to lua
 * return is number of songs received
 */
int mpd_recv_song_list(lua_State *L, struct mpd_connection *conn)
{
        /* create table for results */
        lua_newtable(L);

        /* iterate through songs and add them to table */
        int counter = 0;
        struct mpd_song *song = NULL;
        while ((song = mpd_recv_song(conn)) != NULL)
        {
                counter++;
                lua_pushinteger(L, counter);
                mpd_parse_song(L, song);
                lua_settable(L, -3);
                mpd_song_free(song);
        }

        return counter;
}

/*
 * get current playlist
 * lua function
 * argument is connection
 * return is a table of mpd tags
 */
int mpd_cur_playlist(lua_State *L)
{
        get_mpd_conn(L);

        /* initiate playlist transmission */
        mpd_send_list_queue_meta(conn);

        /* receive songs */
        mpd_recv_song_list(L, conn);

        return 1;
}

/*
 * run a search
 * lua function
 * arguments are connection, boolean for exact search, then pairs of search type/key arguments
 */
int mpd_search(lua_State *L)
{
        get_mpd_conn(L);

        /* determine if search is exact */
        bool exact = false;
        if (!lua_isboolean(L, 2))
        {
                lua_pushstring(L, "invalid arguments.  boolean for exact search must be second");
                return 1;
        }
        exact = lua_toboolean(L, 2);
        mpd_search_db_songs(conn, exact);

        /* add search constraints */
        int c;
        const int nargs = lua_gettop(L);
        for (c = 4; c <= nargs; c+=2)
        {
                if (!lua_isstring(L, c) || !lua_isstring(L, c-1))
                        break;
                const char *type = lua_tostring(L, c-1);
                const char *value = lua_tostring(L, c);
                if (strcasecmp(type, "any") == 0)
                        mpd_search_add_any_tag_constraint(conn, MPD_OPERATOR_DEFAULT, value);
                else
                {
                        enum mpd_tag_type mode = MPD_TAG_COUNT;
                        int i;
                        for (i = 0; i < MPD_TAG_COUNT; i++)
                        {
                                const char *tag = mpd_tag_name(i);
                                if (strcasecmp(type, tag) == 0)
                                {
                                        mode = i;
                                        break;
                                }
                        }
                        if (mode != MPD_TAG_COUNT)
                                mpd_search_add_tag_constraint(conn, MPD_OPERATOR_DEFAULT, mode, value);
                }
        }

        /* run search */
        mpd_search_commit(conn);

        /* receive songs */
        mpd_recv_song_list(L, conn);

        return 1;
}

/*
 * get mpd statistics
 * lua function
 * argument is connection
 * return is a table of statistics
 */
int mpd_stats(lua_State *L)
{
        get_mpd_conn(L);

        /* create a table for stats */
        lua_newtable(L);

        /* get stats from mpd */
        struct mpd_stats *stats = mpd_run_stats(conn);

        /* simple values */
        lua_table_push_int(L, "artists",        mpd_stats_get_number_of_artists(stats));
        lua_table_push_int(L, "albums",         mpd_stats_get_number_of_albums(stats));
        lua_table_push_int(L, "songs",          mpd_stats_get_number_of_songs(stats));
        lua_table_push_int(L, "uptime",         mpd_stats_get_uptime(stats));
        lua_table_push_int(L, "play_time",      mpd_stats_get_play_time(stats));
        lua_table_push_int(L, "db_play_time",   mpd_stats_get_db_play_time(stats));

        /* db update time */
        long update_time = mpd_stats_get_db_update_time(stats);
        char *update_time_str = ctime(&update_time);
        *(strchr(update_time_str, '\n')) = 0;
        lua_table_push_str(L, "db_update_time", update_time_str);

        /* free stats */
        mpd_stats_free(stats);

        return 1;
}

/*
 * set mpd volume
 * lua function
 * arguments are connection, volume to set (0-100)
 * no returns
 */
int mpd_volume(lua_State *L)
{
        get_mpd_conn(L);

        /* get volume from arguments */
        int vol = -1;
        if (lua_isnumber(L, 2) == 1)
                vol = lua_tonumber(L, 2);
        else
                return 0;
        if (vol < 0 || vol > 100)
                return 0;

        /* set volume */
        mpd_run_set_volume(conn, vol);
        return 0;
}

/*
 * set a boolean
 * internal function
 * arguments are lua_State and function to set boolean
 * will parse boolean mode to set from lua_State
 * return status of command run
 */
int mpd_set_bool(lua_State *L, bool (*func)(struct mpd_connection *, bool))
{
        get_mpd_conn(L);

        /* get state from arguments */
        bool mode;
        if (lua_isboolean(L, 2) == 1)
                mode = lua_toboolean(L, 2);
        else
                return 0;

        return func(conn, mode);
}

/*
 * set mpd random state
 * lua function
 * arguments are connection, boolean random state to set
 * no returns
 */
int mpd_random(lua_State *L)
{
        return mpd_set_bool(L, &mpd_run_random);
}

/*
 * set mpd consume state
 * lua function
 * arguments are connection, boolean random state to set
 * no returns
 */
int mpd_consume(lua_State *L)
{
        return mpd_set_bool(L, &mpd_run_consume);
}

/*
 * set mpd repeat state
 * lua function
 * arguments are connection, boolean random state to set
 * no returns
 */
int mpd_repeat(lua_State *L)
{
        return mpd_set_bool(L, &mpd_run_repeat);
}

/*
 * set mpd single state
 * lua function
 * arguments are connection, boolean random state to set
 * no returns
 */
int mpd_single(lua_State *L)
{
        return mpd_set_bool(L, &mpd_run_single);
}

/* index of functions */
static const struct luaL_Reg mpd[] =
{
        {"connect",             mpd_connect},
        {"consume",             mpd_consume},
        {"free_connection",     mpd_free_connection},
        {"next",                mpd_next},
        {"now_playing",         mpd_now_playing},
        {"play",                mpd_play},
        {"playlist",            mpd_cur_playlist},
        {"prev",                mpd_prev},
        {"random",              mpd_random},
        {"repeat",              mpd_repeat},
        {"search",              mpd_search},
        {"set_volume",          mpd_volume},
        {"single",              mpd_single},
        {"state",               mpd_state},
        {"stats",               mpd_stats},
        {"stop",                mpd_stop},
        {"toggle",              mpd_toggle},
        {NULL,                  NULL}
};

/* register functions on load */
int luaopen_mpd (lua_State *L)
{
        luaL_newlib(L, mpd);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "mpd");
        return 1;
}
