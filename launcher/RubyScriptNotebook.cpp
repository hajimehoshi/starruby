#include "RubyScriptNotebook.hpp"

RubyScriptNotebook::RubyScriptNotebook(wxWindow* parent)
     : wxPanel(parent, wxID_ANY)
{
  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  this->SetSizer(sizer);

  this->auiNotebook = new wxAuiNotebook(this, wxID_ANY);
  sizer->Add(this->auiNotebook, 1, wxEXPAND, 0);
}

void RubyScriptNotebook::AddNewPage()
{
  RubyScriptEditor* page = new RubyScriptEditor(this->auiNotebook, this);
  this->auiNotebook->AddPage(page, wxEmptyString, true);
  this->Update();
}

void RubyScriptNotebook::AddPage(const wxFileName& path)
{
  RubyScriptEditor* page = new RubyScriptEditor(this->auiNotebook, this, path);
  this->auiNotebook->AddPage(page, wxEmptyString, true);
  this->Update();
}

RubyScriptEditor* RubyScriptNotebook::GetCurrentPage()
{
  int index = this->auiNotebook->GetSelection();
  return (RubyScriptEditor*)this->auiNotebook->GetPage(index);
}

bool RubyScriptNotebook::HasCurrentPage()
{
  return 0 < this->auiNotebook->GetPageCount();
}

void RubyScriptNotebook::Notify()
{
  this->Update();
}

void RubyScriptNotebook::Update()
{
  int count = this->auiNotebook->GetPageCount();
  for (int i = 0; i < count; i++) {
    RubyScriptEditor* editor = (RubyScriptEditor*)this->auiNotebook->GetPage(i);
    wxString title;
    if (editor->GetPath() != wxEmptyString)
      title += editor->GetPath().GetFullName();
    else
      title += wxT("(No Title)");
    if (editor->IsModified())
      title += wxT(" (*)");
    this->auiNotebook->SetPageText(i, title);
  }
}
