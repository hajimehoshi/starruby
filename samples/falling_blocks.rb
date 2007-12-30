require "starruby"
require "falling_blocks/application_model"
require "falling_blocks/controller"
require "falling_blocks/view"

def main
  application_model = FallingBlocks::ApplicationModel.new(ARGV[0].to_i)
  controller = FallingBlocks::Controller.new
  view = FallingBlocks::View.new
  StarRuby::Game.title = "Falling Blocks Game"
  StarRuby::Game.run(320, 240, :window_scale => 2) do
    controller.update(application_model)
    view.update(application_model, StarRuby::Game.screen)
    GC.start
  end
end

main
