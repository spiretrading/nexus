#ifndef SPIRE_COLOR_CONVERSION_HPP
#define SPIRE_COLOR_CONVERSION_HPP
#include <QColor>

namespace Spire {

  /** Represents an Oklab color. */
  struct OklabColor {

    /** Specifies the perceived lightness. */
    double m_l;

    /** Specifies the distance along the a axis in the Oklab colorspace. */
    double m_a;

    /** Specifies the distance along the b axis in the Oklab colorspace. */
    double m_b;

    /** Constructs an invalid OklabColor with the lab value (-1, -1, -1) */
    OklabColor();

    /** Constructs an OklabColor. */
    OklabColor(double l, double a, double b);
  };

  /** Represents an Oklch color. */
  struct OklchColor {

    /** Specifies the perceived lightness. */
    double m_l;

    /** Represents the measure of the chroma. */
    double m_c;

    /** Represents the hue angle. */
    double m_h;

    /** Constructs an invalid OklchColor with the lch value (-1, -1, 0) */
    OklchColor();

    /** Constructs an OklchColor. */
    OklchColor(double l, double c, double h);
  };

  /** Returns <code>true</code> iff the OklabColor is valid. */
  bool is_valid(const OklabColor& color);

  /** Returns <code>true</code> iff the OklchColor is valid. */
  bool is_valid(const OklchColor& color);

  /**
   * Converts a RGB color into a Oklab color.
   * @param color The RGB color.
   */
  OklabColor to_oklab(const QColor& color);

  /**
   * Converts a Oklab color into a RGB color.
   * @param color The Oklab color.
   */
  QColor to_rgb(const OklabColor& color);

  /**
   * Converts a RGB color into a Oklch color.
   * @param color The RGB color.
   */
  OklchColor to_oklch(const QColor& color);

  /**
   * Converts a Oklch color into a RGB color.
   * @param color The Oklch color.
   */
  QColor to_rgb(const OklchColor& color);

  /**
   * Interpolates the Oklch color from start to stop with ratio.
   * @param start The start color.
   * @param stop The stop color.
   * @param ratio The interpolation ratio.
   */
  OklchColor interpolate(const OklchColor& start, const OklchColor& stop,
    double ratio);

  /**
   * Scales colors from start to stop using oklch interpolation mode.
   * @param start The start color.
   * @param stop The stop color.
   * @param steps The steps by which colors are scaled.
   */
  std::vector<QColor> scale_oklch(const QColor& start, const QColor& stop,
    int steps);

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
