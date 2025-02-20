#include "Spire/BookView/MarketDepth.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QKeyEvent>
#include "Spire/BookView/BboBox.hpp"
#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  optional<int> find_nearest_order(int row, const TableModel& table) {
    auto top = row;
    auto bottom = row;
    while(top >= 0 || bottom < table.get_row_size()) {
      if(top >= 0 && is_order(get_mpid(table, top))) {
        return top;
      }
      if(bottom < table.get_row_size() && is_order(get_mpid(table, bottom))) {
        return bottom;
      }
      --top;
      ++bottom;
    }
    return none;
  }

  void navigate(int current_row, TableView& table_view,
      optional<int>& last_current_row) {
    auto set_current = [&] (int row) {
      table_view.get_current()->set(TableView::Index(row, 0));
      last_current_row = current_row;
      table_view.setFocus();
    };
    if(last_current_row) {
      set_current(*last_current_row);
    } else if(
        auto next = find_nearest_order(current_row, *table_view.get_table())) {
      set_current(*next);
    }
  }

  int find_book_quote(const ListModel<BookQuote>& quotes,
      const std::string& mpid, const Money& price) {
    auto i = std::find_if(quotes.begin(), quotes.end(),
      [&] (const BookQuote& quote) {
        return mpid == quote.m_mpid && price == quote.m_quote.m_price;
      });
    if(i == quotes.end()) {
      return -1;
    }
    return std::distance(quotes.begin(), i);
  }

  auto make_panel(std::shared_ptr<BookViewModel> model,
      std::shared_ptr<QuoteModel> bbo,
      std::shared_ptr<BookViewPropertiesModel> properties,
      const MarketDatabase& markets, Side side) {
    auto panel = new QWidget();
    panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto layout = make_vbox_layout(panel);
    auto bbo_box = new BboBox(std::move(bbo));
    bbo_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addWidget(bbo_box);
    auto table_view = make_book_view_table_view(std::move(model),
      std::move(properties), side, markets);
    table_view->setSizePolicy(QSizePolicy::Expanding,
     QSizePolicy::Expanding);
    layout->addWidget(table_view);
    return std::tuple(panel, bbo_box, table_view);
  }
}

