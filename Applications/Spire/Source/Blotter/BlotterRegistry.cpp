#include "Spire/Blotter/BlotterRegistry.hpp"
#include <algorithm>
#include <fstream>
#include <unordered_set>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Serialization/JsonReceiver.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <QMessageBox>
#include <QSaveFile>
#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Blotter/ControllerPortfolioModel.hpp"
#include "Spire/Blotter/LegacyBlotterSettings.hpp"
#include "Spire/Blotter/MonitoredTasksModel.hpp"
#include "Spire/Blotter/TaskOrdersModel.hpp"
#include "Spire/LegacyUI/UISerialization.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/ConstantValueModel.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace boost::uuids;
using namespace Nexus;
using namespace Spire;

namespace {
  auto load_legacy_settings(const std::filesystem::path& path) {
    auto reader =
      BasicIStreamReader<std::ifstream>(init(path, std::ios::binary));
    auto buffer = SharedBuffer();
    reader.read(out(buffer));
    auto load = [&] (auto& registry) {
      auto receiver = BinaryReceiver<SharedBuffer>(Ref(registry));
      receiver.set(Ref(buffer));
      auto settings = LegacyBlotterSettings();
      receiver.shuttle(settings);
      return BlotterRegistry::State(settings);
    };
    auto registry = TypeRegistry<BinarySender<SharedBuffer>>();
    RegisterSpireTypes(out(registry));
    try {
      return load(registry);
    } catch(const std::exception&) {}
    auto legacy_registry = TypeRegistry<BinarySender<SharedBuffer>>();
    RegisterLegacySpireTypes(out(legacy_registry));
    return load(legacy_registry);
  }

  class AccountOrdersModel : public ArrayListModel<std::shared_ptr<Order>> {
    public:
      AccountOrdersModel(const DirectoryEntry& account, Clients& clients) {
        auto now = clients.get_time_client().get_time();
        auto query = AccountQuery();
        query.set_index(account);
        query.set_range(now, pos_infin);
        query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
        query.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
        clients.get_order_execution_client().query(query,
          m_events.get_slot<std::shared_ptr<Order>>(
            std::bind_front(&AccountOrdersModel::on_order, this)));
        m_history = query_daily_order_submissions(account, now, now,
          clients.get_order_execution_client(),
          m_events.get_slot<std::shared_ptr<Order>>(
            std::bind_front(&AccountOrdersModel::on_order, this)));
      }

    private:
      std::unordered_set<OrderId> m_ids;
      QtTaskQueue m_events;
      RoutineHandler m_history;

      void on_order(const std::shared_ptr<Order>& order) {
        if(m_ids.insert(order->get_info().m_id).second) {
          push(order);
        }
      }
  };

  auto make_account_portfolio(const DirectoryEntry& account, Clients& clients) {
    auto [portfolio, sequence, excluded] = make_portfolio(
      clients.get_risk_client().load_inventory_snapshot(account), account,
      clients.get_order_execution_client());
    auto orders = std::make_shared<Queue<std::shared_ptr<Order>>>();
    for(auto& order : excluded) {
      orders->push(order);
    }
    auto query = AccountQuery();
    query.set_index(account);
    query.set_range(increment(sequence), Sequence::LAST);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    query.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
    clients.get_order_execution_client().query(query, orders);
    return std::make_shared<ControllerPortfolioModel>(
      std::make_shared<ControllerPortfolioModel::Controller>(
        std::move(portfolio), clients.get_market_data_client(), orders));
  }

  void update_membership(auto& list, const auto& model, bool is_included) {
    auto i = std::ranges::find(std::as_const(list), model);
    if(is_included && i == list.cend()) {
      list.push(model);
    } else if(!is_included && i != list.cend()) {
      list.remove(i);
    }
  }
}

