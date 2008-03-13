#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

#include <wx/wx.h>

class MainFrame : public wxFrame
{
public:
  MainFrame();
protected:
  void OnExit(wxCommandEvent& event);
  void OnOpen(wxCommandEvent& event);
  void OnPlay(wxCommandEvent& event);
private:
  DECLARE_EVENT_TABLE();
};

#endif
