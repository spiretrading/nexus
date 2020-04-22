#include "Spire/Catalog/CatalogSettings.hpp"
#include <filesystem>
#include <fstream>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Serialization/ShuttleUuid.hpp>
#include <QMessageBox>
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "Spire/Catalog/BuiltInCatalogEntry.hpp"
#include "Spire/Catalog/CatalogEntry.hpp"
#include "Spire/Catalog/CatalogTabModel.hpp"
#include "Spire/Catalog/RegistryCatalogEntry.hpp"
#include "Spire/Catalog/UserCatalogEntry.hpp"
#include "Spire/UI/UISerialization.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::RegistryService;
using namespace Beam::Serialization;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::signals2;
using namespace boost::uuids;
using namespace Spire;
using namespace Spire::UI;
using namespace std;
using namespace std::filesystem;

namespace {
  void CreateValuesTab(CatalogSettings& settings) {
    unique_ptr<CatalogTabModel> tab = std::make_unique<CatalogTabModel>();
    tab->SetName("Values");
    tab->Add(Ref(*settings.FindEntry(BuiltInCatalogEntry::BOOLEAN_UUID)));
    tab->Add(Ref(*settings.FindEntry(BuiltInCatalogEntry::DATE_TIME_UUID)));
    tab->Add(Ref(*settings.FindEntry(BuiltInCatalogEntry::DECIMAL_UUID)));
    tab->Add(Ref(*settings.FindEntry(BuiltInCatalogEntry::DURATION_UUID)));
    tab->Add(Ref(*settings.FindEntry(BuiltInCatalogEntry::INTEGER_UUID)));
    tab->Add(Ref(*settings.FindEntry(BuiltInCatalogEntry::MONEY_UUID)));
    tab->Add(Ref(*settings.FindEntry(BuiltInCatalogEntry::SECURITY_UUID)));
    tab->Add(Ref(*settings.FindEntry(BuiltInCatalogEntry::SIDE_UUID)));
    tab->Add(Ref(*settings.FindEntry(BuiltInCatalogEntry::TEXT_UUID)));
    tab->Add(Ref(*settings.FindEntry(BuiltInCatalogEntry::TIME_IN_FORCE_UUID)));
    settings.Add(std::move(tab));
  }

  void CreateTasksTab(CatalogSettings& settings) {
    unique_ptr<CatalogTabModel> tab = std::make_unique<CatalogTabModel>();
    tab->SetName("Tasks");
    tab->Add(Ref(*settings.FindEntry(
      BuiltInCatalogEntry::LIMIT_ORDER_TASK_UUID)));
    tab->Add(Ref(*settings.FindEntry(
      BuiltInCatalogEntry::MARKET_ORDER_TASK_UUID)));
    tab->Add(Ref(*settings.FindEntry(BuiltInCatalogEntry::NONE_UUID)));
    tab->Add(Ref(*settings.FindEntry(
      BuiltInCatalogEntry::SINGLE_ORDER_TASK_UUID)));
    settings.Add(std::move(tab));
  }

  void CreateKeyBindingsTab(CatalogSettings& settings) {
    unique_ptr<CatalogTabModel> tab = std::make_unique<CatalogTabModel>();
    tab->SetName("Key Bindings");
    settings.Add(std::move(tab));
  }

  void LoadCatalogEntries(const path& catalogDirectoryPath,
      CatalogSettings& settings) {
    QString warnings;
    for(directory_iterator i(catalogDirectoryPath);
        i != directory_iterator(); ++i) {
      if(!is_regular_file(*i) || i->path().extension() != ".cat") {
        continue;
      }
      try {
        BasicIStreamReader<ifstream> reader(Initialize(*i, ios::binary));
        SharedBuffer buffer;
        reader.Read(Store(buffer));
        TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
        RegisterSpireTypes(Store(typeRegistry));
        auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
        receiver.SetSource(Ref(buffer));
        unique_ptr<UserCatalogEntry> entry = std::make_unique<UserCatalogEntry>(
          settings.GetSettingsPath());
        receiver.Shuttle(*entry);
        settings.Add(StaticCast<std::unique_ptr<CatalogEntry>>(
          std::move(entry)));
      } catch(std::exception&) {
        warnings += QObject::tr("Failed to load: ") +
          QString::fromStdString(path(*i).string()) + "\n";
      }
    }
    if(!warnings.isEmpty()) {
      QMessageBox::warning(nullptr, QObject::tr("Warning"), warnings);
    }
  }

