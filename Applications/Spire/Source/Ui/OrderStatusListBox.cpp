#include "Spire/Ui/OrderStatusListBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  struct OrderStatusQueryModel : QueryModel<OrderStatus> {
    LocalQueryModel<OrderStatus> m_statuses;
    LocalQueryModel<OrderStatus> m_terms;

    void add(OrderStatus status) {
      m_statuses.add(status);
      auto words = to_text(status).split(' ', Qt::SkipEmptyParts);
      for(auto i = 1; i < words.size(); ++i) {
        m_terms.add(words[i], status);
      }
    }

    optional<OrderStatus> parse(const QString& query) override {
      return m_statuses.parse(query);
    }

    QtPromise<std::vector<OrderStatus>>
        submit(const QString& query) override {
      return m_statuses.submit(query).then([=] (auto&& source_result) {
        auto matches = source_result.get();
        auto term_matches = wait(m_terms.submit(query));
        for(auto& value : term_matches) {
          if(std::find(matches.begin(), matches.end(), value) ==
              matches.end()) {
            matches.push_back(value);
          }
        }
        return matches;
      });
    }
  };
}

OrderStatusListBox* Spire::make_order_status_list_box(QWidget* parent) {
  return make_order_status_list_box(
    std::make_shared<ArrayListModel<OrderStatus>>(), parent);
}

OrderStatusListBox* Spire::make_order_status_list_box(
    std::shared_ptr<OrderStatusListModel> current, QWidget* parent) {
  auto query_model = std::make_shared<OrderStatusQueryModel>();
  query_model->add(OrderStatus::CANCELED);
  query_model->add(OrderStatus::CANCEL_REJECT);
  query_model->add(OrderStatus::DONE_FOR_DAY);
  query_model->add(OrderStatus::EXPIRED);
  query_model->add(OrderStatus::FILLED);
  query_model->add(OrderStatus::NEW);
  query_model->add(OrderStatus::PARTIALLY_FILLED);
  query_model->add(OrderStatus::PENDING_CANCEL);
  query_model->add(OrderStatus::PENDING_NEW);
  query_model->add(OrderStatus::REJECTED);
  query_model->add(OrderStatus::STOPPED);
  query_model->add(OrderStatus::SUSPENDED);
  return new TagComboBox<OrderStatus>(std::move(query_model),
    std::move(current), &ListView::default_item_builder, parent);
}
