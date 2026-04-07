#ifndef SPIRE_COMMITCANVASEDITOR_HPP
#define SPIRE_COMMITCANVASEDITOR_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasView.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/Spire.hpp"

class QUndoCommand;
class QWidget;

namespace Spire {

  //! Commits an editor used to edit a CanvasNode.
  /*!
    \param node The CanvasNode that was edited.
    \param editor The editor used to edit the CanvasNode.
    \param view The CanvasNodeModel storing the CanvasNode.
    \param userProfile The user's profile.
  */
  QUndoCommand* CommitCanvasEditor(const CanvasNode& node,
    const QWidget& editor, CanvasNodeModel& model,
    const UserProfile& userProfile);
}

#endif
