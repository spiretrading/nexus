#include "Spire/Blotter/FlattenController.hpp"
#include <algorithm>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include "Nexus/Accounting/InventorySnapshot.hpp"
#include "Spire/Blotter/BlotterController.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OrderWrapperTaskNode.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

namespace {
  struct OrderEntry {
    std::shared_ptr<Order> m_order;
    std::weak_ptr<BlotterModel> m_origin;
    std::shared_ptr<Task> m_task;
    Quantity m_filled;
    Quantity m_remaining;
    int m_sequence;
    bool m_is_account_order;
    bool m_is_flatten;

    OrderEntry(std::shared_ptr<Order> order);

    void update();
  };

  struct Reservation {
    std::weak_ptr<BlotterModel> m_origin;
    Ticker m_ticker;
    Quantity m_quantity;
  };

  struct Account {
    std::unordered_map<Ticker, Position> m_initial;
    std::unordered_map<OrderId, OrderEntry> m_orders;
    std::shared_ptr<Queue<std::shared_ptr<Order>>> m_stream;
    std::list<Reservation> m_reservations;

    Account(const DirectoryEntry& account, Clients& clients);
    ~Account();

    OrderEntry& observe(const std::shared_ptr<Order>& order);
    void update();
  };

  auto get_scope(const BlotterCollection& collection,
      const std::shared_ptr<BlotterModel>& model) {
    auto scope = std::unordered_set<const BlotterModel*>();
    auto pending = std::vector{model};
    while(!pending.empty()) {
      auto current = pending.back();
      pending.pop_back();
      if(scope.insert(current.get()).second) {
        for(auto& link : std::as_const(*collection.get_links(*current))) {
          pending.push_back(link);
        }
      }
    }
    return scope;
  }

  OrderEntry::OrderEntry(std::shared_ptr<Order> order)
    : m_order(std::move(order)),
      m_filled(0),
      m_remaining(m_order->get_info().m_fields.m_quantity),
      m_sequence(-1),
      m_is_account_order(false),
      m_is_flatten(false) {}

  void OrderEntry::update() {
    m_order->get_publisher().with([&] (const auto& reports) {
      if(!reports) {
        return;
      }
      auto first = std::ranges::upper_bound(
        *reports, m_sequence, {}, &ExecutionReport::m_sequence);
      for(auto i = first; i != reports->end(); ++i) {
        if(i->m_sequence <= m_sequence) {
          continue;
        }
        m_sequence = i->m_sequence;
        m_filled += i->m_last_quantity;
        m_remaining -= i->m_last_quantity;
        if(is_terminal(i->m_status)) {
          m_remaining = 0;
        }
      }
    });
    if(m_remaining == 0) {
      m_task.reset();
    }
  }

  Account::Account(const DirectoryEntry& account, Clients& clients)
      : m_stream(std::make_shared<Queue<std::shared_ptr<Order>>>()) {
    auto [portfolio, sequence, excluded] = make_portfolio(
      clients.get_risk_client().load_inventory_snapshot(account), account,
      clients.get_order_execution_client());
    for(auto& inventory : portfolio.get_bookkeeper().get_inventory_range()) {
      m_initial.emplace(inventory.m_position.m_ticker, inventory.m_position);
    }
    for(auto& order : excluded) {
      observe(order).m_is_account_order = true;
    }
    auto query = AccountQuery();
    query.set_index(account);
    query.set_range(increment(sequence), Sequence::LAST);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    query.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
    clients.get_order_execution_client().query(query, m_stream);
  }

  Account::~Account() {
    m_stream->close();
  }

  OrderEntry& Account::observe(const std::shared_ptr<Order>& order) {
    return m_orders.try_emplace(order->get_info().m_id, order).first->second;
  }

  void Account::update() {
    while(auto order = m_stream->try_pop()) {
      observe(*order).m_is_account_order = true;
    }
  }
}

struct FlattenController::State {
  UserProfile* m_profile;
  std::shared_ptr<BlotterCollection> m_collection;
  std::unordered_map<DirectoryEntry, std::unique_ptr<Account>> m_accounts;

  State(
    Ref<UserProfile> profile, std::shared_ptr<BlotterCollection> collection);

