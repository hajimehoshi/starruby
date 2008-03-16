#ifndef RUBYSCRIPTNOTEBOOK_HPP
#define RUBYSCRIPTNOTEBOOK_HPP

#include <wx/wx.h>
#include <wx/aui/auibook.h>
#include <wx/filename.h>
#include "RubyScriptEditor.hpp"
#include "RubyScriptEditorObserver.hpp"

class RubyScriptNotebook : public wxPanel, RubyScriptEditorObserver
{
public:
  RubyScriptNotebook(wxWindow* parent);
  void AddNewPage();
  void AddPage(const wxFileName& path);
  RubyScriptEditor* GetCurrentPage();
  bool HasCurrentPage();
  virtual void Notify();
private:
  void Update();
  wxAuiNotebook* auiNotebook;
};

#endif
