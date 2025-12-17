#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include <fstream>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Serialization/JsonReceiver.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <QMessageBox>
#include "Spire/LegacyUI/UISerialization.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/TimeAndSales/LegacyTimeAndSalesWindowSettings.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;

namespace {
  auto load_legacy_properties(const std::filesystem::path& path) {
    auto properties = LegacyTimeAndSalesWindowSettings::Properties();
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
      throw std::runtime_error("Unable to load time and sales properties.");
    }
    return properties;
  }

  auto convert_legacy_time_and_sales_properties(
      const std::filesystem::path& path) {
    enum LegacyColumns {
      TIME_COLUMN = 0,
      PRICE_COLUMN,
      SIZE_COLUMN,
      MARKET_COLUMN,
      CONDITION_COLUMN
    };
    static const auto LEGACY_COLUMN_COUNT = 5;
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
    if(legacy_properties.m_visible_columns[TIME_COLUMN]) {
      properties.set_visible(TimeAndSalesTableModel::Column::TIME, true);
    }
    if(legacy_properties.m_visible_columns[PRICE_COLUMN]) {
      properties.set_visible(TimeAndSalesTableModel::Column::PRICE, true);
    }
    if(legacy_properties.m_visible_columns[SIZE_COLUMN]) {
      properties.set_visible(TimeAndSalesTableModel::Column::SIZE, true);
    }
    if(legacy_properties.m_visible_columns[MARKET_COLUMN]) {
      properties.set_visible(TimeAndSalesTableModel::Column::MARKET, true);
    }
    if(legacy_properties.m_visible_columns[CONDITION_COLUMN]) {
      properties.set_visible(TimeAndSalesTableModel::Column::CONDITION, true);
    }
    properties.set_visible(TimeAndSalesTableModel::Column::BUYER, true);
    properties.set_visible(TimeAndSalesTableModel::Column::SELLER, true);
    properties.set_grid_enabled(legacy_properties.m_show_grid_lines);
    return properties;
  }
}

const TimeAndSalesProperties& TimeAndSalesProperties::get_default() {
  static auto PROPERTIES = [] {
    auto properties = TimeAndSalesProperties();
    properties.set_highlight_color(BboIndicator::UNKNOWN,
      {QColor(0xEBEBEB), QColor(0x333333)});
    properties.set_highlight_color(BboIndicator::ABOVE_ASK,
      {QColor(0xCAFFDE), QColor(0x007E00)});
    properties.set_highlight_color(BboIndicator::AT_ASK,
      {QColor(0xCAFFDE), QColor(0x007E00)});
    properties.set_highlight_color(BboIndicator::INSIDE,
      {QColor(0xFFFFFF), QColor(Qt::black)});
    properties.set_highlight_color(BboIndicator::AT_BID,
      {QColor(0xFAE8EA), QColor(0xCB1329)});
    properties.set_highlight_color(BboIndicator::BELOW_BID,
      {QColor(0xFAE8EA), QColor(0xCB1329)});
    auto font = QFont("Roboto");
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(10));
    properties.set_font(std::move(font));
    properties.set_visible(TimeAndSalesTableModel::Column::PRICE, true);
    properties.set_visible(TimeAndSalesTableModel::Column::SIZE, true);
    properties.set_visible(TimeAndSalesTableModel::Column::MARKET, true);
    properties.set_visible(TimeAndSalesTableModel::Column::CONDITION, true);
    properties.set_visible(TimeAndSalesTableModel::Column::BUYER, true);
    properties.set_visible(TimeAndSalesTableModel::Column::SELLER, true);
    properties.set_grid_enabled(false);
    return properties;
  }();
  return PROPERTIES;
}

TimeAndSalesProperties::TimeAndSalesProperties() {
  std::iota(m_column_order.begin(), m_column_order.end(), 0);
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

bool TimeAndSalesProperties::is_visible(
    TimeAndSalesTableModel::Column column) const {
  return m_visible_columns.test(m_column_order[static_cast<int>(column)]);
}

void TimeAndSalesProperties::set_visible(
    TimeAndSalesTableModel::Column column, bool is_visible) {
  if(is_visible) {
    m_visible_columns.set(m_column_order[static_cast<int>(column)]);
  } else {
    m_visible_columns.reset(m_column_order[static_cast<int>(column)]);
  }
}

bool TimeAndSalesProperties::is_grid_enabled() const {
  return m_is_grid_enabled;
}

void TimeAndSalesProperties::set_grid_enabled(bool is_enabled) {
  m_is_grid_enabled = is_enabled;
}

const std::array<int, TimeAndSalesProperties::COLUMN_COUNT>&
    TimeAndSalesProperties::get_column_order() const {
  return m_column_order;
}

void TimeAndSalesProperties::move_column(TimeAndSalesTableModel::Column source,
        TimeAndSalesTableModel::Column destination) {
  move_element(m_column_order, static_cast<int>(source),
    static_cast<int>(destination));
}

TimeAndSalesProperties Spire::load_time_and_sales_properties(
    const std::filesystem::path& path) {
  auto file_path = path / "time_and_sales.json";
  try {
    if(!std::filesystem::exists(file_path)) {
      auto legacy_path = path / "time_and_sales.dat";
      if(std::filesystem::exists(legacy_path)) {
        return convert_legacy_time_and_sales_properties(path);
      }
      return TimeAndSalesProperties::get_default();
    }
  } catch(const std::exception& e) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to load time and sales properties: %1.").arg(
        e.what()));
    return TimeAndSalesProperties::get_default();
  }
  auto properties = TimeAndSalesProperties();
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
      QObject::tr("Unable to load time and sales properties, using defaults."));
    return TimeAndSalesProperties::get_default();
  }
  return properties;
}

void Spire::save_time_and_sales_properties(
    const TimeAndSalesProperties& properties,
    const std::filesystem::path& path) {
  auto file_path = path / "time_and_sales.json";
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
      QObject::tr("Unable to save time and sales properties."));
  }
}
