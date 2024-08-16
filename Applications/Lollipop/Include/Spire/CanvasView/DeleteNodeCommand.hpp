#ifndef SPIRE_DELETENODECOMMAND_HPP
#define SPIRE_DELETENODECOMMAND_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QUndoStack>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasNodeModel.hpp"
#include "Spire/CanvasView/CanvasView.hpp"

namespace Spire {

  /*! \class DeleteNodeCommand
      \brief Deletes a CanvasNode from a CanvasNodeModel.
   */
  class DeleteNodeCommand : public QUndoCommand {
    public:

      //! Constructs a DeleteNodeCommand.
      /*!
        \param view The CanvasNodeModel to perform the command to.
        \param coordinate The Coordinate to add the root at.
      */
      DeleteNodeCommand(Beam::Ref<CanvasNodeModel> view,
        const CanvasNodeModel::Coordinate& coordinate);

      virtual void undo();

      virtual void redo();

    private:
      CanvasNodeModel* m_view;
      CanvasNodeModel::Coordinate m_coordinate;
      CanvasNodeModel::Snapshot m_snapshot;
  };
}

#endif
