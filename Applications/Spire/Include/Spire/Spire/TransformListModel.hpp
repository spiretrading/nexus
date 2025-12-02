#ifndef SPIRE_TRANSFORM_LIST_MODEL_HPP
#define SPIRE_TRANSFORM_LIST_MODEL_HPP
#include <concepts>
#include <type_traits>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/ListModelTransactionLog.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/TransformValueModel.hpp"

namespace Spire {

  /**
   * A ListModel that transforms the values of its source list model via a
   * callable.
   * @param <T> The type of values in the list.
   * @param <U> The type of values in the source model's list.
   * @param <F> The type of callable used to transform a value of type <i>U</i>
   *            into a value of type <i>T</i>.
   * @param <G> The type of callable used to invert <i>F</i>.
   */
  template<typename T, typename U, std::invocable<U> F,
    std::invocable<U, T> G> requires
      std::convertible_to<std::invoke_result_t<F, U>, T> &&
        std::convertible_to<std::invoke_result_t<G, U, T>, U>
  class TransformListModel : public ListModel<T> {
    public:

      /** The type of value to model. */
      using Source = U;

      using Type = typename ListModel<T>::Type;
      using OperationSignal = ListModel<T>::OperationSignal;
      using AddOperation = typename ListModel<T>::AddOperation;
      using MoveOperation = typename ListModel<T>::MoveOperation;
      using PreRemoveOperation = typename ListModel<T>::PreRemoveOperation;
      using RemoveOperation = typename ListModel<T>::RemoveOperation;
      using UpdateOperation = typename ListModel<T>::UpdateOperation;
      using StartTransaction = typename ListModel<T>::StartTransaction;
      using EndTransaction = typename ListModel<T>::EndTransaction;

      /**
       * Constructs a TransformListModel.
       * @param source The source model being transformed.
       * @param f The callable used to perform the transformation.
       */
      template<std::convertible_to<F> FF>
      TransformListModel(std::shared_ptr<ListModel<Source>> source, FF&& f);

      /**
       * Constructs a TransformListModel.
       * @param source The source model being transformed.
       * @param f The callable used to perform the transformation.
       * @param g The callable used to invert <i>f</i>.
       */
      template<std::convertible_to<F> FF, std::invocable<T> GG>
      TransformListModel(
        std::shared_ptr<ListModel<Source>> source, FF&& f, GG&& g);

      /**
       * Constructs a TransformListModel.
       * @param source The source model being transformed.
       * @param f The callable used to perform the transformation.
       * @param g The callable used to invert <i>f</i>.
       */
      template<std::convertible_to<F> FF, std::invocable<U, T> GG>
        requires std::constructible_from<G, GG>
      TransformListModel(
        std::shared_ptr<ListModel<Source>> source, FF&& f, GG&& g);

      int get_size() const override;
      const Type& get(int index) const override;
      QValidator::State set(int index, const Type& value) override;
      QValidator::State insert(const Type& value, int index) override;
      QValidator::State move(int source, int destination) override;
      QValidator::State remove(int index) override;
      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;
      using ListModel<T>::insert;
      using ListModel<T>::remove;
      using ListModel<T>::transact;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      std::shared_ptr<ListModel<Source>> m_source;
      mutable boost::optional<Type> m_last;
      mutable F m_f;
      mutable G m_g;
      ListModelTransactionLog<Type> m_transaction;
      boost::signals2::scoped_connection m_connection;

      void on_operation(const typename ListModel<Source>::Operation& operation);
  };

  template<typename T, typename F>
  TransformListModel(std::shared_ptr<T>, F&&) ->
    TransformListModel<std::invoke_result_t<F, typename T::Type>,
      typename T::Type, std::remove_reference_t<F>,
      Details::ThrowTransformInverter<typename T::Type>>;

