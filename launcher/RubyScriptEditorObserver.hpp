#ifndef RUBYSCRIPTEDITOROBSERVER_HPP
#define RUBYSCRIPTEDITOROBSERVER_HPP

class RubyScriptEditorObserver
{
public:
  virtual ~RubyScriptEditorObserver();
  virtual void Notify() = 0;
};

#endif
