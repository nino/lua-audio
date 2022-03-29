-- This is how you might use the library with Lua (not LuaJIT)


local audio = require "libaudio"

print("audio is ", audio)

audio.initialize({
  "assets/hihat.wav",
  "assets/hihat2.wav",
})

while true do
  index = io.read()
  audio.play(index)
end


-- The hard way

-- local audio = package.loadlib("./libaudio.so", "l_lib_info")
-- audio = audio()
