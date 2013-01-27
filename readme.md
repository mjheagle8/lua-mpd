Lua mpd
=======

Lua bindings to libmpdclient, by mjheagle
-----------------------------------------

Lua functions
-------------
`connect`
  - establishes a connection to the mpd server
  - arguments:
    - host (string)
    - port (int)
    - timeout in ms (int)
  - return is light userdata containing a pointer to the connection
`free_connection`
  - frees the connection to the mpd server
  - arguments:
    - connection pointer
  - no returns
`stop`
  - stops playback
  - arguments:
    - connection pointer
  - return is boolean indicating success
`play`
  - starts playback
  - arguments:
    - connection pointer
  - return is boolean indicating success
`toggle`
  - toggles play/pause state
  - arguments:
    - connection pointer
  - return is boolean indicating success
`next`
  - moves to next song in queue
  - arguments:
    - connection pointer
  - return is boolean indicating success
`prev`
  - moves to previous song in queue
  - arguments:
    - connection pointer
  - return is boolean indicating success
`state`
  - obtains the mpd status
  - arguments:
    - connection pointer
  - return is table containing mpd status
`now_playing`
  - obtains information about the now playing track
  - arguments:
    - connection pointer
  - return is a table containing now playing data
`playlist`
  - gets the contents of the queue
  - arguments:
    - connection pointer
  - return is a table, with each element being a table containing song data
`playlists`
  - gets a list of playlists
  - arguments:
    - connection pointer
  - return is a table of playlist names
`search`
  - runs a search
  - arguments:
    - connection pointer
    - boolean indicating whether search terms should be matched exactly
    - tag type to match (string)
    - value to query (string)
    - more than one search parameter can be used, with subsequent searches using
    the 5th/6th arguments, etc
  - return is a table, with each element being a table containing song data
`stats`
  - obtain statistics from mpd server
  - arguments:
    - connection pointer
  - return is a table containing statistics
`volume`
  - set the mpd volume
  - arguments:
    - connection pointer
    - volume (int 0-100)
  - return is boolean indicating success
`random`
  - set the mpd random state
  - arguments:
    - connection pointer
    - boolean to set
  - return is boolean indicating success
`consume`
  - set the mpd consume state
  - arguments:
    - connection pointer
    - boolean to set
  - return is boolean indicating success
`repeat`
  - set the mpd repeat state
  - arguments:
    - connection pointer
    - boolean to set
  - return is boolean indicating success
`single`
  - set the mpd single state
  - arguments:
    - connection pointer
    - boolean to set
  - return is boolean indicating success
`version`
  - obtain the mpd server version
  - arguments:
    - connection pointer
  - return is a string representing the mpd version
`move`
  - move a song in the queue
  - arguments:
    - connection pointer
    - zero-indexed position to move from (int)
    - zero-indexed position to move to (int)
  - return is boolean indicating success

Roadmap
-------
* indicates features implemented
- indicates features to be implemented/completed

- connections
  * connect
  * free
  - passwords
* controls
  * next
  * play
  * previous
  * stop
  * toggle
- enable/disable sound outputs
* get mpd version
- playlist
  - add to playlist
  - clear playlist
  * get playlist
  * list playlists
  - load playlist
  - move song in playlist
  - remove from playlist
  - remove playlist
  - save playlist
* search
* state
  * get current state
  * toggle consume
  * toggle random
  * toggle repeat
  * toggle single
  * modify volume
  * use booleans where possible
* stats
- update database

# vim: ft=markdown ts=2 sw=2
