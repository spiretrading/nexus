#include "Spire/LegacyUI/MacroUndoCommand.hpp"
#include <boost/throw_exception.hpp>
#include "Spire/LegacyUI/IgnoreCommandException.hpp"

using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

MacroUndoCommand::MacroUndoCommand() {}

void MacroUndoCommand::AddCommand(QUndoCommand*&& command) {
  m_commands.emplace_back(move(command));
}

void MacroUndoCommand::undo() {
  bool commandExecuted = false;
  for(auto i = m_commands.rbegin(); i != m_commands.rend(); ++i) {
    try {
      (*i)->undo();
      commandExecuted = true;
    } catch(IgnoreCommandException&) {}
  }
  if(!commandExecuted) {
    BOOST_THROW_EXCEPTION(IgnoreCommandException());
  }
}

void MacroUndoCommand::redo() {
  for(auto i = m_commands.begin(); i != m_commands.end(); ++i) {
    (*i)->redo();
  }
}
