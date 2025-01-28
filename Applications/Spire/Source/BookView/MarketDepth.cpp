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
  const auto& get_mpid(const TableModel& table, int row) {
    return table.get<std::string>(row, static_cast<int>(BookViewColumns::MPID));
  }

  const auto& get_price(const TableModel& table, int row) {
    return table.get<Money>(row, static_cast<int>(BookViewColumns::PRICE));
  }

  auto is_order(const TableModel& table, int row) {
    return get_mpid(table, row).front() == '@';
  }

  optional<int> find_nearest_order(int row, const TableModel& table) {
    auto top = row;
    auto bottom = row;
    while(top >= 0 || bottom < table.get_row_size()) {
      if(top >= 0 && is_order(table, top)) {
        return top;
      }
      if(bottom < table.get_row_size() && is_order(table, bottom)) {
        return bottom;
      }
      --top;
      ++bottom;
    }
    return none;
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
  auto bid_panel = new QWidget();
  bid_panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  auto bid_layout = make_vbox_layout(bid_panel);
  auto bid_bbo =
    new BboBox(make_field_value_model(bbo_quote, &BboQuote::m_bid));
  bid_bbo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  bid_layout->addWidget(bid_bbo);
  link(*this, *bid_bbo);
  m_bid_table_view = make_book_view_table_view(m_model, properties,
    Side::BID, markets);
  m_bid_table_view->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  m_bid_table_view->get_body().installEventFilter(this);
  m_bid_current_connection =
    m_bid_table_view->get_current()->connect_update_signal(
      std::bind_front(&MarketDepth::on_bid_current, this));
  bid_layout->addWidget(m_bid_table_view);
  auto ask_panel = new QWidget();
  ask_panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  auto ask_layout = make_vbox_layout(ask_panel);
  auto ask_bbo =
    new BboBox(make_field_value_model(bbo_quote, &BboQuote::m_ask));
  ask_bbo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  ask_layout->addWidget(ask_bbo);
  link(*this, *ask_bbo);
  m_ask_table_view = make_book_view_table_view(m_model, properties,
    Side::ASK, markets);
  m_ask_table_view->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  m_ask_table_view->get_body().installEventFilter(this);
  m_ask_current_connection =
    m_ask_table_view->get_current()->connect_update_signal(
      std::bind_front(&MarketDepth::on_ask_current, this));
  ask_layout->addWidget(m_ask_table_view);
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
          if(auto next = find_nearest_order(current->m_row,
              *m_ask_table_view->get_table())) {
            m_ask_table_view->get_current()->set(TableView::Index(*next, 0));
            m_ask_table_view->setFocus();
          }
          return true;
        }
      }
    } else if(watched == &m_ask_table_view->get_body()) {
      if(key_event.key() == Qt::Key_Left) {
        if(auto current = m_ask_table_view->get_current()->get()) {
          if(auto next = find_nearest_order(current->m_row,
              *m_bid_table_view->get_table())) {
            m_bid_table_view->get_current()->set(TableView::Index(*next, 0));
            m_bid_table_view->setFocus();
          }
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
  if(current) {
    auto blocker = shared_connection_block(m_ask_current_connection);
    m_ask_table_view->get_current()->set(none);
    if(is_order(*m_bid_table_view->get_table(), current->m_row)) {
      auto& mpid = get_mpid(*m_bid_table_view->get_table(), current->m_row);
      auto& price = get_price(*m_bid_table_view->get_table(), current->m_row);
      if(auto i = find_book_quote(*m_model->get_bids(), mpid, price); i >= 0) {
        m_selected_quote->set(m_model->get_bids()->get(i));
        return;
      }
    }
  }
  m_selected_quote->set(none);
  setFocus();
}

void MarketDepth::on_ask_current(const optional<TableView::Index>& current) {
  if(current) {
    auto blocker = shared_connection_block(m_bid_current_connection);
    m_bid_table_view->get_current()->set(none);
    if(is_order(*m_ask_table_view->get_table(), current->m_row)) {
      auto& mpid = get_mpid(*m_ask_table_view->get_table(), current->m_row);
      auto& price = get_price(*m_ask_table_view->get_table(), current->m_row);
      if(auto i = find_book_quote(*m_model->get_asks(), mpid, price); i >= 0) {
        m_selected_quote->set(m_model->get_asks()->get(i));
        return;
      }
    }
  }
  m_selected_quote->set(none);
  setFocus();
}

void MarketDepth::on_font_property_update(const QFont& font) {
  if(m_font != font) {
    update_style(*this, [&] (auto& style) {
      style.get(Any() > is_a<BboBox>() > is_a<TextBox>()).
        set(Font(font));
    });
    m_font = font;
  }
}
