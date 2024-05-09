#ifndef SPIRE_LIST_VIEW_BUILDER_HPP
#define SPIRE_LIST_VIEW_BUILDER_HPP
#include <concepts>
#include <functional>
#include <memory>
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Defines the requirements of a type compatible with the ListViewBuilder. */
  template<typename T, typename M>
  concept ListViewBuilderConcept = requires(T builder) {
    { builder.mount(std::shared_ptr<M>(), int()) };
    { builder.unmount(static_cast<QWidget*>(nullptr), int()) };
  };

  /**
   * Builder class used to build QWidget's used by a ListView to represent the
   * elements of a ListModel.
   * @param <T> The type of ListModel to mount QWidget's for.
   */
  template<typename T = AnyListModel>
  class ListViewBuilder {
    public:

      /** The type of ListModel to mount QWidget's for. */
      using ListModel = T;

      ListViewBuilder(const ListViewBuilder&) = default;

      ListViewBuilder(ListViewBuilder&&) = default;

      /**
       * Constructs a ListViewBuilder that calls a function to build a new
       * QWidget for every element, and calls delete to unmount widgets.
       * @param view_builder The function to call for every element of the list.
       */
      ListViewBuilder(std::invocable<const std::shared_ptr<ListModel>&, int>
        auto view_builder);

      /**
       * Constructs a ListViewBuilder that calls a function to build a new
       * QWidget for every element, and calls delete to unmount widgets.
       * @param view_builder The function to call for every element of the list.
       */
      template<typename M>
      ListViewBuilder(
          std::invocable<const std::shared_ptr<Spire::ListModel<M>>&, int>
            auto view_builder) requires std::is_same<ListModel, AnyListModel>
        : ListViewBuilder([view_builder = std::move(view_builder)] (
              const std::shared_ptr<ListModel>& model, int index) {
            return view_builder(
              std::static_pointer_cast<Spire::ListModel<M>>(model), index);
          }) {}

      /** Constructs a ListViewBuilder. */
      template<ListViewBuilderConcept<ListModel> B>
      ListViewBuilder(B&& view_builder) requires
        !std::is_same_v<std::remove_cvref_t<B>, ListViewBuilder>
        : m_builder(std::make_shared<WrappedListViewBuilder<
            std::remove_cvref_t<B>>>(std::forward<B>(view_builder))) {}

      /** Constructs a ListViewBuilder. */
      template<typename M>
      ListViewBuilder(ListViewBuilder<M> view_builder) requires
        std::is_same_v<ListModel, AnyListModel> &&
        !std::is_same_v<M, AnyListModel>;

      /**
       * Returns the QWidget to represent a list element.
       * @param model The list model containing the element to represent.
       * @param index The index of the element to represent.
       * @return The QWidget to use to represent the element.
       */
      QWidget* mount(const std::shared_ptr<ListModel>& model, int index);

      /**
       * Releases a previously mounted QWidget.
       * @param widget The QWidget to release.
       * @param index The index of the element the <i>widget</i> represented.
       */
      void unmount(QWidget* widget, int index);

      ListViewBuilder& operator =(const ListViewBuilder&) = default;

      ListViewBuilder& operator =(ListViewBuilder&&) = default;

    private:
      struct VirtualListViewBuilder {
        virtual ~VirtualListViewBuilder() = default;
        virtual QWidget* mount(
          const std::shared_ptr<ListModel>& model, int index) = 0;
        virtual void unmount(QWidget* widget, int index) = 0;
      };
      template<typename B>
      struct WrappedListViewBuilder final : VirtualListViewBuilder {
        using Builder = B;
        Beam::GetOptionalLocalPtr<Builder> m_builder;

        template<typename... Args>
        WrappedListViewBuilder(Args&&... args);
        QWidget* mount(const std::shared_ptr<ListModel>& model, int index)
          override;
        void unmount(QWidget* widget, int index) override;
      };
      std::shared_ptr<VirtualListViewBuilder> m_builder;
  };

  /**
   * Implements a ListViewBuilder that mounts using an std::function and calls
   * delete to unmount.
   * @param <T> The type of ListModel to mount QWidget's for.
   */
  template<typename T>
  class FunctionListViewBuilder {
    public:

      /** The type of ListModel to mount QWidget's for. */
      using ListModel = T;

      /**
       * Constructs a FunctionListViewBuilder.
       * @param view_builder The function used to mount QWidgets.
       */
      FunctionListViewBuilder(
        std::function<QWidget* (const std::shared_ptr<ListModel>&, int)>
          view_builder);

      QWidget* mount(const std::shared_ptr<ListModel>& model, int index);

      void unmount(QWidget* widget, int index);

    private:
      std::function<QWidget* (const std::shared_ptr<ListModel>&, int)>
        m_view_builder;
  };

  /**
   * Implements a ListViewBuilder that converts from a ListModel<T> to an
   * AnyListModel.
   * @param <T> The type of ListModel to mount QWidget's for.
   */
  template<typename T>
  class AnyListViewBuilder {
    public:

      /** The type of ListModel to mount QWidget's for. */
      using ListModel = AnyListModel;

      /**
       * Constructs an AnyListViewBuilder.
       * @param view_builder The ListViewBuilder to convert.
       */
      AnyListViewBuilder(ListViewBuilder<T> view_builder);

      QWidget* mount(const std::shared_ptr<ListModel>& model, int index);

      void unmount(QWidget* widget, int index);

    private:
      ListViewBuilder<T> m_view_builder;
  };

  template<typename T>
  ListViewBuilder<T>::ListViewBuilder(std::invocable<
    const std::shared_ptr<ListModel>&, int> auto view_builder)
    : ListViewBuilder(FunctionListViewBuilder<ListModel>(
        std::move(view_builder))) {}

  template<typename T>
  template<typename M>
  ListViewBuilder<T>::ListViewBuilder(ListViewBuilder<M> view_builder) requires
    std::is_same_v<ListModel, AnyListModel> &&
    !std::is_same_v<M, AnyListModel>
    : ListViewBuilder(AnyListViewBuilder(std::move(view_builder))) {}

  template<typename T>
  QWidget* ListViewBuilder<T>::mount(
      const std::shared_ptr<ListModel>& model, int index) {
    return m_builder->mount(model, index);
  }

  template<typename T>
  void ListViewBuilder<T>::unmount(QWidget* widget, int index) {
    m_builder->unmount(widget, index);
  }

  template<typename T>
  template<typename B>
  template<typename... Args>
  ListViewBuilder<T>::WrappedListViewBuilder<B>::WrappedListViewBuilder(
    Args&&... args)
    : m_builder(std::forward<Args>(args)...) {}

  template<typename T>
  template<typename B>
  QWidget* ListViewBuilder<T>::WrappedListViewBuilder<B>::mount(
      const std::shared_ptr<ListModel>& model, int index) {
    return m_builder->mount(model, index);
  }

  template<typename T>
  template<typename B>
  void ListViewBuilder<T>::WrappedListViewBuilder<B>::unmount(
      QWidget* widget, int index) {
    m_builder->unmount(widget, index);
  }

  template<typename T>
  FunctionListViewBuilder<T>::FunctionListViewBuilder(std::function<
    QWidget* (const std::shared_ptr<ListModel>&, int)> view_builder)
    : m_view_builder(std::move(view_builder)) {}

  template<typename T>
  QWidget* FunctionListViewBuilder<T>::mount(
      const std::shared_ptr<ListModel>& model, int index) {
    return m_view_builder(model, index);
  }

  template<typename T>
  void FunctionListViewBuilder<T>::unmount(QWidget* widget, int index) {
    delete widget;
  }

  template<typename T>
  AnyListViewBuilder<T>::AnyListViewBuilder(ListViewBuilder<T> view_builder)
    : m_view_builder(std::move(view_builder)) {}

  template<typename T>
  QWidget* AnyListViewBuilder<T>::mount(
      const std::shared_ptr<ListModel>& model, int index) {
    return m_view_builder.mount(std::static_pointer_cast<T>(model), index);
  }

  template<typename T>
  void AnyListViewBuilder<T>::unmount(QWidget* widget, int index) {
    m_view_builder.unmount(widget, index);
  }
}

#endif
