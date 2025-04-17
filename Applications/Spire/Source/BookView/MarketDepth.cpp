#include "Spire/BookView/MarketDepth.hpp"
#include <QKeyEvent>
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/BookView/BboBox.hpp"
#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/BookView/BookViewTableView.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
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
    return std::tuple(panel, bbo_box, table_view);
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
  auto [bid_panel, bid_bbo, bid_table_view] =
    make_panel(m_model, properties, Side::BID);
  link(*this, *bid_bbo);
  m_bid_table_view = bid_table_view;
  m_bid_table_view->get_body().installEventFilter(this);
  auto [ask_panel, ask_bbo, ask_table_view] =
    make_panel(m_model, properties, Side::ASK);
  link(*this, *ask_bbo);
  m_ask_table_view = ask_table_view;
  m_ask_table_view->get_body().installEventFilter(this);
  m_current = std::make_shared<CurrentUserOrderModel>(
    std::static_pointer_cast<SortedTableModel>(m_bid_table_view->get_table()),
    m_bid_table_view->get_current(),
    std::static_pointer_cast<SortedTableModel>(m_ask_table_view->get_table()),
    m_ask_table_view->get_current());
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

const std::shared_ptr<CurrentUserOrderModel>& MarketDepth::get_current() const {
  return m_current;
}

bool MarketDepth::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    if(watched == &m_bid_table_view->get_body() &&
        key_event.key() == Qt::Key_Right) {
      m_current->navigate_to_asks();
      return true;
    } else if(watched == &m_ask_table_view->get_body() &&
        key_event.key() == Qt::Key_Left) {
      m_current->navigate_to_bids();
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
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
