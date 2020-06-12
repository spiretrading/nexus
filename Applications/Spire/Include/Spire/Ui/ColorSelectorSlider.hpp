#ifndef SPIRE_COLOR_SELECTOR_SLIDER_HPP
#define SPIRE_COLOR_SELECTOR_SLIDER_HPP
#include <boost/optional/optional.hpp>
#include <QGradientStops>
#include <QImage>
#include <QWidget>

namespace Spire {

  class ColorSelectorSlider : public QWidget {
    public:

      ColorSelectorSlider(const QColor& color, const QGradientStops& stops,
        QWidget* parent = nullptr);

    protected:
      void mouseMoveEvent(QMouseEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      QGradientStops m_gradient_stops;
      QImage m_gradient;
      QImage m_handle;
      int m_last_mouse_x;

      void set_mouse_x(int x);
  };
}

#endif
