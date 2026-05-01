#include "Spire/KeyBindings/KeyBindingsModel.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

KeyBindingsModel::KeyBindingsModel()
    : m_order_task_arguments(
        std::make_shared<ArrayListModel<OrderTaskArguments>>()),
      m_cancel_key_bindings(std::make_shared<CancelKeyBindingsModel>()) {
  m_interactions.insert(std::pair(
    Scope::make_global(QObject::tr("Global").toStdString()),
    std::make_shared<InteractionsKeyBindingsModel>()));
  for(auto i = 0; i < CancelKeyBindingsModel::OPERATION_COUNT; ++i) {
    auto operation = static_cast<CancelKeyBindingsModel::Operation>(i);
    m_connections.add(
      m_cancel_key_bindings->get_binding(operation)->connect_update_signal(
        std::bind_front(&KeyBindingsModel::on_cancel_binding_update, this)));
  }
  m_connections.add(m_order_task_arguments->connect_operation_signal(
    std::bind_front(&KeyBindingsModel::on_order_task_operation, this)));
}

const std::shared_ptr<OrderTaskArgumentsListModel>&
    KeyBindingsModel::get_order_task_arguments() const {
  return m_order_task_arguments;
}

const std::shared_ptr<CancelKeyBindingsModel>&
    KeyBindingsModel::get_cancel_key_bindings() const {
  return m_cancel_key_bindings;
}

const std::shared_ptr<InteractionsKeyBindingsModel>&
    KeyBindingsModel::get_interactions_key_bindings(const Scope& scope) const {
  if(scope.get_countries().size() == 1) {
    auto country = *scope.get_countries().begin();
    auto i = m_interactions.find(country);
    if(i == m_interactions.end()) {
      auto interactions = std::make_shared<InteractionsKeyBindingsModel>(
        get_interactions_key_bindings(Scope::GLOBAL));
      i = m_interactions.insert(std::pair(country, interactions)).first;
    }
    return i->second;
  } else if(scope.get_venues().size() == 1) {
    auto& venue = DEFAULT_VENUES.from(*scope.get_venues().begin());
    auto i = m_interactions.find(venue.m_venue);
    if(i == m_interactions.end()) {
      auto interactions = std::make_shared<InteractionsKeyBindingsModel>(
        get_interactions_key_bindings(venue.m_country_code));
      i = m_interactions.insert(std::pair(venue.m_venue, interactions)).first;
    }
    return i->second;
  } else if(scope.get_tickers().size() == 1) {
    auto ticker = *scope.get_tickers().begin();
    auto i = m_interactions.find(ticker);
    if(i == m_interactions.end()) {
      auto interactions = std::make_shared<InteractionsKeyBindingsModel>(
        get_interactions_key_bindings(ticker.get_venue()));
      i = m_interactions.insert(std::pair(ticker, interactions)).first;
    }
    return i->second;
  } else if(scope.is_global()) {
    return m_interactions.at(scope);
  }
  static auto NONE = std::shared_ptr<InteractionsKeyBindingsModel>();
  return NONE;
}

std::vector<Scope>
    KeyBindingsModel::make_interactions_key_bindings_scopes() const {
  auto scopes = std::vector<Scope>();
  for(auto& entry : m_interactions) {
    if(entry.second->is_detached()) {
      scopes.push_back(entry.first);
    }
  }
  return scopes;
}

void KeyBindingsModel::on_cancel_binding_update(const QKeySequence& sequence) {
  if(sequence.isEmpty()) {
    return;
  }
  for(auto i = 0; i < m_order_task_arguments->get_size(); ++i) {
    auto& arguments = m_order_task_arguments->get(i);
    if(arguments.m_key == sequence) {
      auto updated = arguments;
      updated.m_key = QKeySequence();
      m_order_task_arguments->set(i, updated);
    }
  }
}

void KeyBindingsModel::on_order_task_operation(
    const OrderTaskArgumentsListModel::Operation& operation) {
  visit(operation,
    [&] (const OrderTaskArgumentsListModel::UpdateOperation& update) {
      auto& arguments = m_order_task_arguments->get(update.m_index);
      if(!arguments.m_key.isEmpty()) {
        if(auto conflict =
            m_cancel_key_bindings->find_operation(arguments.m_key)) {
          m_cancel_key_bindings->get_binding(*conflict)->set(QKeySequence());
        }
      }
    },
    [] (const auto&) {});
}
