#ifndef SPIRE_SETNODEVISIBLECOMMAND_HPP
#define SPIRE_SETNODEVISIBLECOMMAND_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QUndoStack>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasNodeModel.hpp"
#include "Spire/CanvasView/CanvasView.hpp"

namespace Spire {

  /*! \class SetNodeVisibleCommand
      \brief Sets whether a CanvasNode is visible.
   */
  class SetNodeVisibleCommand : public QUndoCommand {
    public:

      //! Constructs a SetNodeVisibleCommand.
      /*!
        \param view The CanvasNodeModel to perform the command to.
        \param parent The coordinate of the node's parent.
        \param node The name of the node to modify.
        \param isVisible Whether the CanvasNode should be visible.
      */
      SetNodeVisibleCommand(Beam::Ref<CanvasNodeModel> view,
        const CanvasNodeModel::Coordinate& parent, const std::string& node,
        bool isVisible);

      virtual void undo();

      virtual void redo();

    private:
      CanvasNodeModel* m_view;
      CanvasNodeModel::Coordinate m_parent;
      std::string m_node;
      bool m_isVisible;
      CanvasNodeModel::Snapshot m_snapshot;
  };
}

#endif
