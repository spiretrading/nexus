#ifndef SPIRE_COLOR_SELECTOR_SLIDER_HPP
#define SPIRE_COLOR_SELECTOR_SLIDER_HPP
#include <boost/optional/optional.hpp>
#include <QGradientStops>
#include <QImage>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  class ColorSelectorSlider : public QWidget {
    public:

      using ColorSignal = Signal<void (const QColor& color)>;

      enum class SliderMode {
        VALUE,
        HUE
      };

      ColorSelectorSlider(SliderMode mode, const QColor& color,
        QWidget* parent = nullptr);

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
      SliderMode m_mode;
      QGradientStops m_gradient_stops;
      QImage m_gradient;
      QImage m_handle;
      int m_last_mouse_x;

      int get_mouse_x(const QColor& color);
      void set_mouse_x(int x);
  };
}

#endif
