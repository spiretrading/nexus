#ifndef SPIRE_COLOR_SELECTOR_BUTTON_HPP
#define SPIRE_COLOR_SELECTOR_BUTTON_HPP
#include <QWidget>
#include "Spire/Ui/ColorSelectorDropDown.hpp"
#include "Spire/Ui/FlatButton.hpp"
#include "Spire/Ui/RecentColors.hpp"

namespace Spire {

  class ColorSelectorButton : public QWidget {
    public:

      using ColorSignal = Signal<void (const QColor& color)>;

      ColorSelectorButton(const QColor& current_color,
        const RecentColors& recent_colors, QWidget* parent = nullptr);

      void set_color(const QColor& color);

      boost::signals2::connection connect_color_signal(
        const ColorSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable ColorSignal m_color_signal;
      QColor m_original_color;
      FlatButton* m_button;
      ColorSelectorDropDown* m_dropdown;

      void hide_dropdown();
      void move_color_dropdown();
      void on_button_clicked();
      void on_color_selected(const QColor& color);
  };
}

#endif
