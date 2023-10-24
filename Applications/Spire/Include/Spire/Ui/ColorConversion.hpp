#ifndef SPIRE_COLOR_CONVERSION_HPP
#define SPIRE_COLOR_CONVERSION_HPP
#include <QColor>

namespace Spire {

  /** Represents a Oklab color. */
  struct OklabColor {

    /** Specifies if the color is valid. */
    bool m_is_valid;

    /** Specifies the perceived lightness. */
    double m_l;

    /** Specifies the distance along the a axis in the Oklab colorspace. */
    double m_a;

    /** Specifies the distance along the b axis in the Oklab colorspace. */
    double m_b;
  };

  /** Represents a Oklch color. */
  struct OklchColor {

    /** Specifies if the color is valid. */
    bool m_is_valid;

    /** Specifies the perceived lightness. */
    double m_l;

    /** Represents the measure of the chroma. */
    double m_c;

    /** Represents the hue angle. */
    double m_h;
  };

  /**
   * Converts a RGB color into a Oklab color.
   * @param color The RGG color.
   */
  OklabColor to_oklab(const QColor& color);

  /**
   * Converts a Oklab color into a RGB color.
   * @param color The Oklab color.
   */
  QColor to_rgb(const OklabColor& color);

  /**
   * Converts a RGB color into a Oklch color.
   * @param color The RGG color.
   */
  OklchColor to_oklch(const QColor& color);

  /**
   * Converts a Oklch color into a RGB color.
   * @param color The Oklch color.
   */
  QColor to_rgb(const OklchColor& color);

  /**
   * Calculates the APCA contrast.
   * @param text_luminance The luminance of the text.
   * @param background_luminance The luminance of the background.
   */
  double apca(double text_luminance, double background_luminance);

  /**
   * Calculates the APCA contrast.
   * @param text_color The text color.
   * @param background_color The background color.
   */
  double apca(const QColor& text_color, const QColor& background_color);
}

#endif
