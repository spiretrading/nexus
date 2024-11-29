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
    properties.m_color_scheme = {QColor(0xFFFFFF), QColor(0xE4E8FF),
      QColor(0xCAD1FF), QColor(0xB1BAFF), QColor(0x99A2FF), QColor(0x8289FF),
      QColor(0x6E6EFF)};
    return properties;
  }();
  return PROPERTIES;
}

const BookViewHighlightProperties& BookViewHighlightProperties::get_default() {
  static auto PROPERTIES = [] {
    auto properties = BookViewHighlightProperties();
    properties.m_order_visibility = OrderVisibility::HIGHLIGHTED;
    properties.m_order_highlights = {{{QColor(0xBDFFC5), QColor(0x808080)},
      {QColor(0x3CFF53), QColor(Qt::black)},
      {QColor(Qt::black), QColor(0x3CFF53)},
      {QColor(0xFFC758), QColor(Qt::black)},
      {QColor(0xE63F44), QColor(0xFFFFFF)}}};
    return properties;
  }();
  return PROPERTIES;
}

const QString& Spire::to_text(
    BookViewHighlightProperties::OrderVisibility visibility) {
  if(visibility == BookViewHighlightProperties::OrderVisibility::HIDDEN) {
    static const auto value = QObject::tr("Hide");
    return value;
  } else if(
      visibility == BookViewHighlightProperties::OrderVisibility::VISIBLE) {
    static const auto value = QObject::tr("Show");
    return value;
  } else {
    static const auto value = QObject::tr("Highlight");
    return value;
  }
}

const QString& Spire::to_text(
    BookViewHighlightProperties::MarketHighlightLevel level) {
  if(level == BookViewHighlightProperties::MarketHighlightLevel::TOP) {
    static const auto value = QObject::tr("Top Level");
    return value;
  } else {
    static const auto value = QObject::tr("All Levels");
    return value;
  }
}

const QString& Spire::to_text(
    BookViewHighlightProperties::OrderHighlightState state) {
  if(state == BookViewHighlightProperties::OrderHighlightState::PREVIEW) {
    static const auto value = QObject::tr("Preview");
    return value;
  } else if(
      state == BookViewHighlightProperties::OrderHighlightState::ACTIVE) {
    static const auto value = QObject::tr("Active");
    return value;
  } else if(state == BookViewHighlightProperties::OrderHighlightState::FILLED) {
    static const auto value = QObject::tr("Filled");
    return value;
  } else if(
      state == BookViewHighlightProperties::OrderHighlightState::CANCELED) {
    static const auto value = QObject::tr("Canceled");
    return value;
  } else {
    static const auto value = QObject::tr("Rejected");
    return value;
  }
}
