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
  void OnGamePlay(wxCommandEvent& event);
  void OnNew(wxCommandEvent& event);
  void OnOpen(wxCommandEvent& event);
  void OnSave(wxCommandEvent& event);
  void OnSaveAs(wxCommandEvent& event);
private:
  bool Save(RubyScriptEditor* page);
  bool SaveAs(RubyScriptEditor* page);
  RubyScriptNotebook* rubyScriptNotebook;
  static const int ID_GAMEPLAY = 100;
  DECLARE_EVENT_TABLE();
};

#endif
