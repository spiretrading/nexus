#include "Spire/Spire/Hash.hpp"

std::size_t std::hash<QColor>::operator ()(const QColor& color) const {
  return color.rgba();
}

std::size_t std::hash<QFont>::operator ()(const QFont& font) const {
  return qHash(font);
}

std::size_t std::hash<QImage>::operator ()(const QImage& image) const {
  return 0;
}
