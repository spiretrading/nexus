#ifndef SPIRE_SCROLL_BOX_HPP
#define SPIRE_SCROLL_BOX_HPP
#include "Spire/Styles/StyledWidget.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * A widget that serves as a container for another widget, with scroll bars
   * displayed on overflow.
   */
  class ScrollBox : public Styles::StyledWidget {
    public:

      /**
       * Constructs a ScrollBox.
       * @param body The widget to display.
       * @param parent The parent widget.
       */
      explicit ScrollBox(QWidget* body, QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      QWidget* m_body;
      ScrollableLayer* m_scrollable_layer;
      LayeredWidget* m_layers;

      void on_vertical_scroll(int position);
      void on_horizontal_scroll(int position);
      void update_ranges();
  };
}

#endif
