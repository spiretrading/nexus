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

      void set_color(const QColor& color);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      FlatButton* m_button;
      ColorSelectorDropDown* m_dropdown;

      void move_color_dropdown();
      void on_button_clicked();
      void on_color_selected(const QColor& color);
  };
}

#endif
