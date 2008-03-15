#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

#include <wx/wx.h>
#include "RubyScriptNotebook.hpp"

class MainFrame : public wxFrame
{
public:
  MainFrame();
protected:
  void OnExit(wxCommandEvent& event);
  void OnNew(wxCommandEvent& event);
  void OnOpen(wxCommandEvent& event);
  void OnPlay(wxCommandEvent& event);
  void OnSave(wxCommandEvent& event);
  void OnSaveAs(wxCommandEvent& event);
private:
  RubyScriptNotebook* rubyScriptNotebook;
  DECLARE_EVENT_TABLE();
};

#endif
