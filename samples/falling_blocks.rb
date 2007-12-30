require "starruby"
require "falling_blocks/game"
require "falling_blocks/view"

def main
  game = FallingBlocks::Game.new
  view = FallingBlocks::View.new
  StarRuby::Game.title = "Falling Blocks Game"
  StarRuby::Game.run(320, 240, :window_scale => 2) do
    game.update
    view.update(game, StarRuby::Game.screen)
  end
end

main
