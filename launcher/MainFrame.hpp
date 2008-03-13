#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

#include <wx/wx.h>

class MainFrame : public wxFrame
{
public:
  MainFrame();
protected:
  void OnOpen(wxCommandEvent& event);
  void OnExit(wxCommandEvent& event);
private:
  wxMenuBar* MenuBar;
  DECLARE_EVENT_TABLE();
};

#endif
