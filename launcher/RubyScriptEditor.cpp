#include "RubyScriptEditor.hpp"

BEGIN_EVENT_TABLE(RubyScriptEditor, wxScintilla)

END_EVENT_TABLE()

RubyScriptEditor::RubyScriptEditor(wxWindow* parent)
     : wxScintilla(parent, wxID_ANY)
{
  this->SetLexer(wxSCI_LEX_RUBY);
  wxFont font(12,
              wxFONTFAMILY_TELETYPE,
              wxFONTSTYLE_NORMAL,
              wxFONTWEIGHT_NORMAL,
              false,
              wxT("‚l‚r ƒSƒVƒbƒN"));
  this->StyleSetFont(wxSCI_STYLE_DEFAULT, font);
}
