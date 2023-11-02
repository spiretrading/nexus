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

namespace {
  void CreateValuesTab(CatalogSettings& settings) {
    auto tab = std::make_unique<CatalogTabModel>();
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
    auto tab = std::make_unique<CatalogTabModel>();
    tab->SetName("Tasks");
    tab->Add(
      Ref(*settings.FindEntry(BuiltInCatalogEntry::LIMIT_ORDER_TASK_UUID)));
    tab->Add(
      Ref(*settings.FindEntry(BuiltInCatalogEntry::MARKET_ORDER_TASK_UUID)));
    tab->Add(Ref(*settings.FindEntry(BuiltInCatalogEntry::NONE_UUID)));
    tab->Add(
      Ref(*settings.FindEntry(BuiltInCatalogEntry::SINGLE_ORDER_TASK_UUID)));
    settings.Add(std::move(tab));
  }

  void CreateKeyBindingsTab(CatalogSettings& settings) {
    auto tab = std::make_unique<CatalogTabModel>();
    tab->SetName("Key Bindings");
    settings.Add(std::move(tab));
  }

  void LoadCatalogEntries(const std::filesystem::path& catalogDirectoryPath,
      CatalogSettings& settings) {
    auto warnings = QString();
    for(auto i = std::filesystem::directory_iterator(catalogDirectoryPath);
        i != std::filesystem::directory_iterator(); ++i) {
      if(!std::filesystem::is_regular_file(*i) ||
          i->path().extension() != ".cat") {
        continue;
      }
      try {
        auto reader = BasicIStreamReader<std::ifstream>(
          Initialize(i->path(), std::ios::binary));
        auto buffer = SharedBuffer();
        reader.Read(Store(buffer));
        auto typeRegistry = TypeRegistry<BinarySender<SharedBuffer>>();
        RegisterSpireTypes(Store(typeRegistry));
        auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
        receiver.SetSource(Ref(buffer));
        auto entry =
          std::make_unique<UserCatalogEntry>(settings.GetSettingsPath());
        receiver.Shuttle(*entry);
        settings.Add(
          StaticCast<std::unique_ptr<CatalogEntry>>(std::move(entry)));
      } catch(const std::exception&) {
        warnings += QObject::tr("Failed to load: ") +
          QString::fromStdString(std::filesystem::path(*i).string()) + "\n";
      }
    }
    if(!warnings.isEmpty()) {
      QMessageBox::warning(nullptr, QObject::tr("Warning"), warnings);
    }
  }

  void LoadRemoteCatalogEntries(RegistryClientBox& registryClient,
      CatalogSettings& settings) {
    auto libraryDirectory = RegistryService::LoadOrCreateDirectory(
      registryClient, CatalogSettings::GetCatalogLibraryRegistryPath(),
      RegistryEntry::GetRoot());
    auto libraryEntries = registryClient.LoadChildren(libraryDirectory);
    for(auto& libraryEntry : libraryEntries) {
      if(libraryEntry.m_type != RegistryEntry::Type::VALUE) {
        continue;
      }
      try {
        auto buffer = registryClient.Load(libraryEntry);
        auto typeRegistry = TypeRegistry<BinarySender<SharedBuffer>>();
        RegisterSpireTypes(Store(typeRegistry));
        auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
        receiver.SetSource(Ref(buffer));
        auto entry =
          std::make_unique<RegistryCatalogEntry>(settings.HasRegistryAccess(),
            CatalogSettings::GetCatalogLibraryRegistryPath(), registryClient);
        receiver.Shuttle(*entry);
        settings.Add(
          StaticCast<std::unique_ptr<CatalogEntry>>(std::move(entry)));
      } catch(const std::exception&) {}
    }
  }

  void CreateDefaultCatalog(
      CatalogSettings& settings, RegistryClientBox& registryClient) {
    CreateValuesTab(settings);
    CreateTasksTab(settings);
    CreateKeyBindingsTab(settings);
    LoadRemoteCatalogEntries(registryClient, settings);
  }