  void LoadRemoteCatalogEntries(VirtualRegistryClient& registryClient,
      CatalogSettings& settings) {
    RegistryEntry libraryDirectory = RegistryService::LoadOrCreateDirectory(
      registryClient, CatalogSettings::GetCatalogLibraryRegistryPath(),
      RegistryEntry::GetRoot());
    vector<RegistryEntry> libraryEntries = registryClient.LoadChildren(
      libraryDirectory);
    for(auto i = libraryEntries.begin(); i != libraryEntries.end(); ++i) {
      if(i->m_type != RegistryEntry::Type::VALUE) {
        continue;
      }
      try {
        SharedBuffer buffer = registryClient.Load(*i);
        TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
        RegisterSpireTypes(Store(typeRegistry));
        auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
        receiver.SetSource(Ref(buffer));
        std::unique_ptr<RegistryCatalogEntry> entry =
          std::make_unique<RegistryCatalogEntry>(settings.HasRegistryAccess(),
          CatalogSettings::GetCatalogLibraryRegistryPath(),
          Ref(registryClient));
        receiver.Shuttle(*entry);
        settings.Add(StaticCast<std::unique_ptr<CatalogEntry>>(
          std::move(entry)));
      } catch(std::exception&) {}
    }
  }

  void CreateDefaultCatalog(CatalogSettings& settings,
      VirtualRegistryClient& registryClient) {
    CreateValuesTab(settings);
    CreateTasksTab(settings);
    CreateKeyBindingsTab(settings);
    LoadRemoteCatalogEntries(registryClient, settings);
  }

  void LoadCatalogTabs(const path& catalogDirectoryPath,
      CatalogSettings& settings, VirtualRegistryClient& registryClient) {
    path catalogTabPath = catalogDirectoryPath / "tabs.list";
    if(!exists(catalogTabPath)) {
      CreateDefaultCatalog(settings, registryClient);
      return;
    }
    vector<pair<string, vector<uuid>>> tabs;
    QString warnings;
    try {
      BasicIStreamReader<ifstream> reader(
        Initialize(catalogTabPath, ios::binary));
      SharedBuffer buffer;
      reader.Read(Store(buffer));
      TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
      RegisterSpireTypes(Store(typeRegistry));
      auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
      receiver.SetSource(Ref(buffer));
      receiver.Shuttle(tabs);
    } catch(std::exception&) {
      QMessageBox::warning(nullptr, QObject::tr("Warning"),
        QObject::tr("Unable to load Catalog Tabs."));
      return;
    }
    for(auto i = tabs.begin(); i != tabs.end(); ++i) {
      unique_ptr<CatalogTabModel> tab = std::make_unique<CatalogTabModel>();
      tab->SetName(i->first);
      for(auto j = i->second.begin(); j != i->second.end(); ++j) {
        auto entry = settings.FindEntry(*j);
        if(entry) {
          tab->Add(Ref(*entry));
        }
      }
      settings.Add(std::move(tab));
    }
  }

