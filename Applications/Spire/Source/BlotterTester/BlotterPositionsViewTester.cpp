#include <algorithm>
#include <boost/optional/optional_io.hpp>
#include <QKeyEvent>
#include <QtTest/qtestkeyboard.h>
#include <QtTest/qtestmouse.h>
#include "Spire/Blotter/BlotterPositionsView.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  auto make_entry(const std::string& ticker, Quantity quantity,
      Money price, Money unrealized) {
    auto entry = PortfolioUpdateEntry();
    entry.m_inventory.m_position = Position(
      parse_ticker(ticker), Currencies::CAD, quantity, quantity * price);
    entry.m_unrealized = unrealized;
    return entry;
  }

  struct Fixture {
    std::shared_ptr<ArrayListModel<PortfolioUpdateEntry>> m_portfolio;
    std::shared_ptr<PositionKeyBindingsModel> m_bindings;
    std::unique_ptr<BlotterPositionsView> m_view;
    TableView* m_table;
    std::vector<std::vector<Position>> m_requests;
    boost::signals2::scoped_connection m_connection;

    Fixture()
      : m_portfolio(std::make_shared<ArrayListModel<PortfolioUpdateEntry>>()),
        m_bindings(std::make_shared<PositionKeyBindingsModel>()),
        m_table(nullptr) {}

    void open() {
      initialize_resources();
      m_view = std::make_unique<BlotterPositionsView>(m_portfolio, m_bindings);
      m_table = static_cast<TableView*>(m_view->focusProxy());
      m_connection = m_view->connect_flatten_signal(
        [=, this] (const auto& positions) { m_requests.push_back(positions); });
      m_view->resize(650, 300);
      m_view->show();
      m_table->get_body().setFocus();
      QApplication::processEvents();
    }

    Button& find_button(const QString& name) {
      for(auto widget : m_view->findChildren<QWidget*>()) {
        if(widget->accessibleName() == name) {
          return dynamic_cast<Button&>(*widget);
        }
      }
      throw std::runtime_error("Button not found.");
    }

    void click(Button& button) {
      auto point = button.mapToGlobal(button.rect().center());
      QCursor::setPos(point);
      QTest::mouseClick(m_view->windowHandle(), Qt::LeftButton,
        Qt::NoModifier, m_view->mapFromGlobal(point));
      QApplication::processEvents();
    }

    void key(Qt::Key key, Qt::KeyboardModifiers modifiers) {
      m_table->get_body().setFocus();
      QTest::keyClick(m_view->windowHandle(), key, modifiers);
    }
  };
}

