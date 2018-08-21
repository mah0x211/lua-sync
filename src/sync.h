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
 *  src/sync.h
 *  lua-sync
 *  Created by Masatoshi Teruya on 18/08/15.
 *
 */

#ifndef lua_sync_h
#define lua_sync_h

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
// lua
#include "lauxhlib.h"


// helper macros
static inline void sync_register( lua_State *L, const char *tname,
                                  struct luaL_Reg *mmethods,
                                  struct luaL_Reg *methods )
{
    if( luaL_newmetatable( L, tname ) )
    {
        struct luaL_Reg *ptr = mmethods;

        while( ptr->name ){
            lauxh_pushfn2tbl( L, ptr->name, ptr->func );
            ptr++;
        }

        lua_pushstring( L, "__index" );
        lua_newtable( L );
        ptr = methods;
        while( ptr->name ){
            lauxh_pushfn2tbl( L, ptr->name, ptr->func );
            ptr++;
        }
        lua_rawset( L, -3 );
    }
    lua_pop( L, 1 );
}



// semaphore
#define SYNC_SEMAPHORE_MT   "sync.semaphore"


typedef struct {
    sem_t *sem;
} sync_sem_t;


static inline sem_t *sync_sem_alloc( unsigned int v )
{
    char pathname[] = "/tmp/XXXXXX";
    int fd = mkstemp( pathname );

    if( fd != -1 )
    {
        sem_t *sem = NULL;

        unlink( pathname );
        sem = sem_open( pathname, O_CREAT|O_EXCL, S_IRUSR|S_IWUSR, v );
        close( fd );
        if( (void*)sem != SEM_FAILED ){
            sem_unlink( pathname );
            return sem;
        }
    }

    return NULL;
}


static inline int sync_sem_free( sem_t *sem )
{
    return sem_close( sem );
}


LUALIB_API int luaopen_sync_semaphore( lua_State *L );



#endif
