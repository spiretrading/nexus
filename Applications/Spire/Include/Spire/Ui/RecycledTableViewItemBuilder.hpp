#ifndef SPIRE_RECYCLED_TABLE_VIEW_ITEM_BUILDER_HPP
#define SPIRE_RECYCLED_TABLE_VIEW_ITEM_BUILDER_HPP
#include <deque>
#include <memory>
#include "Spire/Ui/TableViewItemBuilder.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a TableViewItemBuilder that reuses previously mounted QWidgets
   * to avoid constantly allocating and deallocating objects.
   * @param <B> The type used to initialize and reset QWidgets.
   */
  template<typename B>
  class RecycledTableViewItemBuilder {
    public:

      /** The type used to initialize and reset QWidgets. */
      using Builder = B;

      /**
       * Constructs a RecycledTableViewItemBuilder.
       * @param builder The ViewBuilder used to initialize and reset QWidgets.
       */
      explicit RecycledTableViewItemBuilder(Builder builder);

      ~RecycledTableViewItemBuilder();

      QWidget* mount(
        const std::shared_ptr<TableModel>& table, int row, int column);

      void unmount(QWidget* widget);

    private:
      Builder m_builder;
      std::vector<std::deque<QWidget*>> m_pool;
      std::unordered_map<QWidget*, int> m_columns;

      std::deque<QWidget*>& get_pool(int column);
  };

  template<typename B>
  RecycledTableViewItemBuilder<B>::RecycledTableViewItemBuilder(Builder builder)
    : m_builder(std::move(builder)) {}

  template<typename B>
  RecycledTableViewItemBuilder<B>::~RecycledTableViewItemBuilder() {
    for(auto& row : m_pool) {
      while(!row.empty()) {
        auto widget = row.front();
        row.pop_front();
        m_builder.unmount(widget);
      }
    }
  }

  template<typename B>
  QWidget* RecycledTableViewItemBuilder<B>::mount(
      const std::shared_ptr<TableModel>& table, int row, int column) {
    auto& pool = get_pool(column);
    if(pool.empty()) {
      auto widget = m_builder.mount(table, row, column);
      m_columns[widget] = column;
      return widget;
    } else {
      auto widget = pool.front();
      pool.pop_front();
      m_builder.reset(*widget, table, row, column);
      return widget;
    }
  }

  template<typename B>
  void RecycledTableViewItemBuilder<B>::unmount(QWidget* widget) {
    auto& pool = get_pool(m_columns[widget]);
    pool.push_back(widget);
  }

  template<typename B>
  std::deque<QWidget*>& RecycledTableViewItemBuilder<B>::get_pool(int column) {
    while(static_cast<int>(m_pool.size()) < column + 1) {
      m_pool.push_back({});
    }
    return m_pool[column];
  }
}

#endif
