# Really bad Lua binding for miniaudio

Compile the library with `make lib` and then you should be able to use it as
shown in `use.lua`. Currently seems to work with Lua v5.4 but not Lua v5.1. I'm
planning to fix that because my neovim version is built with Lua v5.1 (or some
equivalent version of LuaJIT, I really have no clue, please help).

I should probably use CMake or some nonensense to make the build more portable.
