#include "Spire/Spire/Hash.hpp"

std::size_t std::hash<QColor>::operator ()(const QColor& color) const {
  if(color.spec() == QColor::Hsl) {
    return color.alpha();
  } else if(color.spec() == QColor::Hsv) {
    return qRgba(0, color.hsvSaturation(), color.value(), color.alpha());
  }
  return color.rgba();
}

std::size_t std::hash<QFont>::operator ()(const QFont& font) const {
  return qHash(font);
}

std::size_t std::hash<QImage>::operator ()(const QImage& image) const {
  return 0;
}
