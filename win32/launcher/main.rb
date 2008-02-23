require "swin"
require "starruby"
require "yaml"

module StarRubyLauncher
  
  TITLE = "Star Ruby Launcher Ver. #{StarRuby::VERSION}".freeze
  
  TEXT_COLOR = StarRuby::Color.new(0x66, 0x44, 0x44)
  BORDER_COLOR = StarRuby::Color.new(0xbb, 0xbb, 0xbb)
  
  NORMAL_FONT = StarRuby::Font.new("MS UI Gothic", 12)
  TITLE_FONT = StarRuby::Font.new("Georgia", 18)
  
  class Button
    
    attr_accessor :label
    attr_accessor :x
    attr_accessor :y
    attr_accessor :width
    attr_accessor :height
    
    def initialize(label, x, y, width, height, &block)
      @label = label
      @x = x
      @y = y
      @width = width
      @height = height
      @on_clicked = block
    end
    
    def hover?
      mouse_x, mouse_y = StarRuby::Input.mouse_location
      (x...(x + width)).include?(mouse_x) and
      (y...(y + height)).include?(mouse_y)
    end
    
    def update
      clicked = StarRuby::Input.keys(:mouse, :duration => 1).include?(:left)
      @on_clicked.call if clicked and hover?
    end
    
    def render(s)
      if hover?
        s.render_rect(x, y, width, height, StarRuby::Color.new(0x99, 0x33, 0x33, 32))
      end
      s.render_line(x, y, x + width, y, BORDER_COLOR)
      s.render_line(x, y, x, y + height, BORDER_COLOR)
      s.render_line(x + width, y, x + width, y + height, BORDER_COLOR)
      s.render_line(x, y + height, x + width, y + height, BORDER_COLOR)
      width, height = NORMAL_FONT.get_size(label)
      x = self.x + 16
      y = self.y + (self.height - height) / 2
      s.render_text(label, x, y, NORMAL_FONT, TEXT_COLOR, false)
    end
    
  end
  
  class Main
    
    
    CONFIG_FILE_NAME = "config.yaml"
    
    include StarRuby
    
    def initialize
      if File.exist?(CONFIG_FILE_NAME)
        @config = YAML.load_file(CONFIG_FILE_NAME)
      else
        @config = {:path => ""}
        save_config
      end
      @buttons = {}
      @buttons[:load] = Button.new("Load", 16, 56, 288, 32) do
        dialog = SWin::CommonDialog
        dialog_option = [["Ruby Script File (*.rb)","*.rb"]]
        begin
          pwd = Dir.pwd
          @config[:path] = dialog.openFilename(nil, dialog_option)
        ensure
          Dir.chdir(pwd)
        end
      end
      #@buttons[:play] = Button.new("Play") do
      #end
      @buttons[:exit] = Button.new("Exit", 16, 192, 288, 32) do
        Game.terminate
      end
      Game.title = TITLE
      Game.cursor_visible = true
    end
    
    def main
      loop do
        @following_proc = nil
        Game.run(320, 240) do
          @buttons.each do |key, button|
            button.update
          end
          s = Game.screen
          s.fill_rect(0, 0, 320, 40,   Color.new(0x99, 0x33, 0x33))
          s.fill_rect(0, 40, 320, 200, Color.new(0xff, 0xff, 0xff))
          s.render_text(TITLE, 16, 10, TITLE_FONT, Color.new(255, 255, 255), true)
          @buttons.each do |key, button|
            button.render(s)
          end
          if @config[:path] and not @config[:path].empty?
            load_button = @buttons[:load]
            x = load_button.x
            y = load_button.y + load_button.height + 16
            str = @config[:path]
            while 288 < NORMAL_FONT.get_size(str)[0]
              head = str[/^.{#{str.length - 22}}/]
              tail = str[/.{16}$/]
              str = "#{head} ... #{tail}"
            end
            s.render_text(str, x, y, NORMAL_FONT, TEXT_COLOR)
          end
          x, y = Input.mouse_location
=begin
        file_name = nil
        if file_name
          following_proc = proc do
            load(file_name)
          end
        end
=end
        end
        @following_proc ? @following_proc.call : break
      end
    ensure
      save_config
    end
    
    private
    
    def save_config
      open(CONFIG_FILE_NAME, "w") do |fp|
        YAML.dump(@config, fp)
      end
    end
    
  end
  
end

StarRubyLauncher::Main.new.main
