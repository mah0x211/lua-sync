require('luacov')
local testcase = require('testcase')
local assert = require('assert')
local mutex = require('sync.mutex')

function testcase.trylock_returns_true_when_unlocked()
    local m = mutex.new()
    assert.is_true(m:trylock())
    assert.is_true(m:unlock())
    m:destroy()
end

function testcase.trylock_succeeds_when_already_locked_by_same_instance()
    local m = mutex.new()
    assert.is_true(m:lock())
    -- the mutex tracks lock state per-instance, so trylock succeeds
    -- even when already locked by this userdata
    assert.is_true(m:trylock())
    m:unlock()
    m:destroy()
end
