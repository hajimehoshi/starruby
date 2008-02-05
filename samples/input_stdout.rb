require "starruby"
include StarRuby

puts "The count of Gamepad: #{Input.gamepad_count}"
puts "Press any gamepad key"
puts "Exit if you press the button 1 and the button 2"

loop do
  Input.update
  unless (keys = Input.keys(:gamepad)).empty?
    puts "Gamepad: #{keys.inspect}"
    break if keys.include?(1) and keys.include?(2)
  end
  sleep 0.1
end
