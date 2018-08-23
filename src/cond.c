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
 *  src/cond.c
 *  lua-sync
 *  Created by Masatoshi Teruya on 18/08/15.
 *
 */

#include "sync.h"
#include <math.h>


static int timedwait_lua( lua_State *L )
{
    sync_cond_t *c = luaL_checkudata( L, 1, SYNC_COND_MT );
    lua_Number sec = lauxh_checknumber( L, 2 );
    double isec = 0.0;
    double fsec = modf( sec, &isec );
    struct timespec abstime = { 0 };

    lauxh_argcheck( L, sec >= 0, 2, "sec must be greater or equal to 0" );

#if defined(CLOCK_REALTIME_COARSE)
    clock_gettime( CLOCK_REALTIME_COARSE, &abstime );
#else
    clock_gettime( CLOCK_REALTIME, &abstime );
#endif

    abstime.tv_nsec += fsec * 1000000000ULL;
    if( abstime.tv_nsec > 1000000000ULL ){
        abstime.tv_sec += isec + abstime.tv_nsec / 1000000000ULL;
        abstime.tv_nsec %= 1000000000ULL;
    }
    else {
        abstime.tv_sec += isec;
    }

    if( sync_cond_timedwait( c->cond, c->mutex, &abstime ) ){
        lua_pushboolean( L, 0 );
        lua_pushstring( L, strerror( errno ) );
        return 2;
    }

    lua_pushboolean( L, 1 );

    return 1;
}


static int wait_lua( lua_State *L )
{
    sync_cond_t *c = luaL_checkudata( L, 1, SYNC_COND_MT );

    if( sync_cond_wait( c->cond, c->mutex ) ){
        lua_pushboolean( L, 0 );
        lua_pushstring( L, strerror( errno ) );
        return 2;
    }

    lua_pushboolean( L, 1 );

    return 1;
}


static int broadcast_lua( lua_State *L )
{
    sync_cond_t *c = luaL_checkudata( L, 1, SYNC_COND_MT );

    if( sync_cond_broadcast( c->cond ) ){
        lua_pushboolean( L, 0 );
        lua_pushstring( L, strerror( errno ) );
        return 2;
    }

    lua_pushboolean( L, 1 );

    return 1;
}


static int signal_lua( lua_State *L )
{
    sync_cond_t *c = luaL_checkudata( L, 1, SYNC_COND_MT );

    if( sync_cond_signal( c->cond ) ){
        lua_pushboolean( L, 0 );
        lua_pushstring( L, strerror( errno ) );
        return 2;
    }

    lua_pushboolean( L, 1 );

    return 1;
}


static int unlock_lua( lua_State *L )
{
    sync_unlockop_lua( L, sync_cond_t, SYNC_COND_MT, sync_mutex_unlock );
}


static int trylock_lua( lua_State *L )
{
    sync_cond_t *c = luaL_checkudata( L, 1, SYNC_COND_MT );

    if( c->locked == 0 && sync_mutex_trylock( c->mutex ) ){
        lua_pushboolean( L, 0 );
        lua_pushstring( L, strerror( errno ) );
        lua_pushboolean( L, errno == EBUSY );
        return 3;
    }

    c->locked = 1;
    lua_pushboolean( L, 1 );

    return 1;
}


static int lock_lua( lua_State *L )
{
    sync_lockop_lua( L, sync_cond_t, SYNC_COND_MT, sync_mutex_lock );
}


static int destroy_lua( lua_State *L )
{
    sync_cond_t *c = luaL_checkudata( L, 1, SYNC_COND_MT );

    if( c->cond )
    {
        if( sync_cond_destroy( c->cond ) ){
            lua_pushboolean( L, 0 );
            lua_pushstring( L, strerror( errno ) );
            lua_pushboolean( L, errno == EBUSY );
            return 3;
        }
        sync_cond_free( c->cond );
        c->cond = NULL;
    }

    if( c->mutex )
    {
        if( c->locked ){
            c->locked = 0;
            sync_mutex_unlock( c->mutex );
        }

        if( sync_mutex_destroy( c->mutex ) ){
            lua_pushboolean( L, 0 );
            lua_pushstring( L, strerror( errno ) );
            lua_pushboolean( L, errno == EBUSY );
            return 3;
        }
        sync_mutex_free( c->mutex );
        c->mutex = NULL;
    }

    lua_pushboolean( L, 1 );

    return 1;
}


static int tostring_lua( lua_State *L )
{
    lua_pushfstring( L, SYNC_COND_MT ": %p", lua_touserdata( L, 1 ) );
    return 1;
}


static int gc_lua( lua_State *L )
{
    sync_cond_t *c = lua_touserdata( L, 1 );

    if( c->locked ){
        c->locked = 0;
        sync_mutex_unlock( c->mutex );
    }

    return 0;
}


static int new_lua( lua_State *L )
{
    lua_settop( L, 0 );
    sync_cond_t *c = lua_newuserdata( L, sizeof( sync_cond_t ) );

    c->locked = 0;
    if( ( c->mutex = sync_mutex_alloc() ) )
    {
        if( ( c->cond = sync_cond_alloc() ) ){
            lauxh_setmetatable( L, SYNC_COND_MT );
            return 1;
        }
        sync_mutex_free( c->mutex );
    }

    lua_pushnil( L );
    lua_pushstring( L, strerror( errno ) );

    return 2;
}


LUALIB_API int luaopen_sync_cond( lua_State *L )
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
        { "signal", signal_lua },
        { "broadcast", broadcast_lua },
        { "wait", wait_lua },
        { "timedwait", timedwait_lua },
        { NULL, NULL }
    };

    sync_register( L, SYNC_COND_MT, mmethods, methods );

    // add new function
    lua_newtable( L );
    lauxh_pushfn2tbl( L, "new", new_lua );

    return 1;
}

