#ifndef SPIRE_SCROLLABLE_LAYER_HPP
#define SPIRE_SCROLLABLE_LAYER_HPP
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

class QSpacerItem;

namespace Spire {

  /**
   * Implements a widget that is used as a layer containing a vertical and
   * horizontal scroll bar.
   */
  class ScrollableLayer : public QWidget {
    public:

      /**
       * Constructs a ScrollableLayer.
       * @param parent The parent widget.
       */
      explicit ScrollableLayer(QWidget* parent = nullptr);

      /** Returns the vertical ScrollBar. */
      ScrollBar& get_vertical_scroll_bar();

      /** Returns the horizontal ScrollBar. */
      ScrollBar& get_horizontal_scroll_bar();

      void keyPressEvent(QKeyEvent* event) override;

      void wheelEvent(QWheelEvent* event) override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      QSpacerItem* m_transparent_spacer;
      ScrollBar* m_vertical_scroll_bar;
      ScrollBar* m_horizontal_scroll_bar;

      void update_mask();
  };
}

#endif
