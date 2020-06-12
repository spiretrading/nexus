#ifndef SPIRE_COLOR_SELECTOR_HEX_INPUT_WIDGET_HPP
#define SPIRE_COLOR_SELECTOR_HEX_INPUT_WIDGET_HPP
#include <QLineEdit>
#include <QWidget>

namespace Spire {

  class ColorSelectorHexInputWidget : public QWidget {
    public:

      ColorSelectorHexInputWidget(const QColor& current_color,
        QWidget* parent = nullptr);

    private:
      QLineEdit* m_line_edit;

      void on_text_changed(const QString& text);
  };
}

#endif
