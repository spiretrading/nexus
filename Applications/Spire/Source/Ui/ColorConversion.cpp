#include "Spire/Ui/ColorConversion.hpp"
#include <numbers>

using namespace Spire;

namespace SA98G {
  const double MAIN_TRC = 2.4;
  const double RED_COEFFICIENT = 0.2126729;
  const double GREEN_COEFFICIENT = 0.7151522;
  const double BLUE_COEFFICIENT = 0.0721750;
  const double NORMAL_BLACKGROUND = 0.56;
  const double NORMAL_TEXT = 0.57;
  const double REVERT_TEXT = 0.62;
  const double REVERT_BLACKGROUND = 0.65;
  const double BLACK_THRESHOLD = 0.022;
  const double BLACK_EXP = 1.414;
  const double SCALE_BLACK_ON_WHITE = 1.14;
  const double SCALE_WHITE_ON_BLACK = 1.14;
  const double LOW_BLACK_ON_WHITE_OFFSET = 0.027;
  const double LOW_WHITE_ON_BLACK_OFFSET = 0.027;
  const double DELTA_YMIN = 0.0005;
  const double LOW_CLIP = 0.1;
};

namespace {
  double to_screen_luminance(const QColor& color) {
    auto simple_exp = [] (auto value) {
      return std::pow(value / 255.0, 2.4);
    };
    return SA98G::RED_COEFFICIENT * simple_exp(color.red()) +
      SA98G::GREEN_COEFFICIENT * simple_exp(color.green()) +
      SA98G::BLUE_COEFFICIENT * simple_exp(color.blue());
  }

  double atan2_degree(double y, double x) {
    if(y == 0.0 && x == 0.0) {
      return 0.0;
    }
    return std::atan2(y, x) * 180.0 / std::numbers::pi;
  }

  double normalize_hue(double hue) {
    hue = std::fmod(hue, 360.0);
    if(!hue) {
      return 0.0;
    } else if(hue < 0.0) {
      return hue + 360.0;
    }
    return hue;
  }
}

OklabColor::OklabColor()
  : m_l(-1),
    m_a(-1),
    m_b(-1) {}

OklabColor::OklabColor(double l, double a, double b)
  : m_l(l),
    m_a(a),
    m_b(b) {}

OklchColor::OklchColor()
  : m_l(-1),
    m_c(-1),
    m_h(0) {}

OklchColor::OklchColor(double l, double c, double h)
  : m_l(l),
    m_c(c),
    m_h(h) {}

bool Spire::is_valid(const OklabColor& color) {
  return color.m_l >= 0. && color.m_l <= 1. &&
    color.m_a >= -0.4 && color.m_a <= 0.4 &&
    color.m_b >= -0.4 && color.m_b <= 0.4;
}

bool Spire::is_valid(const OklchColor& color) {
  return color.m_l >= 0. && color.m_l <= 1.0 &&
    color.m_c >= 0. && color.m_c <= 0.5;
}

OklabColor Spire::to_oklab(const QColor& color) {
  auto lab = OklabColor();
  if(!color.isValid()) {
    return lab;
  }
  auto gamma_to_linear = [] (auto value) {
    if(value > 0.04045) {
      return std::pow((value + 0.055) / 1.055, 2.4);
    }
    return value / 12.92;
  };
  auto r = gamma_to_linear(color.redF());
  auto g = gamma_to_linear(color.greenF());
  auto b = gamma_to_linear(color.blueF());
  auto l = 0.4122214708 * r + 0.5363325363 * g + 0.0514459929 * b;
  auto m = 0.2119034982 * r + 0.6806995451 * g + 0.1073969566 * b;
  auto s = 0.0883024619 * r + 0.2817188376 * g + 0.6299787005 * b;
  l = std::cbrt(l);
  m = std::cbrt(m);
  s = std::cbrt(s);
  lab.m_l = 0.2104542553 * l + 0.7936177850 * m - 0.0040720468 * s;
  if(color.redF() == color.greenF() && color.greenF() == color.blueF()) {
    lab.m_a = 0.0;
    lab.m_b = 0.0;
  } else {
    lab.m_a = 1.9779984951 * l - 2.4285922050 * m + 0.4505937099 * s;
    lab.m_b = 0.0259040371 * l + 0.7827717662 * m - 0.8086757660 * s;
  }
  return lab;
}

