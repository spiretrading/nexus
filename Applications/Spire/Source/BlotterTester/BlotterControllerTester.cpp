#include <QEvent>
#include <QPointer>
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Spire/Blotter/BlotterController.hpp"
#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

namespace {
  struct Fixture {
    TestEnvironment m_environment;
    std::shared_ptr<BlotterCollection> m_collection;
    std::shared_ptr<BlotterModel> m_model;

    Fixture() {
      initialize_resources();
      auto clients = TestClients(Ref(m_environment));
      m_collection = std::make_shared<BlotterCollection>(
        clients.get_market_data_client());
      m_model = m_collection->add(std::make_shared<CompositeBlotterModel>(
        DirectoryEntry(), false, std::make_shared<LocalTextModel>("Test"),
        std::make_shared<LocalBooleanModel>(false),
        std::make_shared<LocalBooleanModel>(false),
        std::make_shared<ArrayListModel<BlotterTaskEntry>>(),
        std::make_shared<ArrayListModel<BlotterTaskMonitor>>(),
        std::make_shared<ArrayListModel<std::shared_ptr<Order>>>(),
        std::make_shared<ArrayListModel<PortfolioUpdateEntry>>()));
    }

    auto make_controller() {
      return std::make_unique<BlotterController>(m_model, m_collection,
        std::make_shared<CancelKeyBindingsModel>(),
        std::make_shared<PositionKeyBindingsModel>());
    }
  };
}

TEST_SUITE("BlotterController") {
  UI_TEST_CASE("open_close") {
    auto fixture = Fixture();
    auto controller = fixture.make_controller();
    auto updates = std::vector<bool>();
    auto connection = boost::signals2::scoped_connection(
      controller->is_open()->connect_update_signal([&] (auto value) {
        updates.push_back(value);
      }));
    REQUIRE_FALSE(controller->is_open()->get());
    controller->close();
    REQUIRE(updates.empty());
    auto window = QPointer(&controller->open());
    REQUIRE(controller->is_open()->get());
    REQUIRE(window->isVisible());
    REQUIRE(window->get_model() == fixture.m_model);
    REQUIRE(&controller->open() == window.data());
    REQUIRE(updates == std::vector{true});
    SUBCASE("controller_close") {
      controller->close();
    }
    SUBCASE("window_close") {
      REQUIRE(window->close());
    }
    REQUIRE_FALSE(controller->is_open()->get());
    REQUIRE_FALSE(window->isVisible());
    REQUIRE(updates == std::vector{true, false});
    controller->close();
    REQUIRE(updates.size() == 2);
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    REQUIRE(window.isNull());
  }

  UI_TEST_CASE("reopen") {
    auto fixture = Fixture();
    fixture.m_model->is_pinned()->set(true);
    auto controller = fixture.make_controller();
    auto window = QPointer(&controller->open());
    controller->close();
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    REQUIRE(window.isNull());
    fixture.m_model->get_name()->set("Renamed");
    auto& reopened = controller->open();
    REQUIRE(controller->is_open()->get());
    REQUIRE(controller->get_model() == fixture.m_model);
    REQUIRE(reopened.get_model() == fixture.m_model);
    REQUIRE(reopened.windowTitle().contains("Renamed"));
    REQUIRE(reopened.isVisible());
  }

  UI_TEST_CASE("remove_on_close") {
    auto fixture = Fixture();
    auto controller = fixture.make_controller();
    auto is_open = controller->is_open();
    auto connection = boost::signals2::scoped_connection(
      is_open->connect_update_signal([&] (auto value) {
        if(!value) {
          fixture.m_collection->remove(0);
          controller.reset();
        }
      }));
    auto window = QPointer(&controller->open());
    REQUIRE(window->close());
    REQUIRE_FALSE(controller);
    REQUIRE_FALSE(is_open->get());
    REQUIRE(fixture.m_collection->get_size() == 0);
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    REQUIRE(window.isNull());
  }

  UI_TEST_CASE("destroy_open_controller") {
    auto fixture = Fixture();
    auto controller = fixture.make_controller();
    auto is_open = controller->is_open();
    auto window = QPointer(&controller->open());
    controller.reset();
    REQUIRE(window.isNull());
    REQUIRE_FALSE(is_open->get());
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  }
}