BlotterRegistry::State Spire::load_blotter_settings(
    const std::filesystem::path& path) {
  try {
    auto file = path / "blotters.json";
    if(!std::filesystem::exists(file)) {
      auto legacy = path / "blotters.dat";
      if(std::filesystem::exists(legacy)) {
        return load_legacy_settings(legacy);
      }
      return BlotterRegistry::State();
    }
    auto reader = BasicIStreamReader<std::ifstream>(init(file));
    auto buffer = SharedBuffer();
    reader.read(out(buffer));
    auto registry = TypeRegistry<JsonSender<SharedBuffer>>();
    RegisterSpireTypes(out(registry));
    auto receiver = JsonReceiver<SharedBuffer>(Ref(registry));
    receiver.set(Ref(buffer));
    auto state = BlotterRegistry::State();
    receiver.shuttle(state);
    return state;
  } catch(const std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to load blotters, using defaults."));
    return BlotterRegistry::State();
  }
}

void Spire::save_blotter_settings(
    const BlotterRegistry::State& state, const std::filesystem::path& path) {
  auto registry = TypeRegistry<JsonSender<SharedBuffer>>();
  RegisterSpireTypes(out(registry));
  auto sender = JsonSender<SharedBuffer>(Ref(registry));
  auto buffer = SharedBuffer();
  sender.set(Ref(buffer));
  sender.shuttle(state);
  auto file = QSaveFile(QString::fromStdWString(
    (path / "blotters.json").wstring()));
  if(!file.open(QIODevice::WriteOnly) ||
      file.write(buffer.get_data(), buffer.get_size()) !=
        static_cast<qint64>(buffer.get_size()) || !file.commit()) {
    throw std::runtime_error("Unable to save blotters.");
  }
}

struct BlotterRegistry::Entry {
  std::shared_ptr<BlotterModel> m_source;
  std::unique_ptr<BlotterController> m_controller;
  boost::signals2::scoped_connection m_open_connection;
  boost::signals2::scoped_connection m_pinned_connection;
  boost::signals2::scoped_connection m_active_connection;
};

BlotterRegistry::State::State(const LegacyBlotterSettings& settings) {
  auto names = std::unordered_map<std::string, uuid>();
  for(auto& legacy : settings.m_blotters) {
    if(names.contains(legacy.m_name)) {
      continue;
    }
    auto blotter = BlotterSettings();
    blotter.m_id = random_generator()();
    blotter.m_name = QString::fromStdString(legacy.m_name);
    blotter.m_account = legacy.m_account;
    blotter.m_is_consolidated = legacy.m_is_consolidated;
    for(auto& monitor : legacy.m_task_properties.m_task_monitors) {
      auto converted = BlotterSettings::Monitor();
      converted.m_name = QString::fromStdString(monitor.m_name);
      if(monitor.m_monitor) {
        converted.m_expression = CanvasNode::Clone(*monitor.m_monitor);
      }
      blotter.m_monitors.push_back(std::move(converted));
    }
    names.emplace(legacy.m_name, blotter.m_id);
    m_blotters.push_back(std::move(blotter));
  }
  for(auto& blotter : m_blotters) {
    auto legacy = std::ranges::find(settings.m_blotters,
      blotter.m_name.toStdString(), &LegacyBlotterSettings::Properties::m_name);
    for(auto& name : legacy->m_links) {
      auto source = names.find(name);
      if(source != names.end()) {
        blotter.m_links.push_back(source->second);
      }
    }
  }
  auto active = names.find(settings.m_active_blotter);
  if(active != names.end()) {
    m_active = active->second;
  }
}

BlotterRegistry::BlotterRegistry(Ref<UserProfile> profile)
    : m_profile(profile.get()),
      m_collection(std::make_shared<BlotterCollection>(
        profile->GetClients().get_market_data_client())),
      m_blotters(m_collection),
      m_available(std::make_shared<ArrayListModel<BlottersModel::Type>>()),
      m_read_only_available(m_available),
      m_pinned(std::make_shared<ArrayListModel<BlottersModel::Type>>()),
      m_read_only_pinned(m_pinned),
      m_active(std::make_shared<LocalValueModel<ActiveModel::Type>>()),
      m_read_only_active(m_active),
      m_position_bindings(std::make_shared<PositionKeyBindingsModel>()),
      m_is_updating_active(false) {
  set_active(*get_consolidated().get_model());
}

BlotterRegistry::~BlotterRegistry() {
  m_entries.clear();
}

const std::shared_ptr<const BlottersModel>&
    BlotterRegistry::get_blotters() const {
  return m_blotters;
}

const std::shared_ptr<const BlottersModel>&
    BlotterRegistry::get_available() const {
  return m_read_only_available;
}

