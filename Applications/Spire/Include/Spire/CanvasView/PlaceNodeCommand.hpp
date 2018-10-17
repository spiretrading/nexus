#ifndef SPIRE_PLACENODECOMMAND_HPP
#define SPIRE_PLACENODECOMMAND_HPP
#include <memory>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <QUndoStack>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasNodeModel.hpp"
#include "Spire/CanvasView/CanvasView.hpp"

namespace Spire {

  /*! \class PlaceNodeCommand
      \brief Places a CanvasNode into a CanvasNodeModel.
   */
  class PlaceNodeCommand : public QUndoCommand {
    public:

      //! Places a CanvasNode into a CanvasNodeModel.
      /*!
        \param view The CanvasNodeModel to perform the command to.
        \param coordinate The Coordinate of the node to place.
        \param node The node to place.
        \param overrideReadOnly Whether to override the CanvasNode's read-only
               property.
        \return The CanvasNode that was placed.
      */
      static const CanvasNode& PlaceNode(Beam::Out<CanvasNodeModel> view,
        const CanvasNodeModel::Coordinate coordinate, const CanvasNode& node,
        bool overrideReadOnly);

      //! Tries to place a CanvasNode into a CanvasNodeModel.
      /*!
        \param view The CanvasNodeModel to perform the command to.
        \param coordinate The Coordinate of the node to place.
        \param node The node to place.
        \param overrideReadOnly Whether to override the CanvasNode's read-only
               property.
        \return The CanvasNode that was placed.
      */
      static boost::optional<const CanvasNode&> TryPlaceNode(
        Beam::Out<CanvasNodeModel> view,
        const CanvasNodeModel::Coordinate coordinate, const CanvasNode& node,
        bool overrideReadOnly);

      //! Constructs a PlaceNodeCommand.
      /*!
        \param view The CanvasNodeModel to perform the command to.
        \param coordinate The Coordinate of the node to place.
        \param node The node to place.
      */
      PlaceNodeCommand(Beam::Ref<CanvasNodeModel> view,
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
