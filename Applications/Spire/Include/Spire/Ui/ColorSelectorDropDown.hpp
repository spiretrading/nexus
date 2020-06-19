#ifndef SPIRE_COLOR_SELECTOR_DROP_DOWN_HPP
#define SPIRE_COLOR_SELECTOR_DROP_DOWN_HPP
#include <QGridLayout>
#include <QHBoxLayout>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/ColorSelectorHexInputWidget.hpp"
#include "Spire/Ui/ColorSelectorHueSlider.hpp"
#include "Spire/Ui/ColorSelectorValueSlider.hpp"
#include "Spire/Ui/DropShadow.hpp"

namespace Spire {

  class ColorSelectorDropDown : public QWidget {
    public:

      using ColorSignal = Signal<void (const QColor& color)>;

      ColorSelectorDropDown(const QColor& current_color,
        QWidget* parent = nullptr);

      void set_color(const QColor& color);

      boost::signals2::connection connect_color_signal(
        const ColorSignal::slot_type& slot) const;

    protected:
      void childEvent(QChildEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable ColorSignal m_color_signal;
      QColor m_current_color;
      QGridLayout* m_basic_colors_layout;
      QHBoxLayout* m_recent_colors_layout;
      ColorSelectorValueSlider* m_color_value_slider;
      ColorSelectorHueSlider* m_color_hue_slider;
      ColorSelectorHexInputWidget* m_hex_input;
      DropShadow* m_drop_shadow;

      void add_basic_color_button(int x, int y, const QColor& color);
      void add_recent_color_button(const QColor& color);
      void on_color_button_clicked(const QColor& color);
      void on_color_selected(const QColor& color);
  };
}

#endif
