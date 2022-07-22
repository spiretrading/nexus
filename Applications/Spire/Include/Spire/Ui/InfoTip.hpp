#ifndef SPIRE_INFO_TIP_HPP
#define SPIRE_INFO_TIP_HPP
#include <boost/signals2/connection.hpp>
#include <QTimer>
#include <QWidget>

namespace Spire {

  /**
   * Represents a pop-up window for providing additional context for a
   * component.
   */
  class InfoTip : public QWidget {
    public:

      /**
       * Constructs an InfoTip.
       * @param body The component to display in the InfoTip.
       * @param parent The parent widget.
       */
      explicit InfoTip(QWidget* body, QWidget* parent = nullptr);

      /**
       * Sets if the InfoTip will remain visible when hovered.
       * @param is_interactive True iff the InfoTip is interactive.
       */
      void set_interactive(bool is_interactive);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void leaveEvent(QEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      enum class Orientation {
        BOTTOM_LEFT,
        BOTTOM_RIGHT,
        TOP_LEFT,
        TOP_RIGHT
      };
      enum class BodyOrientation {
        LEFT,
        RIGHT
      };

      QWidget* m_body;
      QWidget* m_container;
      bool m_is_interactive;
      QTimer m_show_timer;
      QColor m_background_color;
      QColor m_border_color;
      int m_border_size;
      boost::signals2::scoped_connection m_style_connection;

      void fade_in();
      void fade_out();
      QPainterPath get_arrow_path() const;
      BodyOrientation get_body_orientation() const;
      QScreen* get_current_screen(const QPoint& point) const;
      QMargins get_margins() const;
      Orientation get_orientation() const;
      QPoint get_position() const;
      QRect hover_rect() const;
      QPixmap render_background() const;
      void on_fade_out_finished();
      void on_show_timeout();
      void on_style();
  };
}

#endif
