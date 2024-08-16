#ifndef SPIRE_COPYNODECOMMAND_HPP
#define SPIRE_COPYNODECOMMAND_HPP
#include <memory>
#include <QUndoCommand>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/CanvasView/CanvasView.hpp"

namespace Spire {

  /*! \class CopyNodeCommand
      \brief Copies a CanvasNode to the clipboard.
   */
  class CopyNodeCommand : public QUndoCommand {
    public:

      //! Constructs a CopyNodeCommand.
      /*!
        \param node The CanvasNode to copy to the clipboard.
      */
      CopyNodeCommand(const CanvasNode& node);

      virtual ~CopyNodeCommand() = default;

      virtual void undo();

      virtual void redo();

    private:
      std::unique_ptr<CanvasNode> m_copyNode;
  };
}

#endif
