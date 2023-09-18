#ifndef SPIRE_SCROLLABLE_LAYER_HPP
#define SPIRE_SCROLLABLE_LAYER_HPP
#include <QGridLayout>
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

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
      ScrollBar* m_vertical_scroll_bar;
      ScrollBar* m_horizontal_scroll_bar;
      QGridLayout* m_layout;
      Box* m_corner_box;
      bool m_is_horizontal_scroll_bar_opaque;
      bool m_is_vertical_scroll_bar_opaque;
      boost::signals2::scoped_connection
        m_horizontal_scroll_bar_style_connection;
      boost::signals2::scoped_connection m_vertical_scroll_bar_style_connection;

      void update_mask();
      void update_layout();
      void on_horizontal_scroll_track_style();
      void on_vertical_scroll_track_style();
  };
}

#endif
