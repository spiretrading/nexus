#ifndef SPIRE_INFO_TIP_HPP
#define SPIRE_INFO_TIP_HPP
#include <boost/signals2/connection.hpp>
#include <QTimer>
#include <QWidget>
#include "Spire/Ui/HoverObserver.hpp"

class QPropertyAnimation;

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
      void changeEvent(QEvent* event) override;
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
      enum class FadeState {
        NONE,
        FADING_IN,
        FADING_OUT
      };
      QWidget* m_parent;
      QWidget* m_container;
      bool m_is_interactive;
      HoverObserver m_hover_observer;
      QTimer m_show_timer;
      QTimer m_interactive_timer;
      QColor m_background_color;
      QColor m_border_color;
      int m_border_size;
      FadeState m_fade_state;
      QPropertyAnimation* m_animation;
      boost::signals2::scoped_connection m_style_connection;

      QRect get_interactive_region() const;
      bool is_hovered() const;
      void fade_in();
      void fade_out();
      QPainterPath get_arrow_path() const;
      BodyOrientation get_body_orientation() const;
      QScreen* get_current_screen(const QPoint& point) const;
      QMargins get_margins() const;
      Orientation get_orientation() const;
      QPoint get_position() const;
      QPixmap render_background() const;
      void on_hover(HoverObserver::State state);
      void on_fade_in_finished();
      void on_fade_out_finished();
      void on_show_timeout();
      void on_interactive_timeout();
      void on_style();
  };
}

#endif
