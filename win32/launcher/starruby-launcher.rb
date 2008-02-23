require "swin"
require "starruby"
require "yaml"

module StarRubyLauncher
  
  TITLE = "Star Ruby Launcher"
  SUB_TITLE = "Ver. #{StarRuby::VERSION}"
  
  NORMAL_COLOR = StarRuby::Color.new(0x66, 0x44, 0x44)
  DISABLED_COLOR = StarRuby::Color.new(0x66, 0x44, 0x44, 64)
  
  NORMAL_FONT = StarRuby::Font.new("MS UI Gothic", 12)
  TITLE_FONT = StarRuby::Font.new("Georgia", 18)
  SUB_TITLE_FONT = StarRuby::Font.new("Georgia", 14)
  
  class Button
    
    attr_accessor :label
    attr_accessor :x
    attr_accessor :y
    attr_accessor :width
    attr_accessor :height
    
    def enabled?
      @enabled
    end
    
    def enabled=(val)
      @enabled = val
    end
    
    def initialize(label, x, y, width, height, &block)
      @label = label
      @x = x
      @y = y
      @width = width
      @height = height
      @on_clicked = block
      @enabled = true
    end
    
    def hover?
      mouse_x, mouse_y = StarRuby::Input.mouse_location
      (x...(x + width)).include?(mouse_x) and
      (y...(y + height)).include?(mouse_y)
    end
    
    def update
      clicked = StarRuby::Input.keys(:mouse, :duration => 1).include?(:left)
      @on_clicked.call(self) if enabled? and clicked and hover?
    end
    
    def render(s)
      if hover? and enabled?
        color = StarRuby::Color.new(0x99, 0x33, 0x33, 32)
        s.render_rect(x, y, width, height, color)
      end
      color = StarRuby::Color.new(0xbb, 0xbb, 0xbb)
      s.render_line(x, y, x + width, y, color)
      s.render_line(x, y, x, y + height, color)
      color = StarRuby::Color.new(0x66, 0x66, 0x66)
      s.render_line(x + width, y, x + width, y + height, color)
      s.render_line(x, y + height, x + width, y + height, color)
      width, height = NORMAL_FONT.get_size(label)
      x = self.x + 12
      y = self.y + (self.height - height) / 2
      str = label
      while self.width - 32 < NORMAL_FONT.get_size(str)[0]
        head = str[Regexp.new("^" + "." * (str.length - 22))]
        tail = str[/.{16}$/]
        str = "#{head} ... #{tail}"
      end
      color = enabled? ? NORMAL_COLOR : DISABLED_COLOR
      s.render_text(str, x, y, NORMAL_FONT, color, false)
    end
    
  end
  
  class Main
    
    CONFIG_FILE_PATH = File.join(Dir.pwd, "config.yaml")
    
    include StarRuby
    
    def initialize
      if File.exist?(CONFIG_FILE_PATH)
        @config = YAML.load_file(CONFIG_FILE_PATH)
      else
        @config = {:script_path => ""}
        save_config
      end
      @buttons = {}
      Game.title = TITLE
    end
    
    def main
      if ARGV[0]
        load(ARGV[0])
        exit
      end
      following_proc = nil
      loop do
        following_proc = nil
        Game.run(320, 240, :cursor => true) do
          label = "Script File:  #{@config[:script_path]}"
          @buttons[:load] ||= Button.new(label, 16, 56, 288, 24) do |b|
            dialog = SWin::CommonDialog
            dialog_option = [["Ruby Script File (*.rb)","*.rb"]]
            begin
              pwd = Dir.pwd
              new_path = dialog.openFilename(nil, dialog_option)
              @config[:script_path] = new_path if new_path
            ensure
              Dir.chdir(pwd)
            end
            b.label = "Script File:  #{@config[:script_path]}"
          end
          @buttons[:play] ||= Button.new("Play", 16, 96, 288, 24) do
            following_proc = proc do
              begin
                title = Game.title
                Game.title = ""
                begin
                  load_path = $LOAD_PATH.dup
                  $LOAD_PATH.clear
                  $LOAD_PATH << Dir.pwd + "/lib/1.8"
                  $LOAD_PATH << "."
                  Dir.chdir(File.dirname(@config[:script_path])) do
                    begin
                      load(@config[:script_path])
                    ensure
                      Audio.stop_bgm
                      Audio.stop_all_ses
                    end
                  end
                ensure
                  $LOAD_PATH.clear
                  $LOAD_PATH.push(*load_path)
                end
              ensure
                Game.title = title
              end
            end
            Game.terminate
          end
          path = @config[:script_path]
          @buttons[:play].enabled = (path and not path.empty?)
          @buttons[:exit] ||= Button.new("Exit", 16, 200, 288, 24) do
            Game.terminate
          end

          @buttons.each {|key, button| button.update}
          
          s = Game.screen
          s.fill_rect(0, 0, 320, 40,   Color.new(0x99, 0x33, 0x33))
          s.fill_rect(0, 40, 320, 200, Color.new(0xff, 0xff, 0xff))

          x, y = 16, 10
          s.render_text(TITLE, x, y, TITLE_FONT, Color.new(255, 255, 255), true)
          width, height = TITLE_FONT.get_size(TITLE)

          x += width + 8
          y += height - SUB_TITLE_FONT.get_size(SUB_TITLE)[1]
          color = Color.new(0xcc, 0xdd, 0xdd)
          s.render_text(SUB_TITLE, x, y, SUB_TITLE_FONT, color, true)
          @buttons.each do |key, button|
            button.render(s)
          end
        end
        following_proc ? following_proc.call : break
      end
    ensure
      save_config
    end
    
    private
    
    def save_config
      open(CONFIG_FILE_PATH, "w") do |fp|
        YAML.dump(@config, fp)
      end
    end
    
  end
  
end

StarRubyLauncher::Main.new.main
