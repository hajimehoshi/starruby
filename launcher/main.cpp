#include <wx/wx.h>
#include "MainFrame.hpp"

class MainApp : public wxApp
{
public:
  bool OnInit() {
    MainFrame* frame = new MainFrame();
    SetTopWindow(frame);
    frame->Show();
    return true;
  }
};

IMPLEMENT_APP(MainApp);