  Position get_position(Account& account,
    const std::shared_ptr<BlotterModel>& model, const Ticker& ticker);
};

FlattenController::State::State(
  Ref<UserProfile> profile, std::shared_ptr<BlotterCollection> collection)
  : m_profile(profile.get()),
    m_collection(std::move(collection)) {}

Position FlattenController::State::get_position(Account& account,
    const std::shared_ptr<BlotterModel>& model, const Ticker& ticker) {
  auto scope = get_scope(*m_collection, model);
  auto includes_account = std::ranges::any_of(scope, [] (auto model) {
    return model->is_consolidated();
  });
  auto position = Position(ticker, CurrencyId(), 0, Money::ZERO);
  auto selected = std::unordered_set<OrderId>();
  account.update();
  if(model->is_consolidated()) {
    auto initial = account.m_initial.find(ticker);
    if(initial != account.m_initial.end()) {
      position = initial->second;
    }
    for(auto& [id, entry] : account.m_orders) {
      if(entry.m_is_account_order) {
        selected.insert(id);
      }
    }
  } else {
    auto add = [&] (const auto& order) {
      account.observe(order);
      selected.insert(order->get_info().m_id);
    };
    for(auto& order : std::as_const(*model->get_orders())) {
      add(order);
    }
    for(auto& entry : std::as_const(*model->get_tasks())) {
      entry.m_task->GetContext().GetOrderPublisher().with(
        [&] (const auto& orders) {
          if(orders) {
            for(auto& order : *orders) {
              add(order);
            }
          }
        });
    }
  }
  for(auto& [id, entry] : account.m_orders) {
    if(entry.m_is_flatten &&
        (includes_account || scope.contains(entry.m_origin.lock().get()))) {
      selected.insert(id);
    }
  }
  for(auto id : selected) {
    auto& entry = account.m_orders.at(id);
    auto& fields = entry.m_order->get_info().m_fields;
    if(fields.m_ticker != ticker) {
      continue;
    }
    entry.update();
    position.m_currency = fields.m_currency;
    auto quantity = entry.m_filled;
    if(entry.m_is_flatten) {
      quantity += entry.m_remaining;
    }
    position.m_quantity += get_direction(fields.m_side) * quantity;
  }
  for(auto& reservation : account.m_reservations) {
    if(reservation.m_ticker == ticker && (includes_account ||
        scope.contains(reservation.m_origin.lock().get()))) {
      position.m_quantity += reservation.m_quantity;
    }
  }
  return position;
}

FlattenController::FlattenController(
  Ref<UserProfile> profile, std::shared_ptr<BlotterCollection> collection)
  : m_state(std::make_unique<State>(profile, std::move(collection))) {}

FlattenController::~FlattenController() = default;

void FlattenController::flatten(
    BlotterController& controller, const Ticker& ticker) {
  auto model = controller.get_model();
  auto& account = [&] () -> auto& {
    auto i = m_state->m_accounts.find(model->get_account());
    if(i == m_state->m_accounts.end()) {
      auto account = std::make_unique<Account>(
        model->get_account(), m_state->m_profile->GetClients());
      i = m_state->m_accounts.emplace(
        model->get_account(), std::move(account)).first;
    }
    return *i->second;
  }();
  auto position = m_state->get_position(account, model, ticker);
  auto quantity = -position.m_quantity;
  if(quantity == 0) {
    return;
  }
  auto fields = make_market_order_fields(model->get_account(), ticker,
    position.m_currency, get_side(quantity),
    DESTINATIONS.get_preferred_destination(ticker.get_venue()).m_id,
    abs(quantity));
  auto reservation = account.m_reservations.emplace(
    account.m_reservations.end(), model, ticker, quantity);
  auto order = std::shared_ptr<Order>();
  try {
    order =
      m_state->m_profile->GetClients().get_order_execution_client().submit(
        fields);
  } catch(const std::exception&) {
    account.m_reservations.erase(reservation);
    throw;
  }
  auto& entry = account.observe(order);
  entry.m_origin = model;
  entry.m_is_account_order = true;
  entry.m_is_flatten = true;
  account.m_reservations.erase(reservation);
  entry.m_task = controller.submit(
    OrderWrapperTaskNode(order, *m_state->m_profile, "Flatten"));
}
