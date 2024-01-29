#ifndef SPIRE_COMMANDACTION_HPP
#define SPIRE_COMMANDACTION_HPP
#include <memory>
#include <QAction>
#include "Spire/LegacyUI/LegacyUI.hpp"

class QUndoCommand;

namespace Spire {
namespace LegacyUI {

  /*! \class CommandAction
      \brief Represents a QAction that's associated with a QUndoCommand.
   */
  class CommandAction : public QAction {
    public:

      //! Constructs a CommandAction.
      /*!
        \param command The QUndoCommand this action should perform.
        \param parent The parent this action belongs to.
      */
      CommandAction(std::unique_ptr<QUndoCommand>&& command,
        QObject* parent = nullptr);

      virtual ~CommandAction();

      //! Returns the QUndoCommand this action should perform.
      std::unique_ptr<QUndoCommand>& GetCommand();

    private:
      std::unique_ptr<QUndoCommand> m_command;
  };
}
}

#endif
