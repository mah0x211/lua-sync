require('luacov')
local testcase = require('testcase')
local fork = require('testcase.fork')
local sleep = require('testcase.timer').sleep
local assert = require('assert')
local semaphore = require('sync.semaphore')

-- NOTE: named semaphores (sem_open) may not be available in all
-- environments (e.g. CI containers without /dev/shm).

function testcase.module_loads()
    assert.is_table(semaphore)
    assert.is_function(semaphore.new)
end

function testcase.new_returns_nil_when_unsupported()
    local s, err = semaphore.new()
    if s then
        assert.match(tostring(s), '^sync%.semaphore: 0x', false)
        assert.is_true(s:post())
        assert.is_true(s:wait())
        assert.is_true(s:close())
    else
        assert.is_nil(s)
        assert.is_string(err)
    end
end

function testcase.trywait_fails_when_empty()
    local s = semaphore.new()
    if not s then
        return
    end
    local ok = s:trywait()
    assert.is_false(ok)
    s:close()
end

function testcase.trywait_succeeds_after_post()
    local s = semaphore.new()
    if not s then
        return
    end
    s:post()
    assert.is_true(s:trywait())
    s:close()
end

function testcase.close_is_idempotent()
    local s = semaphore.new()
    if not s then
        return
    end
    assert.is_nil(s:close())
    assert.is_nil(s:close())
end

function testcase.gc_closes_when_not_closed()
    local s = semaphore.new()
    if not s then
        return
    end
    s = nil
    collectgarbage('collect')
    collectgarbage('collect')
end

function testcase.wait_blocks_until_child_posts()
    local s = semaphore.new()
    if not s then
        return
    end
    local p = assert(fork())
    if p:is_child() then
        sleep(0.5)
        s:post()
        s:close()
    else
        assert.is_true(s:wait())
        local stat = assert(p:wait())
        assert.is_table(stat)
        s:close()
    end
end
