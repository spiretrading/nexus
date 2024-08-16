#include "Spire/UI/CommandAction.hpp"
#include <QUndoCommand>

using namespace Spire;
using namespace Spire::UI;
using namespace std;

CommandAction::CommandAction(unique_ptr<QUndoCommand>&& command,
    QObject* parent)
    : QAction(parent),
      m_command(std::move(command)) {}

CommandAction::~CommandAction() {}

unique_ptr<QUndoCommand>& CommandAction::GetCommand() {
  return m_command;
}
