#ifndef RUBYSCRIPTEDITOR_HPP
#define RUBYSCRIPTEDITOR_HPP

#include <wx/wx.h>

class RubyScriptEditor : public wxPanel
{
public:
  RubyScriptEditor(wxWindow* parent, const wxString& path = wxEmptyString);
  const wxString& GetPath();
  bool IsModified();
protected:
  void OnEdit(wxCommandEvent& event);
private:
  wxString path;
  wxTextCtrl* textCtrl;
  static const int TEXT_CTRL_ID = 100;
  DECLARE_EVENT_TABLE();
};

#endif