  void LoadCatalogTabs(const std::filesystem::path& catalogDirectoryPath,
      CatalogSettings& settings, RegistryClientBox& registryClient) {
    auto catalogTabPath = catalogDirectoryPath / "tabs.list";
    if(!std::filesystem::exists(catalogTabPath)) {
      CreateDefaultCatalog(settings, registryClient);
      return;
    }
    auto tabs = std::vector<std::pair<std::string, std::vector<uuid>>>();
    auto warnings = QString();
    try {
      auto reader = BasicIStreamReader<std::ifstream>(
        Initialize(catalogTabPath, std::ios::binary));
      auto buffer = SharedBuffer();
      reader.Read(Store(buffer));
      auto typeRegistry = TypeRegistry<BinarySender<SharedBuffer>>();
      RegisterSpireTypes(Store(typeRegistry));
      auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
      receiver.SetSource(Ref(buffer));
      receiver.Shuttle(tabs);
    } catch(const std::exception&) {
      QMessageBox::warning(nullptr, QObject::tr("Warning"),
        QObject::tr("Unable to load Catalog Tabs."));
      return;
    }
    for(auto& t : tabs) {
      auto tab = std::make_unique<CatalogTabModel>();
      tab->SetName(t.first);
      for(auto& j : t.second) {
        if(auto entry = settings.FindEntry(j)) {
          tab->Add(Ref(*entry));
        }
      }
      settings.Add(std::move(tab));
    }
  }

  void SaveCatalogTabs(const std::filesystem::path& catalogDirectoryPath,
      const CatalogSettings& settings) {
    auto catalogTabPath = catalogDirectoryPath / "tabs.list";
    auto tabs = std::vector<std::pair<std::string, std::vector<uuid>>>();
    auto& catalogTabs = settings.GetCatalogTabs();
    for(auto& tab : catalogTabs) {
      if(tab.get() == &settings.GetAllTab()) {
        continue;
      }
      auto entries = std::vector<uuid>();
      auto& catalogEntries = tab->GetEntries();
      for(auto& entry : catalogEntries) {
        entries.push_back(entry->GetUid());
      }
      tabs.push_back(make_pair(tab->GetName(), entries));
    }
    try {
      auto typeRegistry = TypeRegistry<BinarySender<SharedBuffer>>();
      RegisterSpireTypes(Store(typeRegistry));
      auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
      auto buffer = SharedBuffer();
      sender.SetSink(Ref(buffer));
      sender.Shuttle(tabs);
      auto writer = BasicOStreamWriter<std::ofstream>(
        Initialize(catalogTabPath, std::ios::binary));
      writer.Write(buffer);
    } catch(const std::exception&) {
      QMessageBox::warning(nullptr, QObject::tr("Warning"),
        QObject::tr("Failed to save Catalog Tabs."));
    }
  }
}

void CatalogSettings::Load(Out<UserProfile> userProfile) {
  auto builtInEntries = BuiltInCatalogEntry::LoadBuiltInCatalogEntries();
  auto& settings = userProfile->GetCatalogSettings();
  for(auto& entry : builtInEntries) {
    settings.Add(std::move(entry));
  }
  auto catalogDirectoryPath = userProfile->GetProfilePath() / "catalog";
  if(!std::filesystem::exists(catalogDirectoryPath)) {
    std::filesystem::create_directory(catalogDirectoryPath);
    CreateDefaultCatalog(
      settings, userProfile->GetServiceClients().GetRegistryClient());
    return;
  }
  LoadCatalogEntries(catalogDirectoryPath, settings);
  LoadRemoteCatalogEntries(
    userProfile->GetServiceClients().GetRegistryClient(), settings);
  LoadCatalogTabs(catalogDirectoryPath, settings,
    userProfile->GetServiceClients().GetRegistryClient());
}

void CatalogSettings::Save(const UserProfile& userProfile) {
  auto catalogDirectoryPath = userProfile.GetProfilePath() / "catalog";
  SaveCatalogTabs(catalogDirectoryPath, userProfile.GetCatalogSettings());
}

const std::string& CatalogSettings::GetCatalogLibraryRegistryPath() {
  static auto path = std::string("spire/catalog_entries/library");
  return path;
}

