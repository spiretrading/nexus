#ifndef SPIRE_MOVENODECOMMAND_HPP
#define SPIRE_MOVENODECOMMAND_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QUndoStack>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasNodeModel.hpp"
#include "Spire/CanvasView/CanvasView.hpp"

namespace Spire {

  /*! \class MoveNodeCommand
      \brief Moves a CanvasNode into a CanvasNodeModel.
   */
  class MoveNodeCommand : public QUndoCommand {
    public:

      //! Constructs a MoveNodeCommand.
      /*!
        \param view The CanvasNodeModel to perform the command to.
        \param source The coordinate of the node to move.
        \param destination The coordinate to move the node to.
      */
      MoveNodeCommand(Beam::Ref<CanvasNodeModel> view,
        const CanvasNodeModel::Coordinate& source,
        const CanvasNodeModel::Coordinate& destination);

      virtual void undo();

      virtual void redo();

    private:
      CanvasNodeModel* m_view;
      CanvasNodeModel::Coordinate m_source;
      CanvasNodeModel::Coordinate m_destination;
      CanvasNodeModel::Snapshot m_snapshot;
  };
}

#endif
