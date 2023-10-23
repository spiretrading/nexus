#ifndef SPIRE_COLOR_CONVERSION_HPP
#define SPIRE_COLOR_CONVERSION_HPP
#include <QColor>

namespace Spire {

  struct OklabColor {
    bool m_is_valid;
    double m_l;
    double m_a;
    double m_b;
  };

  struct OklchColor {
    bool m_is_valid;
    double m_l;
    double m_c;
    double m_h;
  };

  OklabColor to_oklab(const QColor& color);

  QColor to_rgb(const OklabColor& color);

  OklchColor to_oklch(const QColor& color);

  QColor to_rgb(const OklchColor& color);

  double apca(double text_luminance, double background_luminance);

  double apca(const QColor& text_color, const QColor& background_color);
}

#endif
