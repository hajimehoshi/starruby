#ifndef RUBYSCRIPTNOTEBOOK_HPP
#define RUBYSCRIPTNOTEBOOK_HPP

#include <wx/wx.h>
#include <wx/aui/auibook.h>

class RubyScriptNotebook : public wxPanel
{
public:
  RubyScriptNotebook(wxWindow* parent);
  void AddNewPage();
  void AddPage(const wxString& path);
protected:
  void OnEdit(wxCommandEvent& event);
private:
  void Update();
  wxAuiNotebook* auiNotebook;
  DECLARE_EVENT_TABLE();
};

#endif
