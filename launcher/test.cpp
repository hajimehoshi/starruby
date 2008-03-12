#include "wx/wx.h"

class MyApp: public wxApp
{
public:
  bool OnInit();
};

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
  wxFrame* frame = new wxFrame(NULL, -1, wxT("sample"), wxPoint(-1, -1), wxSize(600, 400));
  SetTopWindow(frame);
  frame->Show();
  return true;
}
