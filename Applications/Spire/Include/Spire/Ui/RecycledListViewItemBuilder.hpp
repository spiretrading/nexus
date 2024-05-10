#ifndef SPIRE_RECYCLED_LIST_VIEW_ITEM_BUILDER_HPP
#define SPIRE_RECYCLED_LIST_VIEW_ITEM_BUILDER_HPP
#include <deque>
#include <memory>
#include "Spire/Ui/ListViewItemBuilder.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a ListViewItemBuilder that reuses previously mounted QWidgets to
   * avoid constantly allocating and deallocating objects.
   * @param <B> The type used to initialize and reset QWidgets.
   */
  template<typename B>
  class RecycledListViewItemBuilder {
    public:

      /** The type used to initialize and reset QWidgets. */
      using Builder = B;

      /**
       * Constructs a RecycledListViewItemBuilder.
       * @param builder The ViewBuilder used to initialize and reset QWidgets.
       */
      explicit RecycledListViewItemBuilder(Builder builder);

      ~RecycledListViewItemBuilder();

      template<typename T>
      QWidget* mount(const std::shared_ptr<T>& list, int index);

      void unmount(QWidget* widget, int index);

    private:
      Builder m_builder;
      std::deque<QWidget*> m_pool;
  };

  template<typename B>
  RecycledListViewItemBuilder<B>::RecycledListViewItemBuilder(Builder builder)
    : m_builder(std::move(builder)) {}

  template<typename B>
  RecycledListViewItemBuilder<B>::~RecycledListViewItemBuilder() {
    while(!m_pool.empty()) {
      auto widget = m_pool.front();
      m_pool.pop_front();
      m_builder.unmount(widget);
    }
  }

  template<typename B>
  template<typename T>
  QWidget* RecycledListViewItemBuilder<B>::mount(
      const std::shared_ptr<T>& list, int index) {
    if(m_pool.empty()) {
      return m_builder.mount(list, index);
    } else {
      auto widget = m_pool.front();
      m_pool.pop_front();
      m_builder.reset(*widget, list, index);
      return widget;
    }
  }

  template<typename B>
  void RecycledListViewItemBuilder<B>::unmount(QWidget* widget, int index) {
    m_pool.push_back(widget);
  }
}

#endif
