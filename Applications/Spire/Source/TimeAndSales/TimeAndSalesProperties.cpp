#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include <filesystem>
#include <fstream>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <QMessageBox>
#include "Spire/LegacyUI/UISerialization.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;
using namespace std::filesystem;

TimeAndSalesProperties TimeAndSalesProperties::GetDefault() {
  TimeAndSalesProperties properties;
  for(auto i = properties.m_visibleColumns.begin();
      i != properties.m_visibleColumns.end(); ++i) {
    *i = true;
  }
  properties.m_visibleColumns[TimeAndSalesModel::TIME_COLUMN] = false;
  properties.m_visibleColumns[TimeAndSalesModel::CONDITION_COLUMN] = false;
  properties.m_showGridLines = false;
  properties.m_horizontalScrollBarVisible = true;
  properties.m_verticalScrollBarVisible = true;
  properties.m_priceRangeBackgroundColor[TimeAndSalesModel::UNKNOWN] =
    QColor(243, 243, 243);
  properties.m_priceRangeForegroundColor[TimeAndSalesModel::UNKNOWN] =
    QColor(0, 0, 0);
  properties.m_priceRangeBackgroundColor[TimeAndSalesModel::ABOVE_ASK] =
    QColor(196, 250, 194);
  properties.m_priceRangeForegroundColor[TimeAndSalesModel::ABOVE_ASK] =
    QColor(0, 119, 53);
  properties.m_priceRangeBackgroundColor[TimeAndSalesModel::AT_ASK] =
    QColor(196, 250, 194);
  properties.m_priceRangeForegroundColor[TimeAndSalesModel::AT_ASK] =
    QColor(0, 119, 53);
  properties.m_priceRangeBackgroundColor[TimeAndSalesModel::INSIDE] =
    QColor(243, 243, 243);
  properties.m_priceRangeForegroundColor[TimeAndSalesModel::INSIDE] =
    QColor(0, 0, 0);
  properties.m_priceRangeBackgroundColor[TimeAndSalesModel::AT_BID] =
    QColor(225, 220, 220);
  properties.m_priceRangeForegroundColor[TimeAndSalesModel::AT_BID] =
    QColor(192, 0, 0);
  properties.m_priceRangeBackgroundColor[TimeAndSalesModel::BELOW_BID] =
    QColor(225, 220, 220);
  properties.m_priceRangeForegroundColor[TimeAndSalesModel::BELOW_BID] =
    QColor(192, 0, 0);
  properties.m_font = QFont("Arial", 8, QFont::Bold);
  return properties;
}

void TimeAndSalesProperties::Load(Out<UserProfile> userProfile) {
  path timeAndSalesFilePath = userProfile->GetProfilePath() /
    "time_and_sales.dat";
  if(!exists(timeAndSalesFilePath)) {
    userProfile->SetDefaultTimeAndSalesProperties(GetDefault());
    return;
  }
  TimeAndSalesProperties properties;
  try {
    BasicIStreamReader<ifstream> reader(
      init(timeAndSalesFilePath, ios::binary));
    SharedBuffer buffer;
    reader.read(out(buffer));
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(out(typeRegistry));
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
    receiver.set(Ref(buffer));
    receiver.shuttle(properties);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to load time and sales properties, using defaults."));
    properties = GetDefault();
  }
  userProfile->SetDefaultTimeAndSalesProperties(properties);
}

void TimeAndSalesProperties::Save(const UserProfile& userProfile) {
  path timeAndSalesFilePath = userProfile.GetProfilePath() /
    "time_and_sales.dat";
  try {
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(out(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    SharedBuffer buffer;
    sender.set(Ref(buffer));
    sender.shuttle(userProfile.GetDefaultTimeAndSalesProperties());
    BasicOStreamWriter<ofstream> writer(
      init(timeAndSalesFilePath, ios::binary));
    writer.write(buffer);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to save time and sales properties."));
  }
}

TimeAndSalesProperties::TimeAndSalesProperties() {}

const std::array<QColor, TimeAndSalesProperties::PRICE_RANGE_COUNT>&
    TimeAndSalesProperties::GetPriceRangeForegroundColor() const {
  return m_priceRangeForegroundColor;
}

std::array<QColor, TimeAndSalesProperties::PRICE_RANGE_COUNT>&
    TimeAndSalesProperties::GetPriceRangeForegroundColor() {
  return m_priceRangeForegroundColor;
}

const std::array<QColor, TimeAndSalesProperties::PRICE_RANGE_COUNT>&
    TimeAndSalesProperties::GetPriceRangeBackgroundColor() const {
  return m_priceRangeBackgroundColor;
}

std::array<QColor, TimeAndSalesProperties::PRICE_RANGE_COUNT>&
    TimeAndSalesProperties::GetPriceRangeBackgroundColor() {
  return m_priceRangeBackgroundColor;
}

const std::array<bool, TimeAndSalesProperties::COLUMN_COUNT>&
    TimeAndSalesProperties::GetVisibleColumns() const {
  return m_visibleColumns;
}

std::array<bool, TimeAndSalesProperties::COLUMN_COUNT>&
    TimeAndSalesProperties::GetVisibleColumns() {
  return m_visibleColumns;
}

bool TimeAndSalesProperties::GetShowGridLines() const {
  return m_showGridLines;
}

void TimeAndSalesProperties::SetShowGridLines(bool value) {
  m_showGridLines = value;
}

bool TimeAndSalesProperties::IsVerticalScrollBarVisible() const {
  return m_verticalScrollBarVisible;
}

void TimeAndSalesProperties::SetVerticalScrollBarVisible(bool isVisible) {
  m_verticalScrollBarVisible = isVisible;
}

bool TimeAndSalesProperties::IsHorizontalScrollBarVisible() const {
  return m_horizontalScrollBarVisible;
}

void TimeAndSalesProperties::SetHorizontalScrollBarVisible(bool isVisible) {
  m_horizontalScrollBarVisible = isVisible;
}

const QFont& TimeAndSalesProperties::GetFont() const {
  return m_font;
}

void TimeAndSalesProperties::SetFont(const QFont& font) {
  m_font = font;
}