  template<typename T, typename F,
    std::invocable<std::invoke_result_t<F, typename T::Type>> G>
  TransformListModel(std::shared_ptr<T>, F&&, G&&) ->
    TransformListModel<std::invoke_result_t<F, typename T::Type>,
      typename T::Type, std::remove_reference_t<F>,
      Details::TransformValueModelCollapser<std::remove_reference_t<G>>>;

  template<typename T, typename F, std::invocable<
    typename T::Type, std::invoke_result_t<F, typename T::Type>> G>
  TransformListModel(std::shared_ptr<T>, F&&, G&&) ->
    TransformListModel<std::invoke_result_t<F, typename T::Type>,
      typename T::Type, std::remove_reference_t<F>, std::remove_reference_t<G>>;

  template<typename T, std::invocable<typename T::Type> F>
  auto make_transform_list_model(std::shared_ptr<T> source, F&& f) {
    using Model =
      decltype(TransformListModel(std::move(source), std::forward<F>(f)));
    return std::make_shared<Model>(std::move(source), std::forward<F>(f));
  }

  template<typename T, std::invocable<typename T::Type> F,
    std::invocable<typename T::Type,
      std::invoke_result_t<F, typename T::Type>> G> requires
        std::convertible_to<std::invoke_result_t<F, typename T::Type>,
          std::invoke_result_t<F, typename T::Type>> &&
            std::convertible_to<std::invoke_result_t<
              G, typename T::Type, std::invoke_result_t<F, typename T::Type>>,
              typename T::Type>
  auto make_transform_list_model(std::shared_ptr<T> source, F&& f, G&& g) {
    using Model = decltype(TransformListModel(
      std::move(source), std::forward<F>(f), std::forward<G>(g)));
    return std::make_shared<Model>(
      std::move(source), std::forward<F>(f), std::forward<G>(g));
  }

  template<typename T, typename U, std::invocable<U> F,
    std::invocable<U, T> G> requires
      std::convertible_to<std::invoke_result_t<F, U>, T> &&
        std::convertible_to<std::invoke_result_t<G, U, T>, U>
  template<std::convertible_to<F> FF>
  TransformListModel<T, U, F, G>::TransformListModel(
    std::shared_ptr<ListModel<Source>> source, FF&& f)
    : TransformListModel(std::move(source), std::forward<FF>(f),
        Details::ThrowTransformInverter<Source>()) {}

  template<typename T, typename U, std::invocable<U> F,
    std::invocable<U, T> G> requires
      std::convertible_to<std::invoke_result_t<F, U>, T> &&
        std::convertible_to<std::invoke_result_t<G, U, T>, U>
  template<std::convertible_to<F> FF, std::invocable<T> GG>
  TransformListModel<T, U, F, G>::TransformListModel(
    std::shared_ptr<ListModel<Source>> source, FF&& f, GG&& g)
    : TransformListModel(std::move(source), std::forward<FF>(f),
        Details::TransformValueModelCollapser(std::forward<GG>(g))) {}

  template<typename T, typename U, std::invocable<U> F,
    std::invocable<U, T> G> requires
      std::convertible_to<std::invoke_result_t<F, U>, T> &&
        std::convertible_to<std::invoke_result_t<G, U, T>, U>
  template<std::convertible_to<F> FF, std::invocable<U, T> GG>
    requires std::constructible_from<G, GG>
  TransformListModel<T, U, F, G>::TransformListModel(
    std::shared_ptr<ListModel<Source>> source, FF&& f, GG&& g)
      : m_source(std::move(source)),
        m_f(std::forward<FF>(f)),
        m_g(std::forward<GG>(g)) {
    m_connection = m_source->connect_operation_signal(
      std::bind_front(&TransformListModel::on_operation, this));
  }

  template<typename T, typename U, std::invocable<U> F,
    std::invocable<U, T> G> requires
      std::convertible_to<std::invoke_result_t<F, U>, T> &&
        std::convertible_to<std::invoke_result_t<G, U, T>, U>
  int TransformListModel<T, U, F, G>::get_size() const {
    return m_source->get_size();
  }

