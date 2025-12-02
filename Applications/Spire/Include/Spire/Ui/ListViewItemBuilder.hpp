#ifndef SPIRE_LIST_VIEW_ITEM_BUILDER_HPP
#define SPIRE_LIST_VIEW_ITEM_BUILDER_HPP
#include <concepts>
#include <functional>
#include <memory>
#include <Beam/Pointers/LocalPtr.hpp>
#include <QWidget>
#include "Spire/Spire/ListModel.hpp"

namespace Spire {

  /**
   * Defines the requirements of a type compatible with the ListViewItemBuilder.
   */
  template<typename T, typename M>
  concept ListViewItemBuilderConcept = requires(T builder) {
    { builder.mount(std::shared_ptr<M>(), int()) };
    { builder.unmount(static_cast<QWidget*>(nullptr), int()) };
  };

  /**
   * Builder class used to build QWidget's used by a ListView to represent the
   * elements of a ListModel.
   * @param <T> The type of ListModel to mount QWidget's for.
   */
  template<typename T = AnyListModel>
  class ListViewItemBuilder {
    public:

      /** The type of ListModel to mount QWidget's for. */
      using ListModel = T;

      /**
       * Constructs a ListViewItemBuilder that calls a function to build a new
       * QWidget for every element, and calls delete to unmount widgets.
       * @param builder The function to call for every element of the list.
       */
      ListViewItemBuilder(std::invocable<const std::shared_ptr<ListModel>&, int>
        auto builder);

      /**
       * Constructs a ListViewItemBuilder that calls a function to build a new
       * QWidget for every element, and calls delete to unmount widgets.
       * @param view_builder The function to call for every element of the list.
       */
      template<typename M>
      ListViewItemBuilder(
        std::invocable<const std::shared_ptr<Spire::ListModel<M>>&, int> auto
          builder) requires std::is_same_v<T, AnyListModel>;

      /** Constructs a ListViewItemBuilder. */
      template<ListViewItemBuilderConcept<T> B>
      ListViewItemBuilder(B&& builder) requires(
        !std::is_same_v<std::remove_cvref_t<B>, ListViewItemBuilder>);

      /** Constructs a ListViewItemBuilder. */
      template<typename M>
      ListViewItemBuilder(ListViewItemBuilder<M> builder) requires(
        std::is_same_v<ListModel, AnyListModel> &&
          !std::is_same_v<M, AnyListModel>);

      ListViewItemBuilder(const ListViewItemBuilder&) = default;
      ListViewItemBuilder(ListViewItemBuilder&&) = default;

      /**
       * Returns the QWidget to represent a list element.
       * @param list The list containing the element to represent.
       * @param index The index of the element to represent.
       * @return The QWidget to use to represent the element.
       */
      QWidget* mount(const std::shared_ptr<ListModel>& list, int index);

      /**
       * Releases a previously mounted QWidget.
       * @param widget The QWidget to release.
       * @param index The index of the element the <i>widget</i> represented.
       */
      void unmount(QWidget* widget, int index);

      ListViewItemBuilder& operator =(const ListViewItemBuilder&) = default;
      ListViewItemBuilder& operator =(ListViewItemBuilder&&) = default;

    private:
      struct VirtualListViewItemBuilder {
        virtual ~VirtualListViewItemBuilder() = default;

        virtual QWidget* mount(
          const std::shared_ptr<ListModel>& list, int index) = 0;
        virtual void unmount(QWidget* widget, int index) = 0;
      };
      template<typename B>
      struct WrappedListViewItemBuilder final : VirtualListViewItemBuilder {
        using Builder = B;
        Beam::local_ptr_t<Builder> m_builder;

        template<typename... Args>
        WrappedListViewItemBuilder(Args&&... args);

        QWidget* mount(const std::shared_ptr<ListModel>& list, int index)
          override;
        void unmount(QWidget* widget, int index) override;
      };
      std::shared_ptr<VirtualListViewItemBuilder> m_builder;
  };

  /**
   * Implements a ListViewItemBuilder that mounts using an std::function and
   * calls delete to unmount.
   * @param <T> The type of ListModel to mount QWidget's for.
   */
  template<typename T>
  class FunctionListViewItemBuilder {
    public:

      /** The type of ListModel to mount QWidget's for. */
      using ListModel = T;

      /**
       * Constructs a FunctionListViewItemBuilder.
       * @param builder The function used to mount QWidgets.
       */
      FunctionListViewItemBuilder(
        std::function<QWidget* (const std::shared_ptr<ListModel>&, int)>
          builder);

      QWidget* mount(const std::shared_ptr<ListModel>& list, int index);
      void unmount(QWidget* widget, int index);

