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

/*
 * table helper function to push an integer
 * internal function
 * arguments are the lua_State, key name, and value
 */
void lua_table_push_int(lua_State *L, const char *key, const int value)
{
        lua_pushstring(L, key);
        lua_pushinteger(L, value);
        lua_settable(L, -3);
}

/*
 * table helper function to push a float
 * internal function
 * arguments are the lua_State, key name, and value
 */
void lua_table_push_float(lua_State *L, const char *key, const float value)
{
        lua_pushstring(L, key);
        lua_pushnumber(L, value);
        lua_settable(L, -3);
}

/*
 * table helper function to push a string
 * internal function
 * arguments are the lua_State, key name, and value
 */
void lua_table_push_str(lua_State *L, const char *key, const char *value)
{
        lua_pushstring(L, key);
        lua_pushstring(L, value);
        lua_settable(L, -3);
}

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
        printf("host: %s\n", host);
        if (!lua_isnumber(L, 2))
                goto errorout;
        const int port = lua_tonumber(L, 2);
        printf("port: %d\n", port);
        if (!lua_isnumber(L, 3))
                goto errorout;
        const int timeout = lua_tonumber(L, 3);
        printf("timeout: %d\n", timeout);

        /* establish connection */
        struct mpd_conection *conn = mpd_connection_new(host, port, timeout);
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
        /* get mpd connection from arguments */
        struct mpd_connection *conn = NULL;
        if (lua_islightuserdata(L, 1) == 1)
                 conn = (struct mpd_connection *)lua_topointer(L, 1);
        else
                return 0;

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
        /* get mpd connection from arguments */
        struct mpd_connection *conn = NULL;
        if (lua_islightuserdata(L, 1) == 1)
                 conn = (struct mpd_connection *)lua_topointer(L, 1);
        else
                return 0;

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
        /* get mpd connection from arguments */
        struct mpd_connection *conn = NULL;
        if (lua_islightuserdata(L, 1) == 1)
                 conn = (struct mpd_connection *)lua_topointer(L, 1);
        else
                return 0;

        /* get status from mpd */
        struct mpd_status *status = NULL;
        status = mpd_run_status(conn);

        /* create table for results */
        lua_newtable(L);

        /* get state */
        enum mpd_state state = mpd_status_get_state(status);
        if (state == MPD_STATE_STOP)
                lua_table_push_str(L, "state", "stopped");
        else if (state == MPD_STATE_PLAY)
                lua_table_push_str(L, "state", "playing");
        else if (state == MPD_STATE_PAUSE)
                lua_table_push_str(L, "state", "paused");
        else
        {
                lua_table_push_str(L, "state", "unknown");
                return 1;
        }

        /* simple values */
        lua_table_push_int(L, "volume",         mpd_status_get_volume(status));
        lua_table_push_int(L, "random",         mpd_status_get_random(status));
        lua_table_push_int(L, "repeat",         mpd_status_get_repeat(status));
        lua_table_push_int(L, "single",         mpd_status_get_single(status));
        lua_table_push_int(L, "consume",        mpd_status_get_consume(status));
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
}

/*
 * get now playing track data
 * lua function
 * argument is connection
 * return is a table with tag data
 */
int mpd_now_playing(lua_State *L)
{
        /* get mpd connection from arguments */
        struct mpd_connection *conn = NULL;
        if (lua_islightuserdata(L, 1) == 1)
                conn = (struct mpd_connection *)lua_topointer(L, 1);
        else
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
 * get current playlist
 * lua function
 * argument is connection
 * return is a table of mpd tags
 */
int mpd_cur_playlist(lua_State *L)
{
        /* get mpd connection from arguments */
        struct mpd_connection *conn = NULL;
        if (lua_islightuserdata(L, 1) == 1)
                conn = (struct mpd_connection *)lua_topointer(L, 1);
        else
                return 0;

        /* create table for playlist */
        lua_newtable(L);

        /* initiate playlist transmission */
        mpd_send_list_queue_meta(conn);

        /* loop through songs */
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
        printf("%d songs recieved\n", counter);

        return 1;
}

/* index of functions */
static const struct luaL_Reg mpd[] =
{
        {"connect",             mpd_connect},
        {"play",                mpd_play},
        {"stop",                mpd_stop},
        {"next",                mpd_next},
        {"prev",                mpd_prev},
        {"toggle",              mpd_toggle},
        {"state",               mpd_state},
        {"now_playing",         mpd_now_playing},
        {"playlist",            mpd_cur_playlist},
        {"free_connection",     mpd_free_connection},
        {NULL,                  NULL}
};

/* register functions on load */
int luaopen_mpd (lua_State *L)
{
        luaL_newlib(L, mpd);
        lua_setglobal(L, "mpd");
        return 1;
}
