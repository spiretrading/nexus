#ifndef SPIRE_TEXT_INPUT_WIDGET_HPP
#define SPIRE_TEXT_INPUT_WIDGET_HPP
#include <QLineEdit>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Represents a Spire-style line edit.
  class TextInputWidget : public QLineEdit {
    public:

      /** The styles available to render the TextInputWidget. */
      enum class Style {

        /** Render using the default style. */
        DEFAULT,

        /** Render using the table cell style. */
        CELL
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

      //! Returns the left text padding.
      int get_padding() const;

      //! Sets the TextInputWidget's style.
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
