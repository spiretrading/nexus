#include "Spire/Blotter/BlotterSettings.hpp"
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Utilities/AssertionException.hpp>
#include <QMessageBox>
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterTaskProperties.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/UI/UISerialization.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::UI;

namespace {
  void LoadDefaultBlotter(Out<UserProfile> userProfile) {
    auto& settings = userProfile->GetBlotterSettings();
    settings.SetActiveBlotter(settings.GetConsolidatedBlotter());
    settings.SetDefaultBlotterTaskProperties(
      BlotterTaskProperties::GetDefault());
    settings.SetDefaultOrderLogProperties(OrderLogProperties::GetDefault());
  }

  struct BlotterModelProperties {
    std::string m_name;
    DirectoryEntry m_account;
    bool m_isConsolidated;
    std::vector<std::string> m_links;
    BlotterTaskProperties m_taskProperties;
    OrderLogProperties m_orderLogProperties;

    BlotterModelProperties() = default;
    BlotterModelProperties(const BlotterModel& model);

    template<typename Shuttler>
    void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  struct BlotterSettingsData {
    std::vector<BlotterModelProperties> m_blotters;
    std::string m_activeBlotter;
    BlotterTaskProperties m_defaultTaskProperties;

    template<typename Shuttler>
    void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void BlotterSettingsData::Shuttle(Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("blotters", m_blotters);
    shuttle.Shuttle("active_blotter", m_activeBlotter);
    shuttle.Shuttle("default_task_properties", m_defaultTaskProperties);
  }

  BlotterModelProperties::BlotterModelProperties(const BlotterModel& model)
      : m_name(model.GetName()),
        m_account(model.GetExecutingAccount()),
        m_isConsolidated(model.IsConsolidated()),
        m_taskProperties(model.GetTasksModel().GetProperties()),
        m_orderLogProperties(model.GetOrderLogModel().GetProperties()) {
    for(auto& blotter : model.GetLinkedBlotters()) {
      m_links.push_back(blotter->GetName());
    }
  }

  template<typename Shuttler>
  void BlotterModelProperties::Shuttle(
      Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("account", m_account);
    shuttle.Shuttle("is_consolidated", m_isConsolidated);
    shuttle.Shuttle("links", m_links);
    shuttle.Shuttle("task_properties", m_taskProperties);
    shuttle.Shuttle("order_log_properties", m_orderLogProperties);
  }
}

void BlotterSettings::Load(Out<UserProfile> userProfile) {
  auto blottersFilePath = userProfile->GetProfilePath() / "blotters.dat";
  if(!std::filesystem::exists(blottersFilePath)) {
    LoadDefaultBlotter(Store(userProfile));
    return;
  }
  auto data = BlotterSettingsData();
  try {
    auto reader = BasicIStreamReader<std::ifstream>(
      Initialize(blottersFilePath, std::ios::binary));
    auto buffer = SharedBuffer();
    reader.Read(Store(buffer));
    auto typeRegistry = TypeRegistry<BinarySender<SharedBuffer>>();
    RegisterSpireTypes(Store(typeRegistry));
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(data);
  } catch(const std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to load blotters, using defaults."));
    LoadDefaultBlotter(Store(userProfile));
    return;
  }
  auto& settings = userProfile->GetBlotterSettings();
  auto nameToBlotter = std::unordered_map<std::string, BlotterModel*>();
  for(auto& setting : data.m_blotters) {
    auto account =
      userProfile->GetServiceClients().GetServiceLocatorClient().FindAccount(
        setting.m_account.m_name);
    if(!account) {

      // TODO: Account doesn't exist.
      account = setting.m_account;
    }
    auto blotter = std::make_unique<BlotterModel>(setting.m_name, *account,
      setting.m_isConsolidated, Ref(*userProfile), setting.m_taskProperties,
      setting.m_orderLogProperties);
    blotter->SetPersistent(true);
    nameToBlotter.insert(std::pair(setting.m_name, blotter.get()));
    settings.AddBlotter(std::move(blotter));
  }
  for(auto& setting : data.m_blotters) {
    auto& model = *nameToBlotter[setting.m_name];
    for(auto& link : setting.m_links) {
      auto blotterIterator = nameToBlotter.find(link);
      if(blotterIterator != nameToBlotter.end()) {
        model.Link(Ref(*blotterIterator->second));
      }
    }
  }
  auto activeBlotterIterator = nameToBlotter.find(data.m_activeBlotter);
  if(activeBlotterIterator != nameToBlotter.end()) {
    settings.SetActiveBlotter(*activeBlotterIterator->second);
  } else {
    settings.SetActiveBlotter(settings.GetConsolidatedBlotter());
  }
  settings.SetDefaultBlotterTaskProperties(data.m_defaultTaskProperties);
}

void BlotterSettings::Save(const UserProfile& userProfile) {
  auto blottersFilePath = userProfile.GetProfilePath() / "blotters.dat";
  try {
    auto typeRegistry = TypeRegistry<BinarySender<SharedBuffer>>();
    RegisterSpireTypes(Store(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    auto buffer = SharedBuffer();
    sender.SetSink(Ref(buffer));
    auto& settings = userProfile.GetBlotterSettings();
    auto data = BlotterSettingsData();
    for(auto& setting : settings.GetAllBlotters()) {
      if(setting->IsPersistent()) {
        data.m_blotters.push_back(BlotterModelProperties(*setting));
      }
    }
    data.m_activeBlotter = settings.GetActiveBlotter().GetName();
    data.m_defaultTaskProperties = settings.GetDefaultBlotterTaskProperties();
    sender.Shuttle(data);
    auto writer = BasicOStreamWriter<std::ofstream>(
      Initialize(blottersFilePath, std::ios::binary));
    writer.Write(buffer);
  } catch(const std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to save blotters."));
  }
}

BlotterSettings::BlotterSettings(Ref<UserProfile> userProfile)
  : m_userProfile(userProfile.Get()) {}

const std::vector<std::unique_ptr<BlotterModel>>&
    BlotterSettings::GetAllBlotters() const {
  return m_blotters;
}

const BlotterTaskProperties&
    BlotterSettings::GetDefaultBlotterTaskProperties() const {
  return m_defaultBlotterTaskProperties;
}

void BlotterSettings::SetDefaultBlotterTaskProperties(
    const BlotterTaskProperties& properties) {
  m_defaultBlotterTaskProperties = properties;
}

const OrderLogProperties&
    BlotterSettings::GetDefaultOrderLogProperties() const {
  return m_defaultOrderLogProperties;
}

void BlotterSettings::SetDefaultOrderLogProperties(
    const OrderLogProperties& properties) {
  m_defaultOrderLogProperties = properties;
}

void BlotterSettings::AddBlotter(std::unique_ptr<BlotterModel>&& blotter) {
  if(blotter->IsConsolidated()) {
    auto consolidatedBlotterIterator =
      m_consolidatedBlotters.find(blotter->GetExecutingAccount());
    if(consolidatedBlotterIterator != m_consolidatedBlotters.end()) {
      RemoveBlotter(*consolidatedBlotterIterator->second);
    }
  }
  auto selfBlotter = blotter.get();
  m_blotters.emplace_back(std::move(blotter));
  if(selfBlotter->IsConsolidated()) {
    m_consolidatedBlotters.insert(
      std::pair(selfBlotter->GetExecutingAccount(), selfBlotter));
  } else {
    auto& consolidatedBlotter =
      GetConsolidatedBlotter(selfBlotter->GetExecutingAccount());
    consolidatedBlotter.Link(Ref(*selfBlotter));
  }
  m_blotterAddedSignal(*m_blotters.back());
}

void BlotterSettings::RemoveBlotter(const BlotterModel& blotter) {
  for(auto i = m_blotters.begin(); i != m_blotters.end(); ++i) {
    if(i->get() == &blotter) {
      auto selfBlotter = std::move(*i);
      m_blotters.erase(i);
      if(selfBlotter->IsConsolidated()) {
        m_consolidatedBlotters.erase(selfBlotter->GetExecutingAccount());
      }
      m_blotterRemovedSignal(*selfBlotter);
      if(selfBlotter.get() == m_activeBlotter) {
        SetActiveBlotter(GetConsolidatedBlotter());
      }
      break;
    }
  }
}

const BlotterModel& BlotterSettings::GetConsolidatedBlotter() const {
  return GetConsolidatedBlotter(
    m_userProfile->GetServiceClients().GetServiceLocatorClient().GetAccount());
}

BlotterModel& BlotterSettings::GetConsolidatedBlotter() {
  return GetConsolidatedBlotter(
    m_userProfile->GetServiceClients().GetServiceLocatorClient().GetAccount());
}

const BlotterModel& BlotterSettings::GetConsolidatedBlotter(
    const DirectoryEntry& account) const {
  return const_cast<BlotterSettings&>(*this).GetConsolidatedBlotter(account);
}

BlotterModel&
    BlotterSettings::GetConsolidatedBlotter(const DirectoryEntry& account) {
  auto blotterIterator = m_consolidatedBlotters.find(account);
  if(blotterIterator == m_consolidatedBlotters.end()) {
    auto isUserConsolidatedBlotter = account ==
      m_userProfile->GetServiceClients().GetServiceLocatorClient().GetAccount();
    auto name = [&] {
      if(isUserConsolidatedBlotter) {
        return std::string("Global");
      }
      return "Account " + account.m_name;
    }();
    auto consolidatedBlotter = std::make_unique<BlotterModel>(name, account,
      true, Ref(*m_userProfile), BlotterTaskProperties::GetDefault(),
      OrderLogProperties::GetDefault());
    consolidatedBlotter->SetPersistent(isUserConsolidatedBlotter);
    AddBlotter(std::move(consolidatedBlotter));
    return GetConsolidatedBlotter(account);
  }
  return *blotterIterator->second;
}

const BlotterModel& BlotterSettings::GetActiveBlotter() const {
  return *m_activeBlotter;
}

BlotterModel& BlotterSettings::GetActiveBlotter() {
  return *m_activeBlotter;
}

void BlotterSettings::SetActiveBlotter(const BlotterModel& blotter) {
  for(auto& setting : m_blotters) {
    if(setting.get() == &blotter && m_activeBlotter != &blotter) {
      m_activeBlotter = setting.get();
      m_activeBlotterChangedSignal(*m_activeBlotter);
      break;
    }
  }
}

void BlotterSettings::AddRecentlyClosedWindow(const BlotterWindow& window) {
  auto& model = window.GetModel();
  if(m_recentlyClosedBlotters.find(&model) != m_recentlyClosedBlotters.end()) {
    return;
  }
  auto settings = window.GetWindowSettings();
  m_recentlyClosedBlotters[&model] = settings.get();
  m_userProfile->AddRecentlyClosedWindow(std::move(settings));
}

void BlotterSettings::RemoveRecentlyClosedWindow(const BlotterWindow& window) {
  auto& model = window.GetModel();
  auto settingsIterator = m_recentlyClosedBlotters.find(&model);
  if(settingsIterator == m_recentlyClosedBlotters.end()) {
    return;
  }
  auto settings = settingsIterator->second;
  m_recentlyClosedBlotters.erase(&model);
  m_userProfile->RemoveRecentlyClosedWindow(*settings);
}

connection BlotterSettings::ConnectBlotterAddedSignal(
    const BlotterAddedSignal::slot_type& slot) {
  return m_blotterAddedSignal.connect(slot);
}

connection BlotterSettings::ConnectBlotterRemovedSignal(
    const BlotterRemovedSignal::slot_type& slot) {
  return m_blotterRemovedSignal.connect(slot);
}

connection BlotterSettings::ConnectActiveBlotterChangedSignal(
    const ActiveBlotterChangedSignal::slot_type& slot) {
  return m_activeBlotterChangedSignal.connect(slot);
}