  void SaveCatalogTabs(const path& catalogDirectoryPath,
      const CatalogSettings& settings) {
    path catalogTabPath = catalogDirectoryPath / "tabs.list";
    vector<pair<string, vector<uuid>>> tabs;
    const vector<unique_ptr<CatalogTabModel>>& catalogTabs =
      settings.GetCatalogTabs();
    for(auto i = catalogTabs.begin(); i != catalogTabs.end(); ++i) {
      if(i->get() == &settings.GetAllTab()) {
        continue;
      }
      vector<uuid> entries;
      const vector<CatalogEntry*>& catalogEntries = (*i)->GetEntries();
      for(auto j = catalogEntries.begin(); j != catalogEntries.end(); ++j) {
        entries.push_back((*j)->GetUid());
      }
      tabs.push_back(make_pair((*i)->GetName(), entries));
    }
    try {
      TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
      RegisterSpireTypes(Store(typeRegistry));
      auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
      SharedBuffer buffer;
      sender.SetSink(Ref(buffer));
      sender.Shuttle(tabs);
      BasicOStreamWriter<ofstream> writer(
        Initialize(catalogTabPath, ios::binary));
      writer.Write(buffer);
    } catch(std::exception&) {
      QMessageBox::warning(nullptr, QObject::tr("Warning"),
        QObject::tr("Failed to save Catalog Tabs."));
    }
  }
}

void CatalogSettings::Load(Out<UserProfile> userProfile) {
  vector<unique_ptr<CatalogEntry>> builtInEntries =
    BuiltInCatalogEntry::LoadBuiltInCatalogEntries();
  CatalogSettings& settings = userProfile->GetCatalogSettings();
  for(auto i = builtInEntries.begin(); i != builtInEntries.end(); ++i) {
    settings.Add(std::move(*i));
  }
  path catalogDirectoryPath = userProfile->GetProfilePath() / "catalog";
  if(!exists(catalogDirectoryPath)) {
    create_directory(catalogDirectoryPath);
    CreateDefaultCatalog(settings,
      userProfile->GetServiceClients().GetRegistryClient());
    return;
  }
  LoadCatalogEntries(catalogDirectoryPath, settings);
  LoadRemoteCatalogEntries(userProfile->GetServiceClients().GetRegistryClient(),
    settings);
  LoadCatalogTabs(catalogDirectoryPath, settings,
    userProfile->GetServiceClients().GetRegistryClient());
}

void CatalogSettings::Save(const UserProfile& userProfile) {
  path catalogDirectoryPath = userProfile.GetProfilePath() / "catalog";
  SaveCatalogTabs(catalogDirectoryPath, userProfile.GetCatalogSettings());
}

const string& CatalogSettings::GetCatalogLibraryRegistryPath() {
  static string path = "spire/catalog_entries/library";
  return path;
}

CatalogSettings::CatalogSettings(const path& settingsPath,
    bool hasRegistryAccess)
    : m_settingsPath(settingsPath),
      m_hasRegistryAccess(hasRegistryAccess) {
  unique_ptr<CatalogTabModel> tab = std::make_unique<CatalogTabModel>();
  tab->SetName("All");
  Add(std::move(tab));
}

CatalogSettings::~CatalogSettings() {}

const path& CatalogSettings::GetSettingsPath() const {
  return m_settingsPath;
}

bool CatalogSettings::HasRegistryAccess() const {
  return m_hasRegistryAccess;
}

boost::optional<CatalogEntry&> CatalogSettings::FindEntry(
    const uuid& uid) const {
  auto entryIterator = m_uuidToEntry.find(uid);
  if(entryIterator == m_uuidToEntry.end()) {
    return none;
  }
  return *entryIterator->second;
}

boost::optional<CatalogEntry&> CatalogSettings::FindEntry(
    const CanvasNode& node) const {
  auto catalogUuid = CatalogEntry::FindUuid(node);
  if(!catalogUuid.is_initialized()) {
    return none;
  }
  return FindEntry(*catalogUuid);
}

const vector<unique_ptr<CatalogEntry>>& CatalogSettings::GetCatalogEntries()
    const {
  return m_catalogEntries;
}

const CatalogTabModel& CatalogSettings::GetAllTab() const {
  return *m_catalogTabs.front();
}

CatalogTabModel& CatalogSettings::GetAllTab() {
  return *m_catalogTabs.front();
}

