module FallingBlocks
  
  StarRuby::Input.instance_eval do
    alias _orig_keys keys
    
    def keys(device, options = {})
      if device.kind_of?(Enumerable)
        device.inject([]) do |result, d|
          result | _orig_keys(d, options)
        end
      else
        _orig_keys(device, options)
      end
    end
    
    def triggers(device)
      keys(device, :duration => 1)
    end
    
    def repeatings(device)
      keys(device, {
        :duration => 1, :delay => 2, :interval => 0
      })
    end
  end

  class Controller

    attr_reader :model

    def initialize(model)
      @model = model
    end
    
    def update
      send("update_#{model.state}")
    end
    
    def update_start
      if 0 < Input.triggers([:keyboard, :gamepad]).size
        model.start_playing
      end
    end
  
    def update_playing
      if Input.triggers(:keyboard).include?(:c) or
        Input.triggers(:gamepad).include?(3)
        model.pause
      elsif model.flashing?
        @count ||= 20
        @count -= 1
        if @count <= 0
          model.finish_flashing
          @count = nil
        end
      else
        if Input.repeatings([:keyboard, :gamepad]).include?(:left)
          model.try_move(:left)
        elsif Input.repeatings([:keyboard, :gamepad]).include?(:right)
          model.try_move(:right)
        end
        if Input.triggers(:keyboard).include?(:z) or
          Input.triggers(:gamepad).include?(2)
          model.try_rotate(:left)
        elsif Input.triggers(:keyboard).include?(:x) or
          Input.triggers(:gamepad).include?(1)
          model.try_rotate(:right)
        end
        down_pressed = Input.keys([:keyboard, :gamepad]).include?(:down)
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
    end
    
    def update_pause
      if 0 < Input.triggers([:keyboard, :gamepad]).size
        model.resume
      end
    end
    
    def update_gameover
      if 0 < Input.triggers([:keyboard, :gamepad]).size
        model.start
      end
    end
    
  end
  
end
