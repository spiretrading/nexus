#ifndef SPIRE_RECYCLED_LIST_VIEW_BUILDER_HPP
#define SPIRE_RECYCLED_LIST_VIEW_BUILDER_HPP
#include <deque>
#include <memory>
#include "Spire/Ui/ListViewBuilder.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a ListViewBuilder that reuses previously mounted QWidgets to
   * avoid constantly allocating and deallocating objects.
   * @param <B> The type of ViewBuilder used to initialize and reset QWidgets.
   */
  template<typename B>
  class RecycledListViewBuilder {
    public:

      /** The type of ViewBuilder used to initialize and reset QWidgets. */
      using ViewBuilder = B;

      /**
       * Constructs a RecycledListViewBuilder.
       * @param view_builder The ViewBuilder used to initialize and reset
       *        QWidgets.
       */
      explicit RecycledListViewBuilder(ViewBuilder view_builder);

      ~RecycledListViewBuilder();

      template<typename T>
      QWidget* mount(const std::shared_ptr<T>& model, int index);

      void unmount(QWidget* widget, int index);

    private:
      ViewBuilder m_view_builder;
      std::deque<QWidget*> m_pool;
  };

  template<typename B>
  RecycledListViewBuilder<B>::RecycledListViewBuilder(ViewBuilder view_builder)
    : m_view_builder(std::move(view_builder)) {}

  template<typename B>
  RecycledListViewBuilder<B>::~RecycledListViewBuilder() {
    while(!m_pool.empty()) {
      auto widget = m_pool.front();
      m_pool.pop_front();
      m_view_builder.unmount(widget);
    }
  }

  template<typename B>
  template<typename T>
  QWidget* RecycledListViewBuilder<B>::mount(
      const std::shared_ptr<T>& model, int index) {
    if(m_pool.empty()) {
      return m_view_builder.mount(model, index);
    } else {
      auto widget = m_pool.front();
      m_pool.pop_front();
      m_view_builder.reset(*widget, model, index);
      return widget;
    }
  }

  template<typename B>
  void RecycledListViewBuilder<B>::unmount(QWidget* widget, int index) {
    m_pool.push_back(widget);
  }
}

#endif
