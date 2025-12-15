#include "Spire/PortfolioViewer/PortfolioViewerProperties.hpp"
#include <filesystem>
#include <fstream>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <QMessageBox>
#include "Spire/PortfolioViewer/PortfolioViewerWindowSettings.hpp"
#include "Spire/UI/UISerialization.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace std;
using namespace std::filesystem;

namespace {
  struct PortfolioViewerFileSettings {
    PortfolioViewerProperties m_properties;
    boost::optional<PortfolioViewerWindowSettings> m_windowSettings;

    template<IsShuttle S>
    void shuttle(S& shuttle, unsigned int version) {
      shuttle.shuttle("properties", m_properties);
      shuttle.shuttle("window_settings", m_windowSettings);
    }
  };
}

PortfolioViewerProperties PortfolioViewerProperties::GetDefault() {
  PortfolioViewerProperties properties;
  properties.SetSelectingAllGroups(true);
  properties.SetSelectingAllCurrencies(true);
  properties.SetSelectingAllVenues(true);
  properties.GetSelectedSides().insert(Side::NONE);
  properties.GetSelectedSides().insert(Side::ASK);
  properties.GetSelectedSides().insert(Side::BID);
  return properties;
}

void PortfolioViewerProperties::Load(Out<UserProfile> userProfile) {
  path portfolioViewerFilePath = userProfile->GetProfilePath() /
    "portfolio_viewer.dat";
  if(!exists(portfolioViewerFilePath)) {
    userProfile->SetDefaultPortfolioViewerProperties(GetDefault());
    return;
  }
  PortfolioViewerFileSettings settings;
  try {
    BasicIStreamReader<ifstream> reader(
      init(portfolioViewerFilePath, ios::binary));
    SharedBuffer buffer;
    reader.read(out(buffer));
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(out(typeRegistry));
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
    receiver.set(Ref(buffer));
    receiver.shuttle(settings);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr(
      "Unable to load the portfolio viewer properties, using defaults."));
    settings.m_properties = GetDefault();
    settings.m_windowSettings = none;
  }
  userProfile->SetDefaultPortfolioViewerProperties(settings.m_properties);
  if(settings.m_windowSettings.is_initialized()) {
    userProfile->SetInitialPortfolioViewerWindowSettings(
      *settings.m_windowSettings);
  }
}

void PortfolioViewerProperties::Save(const UserProfile& userProfile) {
  path portfolioViewerFilePath = userProfile.GetProfilePath() /
    "portfolio_viewer.dat";
  try {
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(out(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    SharedBuffer buffer;
    sender.set(Ref(buffer));
    PortfolioViewerFileSettings settings;
    settings.m_properties = userProfile.GetDefaultPortfolioViewerProperties();
    settings.m_windowSettings =
      userProfile.GetInitialPortfolioViewerWindowSettings();
    sender.shuttle(settings);
    BasicOStreamWriter<ofstream> writer(
      init(portfolioViewerFilePath, ios::binary));
    writer.write(buffer);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to save the portfolio viewer properties."));
  }
}

PortfolioViewerProperties::PortfolioViewerProperties() {}

const unordered_set<DirectoryEntry>& PortfolioViewerProperties::
    GetSelectedGroups() const {
  return m_selectedGroups;
}

unordered_set<DirectoryEntry>& PortfolioViewerProperties::GetSelectedGroups() {
  return m_selectedGroups;
}

bool PortfolioViewerProperties::IsSelectingAllGroups() const {
  return m_selectAllGroups;
}

void PortfolioViewerProperties::SetSelectingAllGroups(bool value) {
  m_selectAllGroups = value;
}

const unordered_set<CurrencyId>& PortfolioViewerProperties::
    GetSelectedCurrencies() const {
  return m_selectedCurrencies;
}

unordered_set<CurrencyId>& PortfolioViewerProperties::GetSelectedCurrencies() {
  return m_selectedCurrencies;
}

bool PortfolioViewerProperties::IsSelectingAllCurrencies() const {
  return m_selectAllCurrencies;
}

void PortfolioViewerProperties::SetSelectingAllCurrencies(bool value) {
  m_selectAllCurrencies = value;
}

const unordered_set<Venue>& PortfolioViewerProperties::
    GetSelectedVenues() const {
  return m_selectedVenues;
}

unordered_set<Venue>& PortfolioViewerProperties::GetSelectedVenues() {
  return m_selectedVenues;
}

bool PortfolioViewerProperties::IsSelectingAllVenues() const {
  return m_selectAllVenues;
}

void PortfolioViewerProperties::SetSelectingAllVenues(bool value) {
  m_selectAllVenues = value;
}

const unordered_set<Side>& PortfolioViewerProperties::GetSelectedSides() const {
  return m_selectedSides;
}

unordered_set<Side>& PortfolioViewerProperties::GetSelectedSides() {
  return m_selectedSides;
}
