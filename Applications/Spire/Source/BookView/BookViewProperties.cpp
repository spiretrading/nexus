#include "Spire/BookView/BookViewProperties.hpp"
#include <fstream>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Serialization/JsonReceiver.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <QFontDatabase>
#include <QMessageBox>
#include "Spire/LegacyUI/UISerialization.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  auto load_legacy_properties(const std::filesystem::path& path) {
    auto properties = LegacyBookViewWindowSettings::Properties();
    try {
      auto reader =
        BasicIStreamReader<std::ifstream>(init(path, std::ios::binary));
      auto buffer = SharedBuffer();
      reader.read(out(buffer));
      auto type_registry = TypeRegistry<BinarySender<SharedBuffer>>();
      RegisterSpireTypes(out(type_registry));
      auto receiver = BinaryReceiver<SharedBuffer>(Ref(type_registry));
      receiver.set(Ref(buffer));
      receiver.shuttle(properties);
    } catch(std::exception&) {
      throw std::runtime_error("Unable to load book view properties.");
    }
    return properties;
  }

  auto convert_legacy_book_view_properties(const std::filesystem::path& path) {
    auto properties_path = path / "book_view.dat";
    if(!std::filesystem::exists(properties_path)) {
      throw std::runtime_error("book_view.dat not found.");
    }
    auto legacy_properties = load_legacy_properties(properties_path);
    auto properties = BookViewProperties::get_default();
    properties.m_level_properties.m_font = legacy_properties.m_book_quote_font;
    properties.m_level_properties.m_is_grid_enabled =
      legacy_properties.m_show_grid;
    properties.m_level_properties.m_fill_type =
      BookViewLevelProperties::FillType::SOLID;
    if(!legacy_properties.m_book_quote_background_colors.empty()) {
      properties.m_level_properties.m_color_scheme =
        legacy_properties.m_book_quote_background_colors;
    }
    properties.m_highlight_properties.m_order_visibility = [&] {
      using LegacyProperties = LegacyBookViewWindowSettings::Properties;
      if(legacy_properties.m_order_highlight ==
          LegacyProperties::OrderHighlight::HIDE_ORDERS) {
        return BookViewHighlightProperties::OrderVisibility::HIDDEN;
      } else if(legacy_properties.m_order_highlight ==
          LegacyProperties::OrderHighlight::DISPLAY_ORDERS) {
        return BookViewHighlightProperties::OrderVisibility::VISIBLE;
      }
      return BookViewHighlightProperties::OrderVisibility::HIGHLIGHTED;
    }();
    if(!legacy_properties.m_venue_highlights.empty()) {
      properties.m_highlight_properties.m_venue_highlights.clear();
      for(auto& highlight : legacy_properties.m_venue_highlights) {
        auto level = [&] {
          if(highlight.second.m_highlight_all_levels) {
            return BookViewHighlightProperties::VenueHighlightLevel::ALL;
          }
          return BookViewHighlightProperties::VenueHighlightLevel::TOP;
        }();
        auto color = HighlightColor(highlight.second.m_color,
          legacy_properties.m_book_quote_foreground_color);
        properties.m_highlight_properties.m_venue_highlights.push_back(
          BookViewHighlightProperties::VenueHighlight(
            highlight.first, color, level));
      }
    }
    return properties;
  }
}

const BookViewLevelProperties& BookViewLevelProperties::get_default() {
  static auto PROPERTIES = [] {
    auto properties = BookViewLevelProperties();
    properties.m_font = QFontDatabase().font("Roboto", "Medium", -1);
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

const BookViewProperties& BookViewProperties::get_default() {
  static auto PROPERTIES =
    BookViewProperties(BookViewLevelProperties::get_default(),
      BookViewHighlightProperties::get_default());
  return PROPERTIES;
}

const HighlightColor& Spire::get_highlight(const BookViewProperties& properties,
    BookViewHighlightProperties::OrderHighlightState state) {
  return properties.m_highlight_properties.m_order_highlights[
    static_cast<int>(state)];
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
    BookViewHighlightProperties::VenueHighlightLevel level) {
  if(level == BookViewHighlightProperties::VenueHighlightLevel::TOP) {
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

BookViewProperties Spire::load_book_view_properties(
    const std::filesystem::path& path) {
  auto file_path = path / "book_view.json";
  try {
    if(!std::filesystem::exists(file_path)) {
      auto legacy_path = path / "book_view.dat";
      if(std::filesystem::exists(legacy_path)) {
        return convert_legacy_book_view_properties(path);
      }
      return BookViewProperties::get_default();
    }
  } catch(const std::exception& e) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to load book view properties: %1.").arg(e.what()));
    return BookViewProperties::get_default();
  }
  auto properties = BookViewProperties();
  try {
    auto reader = BasicIStreamReader<std::ifstream>(init(file_path));
    auto buffer = SharedBuffer();
    reader.read(out(buffer));
    auto registry = TypeRegistry<JsonSender<SharedBuffer>>();
    RegisterSpireTypes(out(registry));
    auto receiver = JsonReceiver<SharedBuffer>(Ref(registry));
    receiver.set(Ref(buffer));
    receiver.shuttle(properties);
  } catch(const std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to load book view properties, using defaults."));
    return BookViewProperties::get_default();
  }
  return properties;


  return BookViewProperties::get_default();
}

void Spire::save_book_view_properties(
    const BookViewProperties& properties, const std::filesystem::path& path) {
  auto file_path = path / "book_view.json";
  try {
    auto registry = TypeRegistry<JsonSender<SharedBuffer>>();
    RegisterSpireTypes(out(registry));
    auto sender = JsonSender<SharedBuffer>(Ref(registry));
    auto buffer = SharedBuffer();
    sender.set(Ref(buffer));
    sender.shuttle(properties);
    auto writer = BasicOStreamWriter<std::ofstream>(init(file_path));
    writer.write(buffer);
  } catch(const std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to save book view properties."));
  }
}
