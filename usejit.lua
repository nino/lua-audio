-- Usage example with LuaJIT

local ffi = require "ffi"
ffi.cdef [[
  int lj_play(int idx);
  int lj_initialize(const char** sample_paths, size_t num_samples);
]]
local libaudio = ffi.load("./libaudio.so")

function initialize(samples)
  local c_samples = ffi.new("const char*[" .. tostring(#samples) .. "]", samples)
  libaudio.lj_initialize(c_samples, #samples)

end

function play(idx)
  local c_index = ffi.new("int", idx - 1)
  libaudio.lj_play(c_index)
end

initialize( {
  "assets/hihat.wav",
  "assets/hihat2.wav",
})

-- while true do
--   index = tonumber(io.read())
--   play(index)
-- end

play(1)
