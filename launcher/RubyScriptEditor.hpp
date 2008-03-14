#ifndef RUBYSCRIPTEDITOR_HPP
#define RUBYSCRIPTEDITOR_HPP

#include <wx/wx.h>
#include "wxscintilla.h"

class RubyScriptEditor : public wxScintilla
{
public:
  RubyScriptEditor(wxWindow* parent);
private:
  DECLARE_EVENT_TABLE();
};

#endif
