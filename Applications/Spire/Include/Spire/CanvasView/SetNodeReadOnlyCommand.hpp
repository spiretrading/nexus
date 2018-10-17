#ifndef SPIRE_SETNODEREADONLYCOMMAND_HPP
#define SPIRE_SETNODEREADONLYCOMMAND_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QUndoStack>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasNodeModel.hpp"
#include "Spire/CanvasView/CanvasView.hpp"

namespace Spire {

  /*! \class SetNodeReadOnlyCommand
      \brief Sets a Node's read-only property.
   */
  class SetNodeReadOnlyCommand : public QUndoCommand {
    public:

      //! Constructs a SetNodeReadOnlyCommand.
      /*!
        \param view The CanvasNodeModel to perform the command to.
        \param node The coordinate of the node to move.
        \param readOnly Whether the CanvasNode should be set to read-only.
      */
      SetNodeReadOnlyCommand(Beam::Ref<CanvasNodeModel> view,
        const CanvasNodeModel::Coordinate& source, bool readOnly);

      virtual void undo();

      virtual void redo();

    private:
      CanvasNodeModel* m_view;
      CanvasNodeModel::Coordinate m_node;
      bool m_readOnly;
      CanvasNodeModel::Snapshot m_snapshot;
  };
}

#endif
