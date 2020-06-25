#ifndef SPIRE_COLOR_SELECTOR_HUE_SLIDER_HPP
#define SPIRE_COLOR_SELECTOR_HUE_SLIDER_HPP
#include <boost/optional/optional.hpp>
#include <QGradientStops>
#include <QImage>
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Represents a widget for selecting a color with a specific hue.
  class ColorSelectorHueSlider : public QWidget {
    public:

      //! Signal type for color selection.
      /*!
        \param color The selected color.
      */
      using ColorSignal = Signal<void (const QColor& color)>;

      //! Constructs a ColorSelectorHueSlider.
      /*!
        \param color The initial selection color.
        \param parent The parent widget.
      */
      explicit ColorSelectorHueSlider(const QColor& color,
        QWidget* parent = nullptr);

      //! Sets the current color.
      /*!
        \param color The current color.
      */
      void set_color(const QColor& color);

      //! Connects a slot to the color selection signal.
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
      int m_last_mouse_x;

      int get_mouse_x(int hue);
      void set_mouse_x(int x);
      void update_gradient();
      void on_color_changed();
  };
}

#endif
