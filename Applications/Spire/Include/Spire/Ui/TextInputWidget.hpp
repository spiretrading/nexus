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

      //! Constructs a TextInputWidget.
      /*!
        \param text The text to display.
        \param parent The parent widget.
      */
      explicit TextInputWidget(QString text, QWidget* parent = nullptr);

    protected:
      void keyPressEvent(QKeyEvent* event) override;
  };
}

#endif
