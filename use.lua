local audio = require "libaudio"

audio.initialize({
  "assets/hihat.wav",
  "assets/hihat2.wav",
})

while true do
  index = io.read("n")
  audio.play(index)
end
