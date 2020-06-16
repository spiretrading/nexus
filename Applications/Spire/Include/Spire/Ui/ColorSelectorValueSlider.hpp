#ifndef SPIRE_COLOR_SELECTOR_VALUE_SLIDER_HPP
#define SPIRE_COLOR_SELECTOR_VALUE_SLIDER_HPP
#include <boost/optional/optional.hpp>
#include <QGradientStops>
#include <QImage>
#include <QPoint>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  class ColorSelectorValueSlider : public QWidget {
    public:

      using ColorSignal = Signal<void (const QColor& color)>;

      ColorSelectorValueSlider(const QColor& current_color,
        QWidget* parent = nullptr);

      void set_hue(int hue);

      void set_color(const QColor& color);

      boost::signals2::connection connect_color_signal(
        const ColorSignal::slot_type& slot) const;

    protected:
      void mouseMoveEvent(QMouseEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      mutable ColorSignal m_color_signal;
      QColor m_current_color;
      QImage m_gradient;
      QImage m_handle;
      QPoint m_last_mouse_pos;

      QPoint get_mouse_pos(const QColor& color);
      void set_mouse_pos(const QPoint& pos);
  };
}

#endif
