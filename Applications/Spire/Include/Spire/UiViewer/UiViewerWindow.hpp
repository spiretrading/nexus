#ifndef SPIRE_UI_VIEWER_WINDOW_HPP
#define SPIRE_UI_VIEWER_WINDOW_HPP
#include <QGridLayout>
#include "Spire/Ui/ColorSelectorButton.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  //! Displays a window to test Spire widgets.
  class UiViewerWindow : public Window {
    public:

      //! Constructs a UiViewerWindow.
      /*
        \param parent The parent widget.
      */
      UiViewerWindow(QWidget* parent = nullptr);

    private:
      QGridLayout* m_layout;
      ColorSelectorButton* m_color_selector_button;

      void add_color_selector_button(int row);
      void on_create_color_button_color(const QString& color_hex, int row);
      void on_set_color_button_color(const QString& color_hex);
  };
}

#endif
