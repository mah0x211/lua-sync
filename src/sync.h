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
#include <pthread.h>
#include <sys/mman.h>
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



// allocation from shared-mmap
#define sync_shmalloc(t) \
    (t*)mmap(NULL,sizeof(t),PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_SHARED,-1,0)

#define sync_shmfree(t, v)  \
    munmap((void*)(v),sizeof(t))


// helper macros for pthread operations
#define sync_pthread_alloc(t) ({                                                \
    pthread_## t ##_t *v = sync_shmalloc(pthread_## t ##_t);                    \
    if((void*)v != MAP_FAILED){                                                 \
        pthread_## t ##attr_t a;                                                \
        int rc = pthread_## t ##attr_init(&a);                                  \
        if(rc ||                                                                \
           (rc = pthread_## t ##attr_setpshared(&a, PTHREAD_PROCESS_SHARED)) || \
           (rc = pthread_## t ##_init(v, &a)) ){                                \
            sync_shmfree(pthread_## t ##_t, v);                                 \
            v = NULL;                                                           \
            errno = rc;                                                         \
        }                                                                       \
        pthread_## t ##attr_destroy(&a);                                        \
    }                                                                           \
    v;                                                                          \
})


#define sync_pthread_op(opfn, ...) ({   \
    int rc = (opfn)(__VA_ARGS__);       \
    if(rc){                             \
        errno = rc;                     \
        rc = -1;                        \
    }                                   \
    rc;                                 \
})


#define sync_lockop_lua(L, t, tname, lockfn) do{    \
    t *v = luaL_checkudata( L, 1, (tname) );        \
    if( v->locked == 0 && lockfn( v->mutex ) ){     \
        lua_pushboolean( L, 0 );                    \
        lua_pushstring( L, strerror( errno ) );     \
        return 2;                                   \
    }                                               \
    v->locked = 1;                                  \
    lua_pushboolean( L, 1 );                        \
    return 1;                                       \
}while(0)


#define sync_unlockop_lua(L, t, tname, unlockfn) do{    \
    t *v = luaL_checkudata( L, 1, (tname) );            \
    if( v->locked == 1 && unlockfn( v->mutex ) ){       \
        lua_pushboolean( L, 0 );                        \
        lua_pushstring( L, strerror( errno ) );         \
        return 2;                                       \
    }                                                   \
    v->locked = 0;                                      \
    lua_pushboolean( L, 1 );                            \
    return 1;                                           \
}while(0)


#define SYNC_MUTEX_MT   "sync.mutex"

typedef struct {
    int locked;
    pthread_mutex_t *mutex;
} sync_mutex_t;

#define sync_mutex_alloc()      sync_pthread_alloc(mutex)
#define sync_mutex_free(m)      sync_shmfree(pthread_mutex_t, m)
#define sync_mutex_lock(m)      sync_pthread_op(pthread_mutex_lock, m)
#define sync_mutex_trylock(m)   sync_pthread_op(pthread_mutex_trylock, m)
#define sync_mutex_unlock(m)    sync_pthread_op(pthread_mutex_unlock, m)
#define sync_mutex_destroy(m)   sync_pthread_op(pthread_mutex_destroy, m)

LUALIB_API int luaopen_sync_mutex( lua_State *L );



#define SYNC_COND_MT    "sync.cond"

typedef struct {
    int locked;
    int ref;
    pthread_cond_t *cond;
    pthread_mutex_t *mutex;
} sync_cond_t;

#define sync_cond_alloc()       sync_pthread_alloc(cond)
#define sync_cond_free(c)       sync_shmfree(pthread_cond_t, c)
#define sync_cond_signal(c)     sync_pthread_op(pthread_cond_signal, c)
#define sync_cond_broadcast(c)  sync_pthread_op(pthread_cond_broadcast, c)
#define sync_cond_destroy(c)    sync_pthread_op(pthread_cond_destroy, c)
#define sync_cond_wait(c, m)    sync_pthread_op(pthread_cond_wait, c, m)
#define sync_cond_timedwait(c, m, t) \
    sync_pthread_op(pthread_cond_timedwait, c, m, t)

LUALIB_API int luaopen_sync_cond( lua_State *L );



#endif
