#include "MainFrame.hpp"
#include <ruby.h>

BEGIN_EVENT_TABLE(MainFrame, wxFrame)

EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_MENU(wxID_OPEN, MainFrame::OnOpen)

END_EVENT_TABLE()

MainFrame::MainFrame()
: wxFrame(NULL, -1, wxT("Star Ruby Launcher"), wxPoint(-1, -1), wxSize(-1, -1))
{
  wxMenuBar* menuBar = new wxMenuBar();
  this->SetMenuBar(menuBar);
  
  wxMenu* menu = new wxMenu();
  menuBar->Append(menu, wxT("&File"));
  menu->Append(wxID_OPEN, wxT("&Open"), wxEmptyString, wxITEM_NORMAL);
  menu->AppendSeparator();
  menu->Append(wxID_EXIT, wxT("E&xit"), wxEmptyString, wxITEM_NORMAL);

  menu = new wxMenu();
  menuBar->Append(menu, wxT("&Game"));
  menu->Append(wxID_ANY, wxT("&Play"), wxEmptyString, wxITEM_NORMAL);
  
  this->Layout();
}

void MainFrame::OnExit(wxCommandEvent& event)
{
  this->Close(true);
}

void MainFrame::OnOpen(wxCommandEvent& event)
{
}

void MainFrame::OnPlay(wxCommandEvent& event)
{
}
