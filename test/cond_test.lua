require('luacov')
local testcase = require('testcase')
local fork = require('testcase.fork')
local sleep = require('testcase.timer').sleep
local assert = require('assert')
local cond = require('sync.cond')

function testcase.new_returns_object()
    local c = cond.new()
    assert.not_nil(c)
    assert.match(tostring(c), '^sync%.cond: 0x', false)
    c:destroy()
end

function testcase.lock_and_unlock()
    local c = cond.new()
    assert.is_true(c:lock())
    assert.is_true(c:unlock())
    c:destroy()
end

function testcase.trylock_succeeds_when_unlocked()
    local c = cond.new()
    assert.is_true(c:trylock())
    assert.is_true(c:unlock())
    c:destroy()
end

function testcase.signal_returns_true()
    local c = cond.new()
    assert.is_true(c:signal())
    c:destroy()
end

function testcase.broadcast_returns_true()
    local c = cond.new()
    assert.is_true(c:broadcast())
    c:destroy()
end

function testcase.timedwait_returns_false_on_timeout()
    local c = cond.new()
    c:lock()
    local ok = c:timedwait(0.1)
    c:unlock()
    assert.is_false(ok)
    c:destroy()
end

function testcase.destroy_releases_resources()
    local c = cond.new()
    c:lock()
    assert.is_true(c:destroy())
end

function testcase.wait_signals_between_processes()
    local c = cond.new()
    local p = assert(fork())
    if p:is_child() then
        c:lock()
        c:wait()
        c:unlock()
        c:destroy()
    else
        sleep(0.5)
        c:signal()
        local stat = assert(p:wait())
        assert.is_table(stat)
        c:destroy()
    end
end
