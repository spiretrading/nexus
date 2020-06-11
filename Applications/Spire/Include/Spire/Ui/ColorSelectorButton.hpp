#ifndef SPIRE_COLOR_SELECTOR_BUTTON_HPP
#define SPIRE_COLOR_SELECTOR_BUTTON_HPP
#include <QWidget>
#include "Spire/Ui/ColorSelectorButtonDropDown.hpp"
#include "Spire/Ui/FlatButton.hpp"

namespace Spire {

  class ColorSelectorButton : public QWidget {
    public:

      ColorSelectorButton(const QColor& color, QWidget* parent = nullptr);
  
    private:
      FlatButton* m_button;
      ColorSelectorButtonDropDown* m_dropdown;

      void set_button_color(const QColor& color);
  };
}

#endif
