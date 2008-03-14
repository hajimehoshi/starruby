#ifndef SCRIPTEDITOR_HPP
#define SCRIPTEDITOR_HPP

#include <wx/wx.h>

class ScriptEditorPanel : public wxPanel
{
public:
  ScriptEditorPanel(wxWindow* parent);
private:
  DECLARE_EVENT_TABLE();
};

#endif
