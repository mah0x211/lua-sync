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

function testcase.timedwait_succeeds_when_signaled()
    local c = cond.new()
    c:lock()
    local p = assert(fork())
    if p:is_child() then
        sleep(0.2)
        c:signal()
        c:destroy()
    else
        local ok = c:timedwait(1.5)
        assert.is_true(ok)
        c:unlock()
        local stat = assert(p:wait())
        assert.is_table(stat)
        c:destroy()
    end
end

function testcase.wait_succeeds_when_signaled()
    local c = cond.new()
    c:lock()
    local p = assert(fork())
    if p:is_child() then
        sleep(0.2)
        c:signal()
        c:destroy()
    else
        local ok = c:wait()
        assert.is_true(ok)
        c:unlock()
        local stat = assert(p:wait())
        assert.is_table(stat)
        c:destroy()
    end
end

function testcase.destroy_releases_resources()
    local c = cond.new()
    c:lock()
    assert.is_true(c:destroy())
end

function testcase.destroy_returns_true_when_already_destroyed()
    local c = cond.new()
    assert.is_true(c:destroy())
    assert.is_true(c:destroy())
end

function testcase.gc_unlocks_when_locked()
    local c = cond.new()
    c:lock()
    c = nil
    collectgarbage('collect')
    collectgarbage('collect')
end

function testcase.trylock_fails_when_held_by_another_process()
    local c = cond.new()
    local p = assert(fork())
    if p:is_child() then
        c:lock()
        sleep(1)
        c:unlock()
        c:destroy()
    else
        sleep(0.2)
        local ok = c:trylock()
        assert.is_false(ok)
        c:lock()
        local stat = assert(p:wait())
        assert.is_table(stat)
        c:unlock()
        c:destroy()
    end
end
