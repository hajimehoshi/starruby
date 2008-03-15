#include "MainFrame.hpp"

#include <ruby.h>

BEGIN_EVENT_TABLE(MainFrame, wxFrame)

EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_MENU(wxID_NEW,  MainFrame::OnNew)
EVT_MENU(wxID_OPEN, MainFrame::OnOpen)
EVT_MENU(100,       MainFrame::OnPlay)

END_EVENT_TABLE()

MainFrame::MainFrame()
     : wxFrame(NULL, wxID_ANY, wxT("Star Ruby Launcher"))
{
  wxMenuBar* menuBar = new wxMenuBar();
  this->SetMenuBar(menuBar);

  wxMenu* menu = new wxMenu();
  menuBar->Append(menu, wxT("&File"));
  menu->Append(wxID_NEW,    wxT("&New"),        wxEmptyString, wxITEM_NORMAL);
  menu->Append(wxID_OPEN,   wxT("&Open..."),    wxEmptyString, wxITEM_NORMAL);
  menu->Append(wxID_SAVE,   wxT("&Save..."),    wxEmptyString, wxITEM_NORMAL);
  menu->Append(wxID_SAVEAS, wxT("Save &As..."), wxEmptyString, wxITEM_NORMAL);
  menu->AppendSeparator();
  menu->Append(wxID_EXIT, wxT("E&xit"), wxEmptyString, wxITEM_NORMAL);

  menu = new wxMenu();
  menuBar->Append(menu, wxT("&Game"));
  menu->Append(100, wxT("&Play"), wxEmptyString, wxITEM_NORMAL);

  menu = new wxMenu();
  menuBar->Append(menu, wxT("&Help"));
  menu->Append(wxID_ANY, wxT("&About"), wxEmptyString, wxITEM_NORMAL);

  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  this->SetSizer(sizer);

  this->rubyScriptNotebook = new RubyScriptNotebook(this);
  sizer->Add(this->rubyScriptNotebook, 1, wxEXPAND, 0);

#if 0
  wxToolBar* toolBar = new wxToolBar(this, wxID_ANY);
  this->SetToolBar(toolBar);
  toolBar->Realize();
#endif

  wxStatusBar* statusBar = new wxStatusBar(this, wxID_ANY);
  this->SetStatusBar(statusBar);

  this->SetSize(wxSize(640, 480));
  this->Layout();
}

void MainFrame::OnExit(wxCommandEvent& event)
{
  this->Close(true);
  wxExit();
}

void MainFrame::OnNew(wxCommandEvent& event)
{
  this->rubyScriptNotebook->AddNewPage();
}

void MainFrame::OnOpen(wxCommandEvent& event)
{
  wxFileDialog fileDialog(this,
                          wxT("Choose a file to open"),
                          wxEmptyString, wxEmptyString,
                          wxT("Ruby files (*.rb)|*.rb|All files (*.*)|*.*"),
                          wxOPEN);
  if (fileDialog.ShowModal() == wxID_OK)
    this->rubyScriptNotebook->AddPage(fileDialog.GetPath());
}

void MainFrame::OnPlay(wxCommandEvent& event)
{
  ruby_init();
  rb_eval_string("puts 'hello'");
}

void MainFrame::OnSave(wxCommandEvent& event)
{
}

void MainFrame::OnSaveAs(wxCommandEvent& event)
{
}
