#ifndef SPIRE_DROPDOWN_COLOR_PICKER_HPP
#define SPIRE_DROPDOWN_COLOR_PICKER_HPP
#include <QWidget>
#include "spire/ui/ui.hpp"

namespace Spire {

  //! Displays a button that activates a pop-up color picker.
  class DropdownColorPicker : public QWidget {
    public:

      //! Constructs a DropdownColorPicker.
      /*
        \param parent The parent widget.
      */
      DropdownColorPicker(QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void leaveEvent(QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      FlatButton* m_button;
      ColorPicker* m_color_picker;
      QColor m_stored_button_color;

      void on_button_click();
      void on_color_preview(const QColor& color);
      void on_color_selected(const QColor& color);
      void move_color_picker();
      void set_button_color(const QColor& color);
      void store_button_color();
  };
}

#endif
