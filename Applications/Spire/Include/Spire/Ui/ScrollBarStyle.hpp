#ifndef SPIRE_SCROLL_BAR_STYLE_HPP
#define SPIRE_SCROLL_BAR_STYLE_HPP
#include <QProxyStyle>
#include <QScrollBar>

namespace Spire {

  //! Represents a Spire style for QScrollBars.
  class ScrollBarStyle : public QProxyStyle {
    public:

      //! Constructs a ScrollBarStyle.
      /*!
        \param parent The parent widget. A shared style will be destroyed
                      when the given parent widget is destroyed.
      */
      explicit ScrollBarStyle(QWidget* parent);

      //! Sets the height of horizontal scroll bars.
      /*!
        \param height The horizontal scroll bars' height.
      */
      void set_horizontal_scroll_bar_height(int height);

      //! Sets the width of vertical scroll bars.
      /*!
        \param width The vertical scroll bars' width.
      */
      void set_vertical_scroll_bar_width(int width);

      void drawComplexControl(QStyle::ComplexControl control,
        const QStyleOptionComplex* option, QPainter* painter,
        const QWidget* widget = nullptr) const override;

      void drawControl(QStyle::ControlElement element,
        const QStyleOption* option, QPainter* painter,
        const QWidget* widget = nullptr) const override;

      int pixelMetric(PixelMetric metric, const QStyleOption* option = nullptr,
        const QWidget* widget = nullptr) const override;

      int styleHint(QStyle::StyleHint hint,
        const QStyleOption* option = nullptr, const QWidget* widget = nullptr,
        QStyleHintReturn* return_data = nullptr) const override;

      QRect subControlRect(ComplexControl complex_control,
        const QStyleOptionComplex* option, SubControl sub_control,
        const QWidget* widget) const override;

    private:
      int m_horizontal_handle_height;
      int m_vertical_handle_width;

      int get_handle_size(const QScrollBar* scroll_bar) const;
      int get_horizontal_slider_position(const QScrollBar* scroll_bar) const;
      int get_vertical_slider_position(const QScrollBar* scroll_bar) const;
  };
}

#endif
