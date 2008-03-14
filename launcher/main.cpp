#include <wx/wx.h>
#include "MainFrame.hpp"

class MainApp : public wxApp
{
public:
  bool OnInit();
};

IMPLEMENT_APP(MainApp);

bool MainApp::OnInit()
{
  MainFrame* frame = new MainFrame();
  this->SetTopWindow(frame);
  frame->Show(true);
  frame->Enable(true);
  return true;
}