MarketDepth::MarketDepth(std::shared_ptr<BookViewModel> model,
    std::shared_ptr<ValueModel<BboQuote>> bbo_quote,
    std::shared_ptr<BookViewPropertiesModel> properties,
    const MarketDatabase& markets, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_selected_quote(
        std::make_shared<LocalValueModel<optional<BookQuote>>>()),
      m_font_property(make_field_value_model(make_field_value_model(
        properties, &BookViewProperties::m_level_properties),
          &BookViewLevelProperties::m_font)),
      m_font(m_font_property->get()) {
  setFocusPolicy(Qt::StrongFocus);
  auto [bid_panel, bid_bbo, bid_table_view] = make_panel(m_model,
    make_field_value_model(bbo_quote, &BboQuote::m_bid), properties, markets,
    Side::BID);
  link(*this, *bid_bbo);
  m_bid_table_view = bid_table_view;
  m_bid_table_view->get_body().installEventFilter(this);
  m_bid_current_connection =
    m_bid_table_view->get_current()->connect_update_signal(
      std::bind_front(&MarketDepth::on_bid_current, this));
  auto [ask_panel, ask_bbo, ask_table_view] = make_panel(m_model,
    make_field_value_model(bbo_quote, &BboQuote::m_ask), properties, markets,
    Side::ASK);
  link(*this, *ask_bbo);
  m_ask_table_view = ask_table_view;
  m_ask_table_view->get_body().installEventFilter(this);
  m_ask_current_connection =
    m_ask_table_view->get_current()->connect_update_signal(
      std::bind_front(&MarketDepth::on_ask_current, this));
  auto layout = make_hbox_layout(this);
  layout->setSpacing(scale_width(2));
  layout->addWidget(bid_panel, 1);
  layout->addWidget(ask_panel, 1);
  auto& bid_vertical_scroll_bar =
    m_bid_table_view->get_scroll_box().get_vertical_scroll_bar();
  m_bid_position_connection = bid_vertical_scroll_bar.connect_position_signal(
    std::bind_front(&MarketDepth::on_bid_position, this));
  auto& ask_vertical_scroll_bar =
    m_ask_table_view->get_scroll_box().get_vertical_scroll_bar();
  m_ask_position_connection = ask_vertical_scroll_bar.connect_position_signal(
    std::bind_front(&MarketDepth::on_ask_position, this));
  m_bid_operation_connection =
    m_bid_table_view->get_table()->connect_operation_signal(
      std::bind_front(&MarketDepth::on_bid_operation, this));
  m_ask_operation_connection =
    m_ask_table_view->get_table()->connect_operation_signal(
      std::bind_front(&MarketDepth::on_ask_operation, this));
  m_font_property_connection = m_font_property->connect_update_signal(
    std::bind_front(&MarketDepth::on_font_property_update, this));
}

const std::shared_ptr<BookQuoteModel>&
    MarketDepth::get_selected_book_quote() const {
  return m_selected_quote;
}

bool MarketDepth::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    if(watched == &m_bid_table_view->get_body()) {
      if(key_event.key() == Qt::Key_Right) {
        if(auto current = m_bid_table_view->get_current()->get()) {
          navigate(current->m_row, *m_ask_table_view, m_last_current_row);
          return true;
        }
      }
    } else if(watched == &m_ask_table_view->get_body()) {
      if(key_event.key() == Qt::Key_Left) {
        if(auto current = m_ask_table_view->get_current()->get()) {
          navigate(current->m_row, *m_bid_table_view, m_last_current_row);
          return true;
        }
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void MarketDepth::focusInEvent(QFocusEvent* event) {
  if(event->reason() == Qt::ActiveWindowFocusReason &&
      !m_selected_quote->get() &&
      m_bid_table_view->get_table()->get_row_size() > 0) {
    if(auto next = find_nearest_order(0, *m_bid_table_view->get_table())) {
      m_bid_table_view->setFocus();
      m_bid_table_view->get_current()->set(TableView::Index(*next, 0));
    }
  }
}

void MarketDepth::on_side_current(const optional<TableView::Index>& current,
    Side side) {
  auto [last_side_current_row, last_side_opposite_row, table_view,
      opposite_table_view, opposite_connection, quotes] = [&] {
    if(side == Side::BID) {
      return std::tie(m_last_bid_current_row, m_last_ask_current_row,
        m_bid_table_view, m_ask_table_view, m_ask_current_connection,
        m_model->get_bids());
    }
    return std::tie(m_last_ask_current_row, m_last_bid_current_row,
      m_ask_table_view, m_bid_table_view, m_bid_current_connection,
      m_model->get_asks());
  }();
  if(current && last_side_current_row &&
      current->m_row == last_side_current_row) {
    return;
  }
  if(current) {
    last_side_current_row = current->m_row;
    m_last_current_row = none;
    auto blocker = shared_connection_block(opposite_connection);
    opposite_table_view->get_current()->set(none);
    last_side_opposite_row = none;
    auto& mpid = get_mpid(*table_view->get_table(), current->m_row);
    if(is_order(mpid)) {
      auto& price = get_price(*table_view->get_table(), current->m_row);
      if(auto i = find_book_quote(*quotes, mpid, price); i >= 0) {
        m_selected_quote->set(quotes->get(i));
        return;
      }
    }
  } else {
    last_side_current_row = none;
  }
  m_selected_quote->set(none);
  setFocus();
}

void MarketDepth::on_side_operation(const TableModel::Operation& operation,
    Side side) {
  auto table_view = [&] {
    if(side == Side::BID) {
      return m_bid_table_view;
    }
    return m_ask_table_view;
  }();
  if(table_view->get_current()->get()) {
    return;
  }
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      if(m_last_current_row && operation.m_index <= *m_last_current_row) {
        m_last_current_row = none;
      }
    },
    [&] (const TableModel::RemoveOperation& operation) {
      if(m_last_current_row && operation.m_index <= *m_last_current_row) {
        m_last_current_row = none;
      }
    },
    [&] (const TableModel::MoveOperation& operation) {
      if(m_last_current_row &&
          *m_last_current_row >=
            std::min(operation.m_source, operation.m_destination) &&
          *m_last_current_row <=
            std::max(operation.m_source, operation.m_destination)) {
        m_last_current_row = none;
      }
    });
}

void MarketDepth::on_bid_position(int position) {
  auto blocker = shared_connection_block(m_ask_position_connection);
  m_ask_table_view->get_scroll_box().get_vertical_scroll_bar().set_position(
    position);
}

void MarketDepth::on_ask_position(int position) {
  auto blocker = shared_connection_block(m_bid_position_connection);
  m_bid_table_view->get_scroll_box().get_vertical_scroll_bar().set_position(
    position);
}

void MarketDepth::on_bid_current(const optional<TableView::Index>& current) {
  on_side_current(current, Side::BID);
}

void MarketDepth::on_ask_current(const optional<TableView::Index>& current) {
  on_side_current(current, Side::ASK);
}

void MarketDepth::on_bid_operation(const TableModel::Operation& operation) {
  on_side_operation(operation, Side::BID);
}

void MarketDepth::on_ask_operation(const TableModel::Operation& operation) {
  on_side_operation(operation, Side::ASK);
}

void MarketDepth::on_font_property_update(const QFont& font) {
  if(m_font != font) {
    update_style(*this, [&] (auto& style) {
      style.get(Any() > is_a<BboBox>() > is_a<TextBox>()).set(Font(font));
    });
    m_font = font;
  }
}
