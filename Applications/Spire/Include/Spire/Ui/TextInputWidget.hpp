#ifndef SPIRE_TEXT_INPUT_WIDGET_HPP
#define SPIRE_TEXT_INPUT_WIDGET_HPP
#include <QLineEdit>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Represents a Spire-style line edit.
  class TextInputWidget : public QLineEdit {
    public:

      //! Represents style types that the TextInputWidget supports;
      enum class Style {

        //! Style used when embedded in a table or list.
        CELL,

        //! Style used 
        DEFAULT
      };

      //! Constructs a TextInputWidget with the default style.
      /*!
        \param parent The parent widget.
      */
      explicit TextInputWidget(QWidget* parent = nullptr);

      //! Constructs a TextInputWidget with the default style.
      /*!
        \param text The text to display.
        \param parent The parent widget.
      */
      explicit TextInputWidget(QString text, QWidget* parent = nullptr);

      //! Returns the left padding of the current style.
      int get_padding() const;

      //! Sets the input's style.
      void set_style(Style style);

    protected:
      void focusInEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      int m_left_padding;
  };
}

#endif
