#ifndef SPIRE_OPENCANVASEDITOR_HPP
#define SPIRE_OPENCANVASEDITOR_HPP
#include "Spire/CanvasView/CanvasNodeEditor.hpp"
#include "Spire/CanvasView/CanvasView.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Opens an editor to edit a CanvasNode.
  /*!
    \param node The CanvasNode to edit.
    \param model The CanvasNodeModel storing the CanvasNode.
    \param userProfile The UserProfile performing the edit.
    \param event The event that triggered the edit.
  */
  CanvasNodeEditor::EditVariant OpenCanvasEditor(const CanvasNode& node,
    CanvasNodeModel& model, UserProfile& userProfile, QEvent* event);
}

#endif