  template<typename T, typename U, std::invocable<U> F,
    std::invocable<U, T> G> requires
      std::convertible_to<std::invoke_result_t<F, U>, T> &&
        std::convertible_to<std::invoke_result_t<G, U, T>, U>
  const typename TransformListModel<T, U, F, G>::Type&
      TransformListModel<T, U, F, G>::get(int index) const {
    m_last.emplace(m_f(m_source->get(index)));
    return *m_last;
  }

  template<typename T, typename U, std::invocable<U> F,
    std::invocable<U, T> G> requires
      std::convertible_to<std::invoke_result_t<F, U>, T> &&
        std::convertible_to<std::invoke_result_t<G, U, T>, U>
  QValidator::State TransformListModel<T, U, F, G>::set(
      int index, const Type& value) {
    try {
      return m_source->set(index, m_g(m_source->get(index), value));
    } catch(const std::invalid_argument&) {
      return QValidator::State::Invalid;
    }
  }

  template<typename T, typename U, std::invocable<U> F,
    std::invocable<U, T> G> requires
      std::convertible_to<std::invoke_result_t<F, U>, T> &&
        std::convertible_to<std::invoke_result_t<G, U, T>, U>
  QValidator::State TransformListModel<T, U, F, G>::insert(
      const Type& value, int index) {
    try {
      return m_source->insert(m_g(m_source->get(index), value), index);
    } catch(const std::invalid_argument&) {
      return QValidator::State::Invalid;
    }
  }

  template<typename T, typename U, std::invocable<U> F,
    std::invocable<U, T> G> requires
      std::convertible_to<std::invoke_result_t<F, U>, T> &&
        std::convertible_to<std::invoke_result_t<G, U, T>, U>
  QValidator::State TransformListModel<T, U, F, G>::move(
      int source, int destination) {
    return m_source->move(source, destination);
  }

  template<typename T, typename U, std::invocable<U> F,
    std::invocable<U, T> G> requires
      std::convertible_to<std::invoke_result_t<F, U>, T> &&
        std::convertible_to<std::invoke_result_t<G, U, T>, U>
  QValidator::State TransformListModel<T, U, F, G>::remove(int index) {
    return m_source->remove(index);
  }

  template<typename T, typename U, std::invocable<U> F,
    std::invocable<U, T> G> requires
      std::convertible_to<std::invoke_result_t<F, U>, T> &&
        std::convertible_to<std::invoke_result_t<G, U, T>, U>
  boost::signals2::connection
      TransformListModel<T, U, F, G>::connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const {
    return m_transaction.connect_operation_signal(slot);
  }

  template<typename T, typename U, std::invocable<U> F,
    std::invocable<U, T> G> requires
      std::convertible_to<std::invoke_result_t<F, U>, T> &&
        std::convertible_to<std::invoke_result_t<G, U, T>, U>
  void TransformListModel<T, U, F, G>::transact(
      const std::function<void ()>& transaction) {
    m_transaction.transact(transaction);
  }

  template<typename T, typename U, std::invocable<U> F,
    std::invocable<U, T> G> requires
      std::convertible_to<std::invoke_result_t<F, U>, T> &&
        std::convertible_to<std::invoke_result_t<G, U, T>, U>
  void TransformListModel<T, U, F, G>::on_operation(
      const typename ListModel<Source>::Operation& operation) {
    visit(operation,
      [&] (const typename ListModel<Source>::UpdateOperation& operation) {
        m_transaction.push(UpdateOperation(operation.m_index,
          m_f(operation.get_previous()), m_f(operation.get_value())));
      },
      [&] <typename T> (const T& operation) requires
          !std::is_same_v<T, typename ListModel<Source>::UpdateOperation> {
        m_transaction.push(operation);
      });
  }
}

#endif