const std::shared_ptr<const BlottersModel>&
    BlotterRegistry::get_pinned() const {
  return m_read_only_pinned;
}

const std::shared_ptr<const BlotterRegistry::ActiveModel>&
    BlotterRegistry::get_active() const {
  return m_read_only_active;
}

BlotterController& BlotterRegistry::get(const BlotterModel& blotter) const {
  return *m_entries.at(&blotter)->m_controller;
}

BlotterController& BlotterRegistry::get_consolidated() {
  return get_consolidated(
    m_profile->GetClients().get_service_locator_client().get_account());
}

BlotterController& BlotterRegistry::get_consolidated(
    const DirectoryEntry& account) {
  for(auto& model : std::as_const(*m_collection)) {
    if(model->is_consolidated() && model->get_account() == account) {
      return get(*model);
    }
  }
  auto name = [&] {
    if(account ==
        m_profile->GetClients().get_service_locator_client().get_account()) {
      return QObject::tr("Global");
    }
    return QObject::tr("Account %1").arg(
      QString::fromStdString(account.m_name));
  }();
  return add(make_name(std::move(name)), account, true);
}

BlotterController& BlotterRegistry::create(
    QString name, const DirectoryEntry& account) {
  get_consolidated(account);
  return add(make_name(std::move(name)), account, false);
}

void BlotterRegistry::set_active(const BlotterModel& blotter) {
  auto model = get(blotter).get_model();
  if(m_active->get() == model) {
    return;
  }
  m_is_updating_active = true;
  if(auto previous = m_active->get()) {
    previous->is_active()->set(false);
  }
  model->is_active()->set(true);
  m_active->set(model);
  m_is_updating_active = false;
}

BlotterRegistry::State BlotterRegistry::save_state() const {
  auto saved = std::unordered_set<const BlotterModel*>();
  for(auto& model : std::as_const(*m_blotters)) {
    if(model->is_pinned()->get()) {
      saved.insert(model.get());
      for(auto& link : std::as_const(*m_collection->get_links(*model))) {
        if(link->is_consolidated()) {
          saved.insert(link.get());
        }
      }
    }
  }
  auto state = State();
  for(auto& model : std::as_const(*m_blotters)) {
    if(!saved.contains(model.get())) {
      continue;
    }
    auto& controller = get(*model);
    auto settings = BlotterSettings(controller);
    for(auto& link : std::as_const(*m_collection->get_links(*model))) {
      if(saved.contains(link.get())) {
        settings.m_links.push_back(get(*link).m_id);
      }
    }
    state.m_blotters.push_back(std::move(settings));
    if(m_active->get() == model) {
      state.m_active = controller.m_id;
    }
  }
  return state;
}

void BlotterRegistry::restore(const State& state) {
  auto models = std::unordered_map<uuid, std::shared_ptr<BlotterModel>>();
  for(auto& settings : state.m_blotters) {
    if(settings.m_id.is_nil() || models.contains(settings.m_id)) {
      throw std::runtime_error("Invalid blotter identity.");
    }
    auto& controller = [&] () -> BlotterController& {
      if(settings.m_is_consolidated) {
        return get_consolidated(settings.m_account);
      }
      return create(settings.m_name, settings.m_account);
    }();
    settings.apply(controller);
    auto model = controller.get_model();
    if(!model->is_consolidated()) {
      model->is_pinned()->set(true);
    }
    models.emplace(settings.m_id, std::move(model));
  }
  for(auto& settings : state.m_blotters) {
    for(auto& id : settings.m_links) {
      auto source = models.find(id);
      if(source != models.end()) {
        m_collection->link(*models.at(settings.m_id), *source->second);
      }
    }
  }
  auto active = models.find(state.m_active);
  if(active != models.end()) {
    set_active(*active->second);
  }
}

void BlotterRegistry::close() {
  auto models = std::vector<BlottersModel::Type>(
    m_blotters->cbegin(), m_blotters->cend());
  for(auto& model : models) {
    auto i = m_entries.find(model.get());
    if(i != m_entries.end()) {
      i->second->m_controller->close();
    }
  }
}

