#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Spire/Blotter/BlotterCollection.hpp"
#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/SpireTester/ListModelTester.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::Tests;
using namespace Spire;

namespace {
  struct Local {
    std::shared_ptr<ArrayListModel<BlotterTaskEntry>> m_tasks;
    std::shared_ptr<ArrayListModel<BlotterTaskMonitor>> m_monitors;
    std::shared_ptr<ArrayListModel<std::shared_ptr<Order>>> m_orders;
    std::shared_ptr<ArrayListModel<PortfolioUpdateEntry>> m_portfolio;
    std::shared_ptr<BlotterModel> m_model;

    Local(DirectoryEntry account, bool is_consolidated, QString name)
        : m_tasks(std::make_shared<ArrayListModel<BlotterTaskEntry>>()),
          m_monitors(std::make_shared<ArrayListModel<BlotterTaskMonitor>>()),
          m_orders(std::make_shared<ArrayListModel<std::shared_ptr<Order>>>()),
          m_portfolio(
            std::make_shared<ArrayListModel<PortfolioUpdateEntry>>()) {
      m_model = std::make_shared<CompositeBlotterModel>(std::move(account),
        is_consolidated, std::make_shared<LocalTextModel>(std::move(name)),
        std::make_shared<LocalBooleanModel>(false),
        std::make_shared<LocalBooleanModel>(false), m_tasks, m_monitors,
        m_orders, m_portfolio);
    }
  };

  struct Fixture {
    TestEnvironment m_environment;
    UserProfile m_profile;
    BlotterCollection m_collection;

    Fixture()
        : m_profile("", false, false, {}, {},
            get_default_additional_tag_database(), {}, {}, {},
            Clients(std::in_place_type<TestClients>, Ref(m_environment))),
          m_collection(m_profile.GetClients().get_market_data_client()) {
      m_environment.publish(parse_ticker("RY.TSX"), BboQuote(
        make_bid(Money(12), 100), make_ask(Money(13), 100),
        time_from_string("2026-11-23 10:00:00")));
    }

    auto make_task(int value) {
      auto entry = BlotterTaskEntry();
      entry.m_task = std::make_shared<Task>(
        IntegerNode(value), DirectoryEntry(), Ref(m_profile));
      return entry;
    }
  };

  auto make_order(OrderId id, Quantity quantity) {
    auto order = std::make_shared<PrimitiveOrder>(OrderInfo(
      make_limit_order_fields(parse_ticker("RY.TSX"), Currencies::CAD,
        Side::BID, "TSX", quantity, Money(10)), id,
      time_from_string("2026-11-23 10:00:00")));
    accept(*order);
    fill(*order, quantity);
    return order;
  }

  auto get_quantity(const BlotterModel& model) {
    auto quantity = Quantity();
    for(auto& entry : std::as_const(*model.get_portfolio())) {
      quantity += entry.m_inventory.m_position.m_quantity;
    }
    return quantity;
  }
}

