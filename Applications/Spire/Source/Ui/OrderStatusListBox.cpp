#include "Spire/Ui/OrderStatusListBox.hpp"
#include <map>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/ListView.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  template<typename T>
  struct MultiValueQueryModel : QueryModel<T> {
    using Type = typename QueryModel<T>::Type;
    std::multimap<QString, Type> m_values;

    void add(const Type& value) {
      add(to_text(value).toLower(), value);
    }

    void add(const QString& id, const Type& value) {
      m_values.emplace(id.toLower(), value);
    }

    boost::optional<Type> parse(const QString& query) override {
      auto lower_query = query.toLower();
      auto range = m_values.equal_range(lower_query);
      for(auto i = range.first; i != range.second; ++i) {
        if(to_text(i->second).toLower() == lower_query) {
          return i->second;
        }
      }
      return boost::none;
    }

    QtPromise<std::vector<Type>> submit(const QString& query) override {
      auto matches = std::vector<Type>();
      auto lower_query = query.toLower();
      for(auto i = m_values.lower_bound(lower_query);
          i != m_values.end() && i->first.startsWith(lower_query); ++i) {
        if constexpr(requires(const Type& a, const Type& b) { a == b; }) {
          auto j = std::find(matches.begin(), matches.end(), i->second);
          if(j == matches.end()) {
            matches.push_back(i->second);
          }
        } else {
          matches.push_back(i->second);
        }
      }
      return QtPromise(std::move(matches));
    }
  };

  void add_order_status(
      MultiValueQueryModel<OrderStatus>& model, OrderStatus status) {
    model.add(status);
    auto words = to_text(status).split(' ', Qt::SkipEmptyParts);
    for(auto i = 1; i < words.size(); ++i) {
      model.add(words[i], status);
    }
  }
}

OrderStatusListBox* Spire::make_order_status_list_box(QWidget* parent) {
  return make_order_status_list_box(
    std::make_shared<ArrayListModel<OrderStatus>>(), parent);
}

OrderStatusListBox* Spire::make_order_status_list_box(
    std::shared_ptr<OrderStatusListModel> current, QWidget* parent) {
  auto query_model = std::make_shared<MultiValueQueryModel<OrderStatus>>();
  add_order_status(*query_model, OrderStatus::CANCELED);
  add_order_status(*query_model, OrderStatus::CANCEL_REJECT);
  add_order_status(*query_model, OrderStatus::DONE_FOR_DAY);
  add_order_status(*query_model, OrderStatus::EXPIRED);
  add_order_status(*query_model, OrderStatus::FILLED);
  add_order_status(*query_model, OrderStatus::NEW);
  add_order_status(*query_model, OrderStatus::PARTIALLY_FILLED);
  add_order_status(*query_model, OrderStatus::PENDING_CANCEL);
  add_order_status(*query_model, OrderStatus::PENDING_NEW);
  add_order_status(*query_model, OrderStatus::REJECTED);
  add_order_status(*query_model, OrderStatus::STOPPED);
  add_order_status(*query_model, OrderStatus::SUSPENDED);
  return new TagComboBox<OrderStatus>(std::move(query_model),
    std::move(current), &ListView::default_item_builder, parent);
}