QString BlotterRegistry::make_name(QString name) const {
  name = name.trimmed();
  if(name.isEmpty()) {
    name = QObject::tr("Blotter");
  }
  auto candidate = name;
  auto suffix = 2;
  auto is_used = [&] {
    return std::ranges::any_of(std::as_const(*m_blotters),
      [&] (const auto& model) {
        return model->get_name()->get() == candidate;
      });
  };
  while(is_used()) {
    candidate = QString("%1 (%2)").arg(name).arg(suffix);
    ++suffix;
  }
  return candidate;
}

BlotterController& BlotterRegistry::add(
    QString name, const DirectoryEntry& account, bool is_consolidated) {
  auto& clients = m_profile->GetClients();
  auto monitors = make_default_task_monitors();
  auto tasks = std::make_shared<MonitoredTasksModel>(monitors);
  auto orders = [&] () -> std::shared_ptr<OrdersModel> {
    if(is_consolidated) {
      return std::make_shared<AccountOrdersModel>(account, clients);
    }
    return std::make_shared<TaskOrdersModel>(tasks);
  }();
  auto portfolio = [&] () -> std::shared_ptr<PortfolioModel> {
    if(is_consolidated) {
      return make_account_portfolio(account, clients);
    }
    return std::make_shared<ArrayListModel<PortfolioUpdateEntry>>();
  }();
  auto is_pinned = [&] () -> std::shared_ptr<BooleanModel> {
    if(is_consolidated) {
      return make_constant_value_model(
        account == clients.get_service_locator_client().get_account());
    }
    return std::make_shared<LocalBooleanModel>(false);
  }();
  auto entry = std::make_unique<Entry>();
  entry->m_source = std::make_shared<CompositeBlotterModel>(account,
    is_consolidated, std::make_shared<LocalTextModel>(std::move(name)),
    std::make_shared<LocalBooleanModel>(false), std::move(is_pinned), tasks,
    monitors, std::move(orders), std::move(portfolio));
  auto model = m_collection->add(entry->m_source);
  entry->m_controller = std::make_unique<BlotterController>(
    model, tasks, m_collection,
    m_profile->GetKeyBindings()->get_cancel_key_bindings(), m_position_bindings,
    Ref(*m_profile));
  entry->m_open_connection =
    entry->m_controller->is_open()->connect_update_signal(
      std::bind_front(&BlotterRegistry::on_open, this, std::cref(*model)));
  entry->m_pinned_connection = model->is_pinned()->connect_update_signal(
    std::bind_front(&BlotterRegistry::on_pinned, this, std::cref(*model)));
  entry->m_active_connection = model->is_active()->connect_update_signal(
    std::bind_front(&BlotterRegistry::on_active, this, std::cref(*model)));
  auto& controller = *entry->m_controller;
  m_entries.emplace(model.get(), std::move(entry));
  update_lists(*model);
  return controller;
}

void BlotterRegistry::update_lists(const BlotterModel& blotter) {
  auto& controller = get(blotter);
  auto model = controller.get_model();
  update_membership(*m_available, model,
    model->is_pinned()->get() || controller.is_open()->get());
  update_membership(*m_pinned, model,
    model->is_pinned()->get() && !model->is_consolidated());
}

void BlotterRegistry::remove(const BlotterModel& blotter) {
  auto model = get(blotter).get_model();
  if(m_active->get() == model) {
    set_active(*get_consolidated().get_model());
  }
  update_membership(*m_available, model, false);
  update_membership(*m_pinned, model, false);
  m_collection->remove(std::ranges::find(std::as_const(*m_collection), model));
  m_entries.erase(model.get());
}

void BlotterRegistry::on_open(const BlotterModel& blotter, bool is_open) {
  if(!is_open && !blotter.is_pinned()->get()) {
    remove(blotter);
  } else {
    update_lists(blotter);
  }
}

void BlotterRegistry::on_pinned(const BlotterModel& blotter, bool is_pinned) {
  if(!is_pinned && !get(blotter).is_open()->get()) {
    remove(blotter);
  } else {
    update_lists(blotter);
  }
}

void BlotterRegistry::on_active(const BlotterModel& blotter, bool is_active) {
  if(m_is_updating_active) {
    return;
  }
  if(is_active) {
    set_active(blotter);
  } else if(m_active->get().get() == &blotter) {
    blotter.is_active()->set(true);
  }
}
