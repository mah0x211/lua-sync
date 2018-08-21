package = "sync"
version = "scm-1"
source = {
    url = "gitrec://github.com/mah0x211/lua-sync.git"
}
description = {
    summary = "lua-sync provides basic synchronization primitives",
    homepage = "https://github.com/mah0x211/lua-sync",
    license = "MIT/X11",
    maintainer = "Masatoshi Fukunaga"
}
dependencies = {
    "lua >= 5.1",
    "luarocks-fetch-gitrec >= 0.2",
}
build = {
    type = "builtin",
    platforms = {
        linux = {
            ['sync.semaphore'] = {
                libraries = { "pthread", "rt" },
            },
        },
        macosx = {
            modules = {
                ['sync.semaphore'] = {
                    libraries = { "pthread" },
                },
            },
        },
    },
    modules = {
        ['sync.semaphore'] = {
            incdirs = { "deps/lauxhlib" },
            sources = { "src/semaphore.c" },
        },
    },
}