TEST_SUITE("BlotterPositionsView") {
  UI_TEST_CASE("portfolio_columns") {
    auto fixture = Fixture();
    auto first = make_entry("RY.TSX", -200, Money(12), Money(-30));
    auto flat = make_entry("TD.TSX", 0, Money(0), Money(0));
    auto second = make_entry("ABX.TSX", 100, Money(2), Money(10));
    fixture.m_portfolio->push(first);
    fixture.m_portfolio->push(flat);
    fixture.m_portfolio->push(second);
    fixture.open();
    REQUIRE(fixture.m_view->get_portfolio() == fixture.m_portfolio);
    auto& table = *fixture.m_table->get_table();
    REQUIRE(table.get_column_size() == 7);
    REQUIRE(table.get_row_size() == 2);
    REQUIRE(table.get<Ticker>(0, 0) == parse_ticker("RY.TSX"));
    REQUIRE(table.get<Quantity>(0, 1) == 200);
    REQUIRE(table.get<PositionSide>(0, 2).m_side == Side::ASK);
    REQUIRE(to_text(table.at(0, 2)) == "Short");
    REQUIRE(table.get<Money>(0, 3) == Money(12));
    REQUIRE(table.get<Money>(0, 4) == Money(-30));
    REQUIRE(table.get<Money>(0, 5) == Money(-2400));
    REQUIRE(table.get<CurrencyId>(0, 6) == Currencies::CAD);
    REQUIRE(to_text(table.at(1, 2)) == "Long");
    auto& displayed = *fixture.m_table->get_body().get_table();
    REQUIRE(displayed.get<Ticker>(0, 0) == parse_ticker("ABX.TSX"));
    auto sorted = SortedTableModel(fixture.m_table->get_table());
    sorted.set_column_order({{3, SortedTableModel::Ordering::ASCENDING}});
    REQUIRE(sorted.get<Money>(0, 3) == Money(2));
    REQUIRE(sorted.get<Money>(1, 3) == Money(12));
    sorted.set_column_order({{4, SortedTableModel::Ordering::DESCENDING}});
    REQUIRE(sorted.get<Money>(0, 4) == Money(10));
    REQUIRE(sorted.get<Money>(1, 4) == Money(-30));
  }

  UI_TEST_CASE("live_positions") {
    auto fixture = Fixture();
    auto first = make_entry("RY.TSX", 100, Money(12), Money(10));
    auto second = make_entry("TD.TSX", 0, Money(0), Money(0));
    fixture.m_portfolio->push(first);
    fixture.m_portfolio->push(second);
    fixture.open();
    auto& table = *fixture.m_table->get_table();
    auto label = static_cast<MoneyBox*>(nullptr);
    for(auto widget : fixture.m_table->findChildren<QWidget*>()) {
      if(auto money = dynamic_cast<MoneyBox*>(widget)) {
        label = money;
        break;
      }
    }
    REQUIRE(label);
    REQUIRE(label->get_current()->get() == Money(10));
    REQUIRE(label->is_read_only());
    first.m_unrealized = Money(-20);
    first.m_inventory.m_position.m_quantity = -50;
    first.m_inventory.m_position.m_cost_basis = Money(-650);
    fixture.m_portfolio->set(0, first);
    REQUIRE(table.get<Quantity>(0, 1) == 50);
    REQUIRE(table.get<PositionSide>(0, 2).m_side == Side::ASK);
    REQUIRE(table.get<Money>(0, 3) == Money(13));
    REQUIRE(table.get<Money>(0, 4) == Money(-20));
    REQUIRE(label->get_current()->get() == Money(-20));
    second.m_inventory.m_position.m_quantity = 30;
    second.m_inventory.m_position.m_cost_basis = Money(300);
    fixture.m_portfolio->set(1, second);
    REQUIRE(table.get_row_size() == 2);
    first.m_inventory.m_position.m_quantity = 0;
    fixture.m_portfolio->set(0, first);
    REQUIRE(table.get_row_size() == 1);
    REQUIRE(table.get<Ticker>(0, 0) == parse_ticker("TD.TSX"));
    first.m_inventory.m_position.m_quantity = 20;
    first.m_inventory.m_position.m_cost_basis = Money(200);
    fixture.m_portfolio->set(0, first);
    REQUIRE(table.get_row_size() == 2);
    REQUIRE(table.get<Ticker>(0, 0) == parse_ticker("RY.TSX"));
    fixture.m_portfolio->move(0, 1);
    REQUIRE(table.get<Ticker>(0, 0) == parse_ticker("TD.TSX"));
    fixture.m_portfolio->transact([&] {
      fixture.m_portfolio->remove(0);
      fixture.m_portfolio->remove(0);
    });
    REQUIRE(table.get_row_size() == 0);
    REQUIRE_FALSE(fixture.find_button("Flatten all positions").isEnabled());
  }

  UI_TEST_CASE("flatten_commands") {
    auto fixture = Fixture();
    fixture.open();
    auto& flatten = fixture.find_button("Flatten position");
    auto& all = fixture.find_button("Flatten all positions");
    REQUIRE_FALSE(flatten.isEnabled());
    REQUIRE_FALSE(all.isEnabled());
    fixture.key(Qt::Key_F, Qt::ControlModifier | Qt::ShiftModifier);
    REQUIRE(fixture.m_requests.empty());
    auto first = make_entry("RY.TSX", 100, Money(12), Money(10));
    auto second = make_entry("ABX.TSX", -50, Money(10), Money(-20));
    fixture.m_portfolio->push(first);
    fixture.m_portfolio->push(second);
    auto rows = fixture.m_table->get_selection()->get_row_selection();
    clear(*rows);
    REQUIRE_FALSE(flatten.isEnabled());
    REQUIRE(all.isEnabled());
    fixture.click(flatten);
    REQUIRE(fixture.m_requests.empty());
    fixture.click(all);
    REQUIRE(fixture.m_requests.size() == 1);
    REQUIRE(std::ranges::is_permutation(fixture.m_requests.back(),
      std::vector{
        first.m_inventory.m_position, second.m_inventory.m_position}));
    rows->push(1);
    REQUIRE(flatten.isEnabled());
    fixture.click(flatten);
    REQUIRE(fixture.m_requests.size() == 2);
    REQUIRE(fixture.m_requests.back() ==
      std::vector{second.m_inventory.m_position});
    second.m_inventory.m_position.m_quantity = 0;
    fixture.m_portfolio->set(1, second);
    REQUIRE(rows->get_size() == 0);
    REQUIRE_FALSE(flatten.isEnabled());
    REQUIRE(all.isEnabled());
    fixture.click(all);
    REQUIRE(fixture.m_requests.size() == 3);
    REQUIRE(fixture.m_requests.back() ==
      std::vector{first.m_inventory.m_position});
    fixture.m_portfolio->remove(0);
    REQUIRE_FALSE(all.isEnabled());
    fixture.key(Qt::Key_F, Qt::ControlModifier | Qt::ShiftModifier);
    REQUIRE(fixture.m_requests.size() == 3);
  }

  UI_TEST_CASE("sorted_selection") {
    auto fixture = Fixture();
    auto first = make_entry("RY.TSX", 100, Money(12), Money(10));
    auto second = make_entry("ABX.TSX", -50, Money(10), Money(-20));
    fixture.m_portfolio->push(first);
    fixture.m_portfolio->push(second);
    fixture.open();
    auto rows =
      fixture.m_table->get_body().get_selection()->get_row_selection();
    clear(*rows);
    rows->push(0);
    fixture.key(Qt::Key_F, Qt::ControlModifier);
    REQUIRE(fixture.m_requests.size() == 1);
    REQUIRE(fixture.m_requests.back() ==
      std::vector{second.m_inventory.m_position});
    second.m_inventory.m_position.m_quantity = -75;
    second.m_inventory.m_position.m_cost_basis = Money(-750);
    fixture.m_portfolio->set(1, second);
    fixture.m_portfolio->move(1, 0);
    fixture.m_portfolio->insert(
      make_entry("BMO.TSX", 20, Money(5), Money(0)), 0);
    fixture.key(Qt::Key_F, Qt::ControlModifier);
    REQUIRE(fixture.m_requests.size() == 2);
    REQUIRE(fixture.m_requests.back() ==
      std::vector{second.m_inventory.m_position});
    rows->push(2);
    fixture.key(Qt::Key_F, Qt::ControlModifier);
    REQUIRE(fixture.m_requests.size() == 3);
    REQUIRE(std::ranges::is_permutation(fixture.m_requests.back(),
      std::vector{
        first.m_inventory.m_position, second.m_inventory.m_position}));
    fixture.m_portfolio->remove(1);
    fixture.key(Qt::Key_F, Qt::ControlModifier);
    REQUIRE(fixture.m_requests.size() == 4);
    REQUIRE(fixture.m_requests.back() ==
      std::vector{first.m_inventory.m_position});
  }

  UI_TEST_CASE("position_shortcuts") {
    using Operation = PositionKeyBindingsModel::Operation;
    auto fixture = Fixture();
    fixture.m_portfolio->push(
      make_entry("RY.TSX", 100, Money(12), Money(10)));
    fixture.open();
    auto rows = fixture.m_table->get_selection()->get_row_selection();
    clear(*rows);
    fixture.key(Qt::Key_F, Qt::ControlModifier);
    REQUIRE(fixture.m_requests.empty());
    rows->push(0);
    fixture.m_bindings->get_binding(Operation::FLATTEN)->set(
      QKeySequence(Qt::ALT | Qt::Key_F));
    fixture.key(Qt::Key_F, Qt::ControlModifier);
    REQUIRE(fixture.m_requests.empty());
    fixture.key(Qt::Key_F, Qt::AltModifier);
    REQUIRE(fixture.m_requests.size() == 1);
    fixture.m_bindings->get_binding(Operation::FLATTEN)->set(QKeySequence());
    fixture.key(Qt::Key_F, Qt::AltModifier);
    REQUIRE(fixture.m_requests.size() == 1);
    REQUIRE(fixture.find_button("Flatten position").isEnabled());
    clear(*rows);
    fixture.key(Qt::Key_F, Qt::ControlModifier | Qt::ShiftModifier);
    REQUIRE(fixture.m_requests.size() == 2);
    auto repeated = QKeyEvent(QEvent::KeyPress, Qt::Key_F,
      Qt::ControlModifier | Qt::ShiftModifier, QString(), true);
    QApplication::sendEvent(fixture.m_view.get(), &repeated);
    REQUIRE(fixture.m_requests.size() == 2);
  }
}
