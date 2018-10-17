#ifndef SPIRE_CANVASNODEEDITOR_HPP
#define SPIRE_CANVASNODEEDITOR_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/variant/variant.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasView.hpp"
#include "Spire/Spire/Spire.hpp"

class QEvent;
class QUndoCommand;
class QWidget;

namespace Spire {

  /*! \class CanvasNodeEditor
      \brief Implements user interface editors for CanvasNodes.
   */
  class CanvasNodeEditor {
    public:

      //! A variant type for editing a CanvasNode.
      typedef boost::variant<QWidget*, QUndoCommand*> EditVariant;

      //! Constructs a CanvasNodeEditor.
      CanvasNodeEditor() = default;

      //! Returns an editor to use on a CanvasNode.
      /*!
        \param node The CanvasNode to edit.
        \param view The CanvasNodeModel storing the CanvasNode.
        \param userProfile The UserProfile performing the edit.
        \param event The event that triggered the edit.
        \return The type of edit action to perform on the <i>node</i>.
      */
      EditVariant GetEditor(Beam::Ref<const CanvasNode> node,
        Beam::Ref<CanvasNodeModel> view,
        Beam::Ref<UserProfile> userProfile, QEvent* event);

      //! Commits an edit into a QUndoCommand.
      QUndoCommand* Commit();

    private:
      const CanvasNode* m_node;
      CanvasNodeModel* m_view;
      UserProfile* m_userProfile;
      EditVariant m_editVariant;
  };
}

#endif
