#include "MainFrame.hpp"

BEGIN_EVENT_TABLE(MainFrame, wxFrame)

EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_MENU(wxID_OPEN, MainFrame::OnOpen)

END_EVENT_TABLE()

MainFrame::MainFrame()
  : wxFrame(NULL, -1, wxT("Star Ruby Launcher"), wxPoint(-1, -1), wxSize(600, 400))
{
  this->MenuBar = new wxMenuBar();
  this->SetMenuBar(this->MenuBar);
  
  wxMenu* menu = new wxMenu();
  menu->Append(wxID_OPEN, wxT("&Open"), wxEmptyString, wxITEM_NORMAL);
  menu->AppendSeparator();
  menu->Append(wxID_EXIT, wxT("E&xit"), wxEmptyString, wxITEM_NORMAL);
  this->MenuBar->Append(menu, wxT("&File"));
  
  this->Layout();
}

void MainFrame::OnExit(wxCommandEvent& event)
{
  this->Close(true);
}

void MainFrame::OnOpen(wxCommandEvent& event)
{
}
