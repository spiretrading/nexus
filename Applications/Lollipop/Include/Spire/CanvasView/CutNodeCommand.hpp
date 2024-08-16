#ifndef SPIRE_CUTNODECOMMAND_HPP
#define SPIRE_CUTNODECOMMAND_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasNodeModel.hpp"
#include "Spire/CanvasView/CanvasView.hpp"
#include "Spire/CanvasView/DeleteNodeCommand.hpp"

namespace Spire {

  /*! \class CutNodeCommand
      \brief Deletes a CanvasNode and moves it to the clipboard.
   */
  class CutNodeCommand : public QUndoCommand {
    public:

      //! Constructs a CutNodeCommand.
      /*!
        \param view The CanvasNodeModel to perform the command on.
        \param coordinate The Coordinate to add the root at.
      */
      CutNodeCommand(Beam::Ref<CanvasNodeModel> view,
        const CanvasNodeModel::Coordinate& coordinate);

      virtual void undo();

      virtual void redo();

    private:
      bool m_isInitialRedo;
      CanvasNodeModel* m_view;
      CanvasNodeModel::Coordinate m_coordinate;
      DeleteNodeCommand m_deleteCommand;
  };
}

#endif
