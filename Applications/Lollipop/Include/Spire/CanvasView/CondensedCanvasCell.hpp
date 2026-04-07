#ifndef SPIRE_CONDENSEDCANVASCELL_HPP
#define SPIRE_CONDENSEDCANVASCELL_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QWidget>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasView.hpp"
#include "Spire/UI/UI.hpp"

class QLineEdit;

namespace Spire {

  /*! \class CondensedCanvasCell
      \brief Draws a CanvasNode as an editable cell within a
             CondensedCanvasWidget.
   */
  class CondensedCanvasCell : public QWidget {
    public:

      //! Constructs a CondensedCanvasCell.
      /*!
        \param userProfile The user's profile.
        \param parent The parent widget.
        \param node The CanvasNode to display.
      */
      CondensedCanvasCell(Beam::Ref<UserProfile> userProfile,
        Beam::Ref<CondensedCanvasWidget> parent,
        Beam::Ref<const CanvasNode> node);

      //! Returns the displayed CanvasNode.
      const CanvasNode& GetNode() const;

      virtual QSize sizeHint() const;

      virtual QSize minimumSizeHint() const;

    protected:
      virtual bool eventFilter(QObject* object, QEvent* event);

      virtual void focusInEvent(QFocusEvent* event);

      virtual void focusOutEvent(QFocusEvent* event);

    private:
      UserProfile* m_userProfile;
      CondensedCanvasWidget* m_parent;
      const CanvasNode* m_node;
      QLineEdit* m_valueWidget;
      QWidget* m_editor;
  };
}

#endif
