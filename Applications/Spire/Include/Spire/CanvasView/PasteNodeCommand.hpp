#ifndef SPIRE_PASTENODECOMMAND_HPP
#define SPIRE_PASTENODECOMMAND_HPP
#include <QUndoCommand>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasView.hpp"
#include "Spire/CanvasView/CanvasNodeModel.hpp"
#include "Spire/LegacyUI/LegacyUI.hpp"

namespace Spire {

  /*! \class PasteNodeCommand
      \brief Pastes a CanvasNode from the clipboard.
   */
  class PasteNodeCommand : public QUndoCommand {
    public:

      //! Constructs a PasteNodeCommand.
      /*!
        \param view The CanvasNodeModel to perform the command to.
        \param coordinate The Coordinate to paste the CanvasNode into.
      */
      PasteNodeCommand(Beam::Ref<CanvasNodeModel> view,
        const CanvasNodeModel::Coordinate& coordinate);

      virtual ~PasteNodeCommand();

      virtual void undo();

      virtual void redo();

    private:
      CanvasNodeModel* m_view;
      CanvasNodeModel::Coordinate m_coordinate;
      std::unique_ptr<LegacyUI::MacroUndoCommand> m_placeCommand;
  };
}

#endif
