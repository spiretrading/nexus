#ifndef SPIRE_READONLYCHECKBOX_HPP
#define SPIRE_READONLYCHECKBOX_HPP
#include <QCheckBox>
#include "Spire/UI/UI.hpp"

namespace Spire {
namespace UI {

  /*! \class ReadOnlyCheckBox
      \brief Displays a check box that can't be modified by the user.
   */
  class ReadOnlyCheckBox : public QCheckBox {
    public:

      //! Constructs a ReadOnlyCheckBox.
      /*!
        \param parent The parent widget.
      */
      ReadOnlyCheckBox(QWidget* parent = nullptr);

      //! Constructs a ReadOnlyCheckBox.
      /*!
        \param text The text to display to the right of the check box.
        \param parent The parent widget.
      */
      ReadOnlyCheckBox(const QString& text, QWidget* parent = nullptr);

      virtual ~ReadOnlyCheckBox();

    protected:
      virtual void mousePressEvent(QMouseEvent* event);
      virtual void mouseReleaseEvent(QMouseEvent* event);
  };
}
}

#endif
