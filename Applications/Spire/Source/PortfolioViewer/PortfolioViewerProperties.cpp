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
#include "Spire/LegacyUI/UISerialization.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace std;
using namespace std::filesystem;

namespace {
  struct PortfolioViewerFileSettings {
    PortfolioViewerProperties m_properties;
    boost::optional<PortfolioViewerWindowSettings> m_windowSettings;

    template<typename Shuttler>
    void Shuttle(Shuttler& shuttle, unsigned int version) {
      shuttle.Shuttle("properties", m_properties);
      shuttle.Shuttle("window_settings", m_windowSettings);
    }
  };
}

PortfolioViewerProperties PortfolioViewerProperties::GetDefault() {
  PortfolioViewerProperties properties;
  properties.SetSelectingAllGroups(true);
  properties.SetSelectingAllCurrencies(true);
  properties.SetSelectingAllMarkets(true);
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
      Initialize(portfolioViewerFilePath, ios::binary));
    SharedBuffer buffer;
    reader.Read(Store(buffer));
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(Store(typeRegistry));
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(settings);
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
    RegisterSpireTypes(Store(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    SharedBuffer buffer;
    sender.SetSink(Ref(buffer));
    PortfolioViewerFileSettings settings;
    settings.m_properties = userProfile.GetDefaultPortfolioViewerProperties();
    settings.m_windowSettings =
      userProfile.GetInitialPortfolioViewerWindowSettings();
    sender.Shuttle(settings);
    BasicOStreamWriter<ofstream> writer(
      Initialize(portfolioViewerFilePath, ios::binary));
    writer.Write(buffer);
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

const unordered_set<MarketCode>& PortfolioViewerProperties::
    GetSelectedMarkets() const {
  return m_selectedMarkets;
}

unordered_set<MarketCode>& PortfolioViewerProperties::GetSelectedMarkets() {
  return m_selectedMarkets;
}

bool PortfolioViewerProperties::IsSelectingAllMarkets() const {
  return m_selectAllMarkets;
}

void PortfolioViewerProperties::SetSelectingAllMarkets(bool value) {
  m_selectAllMarkets = value;
}

const unordered_set<Side>& PortfolioViewerProperties::GetSelectedSides() const {
  return m_selectedSides;
}

unordered_set<Side>& PortfolioViewerProperties::GetSelectedSides() {
  return m_selectedSides;
}