void CatalogSettings::Add(unique_ptr<CatalogEntry>&& entry) {
  if(dynamic_cast<PersistentCatalogEntry*>(entry.get()) != nullptr) {
    static_cast<PersistentCatalogEntry&>(*entry).Save();
  }
  m_catalogEntries.emplace_back(std::move(entry));
  GetAllTab().Add(Ref(*m_catalogEntries.back()));
  m_uuidToEntry[m_catalogEntries.back()->GetUid()] =
    m_catalogEntries.back().get();
  m_catalogEntryAddedSignal(*m_catalogEntries.back());
}

void CatalogSettings::Remove(const CatalogEntry& entry) {
  for(auto i = m_catalogTabs.begin(); i != m_catalogTabs.end(); ++i) {
    (*i)->Remove(entry);
  }
  unique_ptr<CatalogEntry> selfEntry;
  auto entryIterator = find_if(m_catalogEntries.begin(), m_catalogEntries.end(),
    [&] (const unique_ptr<CatalogEntry>& i) {
      return i.get() == &entry;
    });
  selfEntry = std::move(*entryIterator);
  m_catalogEntries.erase(entryIterator);
  m_uuidToEntry.erase(selfEntry->GetUid());
  if(dynamic_cast<const PersistentCatalogEntry*>(selfEntry.get()) != nullptr) {
    static_cast<const PersistentCatalogEntry&>(*selfEntry).Delete();
  }
  m_catalogEntryRemovedSignal(*selfEntry);
}

void CatalogSettings::Replace(const CatalogEntry& oldEntry,
    unique_ptr<CatalogEntry>&& newEntry) {
  if(oldEntry.GetName() != newEntry->GetName()) {
    for(auto i = m_catalogEntries.begin(); i != m_catalogEntries.end(); ++i) {
      if((*i)->GetName() == newEntry->GetName()) {
        BOOST_THROW_EXCEPTION(runtime_error("Entry already exists."));
      }
    }
  }
  if(dynamic_cast<const PersistentCatalogEntry*>(newEntry.get()) != nullptr) {
    static_cast<const PersistentCatalogEntry&>(*newEntry).Save();
  }
  Remove(oldEntry);
  Add(std::move(newEntry));
}

const vector<unique_ptr<CatalogTabModel>>& CatalogSettings::GetCatalogTabs()
    const {
  return m_catalogTabs;
}

void CatalogSettings::Add(unique_ptr<CatalogTabModel>&& tab) {
  m_catalogTabs.emplace_back(std::move(tab));
  m_catalogTabModelAddedSignal(*m_catalogTabs.back());
}

void CatalogSettings::Remove(const CatalogTabModel& tab) {
  unique_ptr<CatalogTabModel> selfModel;
  auto modelIterator = find_if(m_catalogTabs.begin(), m_catalogTabs.end(),
    [&] (const unique_ptr<CatalogTabModel>& i) {
      return i.get() == &tab;
    });
  selfModel = std::move(*modelIterator);
  m_catalogTabs.erase(modelIterator);
  m_catalogTabModelRemovedSignal(*selfModel);
}

connection CatalogSettings::ConnectCatalogEntryAddedSignal(
    const CatalogEntryAddedSignal::slot_type& slot) const {
  return m_catalogEntryAddedSignal.connect(slot);
}

connection CatalogSettings::ConnectCatalogEntryRemovedSignal(
    const CatalogEntryRemovedSignal::slot_type& slot) const {
  return m_catalogEntryRemovedSignal.connect(slot);
}

connection CatalogSettings::ConnectCatalogTabModelAddedSignal(
    const CatalogTabModelAddedSignal::slot_type& slot) const {
  return m_catalogTabModelAddedSignal.connect(slot);
}

connection CatalogSettings::ConnectCatalogTabModelRemovedSignal(
    const CatalogTabModelRemovedSignal::slot_type& slot) const {
  return m_catalogTabModelRemovedSignal.connect(slot);
}
