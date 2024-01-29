#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorProperties.hpp"
#include <filesystem>
#include <fstream>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <QMessageBox>
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorWindowSettings.hpp"
#include "Spire/LegacyUI/UISerialization.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Queries;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace std;
using namespace std::filesystem;

namespace {
  struct OrderImbalanceIndicatorFileSettings {
    OrderImbalanceIndicatorProperties m_properties;
    boost::optional<OrderImbalanceIndicatorWindowSettings> m_windowSettings;

    template<typename Shuttler>
    void Shuttle(Shuttler& shuttle, unsigned int version) {
      shuttle.Shuttle("properties", m_properties);
      shuttle.Shuttle("window_settings", m_windowSettings);
    }
  };
}

OrderImbalanceIndicatorProperties OrderImbalanceIndicatorProperties::
    GetDefault() {
  OrderImbalanceIndicatorProperties properties;
  properties.m_startTime.m_offset = hours(1);
  properties.m_endTime.m_specialValue = Beam::Queries::Sequence::Last();
  return properties;
}

void OrderImbalanceIndicatorProperties::Load(Out<UserProfile> userProfile) {
  path orderImbalanceIndicatorFilePath = userProfile->GetProfilePath() /
    "order_imbalance_indicator.dat";
  if(!exists(orderImbalanceIndicatorFilePath)) {
    userProfile->SetDefaultOrderImbalanceIndicatorProperties(GetDefault());
    return;
  }
  OrderImbalanceIndicatorFileSettings settings;
  try {
    BasicIStreamReader<ifstream> reader(
      Initialize(orderImbalanceIndicatorFilePath, ios::binary));
    SharedBuffer buffer;
    reader.Read(Store(buffer));
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(Store(typeRegistry));
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(settings);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to load the order imbalance indicator properties,"
      " using defaults."));
    settings.m_properties = GetDefault();
    settings.m_windowSettings = none;
  }
  userProfile->SetDefaultOrderImbalanceIndicatorProperties(
    settings.m_properties);
  if(settings.m_windowSettings.is_initialized()) {
    userProfile->SetInitialOrderImbalanceIndicatorWindowSettings(
      *settings.m_windowSettings);
  }
}

void OrderImbalanceIndicatorProperties::Save(const UserProfile& userProfile) {
  path orderImbalanceIndicatorFilePath = userProfile.GetProfilePath() /
    "order_imbalance_indicator.dat";
  try {
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(Store(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    SharedBuffer buffer;
    sender.SetSink(Ref(buffer));
    OrderImbalanceIndicatorFileSettings settings;
    settings.m_properties =
      userProfile.GetDefaultOrderImbalanceIndicatorProperties();
    settings.m_windowSettings =
      userProfile.GetInitialOrderImbalanceIndicatorWindowSettings();
    sender.Shuttle(settings);
    BasicOStreamWriter<ofstream> writer(
      Initialize(orderImbalanceIndicatorFilePath, ios::binary));
    writer.Write(buffer);
  } catch(const std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to save the order imbalance indicator properties."));
  }
}

bool OrderImbalanceIndicatorProperties::IsDisplayed(MarketCode market) const {
  return m_filteredMarkets.find(market) == m_filteredMarkets.end();
}

bool OrderImbalanceIndicatorProperties::IsFiltered(MarketCode market) const {
  return !IsDisplayed(market);
}

Beam::Queries::Range OrderImbalanceIndicatorProperties::GetTimeRange(
    TimeClientBox& timeClient) const {
  Beam::Queries::Range::Point start;
  if(m_startTime.m_offset.is_initialized()) {
    start = timeClient.GetTime() - *m_startTime.m_offset;
  } else if(m_startTime.m_timeOfDay.is_initialized()) {
    start = ptime(gregorian::day_clock::universal_day(),
      *m_startTime.m_timeOfDay);
  } else {
    start = Beam::Queries::Sequence::Present();
  }
  Beam::Queries::Range::Point end;
  if(m_endTime.m_offset.is_initialized()) {
    end = timeClient.GetTime() - *m_endTime.m_offset;
  } else if(m_endTime.m_timeOfDay.is_initialized()) {
    end = ptime(gregorian::day_clock::universal_day(), *m_endTime.m_timeOfDay);
  } else {
    end = Beam::Queries::Sequence::Last();
  }
  Beam::Queries::Range timeRange(start, end);
  return timeRange;
}
