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
using namespace Beam::Threading;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::UI;
using namespace std;
using namespace std::filesystem;

namespace {
  void LoadDefaultBlotter(Out<UserProfile> userProfile) {
    BlotterSettings& settings = userProfile->GetBlotterSettings();
    settings.SetActiveBlotter(settings.GetConsolidatedBlotter());
    settings.SetDefaultBlotterTaskProperties(
      BlotterTaskProperties::GetDefault());
    settings.SetDefaultOrderLogProperties(OrderLogProperties::GetDefault());
  }

  struct BlotterModelProperties {
    string m_name;
    DirectoryEntry m_account;
    bool m_isConsolidated;
    vector<string> m_links;
    BlotterTaskProperties m_taskProperties;
    OrderLogProperties m_orderLogProperties;

    BlotterModelProperties();
    BlotterModelProperties(const BlotterModel& model);

    template<typename Shuttler>
    void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  struct BlotterSettingsData {
    vector<BlotterModelProperties> m_blotters;
    string m_activeBlotter;
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

  BlotterModelProperties::BlotterModelProperties() {}

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
  void BlotterModelProperties::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("account", m_account);
    shuttle.Shuttle("is_consolidated", m_isConsolidated);
    shuttle.Shuttle("links", m_links);
    shuttle.Shuttle("task_properties", m_taskProperties);
    shuttle.Shuttle("order_log_properties", m_orderLogProperties);
  }
}

void BlotterSettings::Load(Out<UserProfile> userProfile) {
  path blottersFilePath = userProfile->GetProfilePath() / "blotters.dat";
  if(!exists(blottersFilePath)) {
    LoadDefaultBlotter(Store(userProfile));
    return;
  }
  BlotterSettingsData data;
  try {
    BasicIStreamReader<ifstream> reader(
      Initialize(blottersFilePath, ios::binary));
    SharedBuffer buffer;
    reader.Read(Store(buffer));
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(Store(typeRegistry));
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(data);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to load blotters, using defaults."));
    LoadDefaultBlotter(Store(userProfile));
    return;
  }
  BlotterSettings& settings = userProfile->GetBlotterSettings();
  std::unordered_map<string, BlotterModel*> nameToBlotter;
  for(auto i = data.m_blotters.begin(); i != data.m_blotters.end(); ++i) {
    boost::optional<DirectoryEntry> account =
      userProfile->GetServiceClients().GetServiceLocatorClient().FindAccount(
      i->m_account.m_name);
    if(!account.is_initialized()) {

      // TODO: Account doesn't exist.
      account = i->m_account;
    }
    unique_ptr<BlotterModel> blotter = std::make_unique<BlotterModel>(i->m_name,
      *account, i->m_isConsolidated, Ref(*userProfile), i->m_taskProperties,
      i->m_orderLogProperties);
    blotter->SetPersistent(true);
    nameToBlotter.insert(std::make_pair(i->m_name, blotter.get()));
    settings.AddBlotter(std::move(blotter));
  }
  for(auto i = data.m_blotters.begin(); i != data.m_blotters.end(); ++i) {
    BlotterModel& model = *nameToBlotter[i->m_name];
    for(auto j = i->m_links.begin(); j != i->m_links.end(); ++j) {
      auto blotterIterator = nameToBlotter.find(*j);
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
  path blottersFilePath = userProfile.GetProfilePath() / "blotters.dat";
  try {
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(Store(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    SharedBuffer buffer;
    sender.SetSink(Ref(buffer));
    const BlotterSettings& settings = userProfile.GetBlotterSettings();
    BlotterSettingsData data;
    for(auto i = settings.GetAllBlotters().begin();
        i != settings.GetAllBlotters().end(); ++i) {
      if((*i)->IsPersistent()) {
        data.m_blotters.push_back(BlotterModelProperties(**i));
      }
    }
    data.m_activeBlotter = settings.GetActiveBlotter().GetName();
    data.m_defaultTaskProperties = settings.GetDefaultBlotterTaskProperties();
    sender.Shuttle(data);
    BasicOStreamWriter<ofstream> writer(
      Initialize(blottersFilePath, ios::binary));
    writer.Write(buffer);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to save blotters."));
  }
}

BlotterSettings::BlotterSettings(Ref<UserProfile> userProfile)
    : m_userProfile(userProfile.Get()) {}

BlotterSettings::~BlotterSettings() {}

const vector<unique_ptr<BlotterModel>>&
    BlotterSettings::GetAllBlotters() const {
  return m_blotters;
}

const BlotterTaskProperties& BlotterSettings::
    GetDefaultBlotterTaskProperties() const {
  return m_defaultBlotterTaskProperties;
}

void BlotterSettings::SetDefaultBlotterTaskProperties(
    const BlotterTaskProperties& properties) {
  m_defaultBlotterTaskProperties = properties;
}

const OrderLogProperties& BlotterSettings::
    GetDefaultOrderLogProperties() const {
  return m_defaultOrderLogProperties;
}

void BlotterSettings::SetDefaultOrderLogProperties(
    const OrderLogProperties& properties) {
  m_defaultOrderLogProperties = properties;
}

void BlotterSettings::AddBlotter(unique_ptr<BlotterModel>&& blotter) {
  if(blotter->IsConsolidated()) {
    auto consolidatedBlotterIterator = m_consolidatedBlotters.find(
      blotter->GetExecutingAccount());
    if(consolidatedBlotterIterator != m_consolidatedBlotters.end()) {
      RemoveBlotter(*consolidatedBlotterIterator->second);
    }
  }
  BlotterModel* selfBlotter = blotter.get();
  m_blotters.emplace_back(std::move(blotter));
  if(selfBlotter->IsConsolidated()) {
    m_consolidatedBlotters.insert(
      std::make_pair(selfBlotter->GetExecutingAccount(), selfBlotter));
  } else {
    BlotterModel& consolidatedBlotter = GetConsolidatedBlotter(
      selfBlotter->GetExecutingAccount());
    consolidatedBlotter.Link(Ref(*selfBlotter));
  }
  m_blotterAddedSignal(*m_blotters.back());
}

void BlotterSettings::RemoveBlotter(const BlotterModel& blotter) {
  for(auto i = m_blotters.begin(); i != m_blotters.end(); ++i) {
    if(i->get() == &blotter) {
      unique_ptr<BlotterModel> selfBlotter = std::move(*i);
      m_blotters.erase(i);
      if(selfBlotter->IsConsolidated()) {
        m_consolidatedBlotters.erase(selfBlotter->GetExecutingAccount());
      }
      m_blotterRemovedSignal(*selfBlotter);
      if(selfBlotter.get() == m_activeBlotter) {
        SetActiveBlotter(GetConsolidatedBlotter());
      }
      return;
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

BlotterModel& BlotterSettings::GetConsolidatedBlotter(
    const DirectoryEntry& account) {
  auto blotterIterator = m_consolidatedBlotters.find(account);
  if(blotterIterator == m_consolidatedBlotters.end()) {
    bool isUserConsolidatedBlotter =
      account == m_userProfile->GetServiceClients().GetServiceLocatorClient().
      GetAccount();
    string name;
    if(isUserConsolidatedBlotter) {
      name = "Global";
    } else {
      name = "Account " + account.m_name;
    }
    unique_ptr<BlotterModel> consolidatedBlotter =
      std::make_unique<BlotterModel>(name, account, true, Ref(*m_userProfile),
      BlotterTaskProperties::GetDefault(), OrderLogProperties::GetDefault());
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
  for(auto i = m_blotters.begin(); i != m_blotters.end(); ++i) {
    if(i->get() == &blotter && m_activeBlotter != &blotter) {
      m_activeBlotter = i->get();
      m_activeBlotterChangedSignal(*m_activeBlotter);
      return;
    }
  }
}

void BlotterSettings::AddRecentlyClosedWindow(const BlotterWindow& window) {
  const BlotterModel& model = window.GetModel();
  if(m_recentlyClosedBlotters.find(&model) != m_recentlyClosedBlotters.end()) {
    return;
  }
  unique_ptr<WindowSettings> settings = window.GetWindowSettings();
  m_recentlyClosedBlotters[&model] = settings.get();
  m_userProfile->AddRecentlyClosedWindow(std::move(settings));
}

void BlotterSettings::RemoveRecentlyClosedWindow(const BlotterWindow& window) {
  const BlotterModel& model = window.GetModel();
  auto settingsIterator = m_recentlyClosedBlotters.find(&model);
  if(settingsIterator == m_recentlyClosedBlotters.end()) {
    return;
  }
  WindowSettings* settings = settingsIterator->second;
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
