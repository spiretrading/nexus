#include <doctest/doctest.h>
#include <QApplication>
#include "Spire/BookView/AggregateBookViewModel.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/BookView/BookViewTableView.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/ColorConversion.hpp"
#include "Spire/Ui/TableBody.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using UserOrder = BookViewModel::UserOrder;

  struct Fixture {
    std::shared_ptr<BookViewModel> m_model;
    std::shared_ptr<LocalValueModel<BookViewProperties>> m_properties;
    std::unique_ptr<TableView> m_table_view;

    Fixture()
        : m_model(make_local_aggregate_book_view_model()),
          m_properties(
            make_local_value_model(BookViewProperties::get_default())),
          m_table_view(make_book_view_table_view(
            m_model, m_properties, Side::BID)) {
      m_table_view->resize(300, 600);
      m_table_view->show();
      QApplication::processEvents();
    }

    void push_quote(const std::string& mpid, Money price) {
      m_model->get_bids()->push(BookQuote(mpid, true, Venue("XTSE"),
        Quote(price, 100, Side::BID), second_clock::universal_time()));
      QApplication::processEvents();
    }

    QWidget& get_row(int row) {
      return *m_table_view->get_body().find_item(
        TableIndex(row, 0))->parentWidget();
    }

    QWidget& get_content(int row, int column) {
      return m_table_view->get_body().find_item(
        TableIndex(row, column))->get_body();
    }

    const QColor& get_level_color(int level) const {
      return m_properties->get().m_level_properties.m_color_scheme[level];
    }
  };

  auto get_background_color(QWidget& widget) {
    return find<BackgroundColor>(get_computed_block(widget));
  }

  auto get_text_color(QWidget& widget) {
    return find<TextColor>(get_computed_block(widget));
  }

  void require_level(Fixture& fixture, int row, int level) {
    auto& color = fixture.get_level_color(level);
    REQUIRE((get_background_color(fixture.get_row(row)) ==
      BackgroundColor(color)));
    for(auto column = 0; column != BOOK_VIEW_COLUMN_COUNT; ++column) {
      REQUIRE((get_text_color(fixture.get_content(row, column)) ==
        TextColor(apca_text_color(color))));
    }
  }
}

TEST_SUITE("BookViewTableView") {
  UI_TEST_CASE("price_level_rows") {
    auto fixture = Fixture();
    fixture.push_quote("A", Money(10));
    fixture.push_quote("B", Money(9));
    fixture.push_quote("C", Money(8));
    require_level(fixture, 0, 0);
    require_level(fixture, 1, 1);
    require_level(fixture, 2, 2);
    fixture.push_quote("D", Money(11));
    require_level(fixture, 0, 0);
    require_level(fixture, 1, 1);
    require_level(fixture, 2, 2);
    require_level(fixture, 3, 3);
  }

  UI_TEST_CASE("user_order_row") {
    auto fixture = Fixture();
    fixture.push_quote("A", Money(9));
    fixture.m_model->get_bid_orders()->push(
      UserOrder("TSX", Money(10), 100, OrderStatus::NEW));
    QApplication::processEvents();
    auto& active = get_highlight(fixture.m_properties->get(),
      BookViewHighlightProperties::OrderHighlightState::ACTIVE);
    REQUIRE((get_background_color(fixture.get_row(0)) ==
      BackgroundColor(active.m_background_color)));
    for(auto column = 0; column != BOOK_VIEW_COLUMN_COUNT; ++column) {
      REQUIRE((get_text_color(fixture.get_content(0, column)) ==
        TextColor(active.m_text_color)));
    }
  }

  UI_TEST_CASE("recycled_rows") {
    auto fixture = Fixture();
    for(auto i = 0; i != 6; ++i) {
      fixture.push_quote(std::to_string(i), Money(20 - i));
    }
    while(fixture.m_model->get_bids()->get_size() != 0) {
      fixture.m_model->get_bids()->remove(0);
      QApplication::processEvents();
    }
    fixture.push_quote("X", Money(5));
    fixture.push_quote("Y", Money(4));
    fixture.push_quote("Z", Money(3));
    require_level(fixture, 0, 0);
    require_level(fixture, 1, 1);
    require_level(fixture, 2, 2);
  }
}
