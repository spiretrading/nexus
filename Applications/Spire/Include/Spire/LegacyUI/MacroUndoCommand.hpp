#ifndef SPIRE_MACROUNDOCOMMAND_HPP
#define SPIRE_MACROUNDOCOMMAND_HPP
#include <memory>
#include <vector>
#include <QUndoStack>
#include "Spire/LegacyUI/LegacyUI.hpp"

namespace Spire {
namespace LegacyUI {

  /*! \class MacroUndoCommand
      \brief Composes a series of QUndoCommand's together.
   */
  class MacroUndoCommand : public QUndoCommand {
    public:

      //! Constructs a MacroUndoCommand.
      MacroUndoCommand();

      //! Adds a QUndoCommand.
      /*!
        \param command The command to add.
      */
      void AddCommand(QUndoCommand*&& command);

      virtual void undo();

      virtual void redo();

    private:
      std::vector<std::unique_ptr<QUndoCommand>> m_commands;
  };
}
}

#endif
