#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include <fstream>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include "Spire/LegacyUI/UISerialization.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Spire;

namespace {
  struct LegacyTimeAndSalesProperties {
    static const auto PRICE_RANGE_COUNT = 6;
    static const auto COLUMN_COUNT = 5;

    std::array<QColor, PRICE_RANGE_COUNT> m_price_range_foreground_color;
    std::array<QColor, PRICE_RANGE_COUNT> m_price_range_background_color;
    std::array<bool, COLUMN_COUNT> m_visible_columns;
    bool m_show_grid_lines;
    bool m_vertical_scroll_bar_visible;
    bool m_horizontal_scroll_bar_visible;
    QFont m_font;
  };
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<LegacyTimeAndSalesProperties> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, LegacyTimeAndSalesProperties& value,
        unsigned int version) {
      shuttle.Shuttle("price_range_foreground_color",
        value.m_price_range_foreground_color);
      shuttle.Shuttle("price_range_background_color",
        value.m_price_range_background_color);
      shuttle.Shuttle("visible_columns", value.m_visible_columns);
      shuttle.Shuttle("show_grid_lines", value.m_show_grid_lines);
      shuttle.Shuttle(
        "vertical_scroll_bar_visible", value.m_vertical_scroll_bar_visible);
      shuttle.Shuttle("horizontal_scroll_bar_visible",
        value.m_horizontal_scroll_bar_visible);
      shuttle.Shuttle("font", value.m_font);
    }
  };
}

namespace {
  auto load_legacy_properties(const std::filesystem::path& path) {
    auto properties = LegacyTimeAndSalesProperties();
    try {
      auto reader =
        BasicIStreamReader<std::ifstream>(Initialize(path, std::ios::binary));
      auto buffer = SharedBuffer();
      reader.Read(Store(buffer));
      auto type_registry = TypeRegistry<BinarySender<SharedBuffer>>();
      RegisterSpireTypes(Store(type_registry));
      auto receiver = BinaryReceiver<SharedBuffer>(Ref(type_registry));
      receiver.SetSource(Ref(buffer));
      receiver.Shuttle(properties);
    } catch(std::exception&) {
      throw std::runtime_error("Unable to time and sales properties.");
    }
    return properties;
  }

  auto convert_legacy_time_and_sales_properties(
      const std::filesystem::path& path) {
    auto properties_path = path / "time_and_sales.dat";
    if(!std::filesystem::exists(properties_path)) {
      throw std::runtime_error("time_and_sales.dat not found.");
    }
    auto legacy_properties = load_legacy_properties(properties_path);
    auto properties = TimeAndSalesProperties(); 
    for(auto i = 0; i != BBO_INDICATOR_COUNT; ++i) {
      auto color = HighlightColor(
        legacy_properties.m_price_range_background_color[i],
        legacy_properties.m_price_range_foreground_color[i]);
      properties.set_highlight_color(static_cast<BboIndicator>(i), color);
    }
    properties.set_font(legacy_properties.m_font);
    properties.set_grid_enabled(legacy_properties.m_show_grid_lines);
    return properties;
  }
}

const TimeAndSalesProperties& TimeAndSalesProperties::get_default() {
  static auto PROPERTIES = [] {
    auto properties = TimeAndSalesProperties();
    properties.set_highlight_color(BboIndicator::UNKNOWN,
      {QColor(0xFFFFFF), QColor(Qt::black)});
    properties.set_highlight_color(BboIndicator::ABOVE_ASK,
      {QColor(0xEBFFF0), QColor(0x007735)});
    properties.set_highlight_color(BboIndicator::AT_ASK,
      {QColor(0xEBFFF0), QColor(0x007735)});
    properties.set_highlight_color(BboIndicator::INSIDE,
      {QColor(0xFFFFFF), QColor(Qt::black)});
    properties.set_highlight_color(BboIndicator::AT_BID,
      {QColor(0xFFF1F1), QColor(0xB71C1C)});
    properties.set_highlight_color(BboIndicator::BELOW_BID,
      {QColor(0xFFF1F1), QColor(0xB71C1C)});
    auto font = QFont("Roboto");
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(10));
    properties.set_font(std::move(font));
    properties.set_grid_enabled(false);
    return properties;
  }();
  return PROPERTIES;
}

const HighlightColor& TimeAndSalesProperties::get_highlight_color(
    BboIndicator indicator) const {
  return m_highlight_colors[static_cast<int>(indicator)];
}

void TimeAndSalesProperties::set_highlight_color(BboIndicator indicator,
    const HighlightColor& highlight_color) {
  m_highlight_colors[static_cast<int>(indicator)] = highlight_color;
}

const QFont& TimeAndSalesProperties::get_font() const {
  return m_font;
}

void TimeAndSalesProperties::set_font(const QFont& font) {
  m_font = font;
}

bool TimeAndSalesProperties::is_grid_enabled() const {
  return m_is_grid_enabled;
}

void TimeAndSalesProperties::set_grid_enabled(bool is_enabled) {
  m_is_grid_enabled = is_enabled;
}

TimeAndSalesProperties Spire::load_time_and_sales_properties(
    const std::filesystem::path& path) {
  auto file_path = path / "time_and_sales.json";
  if(!std::filesystem::exists(file_path)) {
    auto legacy_path = path / "time_and_sales.dat";
    if(std::filesystem::exists(legacy_path)) {
      return convert_legacy_time_and_sales_properties(path);
    }
    return TimeAndSalesProperties::get_default();
  }
  auto properties = TimeAndSalesProperties();
  try {
    auto reader = BasicIStreamReader<std::ifstream>(
      Initialize(file_path, std::ios::binary));
    auto buffer = SharedBuffer();
    reader.Read(Store(buffer));
    auto type_registry = TypeRegistry<BinarySender<SharedBuffer>>();
    RegisterSpireTypes(Store(type_registry));
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(type_registry));
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(properties);
  } catch(const std::exception&) {
    throw std::runtime_error("Unable to load time and sales properties.");
  }
  return properties;
}

void Spire::save_time_and_sales_properties(
    const TimeAndSalesProperties& properties,
    const std::filesystem::path& path) {
  auto file_path = path / "time_and_sales.json";
  try {
    auto type_registry = TypeRegistry<BinarySender<SharedBuffer>>();
    RegisterSpireTypes(Store(type_registry));
    auto sender = BinarySender<SharedBuffer>(Ref(type_registry));
    auto buffer = SharedBuffer();
    sender.SetSink(Ref(buffer));
    sender.Shuttle(properties);
    auto writer =
      BasicOStreamWriter<std::ofstream>(Initialize(path, std::ios::binary));
    writer.Write(buffer);
  } catch(const std::exception&) {
    throw std::runtime_error("Unable to save time and sales properties.");
  }
}
