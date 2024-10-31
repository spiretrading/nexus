#include "Spire/BookView/BookViewProperties.hpp"
#include <QFontDatabase>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

const BookViewLevelProperties& BookViewLevelProperties::get_default() {
  static auto PROPERTIES = [] {
    auto properties = BookViewLevelProperties();
    properties.m_font = QFontDatabase().font("Roboto", "Regular", -1);
    properties.m_font.setPixelSize(scale_width(10));
    properties.m_is_grid_enabled = false;
    properties.m_color_scheme = {QColor(0xFFFFFF), QColor(0xE7E7FF),
      QColor(0xCFCFFF), QColor(0xB7B7FF), QColor(0x9E9EFF), QColor(0x8686FF),
      QColor(0x6E6EFF)};
    return properties;
  }();
  return PROPERTIES;
}
