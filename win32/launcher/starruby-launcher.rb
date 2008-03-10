require "win32ole"

$LOAD_PATH.clear
$LOAD_PATH << "./lib" << "."
if ARGV[0]
  load(ARGV[0])
else
  wsh = WIN32OLE.new("WScript.Shell")
  wsh.Popup("Please Drag & Drop to execute a script file", 0, "Star Ruby Launcher", 0 + 64 + 0x40000)
end
