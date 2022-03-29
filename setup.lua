function callshell(cmd)
  local proc = io.popen(cmd)
  local output = proc:read("a")
  proc:close()
  return output
end

ninja_file = io.open("build.ninja", "w")

local cflags = callshell("pkg-config --libs luajit --cflags luajit")
-- pkg-config adds these flags, but they make the compilation fail :shrug:
cflags = string.gsub(cflags, "-pagezero_size 10000", "")
cflags = string.gsub(cflags, "-image_base 100000000", "")

ninja_file:write([[
# This file is generated by setup.lua

ninja_required_version = 1.3

flags = ]] .. cflags .. [[

rule cclib
  command = clang $in -shared -o $out $flags

build libaudio.so: cclib audio.c
]])

ninja_file:close()
