#include "Spire/Blotter/OrderLogView.hpp"
#include "Spire/Spire/ListToTableModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  enum class OrderColumn {
    TIME,
    ID,
    STATUS,
    SECURITY,
    CURRENCY,
    TYPE,
    SIDE,
    DESTINATION,
    QUANTITY,
    PRICE,
    TIME_IN_FORCE
  };

  AnyRef extract(const std::shared_ptr<Order>& order, int index) {
    auto column = static_cast<OrderColumn>(index);
    auto& info = order->get_info();
    auto& fields = info.m_fields;
    if(column == OrderColumn::ID) {
      return info.m_id;
    } else if(column == OrderColumn::SECURITY) {
      return fields.m_ticker;
    } else if(column == OrderColumn::CURRENCY) {
      return fields.m_currency;
    } else if(column == OrderColumn::TYPE) {
      return fields.m_type;
    } else if(column == OrderColumn::SIDE) {
      return fields.m_side;
    } else if(column == OrderColumn::DESTINATION) {
      return fields.m_destination;
    } else if(column == OrderColumn::QUANTITY) {
      return fields.m_quantity;
    } else if(column == OrderColumn::PRICE) {
      if(fields.m_type == OrderType::LIMIT) {
        return fields.m_price;
      }
    } else if(column == OrderColumn::TIME_IN_FORCE) {
      return fields.m_time_in_force;
    }
    return QString();
  }
}

OrderLogView::OrderLogView(std::shared_ptr<OrdersModel> orders, QWidget* parent)
    : QWidget(parent),
      m_orders(std::move(orders)) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto column_count = static_cast<int>(OrderColumn::TIME_IN_FORCE) + 1;
  auto table = std::make_shared<ListToTableModel<OrdersModel::Type>>(
    m_orders, column_count, &extract);
  auto table_view = TableViewBuilder(table).
    add_header_item(tr("Time")).
    add_header_item(tr("ID")).
    add_header_item(tr("Status")).
    add_header_item(tr("Security")).
    add_header_item(tr("Currency"), tr("Curr")).
    add_header_item(tr("Type")).
    add_header_item(tr("Side")).
    add_header_item(tr("Destination"), tr("Dest")).
    add_header_item(tr("Quantity"), tr("Qty")).
    add_header_item(tr("Price"), tr("Px")).
    add_header_item(tr("Time In Force"), tr("TIF")).
    make();
  table_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  enclose(*this, *table_view);
  setFocusProxy(table_view);
}

const std::shared_ptr<OrdersModel>& OrderLogView::get_orders() const {
  return m_orders;
}
