/*
 *  Copyright (C) 2018 Masatoshi Fukunaga
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 *  src/mutex.c
 *  lua-sync
 *  Created by Masatoshi Teruya on 18/08/15.
 *
 */

#include "sync.h"


static int unlock_lua( lua_State *L )
{
    sync_unlockop_lua( L, sync_mutex_t, SYNC_MUTEX_MT, sync_mutex_unlock );
}


static int trylock_lua( lua_State *L )
{
    sync_mutex_t *m = luaL_checkudata( L, 1, SYNC_MUTEX_MT );

    if( m->locked == 0 && sync_mutex_trylock( m->mutex ) ){
        lua_pushboolean( L, 0 );
        lua_pushstring( L, strerror( errno ) );
        lua_pushboolean( L, errno == EBUSY );
        return 3;
    }

    m->locked = 1;
    lua_pushboolean( L, 1 );

    return 1;
}

static int lock_lua( lua_State *L )
{
    sync_lockop_lua( L, sync_mutex_t, SYNC_MUTEX_MT, sync_mutex_lock );
}


static int destroy_lua( lua_State *L )
{
    sync_mutex_t *m = luaL_checkudata( L, 1, SYNC_MUTEX_MT );

    if( m->mutex )
    {
        if( m->locked ){
            m->locked = 0;
            sync_mutex_unlock( m->mutex );
        }

        if( sync_mutex_destroy( m->mutex ) ){
            lua_pushboolean( L, 0 );
            lua_pushstring( L, strerror( errno ) );
            return 2;
        }
        sync_mutex_free( m->mutex );
        m->mutex = NULL;
    }

    lua_pushboolean( L, 1 );

    return 1;
}


static int tostring_lua( lua_State *L )
{
    lua_pushfstring( L, SYNC_MUTEX_MT ": %p", lua_touserdata( L, 1 ) );
    return 1;
}


static int gc_lua( lua_State *L )
{
    sync_mutex_t *m = lua_touserdata( L, 1 );

    if( m->mutex && m->locked ){
        sync_mutex_unlock( m->mutex );
    }

    return 0;
}


static int new_lua( lua_State *L )
{
    lua_settop( L, 0 );
    sync_mutex_t *m = lua_newuserdata( L, sizeof( sync_mutex_t ) );

    m->locked = 0;
    if( ( m->mutex = sync_mutex_alloc() ) ){
        lauxh_setmetatable( L, SYNC_MUTEX_MT );
        return 1;
    }

    lua_pushnil( L );
    lua_pushstring( L, strerror( errno ) );

    return 2;
}


LUALIB_API int luaopen_sync_mutex( lua_State *L )
{
    struct luaL_Reg mmethods[] = {
        { "__gc", gc_lua },
        { "__tostring", tostring_lua },
        { NULL, NULL }
    };
    struct luaL_Reg methods[] = {
        { "destroy", destroy_lua },
        { "lock", lock_lua },
        { "trylock", trylock_lua },
        { "unlock", unlock_lua },
        { NULL, NULL }
    };

    sync_register( L, SYNC_MUTEX_MT, mmethods, methods );

    // add new function
    lua_newtable( L );
    lauxh_pushfn2tbl( L, "new", new_lua );

    return 1;
}