QColor Spire::to_rgb(const OklabColor& color) {
  auto rgb = QColor();
  if(!is_valid(color)) {
    return rgb;
  }
  auto l = color.m_l + 0.3963377774 * color.m_a + 0.2158037573 * color.m_b;
  auto m = color.m_l - 0.1055613458 * color.m_a - 0.0638541728 * color.m_b;
  auto s = color.m_l - 0.0894841775 * color.m_a - 1.2914855480 * color.m_b;
  l = l * l * l;
  m = m * m * m;
  s = s * s * s;
  auto r = 4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s;
  auto g = -1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s;
  auto b = -0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s;
  auto linear_to_gamma = [] (auto value) {
    if(value > 0.0031308) {
      return 1.055 * std::pow(value, 1.0 / 2.4) - 0.055;
    }
    return 12.92 * value;
  };
  rgb.setRedF(std::clamp(linear_to_gamma(r), 0.0, 1.0));
  rgb.setGreenF(std::clamp(linear_to_gamma(g), 0.0, 1.0));
  rgb.setBlueF(std::clamp(linear_to_gamma(b), 0.0, 1.0));
  return rgb;
}

OklchColor Spire::to_oklch(const QColor& color) {
  auto lch = OklchColor();
  if(!color.isValid()) {
    return lch;
  }
  auto lab = to_oklab(color);
  lch.m_l = lab.m_l;
  lch.m_c = std::sqrt(lab.m_a * lab.m_a + lab.m_b * lab.m_b);
  auto c = lch.m_c;
  if(lch.m_c < 1.0E-13) {
    lch.m_c = 0.0;
    lch.m_h = 0.0;
  } else {
    lch.m_h = normalize_hue(atan2_degree(lab.m_b, lab.m_a));
  }
  return lch;
}

QColor Spire::to_rgb(const OklchColor& color) {
  auto rgb = QColor();
  if(!is_valid(color)) {
    return rgb;
  }
  auto h = color.m_h * std::numbers::pi / 180;
  auto lab = OklabColor();
  lab.m_l = color.m_l;
  lab.m_a = std::cos(h) * color.m_c;
  lab.m_b = std::sin(h) * color.m_c;
  return to_rgb(lab);
}

double Spire::apca(double text_luminance, double background_luminance) {
  if(std::min(text_luminance, background_luminance) < 0.0 ||
    std::max(text_luminance, background_luminance) > 1.1) {
    return 0.0;
  }
  auto adjust = [] (auto value) {
    if(value >= SA98G::BLACK_THRESHOLD) {
      return value;
    }
    return value + std::pow(SA98G::BLACK_THRESHOLD - value, SA98G::BLACK_EXP);
  };
  text_luminance = adjust(text_luminance);
  background_luminance = adjust(background_luminance);
  if(std::abs(background_luminance - text_luminance) < SA98G::DELTA_YMIN) {
    return 0.0;
  }
  auto output_contrast = [&] {
    if(background_luminance > text_luminance) {
      auto SAPC = (std::pow(background_luminance, SA98G::NORMAL_BLACKGROUND) -
        std::pow(text_luminance, SA98G::NORMAL_TEXT)) *
          SA98G::SCALE_BLACK_ON_WHITE;
      if(SAPC < SA98G::LOW_CLIP) {
        return 0.0;
      }
      return SAPC - SA98G::LOW_BLACK_ON_WHITE_OFFSET;
    } else {
      auto SAPC = (std::pow(background_luminance, SA98G::REVERT_BLACKGROUND) -
        std::pow(text_luminance, SA98G::REVERT_TEXT)) *
          SA98G::SCALE_WHITE_ON_BLACK;
      if(SAPC > -SA98G::LOW_CLIP) {
        return 0.0;
      }
      return SAPC + SA98G::LOW_WHITE_ON_BLACK_OFFSET;
    }
  }();
  return output_contrast * 100.0;
}

double Spire::apca(const QColor& text_color, const QColor& background_color) {
  return apca(to_screen_luminance(text_color),
    to_screen_luminance(background_color));
}

QColor Spire::apca_text_color(const QColor& background_color) {
  if(std::abs(apca(Qt::black, background_color)) >
      std::abs(apca(Qt::white, background_color))) {
    return Qt::black;
  }
  return Qt::white;
}