    private:
      std::function<QWidget* (const std::shared_ptr<ListModel>&, int)>
        m_builder;
  };

  /**
   * Implements a ListViewItemBuilder that converts from a ListModel<T> to an
   * AnyListModel.
   * @param <T> The type of ListModel to mount QWidget's for.
   */
  template<typename T>
  class AnyListViewItemBuilder {
    public:

      /** The type of ListModel to mount QWidget's for. */
      using ListModel = AnyListModel;

      /**
       * Constructs an AnyListViewItemBuilder.
       * @param builder The ListViewItemBuilder to convert.
       */
      AnyListViewItemBuilder(ListViewItemBuilder<T> builder);

      QWidget* mount(const std::shared_ptr<ListModel>& list, int index);
      void unmount(QWidget* widget, int index);

    private:
      ListViewItemBuilder<T> m_builder;
  };

  template<typename T>
  ListViewItemBuilder<T>::ListViewItemBuilder(std::invocable<
    const std::shared_ptr<ListModel>&, int> auto builder)
    : ListViewItemBuilder(FunctionListViewItemBuilder<ListModel>(
        std::move(builder))) {}

  template<typename T>
  template<typename M>
  ListViewItemBuilder<T>::ListViewItemBuilder(
    std::invocable<const std::shared_ptr<Spire::ListModel<M>>&, int> auto
      builder) requires std::is_same_v<T, AnyListModel>
    : ListViewItemBuilder([builder = std::move(builder)] (
          const std::shared_ptr<ListModel>& list, int index) {
        return builder(
          std::static_pointer_cast<Spire::ListModel<M>>(list), index);
      }) {}

  template<typename T>
  template<ListViewItemBuilderConcept<T> B>
  ListViewItemBuilder<T>::ListViewItemBuilder(B&& builder) requires(
    !std::is_same_v<std::remove_cvref_t<B>, ListViewItemBuilder>)
    : m_builder(std::make_shared<WrappedListViewItemBuilder<
        std::remove_cvref_t<B>>>(std::forward<B>(builder))) {}

  template<typename T>
  template<typename M>
  ListViewItemBuilder<T>::ListViewItemBuilder(ListViewItemBuilder<M> builder)
    requires(std::is_same_v<ListModel, AnyListModel> &&
      !std::is_same_v<M, AnyListModel>)
    : ListViewItemBuilder(AnyListViewItemBuilder(std::move(builder))) {}

  template<typename T>
  QWidget* ListViewItemBuilder<T>::mount(
      const std::shared_ptr<ListModel>& list, int index) {
    return m_builder->mount(list, index);
  }

  template<typename T>
  void ListViewItemBuilder<T>::unmount(QWidget* widget, int index) {
    m_builder->unmount(widget, index);
  }

  template<typename T>
  template<typename B>
  template<typename... Args>
  ListViewItemBuilder<T>::
      WrappedListViewItemBuilder<B>::WrappedListViewItemBuilder(Args&&... args)
    : m_builder(std::forward<Args>(args)...) {}

  template<typename T>
  template<typename B>
  QWidget* ListViewItemBuilder<T>::WrappedListViewItemBuilder<B>::mount(
      const std::shared_ptr<ListModel>& list, int index) {
    return m_builder->mount(list, index);
  }

  template<typename T>
  template<typename B>
  void ListViewItemBuilder<T>::WrappedListViewItemBuilder<B>::unmount(
      QWidget* widget, int index) {
    m_builder->unmount(widget, index);
  }

  template<typename T>
  FunctionListViewItemBuilder<T>::FunctionListViewItemBuilder(std::function<
    QWidget* (const std::shared_ptr<ListModel>&, int)> builder)
    : m_builder(std::move(builder)) {}

  template<typename T>
  QWidget* FunctionListViewItemBuilder<T>::mount(
      const std::shared_ptr<ListModel>& list, int index) {
    return m_builder(list, index);
  }

  template<typename T>
  void FunctionListViewItemBuilder<T>::unmount(QWidget* widget, int index) {
    delete widget;
  }

  template<typename T>
  AnyListViewItemBuilder<T>::AnyListViewItemBuilder(
    ListViewItemBuilder<T> builder)
    : m_builder(std::move(builder)) {}

  template<typename T>
  QWidget* AnyListViewItemBuilder<T>::mount(
      const std::shared_ptr<ListModel>& list, int index) {
    return m_builder.mount(std::static_pointer_cast<T>(list), index);
  }

  template<typename T>
  void AnyListViewItemBuilder<T>::unmount(QWidget* widget, int index) {
    m_builder.unmount(widget, index);
  }
}

#endif
