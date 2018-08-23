# lua-sync

lua-sync provides basic synchronization primitives.

## Dependencies

- luarocks-fetch-gitrec: <https://github.com/siffiejoe/luarocks-fetch-gitrec>
- lauxhlib: <https://github.com/mah0x211/lauxhlib>


## Installation

```bash
$ luarocks install sync --from=http://mah0x211.github.io/rocks/
```


## Semaphores

bindings to the POSIX semaphore with process sharing attribute.


### sem, err = semaphore.new( [n] )

create an instance of semaphore.

**Parameters**

- `n:uint32`: initial value.

**Returns**

- `sem:sync.semaphore`: instance of [sync.semaphore](#syncsemaphore-instance-methods).
- `err:string`: error string.

**Example**

```lua
local semaphore = require('sync.semaphore')
local sem = semaphore.new()

print( sem ) -- sync.semaphore: 0x0020bac8
```


## sync.semaphore Instance Methods

`sync.semaphore` instance has following methods.


### sem:close()

close a semaphore.


### ok, err = sem:post()

increment the value of semaphore.

**Returns**

- `ok:boolean`: true on success.
- `err:string`: error message.


### ok, err = sem:wait()

decrement the value of semaphore. if current value is 0, the calling process will block until either possible to perform the decrement or interrupted by signal.

**Returns**

- `ok:boolean`: true on success.
- `err:string`: error message.


### ok, err, again = sem:trywait()

attempt to decrement a value of semaphore without blocking.

**Returns**

- `ok:boolean`: true on success.
- `err:string`: error message.
- `again:boolean`: true if errno is `EAGAIN`.


## Mutual Exclusion Locks

bindings to the pthread mutex with process sharing attribute.

### m, err = mutex.new()

create an instance of mutex.

**Returns**

- `m:sync.mutex`: instance of [sync.mutex](#syncmutex-instance-methods).
- `err:string`: error string.

**Example**

```lua
local mutex = require('sync.mutex')
local m = mutex.new()

print( m ) -- sync.mutex: 0x0020c0e0
```


## sync.mutex Instance Methods

`sync.mutex` instance has following methods.


### ok, err, busy = m:destroy()

unlock a mutex and free resources allocated for a mutex.

**Returns**

- `ok:boolean`: true on success.
- `err:string`: error message.
- `busy:boolean`: true if errno is `EBUSY`.


### ok, err = m:lock()

lock a mutex. if the mutex is already locked, the calling process will block until the mutex becomes available.

**Returns**

- `ok:boolean`: true on success.
- `err:string`: error message.


### ok, err = m:trylock()

lock a mutex without blocking.

**Returns**

- `ok:boolean`: true on success.
- `err:string`: error message.
- `busy:boolean`: true if errno is `EBUSY`.


### ok, err = m:unlock()

unlock a mutex.

**NOTE**: if mutex is locked, it is automatically unlocked by the GC.

**Returns**

- `ok:boolean`: true on success.
- `err:string`: error message.



## Condition Variables

bindings to the pthread cond with process sharing attribute.

### c err = cond.new()

create an instance of cond.

**Returns**

- `c:sync.cond`: instance of [sync.cond](#synccond-instance-methods).
- `err:string`: error string.

**Example**

```lua
local cond = require('sync.cond')
local c = cond.new()

print( c ) -- sync.cond: 0x0020c118
```


## sync.cond Instance Methods

`sync.cond` instance has following methods.


### ok, err, busy = c:destroy()

unlock a mutex and free resources allocated for a cond.

**Returns**

- `ok:boolean`: true on success.
- `err:string`: error message.
- `busy:boolean`: true if errno is `EBUSY`.


### ok, err = c:lock()

lock a mutex. if the mutex is already locked, the calling process will block until the mutex becomes available.

**Returns**

- `ok:boolean`: true on success.
- `err:string`: error message.


### ok, err = c:trylock()

lock a mutex without blocking.

**Returns**

- `ok:boolean`: true on success.
- `err:string`: error message.
- `busy:boolean`: true if errno is `EBUSY`.


### ok, err = c:unlock()

unlock a mutex.

**NOTE**: if mutex is locked, it is automatically unlocked by the GC.

**Returns**

- `ok:boolean`: true on success.
- `err:string`: error message.


### ok, err = c:signal()

unblock a process waiting for a condition variable.

**Returns**

- `ok:boolean`: true on success.
- `err:string`: error message.


### ok, err = c:broadcast()

unblock all process waiting for a condition variable.

**Returns**

- `ok:boolean`: true on success.
- `err:string`: error message.


### ok, err = c:wait()

atomically unlocks the mutex and waits for the cond to be signaled.

**Returns**

- `ok:boolean`: true on success.
- `err:string`: error message.


### ok, err = c:timedwait( sec )

atomically unlocks the mutex and waits for the cond to be signaled or wait for the specified seconds.       

**Parameters**

- `sec:number`: unsigned number.

**Returns**

- `ok:boolean`: true on success.
- `err:string`: error message.
- `timeout:boolean`: true on timeout


