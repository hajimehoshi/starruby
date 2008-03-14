#include "MainFrame.hpp"
#include <wx/notebook.h>
#include <ruby.h>

BEGIN_EVENT_TABLE(MainFrame, wxFrame)

EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_MENU(wxID_OPEN, MainFrame::OnOpen)
EVT_MENU(100,       MainFrame::OnPlay)

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
  menu->Append(100, wxT("&Play"), wxEmptyString, wxITEM_NORMAL);

  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  this->SetSizer(sizer);

  wxNotebook* notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
  sizer->Add(notebook, 1, wxEXPAND, 0);
  wxPanel* panel = new wxPanel(notebook, wxID_ANY);
  notebook->AddPage(panel, wxT("New Script"));

  this->SetSize(wxSize(640, 480));
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
  ruby_init();
  rb_eval_string("puts 'hello'");
}
