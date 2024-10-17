#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include <fstream>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Serialization/JsonReceiver.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include "Spire/LegacyUI/UISerialization.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/TimeAndSales/LegacyTimeAndSalesWindowSettings.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Spire;

namespace {
  auto load_legacy_properties(const std::filesystem::path& path) {
    auto properties = LegacyTimeAndSalesWindowSettings::Properties();
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
    auto reader = BasicIStreamReader<std::ifstream>(Initialize(file_path));
    auto buffer = SharedBuffer();
    reader.Read(Store(buffer));
    auto registry = TypeRegistry<JsonSender<SharedBuffer>>();
    RegisterSpireTypes(Store(registry));
    auto receiver = JsonReceiver<SharedBuffer>(Ref(registry));
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
    auto registry = TypeRegistry<JsonSender<SharedBuffer>>();
    RegisterSpireTypes(Store(registry));
    auto sender = JsonSender<SharedBuffer>(Ref(registry));
    auto buffer = SharedBuffer();
    sender.SetSink(Ref(buffer));
    sender.Shuttle(properties);
    auto writer = BasicOStreamWriter<std::ofstream>(Initialize(file_path));
    writer.Write(buffer);
  } catch(const std::exception&) {
    throw std::runtime_error("Unable to save time and sales properties.");
  }
}
