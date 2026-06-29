package = "sync"
version = "scm-1"
source = {
    url = "git+https://github.com/mah0x211/lua-sync.git",
}
description = {
    summary = "lua-sync provides basic synchronization primitives",
    homepage = "https://github.com/mah0x211/lua-sync",
    license = "MIT/X11",
    maintainer = "Masatoshi Fukunaga",
}
dependencies = {
    "lua >= 5.1",
    "lauxhlib >= 0.5.0",
}
external_dependencies = {
    PTHREAD = {
        header = "pthread.h",
        library = "pthread",
    },
}
build = {
    type = "builtin",
    platforms = {
        linux = {
            ["sync.semaphore"] = {
                libraries = {
                    "pthread",
                    "rt",
                },
            },
        },
        macosx = {
            modules = {
                ["sync.semaphore"] = {
                    libraries = {
                        "pthread",
                    },
                },
            },
        },
    },
    modules = {
        ["sync.semaphore"] = {
            sources = {
                "src/semaphore.c",
            },
        },
        ["sync.mutex"] = {
            incdirs = {
                "$(PTHREAD_INCDIR)",
            },
            libdirs = {
                "$(PTHREAD_LIBDIR)",
            },
            sources = {
                "src/mutex.c",
            },
            libraries = {
                "pthread",
            },
        },
        ["sync.cond"] = {
            incdirs = {
                "$(PTHREAD_INCDIR)",
            },
            libdirs = {
                "$(PTHREAD_LIBDIR)",
            },
            sources = {
                "src/cond.c",
            },
        },
    },
}
