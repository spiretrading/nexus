#ifndef SPIRE_RECYCLED_LIST_VIEW_BUILDER_HPP
#define SPIRE_RECYCLED_LIST_VIEW_BUILDER_HPP
#include <deque>
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Spire/OptionalScalarValueModelDecorator.hpp"
#include "Spire/Spire/ProxyScalarValueModel.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/ListViewBuilder.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a ListViewBuilder that recycles previously unmounted QWidgets.
   * @param <T> The type of ListModel to mount QWidget's for.
   */
  template<typename T>
  class RecycledListViewBuilder {
    public:

      /** The type of ListModel to mount QWidget's for. */
      using ListModel = T;

      QWidget* mount(const std::shared_ptr<ListModel>& model, int index);

      void unmount(QWidget* widget, int index);

    private:
      std::deque<QWidget*> m_pool;
  };

  template<typename T>
  QWidget* RecycledListViewBuilder<T>::mount(
      const std::shared_ptr<ListModel>& model, int index) {
    if(m_pool.empty()) {
      if constexpr(std::is_same_v<typename ListModel::Type, int>) {
        auto proxy = make_proxy_scalar_value_model(
          make_optional_scalar_value_model_decorator(
            make_scalar_value_model_decorator(
              make_list_value_model(model, index))));
        auto widget = new IntegerBox(std::move(proxy), {});
        return widget;
      }
    } else {
      auto widget = m_pool.front();
      m_pool.pop_front();
      if constexpr(std::is_same_v<typename ListModel::Type, int>) {
        static_cast<ProxyScalarValueModel<boost::optional<int>>*>(
          &*static_cast<IntegerBox*>(widget)->get_current())->set_source(
            make_optional_scalar_value_model_decorator(
              make_scalar_value_model_decorator(
                make_list_value_model(model, index))));
      }
      return widget;
    }
  }

  template<typename T>
  void RecycledListViewBuilder<T>::unmount(QWidget* widget, int index) {
    m_pool.push_back(widget);
  }
}

#endif
