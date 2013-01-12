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

/* create a connection to mpd */
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

/* toggle mpd status */
int mpd_toggle(lua_State *L)
{
        /* get mpd connection from arguments */
        struct mpd_connection *conn = NULL;
        if (lua_islightuserdata(L, 1) == 1)
                 conn = (struct mpd_connection *)lua_topointer(L, 1);
        else
                return 0;

        /* run command */
        mpd_run_toggle_pause(conn);

        return 0;
}

/* free mpd connection */
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

/* index of functions */
static const struct luaL_Reg mpd[] =
{
        {"connect",             mpd_connect},
        {"toggle",              mpd_toggle},
        {"free_connection",     mpd_free_connection},
        {NULL,          NULL}
};

/* register functions on load */
int luaopen_mpd (lua_State *L)
{
        luaL_newlib(L, mpd);
        lua_setglobal(L, "mpd");
        return 1;
}
