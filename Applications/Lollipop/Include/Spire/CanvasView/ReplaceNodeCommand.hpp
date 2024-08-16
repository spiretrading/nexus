#ifndef SPIRE_REPLACENODECOMMAND_HPP
#define SPIRE_REPLACENODECOMMAND_HPP
#include <memory>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <QUndoStack>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasNodeModel.hpp"
#include "Spire/CanvasView/CanvasView.hpp"

namespace Spire {

  /*! \class ReplaceNodeCommand
      \brief Replaces one child CanvasNode with another.
   */
  class ReplaceNodeCommand : public QUndoCommand {
    public:

      //! Replaces one CanvasNode with another.
      /*!
        \param view The CanvasNodeModel containing the CanvasNode to replace.
        \param source The source to replace.
        \param replacement The replacement CanvasNode.
        \return The newly replaced CanvasNode.
      */
      static const CanvasNode& Replace(Beam::Out<CanvasNodeModel> view,
        const CanvasNodeModel::Coordinate& source,
        const CanvasNode& replacement);

      //! Replaces one CanvasNode with another.
      /*!
        \param view The CanvasNodeModel containing the CanvasNode to replace.
        \param source The source to replace.
        \param replacement The replacement CanvasNode.
        \return The newly replaced CanvasNode.
      */
      static const CanvasNode& Replace(Beam::Out<CanvasNodeModel> view,
        const CanvasNode& source, const CanvasNode& replacement);

      //! Constructs a ReplaceNodeCommand.
      /*!
        \param view The CanvasNodeModel to perform the command to.
        \param coordinate The Coordinate of the node to replace.
        \param node The replacement CanvasNode.
      */
      ReplaceNodeCommand(Beam::Ref<CanvasNodeModel> view,
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
