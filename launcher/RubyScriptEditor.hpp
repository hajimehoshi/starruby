#ifndef RUBYSCRIPTEDITOR_HPP
#define RUBYSCRIPTEDITOR_HPP

#include <wx/wx.h>
#include <wx/filename.h>
#include "RubyScriptEditorObserver.hpp"

class RubyScriptEditor : public wxPanel
{
public:
  RubyScriptEditor(wxWindow* parent,
                   RubyScriptEditorObserver* observer,
                   const wxFileName& path = wxFileName(wxEmptyString));
  const wxFileName& GetPath();
  bool IsModified();
  bool Save();
  void SetPath(const wxFileName& path);
protected:
  void OnEdit(wxCommandEvent& event);
private:
  RubyScriptEditorObserver* observer;
  wxFileName path;
  wxTextCtrl* textCtrl;
  static const int TEXT_CTRL_ID = 100;
  DECLARE_EVENT_TABLE();
};

#endif
