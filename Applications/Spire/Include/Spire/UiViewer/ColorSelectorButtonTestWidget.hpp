#ifndef SPIRE_COLOR_SELECTOR_BUTTON_TEST_WIDGET_HPP
#define SPIRE_COLOR_SELECTOR_BUTTON_TEST_WIDGET_HPP
#include <QGridLayout>
#include <QWidget>
#include "Spire/Ui/ColorSelectorButton.hpp"
#include "Spire/Ui/FlatButton.hpp"
#include "Spire/Ui/TextInputWidget.hpp"

namespace Spire {

  //! Displays a widget for testing a ColorSelectorButton.
  class ColorSelectorButtonTestWidget : public QWidget {
    public:

      //! Constructs a ColorSelectorButtonTestWidget.
      /*
        \param parent The parent widget.
      */
      explicit ColorSelectorButtonTestWidget(QWidget* parent = nullptr);

    private:
      ColorSelectorButton* m_color_selector_button;
      QGridLayout* m_layout;
      TextInputWidget* m_set_color_input;
      FlatButton* m_set_color_button;
      TextInputWidget* m_create_color_input;
      FlatButton* m_create_color_button;

      void reset_tab_order();
      void on_create_color_button_color(const QString& color);
      void on_set_color_button_color(const QString& color_hex);
  };
}

#endif
