require('luacov')
local testcase = require('testcase')
local assert = require('assert')
local semaphore = require('sync.semaphore')

-- NOTE: named semaphores (sem_open) may not be available in all
-- environments (e.g. CI containers without /dev/shm).  These tests
-- verify that the module loads and handles the unsupported case.

function testcase.module_loads()
    assert.is_table(semaphore)
    assert.is_function(semaphore.new)
end

function testcase.new_returns_nil_when_unsupported()
    local s, err = semaphore.new()
    if s then
        -- named semaphores are supported in this environment
        assert.match(tostring(s), '^sync%.semaphore: 0x', false)
        assert.is_true(s:post())
        assert.is_true(s:wait())
        assert.is_true(s:close())
    else
        -- unsupported: new() returns nil with an error string
        assert.is_nil(s)
        assert.is_string(err)
    end
end
