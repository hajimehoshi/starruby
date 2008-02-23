#!/usr/bin/env ruby

require "starruby"
include StarRuby

if Input.gamepad_count == 0
  $stderr.puts "There is no gamepad"
  exit
end

$stdout.puts "The count of Gamepad: #{Input.gamepad_count}"
$stdout.puts "Press any gamepad key"
$stdout.puts "Exit if you press the button 1 and the button 2"
$stdout.flush

loop do
  Input.update
  unless (keys = Input.keys(:gamepad)).empty?
    $stdout.puts "Gamepad: #{keys.inspect}"
    $stdout.flush
    break if keys.include?(1) and keys.include?(2)
  end
  sleep 0.1
end
