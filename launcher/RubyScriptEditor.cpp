#include <wx/textfile.h>
#include "RubyScriptEditor.hpp"

BEGIN_EVENT_TABLE(RubyScriptEditor, wxPanel)

EVT_TEXT(TEXT_CTRL_ID, RubyScriptEditor::OnEdit)

END_EVENT_TABLE()

RubyScriptEditor::RubyScriptEditor(wxWindow* parent, const wxString& path)
     : wxPanel(parent, wxID_ANY)
{
  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  this->SetSizer(sizer);
  
  this->textCtrl = new wxTextCtrl(this, TEXT_CTRL_ID,
                                  wxEmptyString,
                                  wxDefaultPosition,
                                  wxDefaultSize,
                                  wxTE_MULTILINE);
  sizer->Add(this->textCtrl, 1, wxEXPAND, 0);
  this->textCtrl->SetFont(wxFont(12,
                                 wxFONTFAMILY_TELETYPE,
                                 wxFONTSTYLE_NORMAL,
                                 wxFONTWEIGHT_NORMAL,
                                 false,
                                 wxT("ＭＳ ゴシック")));
  this->path = path;
  if (path != wxEmptyString)
    this->textCtrl->LoadFile(path);
}

const wxString& RubyScriptEditor::GetPath()
{
  return this->path;
}

bool RubyScriptEditor::IsModified()
{
  return this->textCtrl->IsModified();
}

void RubyScriptEditor::OnEdit(wxCommandEvent& event)
{
  this->textCtrl->MarkDirty();
  event.Skip();
}
