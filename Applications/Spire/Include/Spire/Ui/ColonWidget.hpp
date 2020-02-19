#ifndef SPIRE_COLON_WIDGET_HPP
#define SPIRE_COLON_WIDGET_HPP
#include <QWidget>

namespace Spire {

  //! Displays a text colon with styled top and bottom border for use with
  //! connecting custom line edits.
  class ColonWidget : public QWidget {
    public:

      //! Constructs a ColonWidget.
      /*
        \param parent The parent widget.
      */
      explicit ColonWidget(QWidget* parent = nullptr);

      //! Sets the default, non-active border style.
      void set_default_style();

      //! Sets the active (hovered and/or focused) border style.
      void set_active_style();

    protected:
      void paintEvent(QPaintEvent* event) override;

    private:
      QColor m_border_color;
  };
}

#endif
