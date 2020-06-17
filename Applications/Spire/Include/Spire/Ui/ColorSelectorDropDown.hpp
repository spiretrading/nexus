#ifndef SPIRE_COLOR_SELECTOR_DROP_DOWN_HPP
#define SPIRE_COLOR_SELECTOR_DROP_DOWN_HPP
#include <QGridLayout>
#include <QHBoxLayout>
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

      void add_basic_color_button(QGridLayout* layout, int x, int y,
        const QColor& color);
      void add_recent_color_button(QHBoxLayout* layout, const QColor& color);
      void on_color_button_clicked(const QColor& color);
      void on_recent_color_button_clicked(const QColor& color);
  };
}

#endif
