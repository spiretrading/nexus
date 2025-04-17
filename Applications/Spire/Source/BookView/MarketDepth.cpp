#include "Spire/BookView/MarketDepth.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/BookView/BboBox.hpp"
#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/BookView/BookViewTableView.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/ListToTableModel.hpp"
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
  struct CurrentModel : ValueModel<optional<BookViewModel::UserOrder>> {
    std::shared_ptr<SortedTableModel> m_table;
    std::shared_ptr<ValueModel<optional<TableIndex>>> m_table_current;
    LocalValueModel<optional<BookViewModel::UserOrder>> m_current;
    scoped_connection m_connection;

    CurrentModel(std::shared_ptr<SortedTableModel> table,
        std::shared_ptr<ValueModel<optional<TableIndex>>> table_current)
        : m_table(std::move(table)),
          m_table_current(std::move(table_current)) {
      m_connection = m_table_current->connect_update_signal(
        std::bind_front(&CurrentModel::on_current, this));
    }

    const Type& get() const override {
      return m_current.get();
    }

    QValidator::State test(const Type& value) const override {
      return m_current.test(value);
    }

    QValidator::State set(const Type& value) override {
      return m_current.set(value);
    }

    connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override {
      return m_current.connect_update_signal(slot);
    }

    void on_current(const optional<TableIndex>& current) {
      if(!current) {
        m_current.set(none);
        return;
      }
      auto source_row = m_table->index_to_source(current->m_row);
      auto source_list = static_cast<const ListToTableModel<BookEntry>&>(
        *m_table->get_source().get()).get_source();
      auto& entry = source_list->get(source_row);
      if(auto user_order = boost::get<BookViewModel::UserOrder>(&entry)) {
        m_current.set(*user_order);
      } else {
        m_current.set(none);
      }
    }
  };

  struct ConsolidatedCurrentModel : MarketDepth::CurrentUserOrderModel {
    std::shared_ptr<ValueModel<optional<BookViewModel::UserOrder>>>
      m_current_bid;
    std::shared_ptr<ValueModel<optional<BookViewModel::UserOrder>>>
      m_current_ask;
    LocalValueModel<optional<MarketDepth::CurrentUserOrder>> m_current;
    scoped_connection m_bid_connection;
    scoped_connection m_ask_connection;

    ConsolidatedCurrentModel(
        std::shared_ptr<ValueModel<optional<BookViewModel::UserOrder>>>
          current_bid,
        std::shared_ptr<ValueModel<optional<BookViewModel::UserOrder>>>
          current_ask)
        : m_current_bid(std::move(current_bid)),
          m_current_ask(std::move(current_ask)) {
      m_bid_connection = m_current_bid->connect_update_signal(
        std::bind_front(&ConsolidatedCurrentModel::on_bid, this));
      m_ask_connection = m_current_bid->connect_update_signal(
        std::bind_front(&ConsolidatedCurrentModel::on_ask, this));
    }

    const Type& get() const override {
      return m_current.get();
    }

    QValidator::State test(const Type& value) const override {
      return m_current.test(value);
    }

    QValidator::State set(const Type& value) override {
      return m_current.set(value);
    }

    connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override {
      return m_current.connect_update_signal(slot);
    }

    void on_bid(const optional<BookViewModel::UserOrder>& user_order) {
      if(user_order) {
        m_current.set(MarketDepth::CurrentUserOrder(*user_order, Side::BID));
      } else if(auto& current = m_current_ask->get()) {
        m_current.set(MarketDepth::CurrentUserOrder(*current, Side::ASK));
      }
    }

    void on_ask(const optional<BookViewModel::UserOrder>& user_order) {
      if(user_order) {
        m_current.set(MarketDepth::CurrentUserOrder(*user_order, Side::ASK));
      } else if(auto& current = m_current_bid->get()) {
        m_current.set(MarketDepth::CurrentUserOrder(*current, Side::BID));
      }
    }
  };

  auto make_panel(std::shared_ptr<BookViewModel> model,
      std::shared_ptr<BookViewPropertiesModel> properties, Side side) {
    auto panel = new QWidget();
    panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto layout = make_vbox_layout(panel);
    auto bbo_accessor = [&] {
      if(side == Side::BID) {
        return &BboQuote::m_bid;
      }
      return &BboQuote::m_ask;
    }();
    auto bbo = make_field_value_model(model->get_bbo_quote(), bbo_accessor);
    auto bbo_box = new BboBox(std::move(bbo));
    bbo_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addWidget(bbo_box);
    auto table_view =
      make_book_view_table_view(std::move(model), std::move(properties), side);
    table_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(table_view);
    auto table =
      std::static_pointer_cast<SortedTableModel>(table_view->get_table());
    auto current = table_view->get_current();
    auto current_user_order = std::make_shared<CurrentModel>(
      std::move(table), std::move(current));
    return std::tuple(panel, bbo_box, table_view, current_user_order);
  }
}

MarketDepth::MarketDepth(std::shared_ptr<BookViewModel> model,
    std::shared_ptr<BookViewPropertiesModel> properties, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_font_property(make_field_value_model(make_field_value_model(
        properties, &BookViewProperties::m_level_properties),
          &BookViewLevelProperties::m_font)),
      m_font(m_font_property->get()) {
  setFocusPolicy(Qt::StrongFocus);
  auto [bid_panel, bid_bbo, bid_table_view, current_bid] =
    make_panel(m_model, properties, Side::BID);
  link(*this, *bid_bbo);
  m_bid_table_view = bid_table_view;
  auto [ask_panel, ask_bbo, ask_table_view, current_ask] =
    make_panel(m_model, properties, Side::ASK);
  link(*this, *ask_bbo);
  m_ask_table_view = ask_table_view;
  m_current = std::make_shared<ConsolidatedCurrentModel>(
    std::move(current_bid), std::move(current_ask));
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

const std::shared_ptr<MarketDepth::CurrentUserOrderModel>&
    MarketDepth::get_current() const {
  return m_current;
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

void MarketDepth::on_font_property_update(const QFont& font) {
  if(m_font != font) {
    update_style(*this, [&] (auto& style) {
      style.get(Any() > is_a<BboBox>() > is_a<TextBox>()).set(Font(font));
    });
    m_font = font;
  }
}
