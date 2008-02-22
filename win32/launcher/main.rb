require "gtk2"

title = "Star Ruby Launcher"

window = Gtk::Window.new
window.signal_connect("delete_event") do
  Gtk.main_quit
  false
end
window.border_width = 10
window.title = title

button = Gtk::Button.new("Play the Game")
clicked = button.signal_connect("clicked") do |w|
  begin
    # button.signal_handler_block(clicked)
    # w.sensitive = false
    state = button.state
    button.state = Gtk::StateType::INSENSITIVE
    load("game.rb")
  ensure
    # button.signal_handler_unblock(clicked)
    # w.sensitive = true
    # button.state = state
    button.state = Gtk::StateType::ACTIVE
  end
end
window.add(button)

window.show_all

Gtk.main