TEST_SUITE("BlotterCollection") {
  UI_TEST_CASE("registration") {
    auto fixture = Fixture();
    auto& collection = fixture.m_collection;
    auto account = DirectoryEntry::make_account(1, "one");
    auto local = Local(account, false, "First");
    auto task = fixture.make_task(1);
    task.m_is_pinned = true;
    local.m_tasks->push(task);
    auto operations = std::deque<BlotterCollection::Operation>();
    auto connection = scoped_connection(collection.connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    auto model = collection.add(local.m_model);
    REQUIRE(collection.get_size() == 1);
    REQUIRE(collection.get(0) == model);
    REQUIRE(model->get_account() == account);
    REQUIRE_FALSE(model->is_consolidated());
    REQUIRE(model->get_name() == local.m_model->get_name());
    REQUIRE(model->is_active() == local.m_model->is_active());
    REQUIRE(model->is_pinned() == local.m_model->is_pinned());
    REQUIRE(model->get_tasks()->get(0).m_is_pinned);
    REQUIRE(collection.get_links(*model)->get_size() == 0);
    require_list_transaction<BlotterCollection::Type>(
      operations, {BlotterCollection::AddOperation(0)});
    operations.clear();
    REQUIRE(collection.add(local.m_model) == model);
    REQUIRE(collection.add(model) == model);
    REQUIRE(operations.empty());
    REQUIRE(collection.link(*model, *model) == QValidator::Invalid);
    REQUIRE(collection.link(*model, *local.m_model) == QValidator::Invalid);
    REQUIRE_THROWS_AS(
      collection.get_links(*local.m_model), std::invalid_argument);
    auto foreign =
      Local(DirectoryEntry::make_account(2, "two"), false, "Other");
    auto other = collection.add(foreign.m_model);
    REQUIRE(collection.link(*model, *other) == QValidator::Invalid);
    REQUIRE(collection.unlink(*model, *other) == QValidator::Invalid);
    REQUIRE(collection.get_links(*model)->get_size() == 0);
  }

  UI_TEST_CASE("links") {
    auto fixture = Fixture();
    auto& collection = fixture.m_collection;
    auto account = DirectoryEntry::make_account(1, "one");
    auto first = Local(account, false, "First");
    auto second = Local(account, false, "Second");
    first.m_tasks->push(fixture.make_task(1));
    second.m_tasks->push(fixture.make_task(2));
    first.m_orders->push(make_order(1, 10));
    second.m_orders->push(make_order(2, 20));
    first.m_monitors->push(
      BlotterTaskMonitor("First", std::make_shared<IntegerNode>(11)));
    second.m_monitors->push(
      BlotterTaskMonitor("Second", std::make_shared<IntegerNode>(22)));
    auto a = collection.add(first.m_model);
    auto b = collection.add(second.m_model);
    auto links = collection.get_links(*a);
    auto operations = std::deque<BlotterCollection::Operation>();
    auto connection = scoped_connection(links->connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    REQUIRE(collection.link(*a, *b) == QValidator::Acceptable);
    REQUIRE(links->get_size() == 1);
    REQUIRE(links->get(0) == b);
    REQUIRE(a->get_tasks()->get_size() == 2);
    REQUIRE(b->get_tasks()->get_size() == 1);
    REQUIRE(a->get_orders()->get_size() == 2);
    REQUIRE(get_quantity(*a) == 30);
    REQUIRE(get_quantity(*b) == 20);
    require_list_transaction<BlotterCollection::Type>(
      operations, {BlotterCollection::AddOperation(0)});
    operations.clear();
    REQUIRE(collection.link(*a, *b) == QValidator::Acceptable);
    REQUIRE(operations.empty());
    auto entry = a->get_tasks()->get(1);
    auto observer = entry.m_monitors.front();
    REQUIRE(std::any_cast<Quantity>(observer->get()) == 11);
    REQUIRE(std::any_cast<Quantity>(
      b->get_tasks()->get(0).m_monitors.front()->get()) == 22);
    entry.m_is_pinned = true;
    a->get_tasks()->set(1, entry);
    REQUIRE_FALSE(b->get_tasks()->get(0).m_is_pinned);
    entry = second.m_tasks->get(0);
    entry.m_state = Task::State::ACTIVE;
    second.m_tasks->set(0, entry);
    REQUIRE(a->get_tasks()->get(1).m_is_pinned);
    REQUIRE(a->get_tasks()->get(1).m_state == Task::State::ACTIVE);
    REQUIRE(a->get_tasks()->get(1).m_monitors.front() == observer);
    second.m_tasks->push(fixture.make_task(3));
    second.m_orders->push(make_order(3, 5));
    REQUIRE(a->get_tasks()->get_size() == 3);
    REQUIRE(a->get_orders()->get_size() == 3);
    REQUIRE(get_quantity(*a) == 35);
    second.m_tasks->remove(1);
    second.m_orders->remove(1);
    REQUIRE(get_quantity(*a) == 30);
    REQUIRE(collection.unlink(*a, *b) == QValidator::Acceptable);
    REQUIRE(links->get_size() == 0);
    REQUIRE(a->get_tasks()->get_size() == 1);
    REQUIRE(a->get_orders()->get_size() == 1);
    REQUIRE(get_quantity(*a) == 10);
    require_list_transaction<BlotterCollection::Type>(operations,
      {BlotterCollection::PreRemoveOperation(0),
       BlotterCollection::RemoveOperation(0)});
    operations.clear();
    REQUIRE(collection.unlink(*a, *b) == QValidator::Acceptable);
    REQUIRE(operations.empty());
    second.m_orders->push(make_order(4, 5));
    REQUIRE(a->get_orders()->get_size() == 1);
    REQUIRE(get_quantity(*a) == 10);
  }

  UI_TEST_CASE("cycles_and_overlapping_links") {
    auto fixture = Fixture();
    auto& collection = fixture.m_collection;
    auto account = DirectoryEntry::make_account(1, "one");
    auto locals = std::vector<Local>();
    auto models = std::vector<BlotterCollection::Type>();
    for(auto i = 0; i != 4; ++i) {
      locals.emplace_back(account, false, QString::number(i));
      locals.back().m_tasks->push(fixture.make_task(i));
      locals.back().m_orders->push(make_order(i, 10));
      models.push_back(collection.add(locals.back().m_model));
    }
    auto a = models[0];
    auto b = models[1];
    auto c = models[2];
    auto d = models[3];
    collection.link(*a, *b);
    collection.link(*a, *c);
    collection.link(*b, *d);
    collection.link(*c, *d);
    REQUIRE(a->get_tasks()->get_size() == 4);
    REQUIRE(a->get_orders()->get_size() == 4);
    REQUIRE(get_quantity(*a) == 40);
    collection.link(*d, *a);
    for(auto& model : models) {
      REQUIRE(model->get_tasks()->get_size() == 4);
      REQUIRE(model->get_orders()->get_size() == 4);
      REQUIRE(get_quantity(*model) == 40);
    }
    collection.unlink(*d, *a);
    REQUIRE(d->get_tasks()->get_size() == 1);
    REQUIRE(b->get_tasks()->get_size() == 2);
    REQUIRE(c->get_tasks()->get_size() == 2);
    REQUIRE(a->get_tasks()->get_size() == 4);
    collection.unlink(*a, *b);
    REQUIRE(a->get_tasks()->get_size() == 3);
    REQUIRE(a->get_orders()->get_size() == 3);
    REQUIRE(get_quantity(*a) == 30);
    collection.unlink(*c, *d);
    REQUIRE(a->get_tasks()->get_size() == 2);
    REQUIRE(get_quantity(*a) == 20);
    REQUIRE(get_quantity(*b) == 20);
    REQUIRE(get_quantity(*c) == 10);
    REQUIRE(get_quantity(*d) == 10);
  }

  UI_TEST_CASE("consolidation") {
    auto fixture = Fixture();
    auto& collection = fixture.m_collection;
    auto account = DirectoryEntry::make_account(1, "one");
    auto first = Local(account, false, "First");
    auto second = Local(account, false, "Second");
    auto global = Local(account, true, "Global");
    auto first_order = make_order(1, 10);
    auto second_order = make_order(2, 20);
    first.m_tasks->push(fixture.make_task(1));
    second.m_tasks->push(fixture.make_task(2));
    first.m_orders->push(first_order);
    second.m_orders->push(second_order);
    global.m_orders->push(first_order);
    global.m_orders->push(second_order);
    global.m_orders->push(make_order(3, 5));
    auto snapshot = PortfolioUpdateEntry();
    snapshot.m_inventory.m_position =
      Position(parse_ticker("RY.TSX"), Currencies::CAD, 100, Money(1000));
    global.m_portfolio->push(snapshot);
    auto a = collection.add(first.m_model);
    auto b = collection.add(second.m_model);
    auto consolidated = collection.add(global.m_model);
    REQUIRE(consolidated->is_consolidated());
    REQUIRE(collection.get_links(*consolidated)->get_size() == 2);
    REQUIRE(consolidated->get_tasks()->get_size() == 2);
    REQUIRE(consolidated->get_orders()->get_size() == 3);
    REQUIRE(consolidated->get_portfolio() == global.m_portfolio);
    REQUIRE(get_quantity(*consolidated) == 100);
    REQUIRE(collection.link(*consolidated, *a) == QValidator::Invalid);
    REQUIRE(collection.unlink(*consolidated, *a) == QValidator::Invalid);
    REQUIRE(collection.link(*a, *consolidated) == QValidator::Acceptable);
    REQUIRE(a->get_tasks()->get_size() == 2);
    REQUIRE(a->get_orders()->get_size() == 3);
    REQUIRE(get_quantity(*a) == 35);
    REQUIRE(get_quantity(*b) == 20);
    auto third = Local(account, false, "Third");
    third.m_orders->push(make_order(4, 8));
    auto c = collection.add(third.m_model);
    REQUIRE(collection.get_links(*consolidated)->get_size() == 3);
    REQUIRE(a->get_orders()->get_size() == 4);
    REQUIRE(get_quantity(*a) == 43);
    snapshot.m_inventory.m_position.m_quantity = 120;
    global.m_portfolio->set(0, snapshot);
    REQUIRE(get_quantity(*consolidated) == 120);
    REQUIRE(get_quantity(*a) == 43);
    auto foreign =
      Local(DirectoryEntry::make_account(2, "two"), false, "Other");
    collection.add(foreign.m_model);
    REQUIRE(collection.get_links(*consolidated)->get_size() == 3);
    auto duplicate = Local(account, true, "Duplicate");
    REQUIRE_THROWS_AS(collection.add(duplicate.m_model), std::invalid_argument);
    REQUIRE(collection.get_size() == 5);
    collection.unlink(*a, *consolidated);
    REQUIRE(a->get_orders()->get_size() == 1);
    REQUIRE(get_quantity(*a) == 10);
    REQUIRE(consolidated->get_orders()->get_size() == 4);
  }

  UI_TEST_CASE("removal") {
    auto fixture = Fixture();
    auto& collection = fixture.m_collection;
    auto account = DirectoryEntry::make_account(1, "one");
    auto first = Local(account, false, "First");
    auto second = Local(account, false, "Second");
    auto third = Local(account, false, "Third");
    first.m_orders->push(make_order(1, 10));
    second.m_orders->push(make_order(2, 20));
    third.m_orders->push(make_order(3, 30));
    auto a = collection.add(first.m_model);
    auto b = collection.add(second.m_model);
    auto c = collection.add(third.m_model);
    collection.link(*a, *b);
    collection.link(*b, *c);
    collection.link(*c, *a);
    auto links = collection.get_links(*a);
    auto removed_links = collection.get_links(*b);
    auto weak = std::weak_ptr(b);
    REQUIRE(get_quantity(*a) == 60);
    REQUIRE(collection.remove(1) == QValidator::Acceptable);
    REQUIRE(collection.get_size() == 2);
    REQUIRE(links->get_size() == 0);
    REQUIRE(removed_links->get_size() == 0);
    REQUIRE(a->get_orders()->get_size() == 1);
    REQUIRE(b->get_orders()->get_size() == 1);
    REQUIRE(c->get_orders()->get_size() == 2);
    REQUIRE(get_quantity(*a) == 10);
    REQUIRE(get_quantity(*b) == 20);
    REQUIRE(get_quantity(*c) == 40);
    second.m_orders->push(make_order(4, 5));
    REQUIRE(get_quantity(*a) == 10);
    REQUIRE(get_quantity(*b) == 25);
    REQUIRE(get_quantity(*c) == 40);
    REQUIRE(collection.link(*a, *b) == QValidator::Invalid);
    b.reset();
    REQUIRE(weak.expired());
    collection.remove(0);
    REQUIRE(collection.get_links(*c)->get_size() == 0);
    REQUIRE(get_quantity(*c) == 30);
  }
}
