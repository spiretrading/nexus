#ifndef SPIRE_TEXT_INPUT_WIDGET_HPP
#define SPIRE_TEXT_INPUT_WIDGET_HPP
#include <QLineEdit>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Represents a Spire-style line edit.
  class TextInputWidget : public QLineEdit {
    public:

      //! Constructs a TextInputWidget.
      /*!
        \param parent The parent widget.
      */
      explicit TextInputWidget(QWidget* parent = nullptr);
  };
}

#endif
