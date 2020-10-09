#ifndef SPIRE_UI_VIEWER_WINDOW_HPP
#define SPIRE_UI_VIEWER_WINDOW_HPP
#include <QGridLayout>
#include "Spire/Ui/ColorSelectorButton.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  class UiViewerWindow : public Window {
    public:

      UiViewerWindow(QWidget* parent = nullptr);

    private:
      QGridLayout* m_layout;
      ColorSelectorButton* m_color_selector_button;

      void add_color_selector_button(int row);
  };
}

#endif
