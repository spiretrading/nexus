#ifndef SPIRE_COLOR_SELECTOR_HUE_SLIDER_HPP
#define SPIRE_COLOR_SELECTOR_HUE_SLIDER_HPP
#include <boost/optional/optional.hpp>
#include <QGradientStops>
#include <QImage>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  class ColorSelectorHueSlider : public QWidget {
    public:

      using ColorSignal = Signal<void (const QColor& color)>;

      ColorSelectorHueSlider(const QColor& color, QWidget* parent = nullptr);

      void set_color(const QColor& color);

      void set_hue(int hue);

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
      QGradientStops m_gradient_stops;
      QImage m_gradient;
      QImage m_handle;
      int m_last_mouse_x;

      int get_mouse_x(int hue);
      void set_mouse_x(int x);
  };
}

#endif
