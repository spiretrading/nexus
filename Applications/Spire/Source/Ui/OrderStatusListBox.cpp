#include "Spire/Ui/OrderStatusListBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/Ui/ListView.hpp"

using namespace Nexus;
using namespace Spire;

OrderStatusListBox* Spire::make_order_status_list_box(QWidget* parent) {
  return make_order_status_list_box(
    std::make_shared<ArrayListModel<OrderStatus>>(), parent);
}

OrderStatusListBox* Spire::make_order_status_list_box(
    std::shared_ptr<OrderStatusListModel> current, QWidget* parent) {
  auto query_model = std::make_shared<LocalQueryModel<OrderStatus>>();
  query_model->add(OrderStatus::PENDING_NEW);
  query_model->add(OrderStatus::REJECTED);
  query_model->add(OrderStatus::NEW);
  query_model->add(OrderStatus::PARTIALLY_FILLED);
  query_model->add(OrderStatus::EXPIRED);
  query_model->add(OrderStatus::CANCELED);
  query_model->add(OrderStatus::SUSPENDED);
  query_model->add(OrderStatus::STOPPED);
  query_model->add(OrderStatus::FILLED);
  query_model->add(OrderStatus::DONE_FOR_DAY);
  query_model->add(OrderStatus::PENDING_CANCEL);
  query_model->add(OrderStatus::CANCEL_REJECT);
  return new TagComboBox<OrderStatus>(std::move(query_model),
    std::move(current), &ListView::default_item_builder, parent);
}
