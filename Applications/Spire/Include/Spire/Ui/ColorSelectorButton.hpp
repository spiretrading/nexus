#ifndef SPIRE_COLOR_SELECTOR_BUTTON_HPP
#define SPIRE_COLOR_SELECTOR_BUTTON_HPP
#include <QWidget>
#include "Spire/Ui/ColorSelectorDropDown.hpp"
#include "Spire/Ui/FlatButton.hpp"

namespace Spire {

  class ColorSelectorButton : public QWidget {
    public:

      ColorSelectorButton(const QColor& current_color,
        QWidget* parent = nullptr);
  
    private:
      FlatButton* m_button;
      ColorSelectorDropDown* m_dropdown;

      void set_button_color(const QColor& color);
  };
}

#endif
