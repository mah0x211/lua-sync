std = "max"
include_files = {
    "test/*_test.lua",
}
ignore = {
    "assert",
    "212", -- unused argument
    "311", -- value assigned to variable is unused (for gc tests)
}