CatalogSettings::CatalogSettings(const std::filesystem::path& settingsPath,
    bool hasRegistryAccess)
    : m_settingsPath(settingsPath),
      m_hasRegistryAccess(hasRegistryAccess) {
  auto tab = std::make_unique<CatalogTabModel>();
  tab->SetName("All");
  Add(std::move(tab));
}

const std::filesystem::path& CatalogSettings::GetSettingsPath() const {
  return m_settingsPath;
}

bool CatalogSettings::HasRegistryAccess() const {
  return m_hasRegistryAccess;
}

optional<CatalogEntry&> CatalogSettings::FindEntry(const uuid& uid) const {
  auto entryIterator = m_uuidToEntry.find(uid);
  if(entryIterator == m_uuidToEntry.end()) {
    return none;
  }
  return *entryIterator->second;
}

optional<CatalogEntry&> CatalogSettings::FindEntry(
    const CanvasNode& node) const {
  auto catalogUuid = CatalogEntry::FindUuid(node);
  if(!catalogUuid) {
    return none;
  }
  return FindEntry(*catalogUuid);
}

const std::vector<std::unique_ptr<CatalogEntry>>&
    CatalogSettings::GetCatalogEntries() const {
  return m_catalogEntries;
}

const CatalogTabModel& CatalogSettings::GetAllTab() const {
  return *m_catalogTabs.front();
}

CatalogTabModel& CatalogSettings::GetAllTab() {
  return *m_catalogTabs.front();
}

void CatalogSettings::Add(std::unique_ptr<CatalogEntry>&& entry) {
  if(dynamic_cast<PersistentCatalogEntry*>(entry.get())) {
    static_cast<PersistentCatalogEntry&>(*entry).Save();
  }
  m_catalogEntries.emplace_back(std::move(entry));
  GetAllTab().Add(Ref(*m_catalogEntries.back()));
  m_uuidToEntry[m_catalogEntries.back()->GetUid()] =
    m_catalogEntries.back().get();
  m_catalogEntryAddedSignal(*m_catalogEntries.back());
}

void CatalogSettings::Remove(const CatalogEntry& entry) {
  for(auto& tab : m_catalogTabs) {
    tab->Remove(entry);
  }
  auto entryIterator = find_if(m_catalogEntries.begin(), m_catalogEntries.end(),
    [&] (const auto& i) {
      return i.get() == &entry;
    });
  auto selfEntry = std::move(*entryIterator);
  m_catalogEntries.erase(entryIterator);
  m_uuidToEntry.erase(selfEntry->GetUid());
  if(dynamic_cast<const PersistentCatalogEntry*>(selfEntry.get())) {
    static_cast<const PersistentCatalogEntry&>(*selfEntry).Delete();
  }
  m_catalogEntryRemovedSignal(*selfEntry);
}

void CatalogSettings::Replace(const CatalogEntry& oldEntry,
    std::unique_ptr<CatalogEntry>&& newEntry) {
  if(oldEntry.GetName() != newEntry->GetName()) {
    for(auto& entry : m_catalogEntries) {
      if(entry->GetName() == newEntry->GetName()) {
        BOOST_THROW_EXCEPTION(std::runtime_error("Entry already exists."));
      }
    }
  }
  if(dynamic_cast<const PersistentCatalogEntry*>(newEntry.get())) {
    static_cast<const PersistentCatalogEntry&>(*newEntry).Save();
  }
  Remove(oldEntry);
  Add(std::move(newEntry));
}

const std::vector<std::unique_ptr<CatalogTabModel>>&
    CatalogSettings::GetCatalogTabs() const {
  return m_catalogTabs;
}

void CatalogSettings::Add(std::unique_ptr<CatalogTabModel>&& tab) {
  m_catalogTabs.emplace_back(std::move(tab));
  m_catalogTabModelAddedSignal(*m_catalogTabs.back());
}

void CatalogSettings::Remove(const CatalogTabModel& tab) {
  auto modelIterator = find_if(m_catalogTabs.begin(), m_catalogTabs.end(),
    [&] (const auto& i) {
      return i.get() == &tab;
    });
  auto selfModel = std::move(*modelIterator);
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
