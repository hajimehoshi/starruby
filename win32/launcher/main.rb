require "gtk2"

button = Gtk::Button.new("Hello, World")
button.signal_connect("clicked") do
  puts "Hello, World"
end

window = Gtk::Window.new
window.signal_connect("delete_event") do
  Gtk.main_quit
  false
end
window.border_width = 10
window.title = "Star Ruby Launcher"
window.add(button)
window.show_all

Gtk.main
