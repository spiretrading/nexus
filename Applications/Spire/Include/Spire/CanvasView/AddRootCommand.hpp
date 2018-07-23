#ifndef SPIRE_ADDROOTCOMMAND_HPP
#define SPIRE_ADDROOTCOMMAND_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <QUndoStack>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasNodeModel.hpp"
#include "Spire/CanvasView/CanvasView.hpp"

namespace Spire {

  /*! \class AddRootCommand
      \brief Adds a CanvasNode into a CanvasNodeModel as a root.
   */
  class AddRootCommand : public QUndoCommand {
    public:

      //! Constructs an AddRootCommand.
      /*!
        \param view The CanvasNodeModel to perform the command to.
        \param coordinate The Coordinate to add the root at.
        \param node The node to add.
      */
      AddRootCommand(Beam::RefType<CanvasNodeModel> view,
        const CanvasNodeModel::Coordinate& coordinate, const CanvasNode& node);

      virtual void undo();

      virtual void redo();

    private:
      CanvasNodeModel* m_view;
      CanvasNodeModel::Coordinate m_coordinate;
      std::unique_ptr<CanvasNode> m_node;
      CanvasNodeModel::Snapshot m_snapshot;
  };
}

#endif
