#ifndef SPIRE_ORDER_TASK_ARGUMENTS_CONTENT_CACHE_HPP
#define SPIRE_ORDER_TASK_ARGUMENTS_CONTENT_CACHE_HPP
#include "Spire/KeyBindings/OrderTaskArguments.hpp"

namespace Spire {

  /** Caches the word content of each row in the OrderTaskArgumentsListModel. */
  class OrderTaskArgumentsContentCache {
    public:

      /**
       * Constructs an OrderTaskArgumentsContentCache.
       * @param model The list of OrderTaskArguments.
       * @param additional_tags The additional tag database to use.
       */
      OrderTaskArgumentsContentCache(
        std::shared_ptr<OrderTaskArgumentsListModel> model,
        AdditionalTagDatabase additional_tags);

      /**
       * Returns the word content of the row at the specified index.
       * @param index The index of the row to get.
       */
      const std::vector<QString>& get(int index);

    private:
      std::shared_ptr<OrderTaskArgumentsListModel> m_model;
      AdditionalTagDatabase m_additional_tags;
      std::unordered_map<int, std::vector<QString>> m_cache;
      boost::signals2::scoped_connection m_connection;

      OrderTaskArgumentsContentCache(
        const OrderTaskArgumentsContentCache&) = delete;
      OrderTaskArgumentsContentCache& operator =(
        const OrderTaskArgumentsContentCache&) = delete;
      void on_operation(
        const OrderTaskArgumentsListModel::Operation& operation);
  };
}

#endif
