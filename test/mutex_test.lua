require('luacov')
local testcase = require('testcase')
local fork = require('testcase.fork')
local sleep = require('testcase.timer').sleep
local assert = require('assert')
local mutex = require('sync.mutex')

function testcase.new_returns_object()
    local m = mutex.new()
    assert.not_nil(m)
    assert.match(tostring(m), '^sync%.mutex: 0x', false)
    m:destroy()
end

function testcase.lock_and_unlock()
    local m = mutex.new()
    assert.is_true(m:lock())
    assert.is_true(m:unlock())
    m:destroy()
end

function testcase.trylock_succeeds_when_unlocked()
    local m = mutex.new()
    assert.is_true(m:trylock())
    assert.is_true(m:unlock())
    m:destroy()
end

function testcase.destroy_releases_resources()
    local m = mutex.new()
    m:lock()
    assert.is_true(m:destroy())
end

function testcase.mutex_provides_exclusion_between_processes()
    local m = mutex.new()
    local p = assert(fork())
    if p:is_child() then
        m:lock()
        sleep(1)
        m:unlock()
        m:destroy()
    else
        -- wait for child to acquire the lock
        sleep(0.2)
        local ok = m:trylock()
        -- mutex is held by child, trylock should fail
        assert.is_false(ok)
        m:lock()
        local stat = assert(p:wait())
        assert.is_table(stat)
        m:unlock()
        m:destroy()
    end
end
