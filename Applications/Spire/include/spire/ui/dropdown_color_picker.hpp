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
      void resizeEvent(QResizeEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      FlatButton* m_button;
      ColorPicker* m_color_picker;

      void on_button_click();
      void move_color_picker();
  };
}

#endif
