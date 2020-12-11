#ifndef SPIRE_SCROLL_BAR_STYLE_HPP
#define SPIRE_SCROLL_BAR_STYLE_HPP
#include <QProxyStyle>
#include <QScrollBar>

namespace Spire {

  class ScrollBarStyle : public QProxyStyle {
    public:

      ScrollBarStyle(QStyle* style = nullptr, QWidget* parent = nullptr);

      void set_horizontal_slider_height(int height);

      void set_vertical_slider_width(int width);

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

      int get_horizontal_slider_position(const QScrollBar* scroll_bar) const;
      int get_vertical_slider_position(const QScrollBar* scroll_bar) const;
  };
}

#endif
