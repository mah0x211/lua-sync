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
 *  src/semaphore.c
 *  lua-sync
 *  Created by Masatoshi Teruya on 18/08/18.
 *
 */

#include "sync.h"
#include <math.h>
#include <semaphore.h>


static int trywait_lua( lua_State *L )
{
    sync_sem_t *s = luaL_checkudata( L, 1, SYNC_SEMAPHORE_MT );

    if( sem_trywait( s->sem ) == 0 ){
        lua_pushboolean( L, 1 );
        return 1;
    }

    lua_pushboolean( L, 0 );
    lua_pushstring( L, strerror( errno ) );
    lua_pushboolean( L, errno == EAGAIN );

    return 3;
}


static int wait_lua( lua_State *L )
{
    sync_sem_t *s = luaL_checkudata( L, 1, SYNC_SEMAPHORE_MT );

    if( sem_wait( s->sem ) == 0 ){
        lua_pushboolean( L, 1 );
        return 1;
    }

    lua_pushboolean( L, 0 );
    lua_pushstring( L, strerror( errno ) );

    return 2;
}


static int post_lua( lua_State *L )
{
    sync_sem_t *s = luaL_checkudata( L, 1, SYNC_SEMAPHORE_MT );

    if( sem_post( s->sem ) == 0 ){
        lua_pushboolean( L, 1 );
        return 1;
    }

    lua_pushboolean( L, 0 );
    lua_pushstring( L, strerror( errno ) );

    return 2;
}


static int close_lua( lua_State *L )
{
    sync_sem_t *s = luaL_checkudata( L, 1, SYNC_SEMAPHORE_MT );

    if( s->sem ){
        sync_sem_free( s->sem );
        s->sem = NULL;
    }

    return 0;
}


static int tostring_lua( lua_State *L )
{
    lua_pushfstring( L, SYNC_SEMAPHORE_MT ": %p", lua_touserdata( L, 1 ) );
    return 1;
}


static int new_lua( lua_State *L )
{
    lua_settop( L, 0 );
    sync_sem_t *s = lua_newuserdata( L, sizeof( sync_sem_t ) );
    uint32_t n = lauxh_optuint32( L, 2, 0 );

    if( ( s->sem = sync_sem_alloc( n ) ) ){
        lauxh_setmetatable( L, SYNC_SEMAPHORE_MT );
        return 1;
    }

    lua_pushnil( L );
    lua_pushstring( L, strerror( errno ) );

    return 2;
}


LUALIB_API int luaopen_sync_semaphore( lua_State *L )
{
    struct luaL_Reg mmethods[] = {
        { "__tostring", tostring_lua },
        { NULL, NULL }
    };
    struct luaL_Reg methods[] = {
        { "close", close_lua },
        { "post", post_lua },
        { "wait", wait_lua },
        { "trywait", trywait_lua },
        { NULL, NULL }
    };

    sync_register( L, SYNC_SEMAPHORE_MT, mmethods, methods );

    // add new function
    lua_newtable( L );
    lauxh_pushfn2tbl( L, "new", new_lua );

    return 1;
}

