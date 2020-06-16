#ifndef SPIRE_COLOR_SELECTOR_DROP_DOWN_HPP
#define SPIRE_COLOR_SELECTOR_DROP_DOWN_HPP
#include <QWidget>
#include "Spire/Ui/ColorSelectorHexInputWidget.hpp"
#include "Spire/Ui/ColorSelectorHueSlider.hpp"
#include "Spire/Ui/ColorSelectorValueSlider.hpp"

namespace Spire {

  class ColorSelectorDropDown : public QWidget {
    public:

      ColorSelectorDropDown(const QColor& current_color,
        QWidget* parent = nullptr);

    private:
      ColorSelectorValueSlider* m_color_value_slider;
      ColorSelectorHueSlider* m_color_hue_slider;
      ColorSelectorHexInputWidget* m_hex_input;
  };
}

#endif
