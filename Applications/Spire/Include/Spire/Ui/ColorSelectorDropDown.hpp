#ifndef SPIRE_COLOR_SELECTOR_DROP_DOWN_HPP
#define SPIRE_COLOR_SELECTOR_DROP_DOWN_HPP
#include <QWidget>

namespace Spire {

  class ColorSelectorDropDown : public QWidget {
    public:

      ColorSelectorDropDown(const QColor& current_color,
        QWidget* parent = nullptr);
  };
}

#endif
