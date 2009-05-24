require "starruby"
require "falling_blocks/model"
require "falling_blocks/controller"
require "falling_blocks/view"

def main
  # Model holds the state of this game
  model      = FallingBlocks::Model.new(ARGV[0].to_i)
  # Controller changes the state of the model
  controller = FallingBlocks::Controller.new(model)
  # View renders the screen
  view       = FallingBlocks::View.new(model)
  StarRuby::Game.run(320, 240,
                     :title => "Falling Blocks Game",
                     :window_scale => 2) do |game|
    # If ESC key is pressed, quit this game
    break if Input.keys(:keyboard).include?(:escape)
    # Update the controller
    controller.update
    # Update the view
    view.update(game.screen)
    # Start the gabage collection at each frame
    GC.start
  end
end

main
