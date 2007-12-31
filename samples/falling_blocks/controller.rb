module FallingBlocks
  
  input = StarRuby::Input
  
  def input.dir?(direction)
    
  end
  
  class Controller
    
    def update(model)
      keyboard_keys = Input.keys(:keyboard)
      keyboard_keys_trigger = Input.keys(:keyboard, :duration => 1)
      keyboard_keys_repeating = Input.keys(:keyboard, {
        :duration => 1, :delay => 2, :interval => 0
      })
      gamepad_keys = Input.keys(:gamepad)
      gamepad_keys_trigger = Input.keys(:gamepad, :duration => 1)
      gamepad_keys_repeating = Input.keys(:gamepad, {
        :duration => 1, :delay => 2, :interval => 0
      })
      case model.state
      when :start
        if [:keyboard, :gamepad].any? do |device|
            0 < Input.keys(device, :duration => 1).size
          end
          model.start_playing
        end
      when :playing
        if keyboard_keys_trigger.include?(:c) or
          gamepad_keys_trigger.include?(3)
          model.pause
        elsif model.flashing?
          @count ||= 20
          @count -= 1
          if @count <= 0
            model.finish_flashing
            @count = nil
          end
        else
          if keyboard_keys_repeating.include?(:left) or
            gamepad_keys_repeating.include?(:left)
            model.try_move(:left)
          elsif keyboard_keys_repeating.include?(:right) or
            gamepad_keys_repeating.include?(:right)
            model.try_move(:right)
          end
          if keyboard_keys_trigger.include?(:z) or
            gamepad_keys_trigger.include?(2)
            model.try_rotate(:left)
          elsif keyboard_keys_trigger.include?(:x) or
            gamepad_keys_trigger.include?(1)
            model.try_rotate(:right)
          end
          down_pressed = (keyboard_keys.include?(:down) or
          gamepad_keys.include?(:down))
          if model.falling_piece_landing?
            @count ||= 20
            @count -= down_pressed ? 3 : 1
            if @count <= 0
              model.do_landing
              @count = nil
            end
          else
            model.add_score(1) if down_pressed
            model.try_fall(down_pressed)
          end
        end
      when :pause
        if [:keyboard, :gamepad].any? do |device|
            0 < Input.keys(device, :duration => 1).size
          end
          model.unpause
        end
      when :gameover
        if [:keyboard, :gamepad].any? do |device|
            0 < Input.keys(device, :duration => 1).size
          end
          model.start
        end
      end
    end
    
  end
  
end
