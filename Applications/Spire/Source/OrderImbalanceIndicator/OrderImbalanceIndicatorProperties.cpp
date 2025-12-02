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

    template<IsShuttle S>
    void shuttle(S& shuttle, unsigned int version) {
      shuttle.shuttle("properties", m_properties);
      shuttle.shuttle("window_settings", m_windowSettings);
    }
  };
}

OrderImbalanceIndicatorProperties OrderImbalanceIndicatorProperties::
    GetDefault() {
  OrderImbalanceIndicatorProperties properties;
  properties.m_startTime.m_offset = hours(1);
  properties.m_endTime.m_specialValue = Beam::Sequence::LAST;
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
      init(orderImbalanceIndicatorFilePath, ios::binary));
    SharedBuffer buffer;
    reader.read(out(buffer));
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(out(typeRegistry));
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
    receiver.set(Ref(buffer));
    receiver.shuttle(settings);
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
    RegisterSpireTypes(out(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    SharedBuffer buffer;
    sender.set(Ref(buffer));
    OrderImbalanceIndicatorFileSettings settings;
    settings.m_properties =
      userProfile.GetDefaultOrderImbalanceIndicatorProperties();
    settings.m_windowSettings =
      userProfile.GetInitialOrderImbalanceIndicatorWindowSettings();
    sender.shuttle(settings);
    BasicOStreamWriter<ofstream> writer(
      init(orderImbalanceIndicatorFilePath, ios::binary));
    writer.write(buffer);
  } catch(const std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to save the order imbalance indicator properties."));
  }
}

bool OrderImbalanceIndicatorProperties::IsDisplayed(Venue venue) const {
  return m_filteredVenues.find(venue) == m_filteredVenues.end();
}

bool OrderImbalanceIndicatorProperties::IsFiltered(Venue venue) const {
  return !IsDisplayed(venue);
}

Beam::Range OrderImbalanceIndicatorProperties::GetTimeRange(
    TimeClient& timeClient) const {
  Beam::Range::Point start;
  if(m_startTime.m_offset.is_initialized()) {
    start = timeClient.get_time() - *m_startTime.m_offset;
  } else if(m_startTime.m_timeOfDay.is_initialized()) {
    start = ptime(gregorian::day_clock::universal_day(),
      *m_startTime.m_timeOfDay);
  } else {
    start = Beam::Sequence::PRESENT;
  }
  Beam::Range::Point end;
  if(m_endTime.m_offset.is_initialized()) {
    end = timeClient.get_time() - *m_endTime.m_offset;
  } else if(m_endTime.m_timeOfDay.is_initialized()) {
    end = ptime(gregorian::day_clock::universal_day(), *m_endTime.m_timeOfDay);
  } else {
    end = Beam::Sequence::LAST;
  }
  Beam::Range timeRange(start, end);
  return timeRange;
}
