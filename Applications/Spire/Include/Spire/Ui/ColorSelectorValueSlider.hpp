#ifndef SPIRE_COLOR_SELECTOR_VALUE_SLIDER_HPP
#define SPIRE_COLOR_SELECTOR_VALUE_SLIDER_HPP
#include <boost/optional/optional.hpp>
#include <QGradientStops>
#include <QImage>
#include <QPoint>
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Represents a widget for selecting a color with a specific value and
  //! saturation.
  class ColorSelectorValueSlider : public QWidget {
    public:

      //! Signal type for color selection.
      /*!
        \param color The selected color.
      */
      using ColorSignal = Signal<void (const QColor& color)>;

      //! Constructs a ColorSelectorValueSlider.
      /*!
        \param current_color The initial selection color.
        \param parent The parent widget.
      */
      explicit ColorSelectorValueSlider(const QColor& current_color,
        QWidget* parent = nullptr);

      //! Sets the current color.
      /*!
        \param color The current color.
      */
      void set_color(const QColor& color);

      //! Updates the slider's hue without changing the value and saturation.
      /*!
        \param hue The current hue.
      */
      void set_hue(int hue);

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
      QPoint m_last_mouse_pos;

      QPoint get_mouse_pos(const QColor& color) const;
      void set_mouse_pos(const QPoint& pos);
      void update_gradient();
      void on_color_changed();
  };
}

#endif
