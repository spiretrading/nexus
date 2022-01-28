#ifndef SPIRE_LIST_MODEL_HPP
#define SPIRE_LIST_MODEL_HPP
#include <any>
#include <type_traits>
#include <utility>
#include <vector>
#include <boost/mpl/advance.hpp>
#include <boost/mpl/begin_end.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/size.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <QValidator>
#include "Spire/Spire/Spire.hpp"

namespace Spire {
namespace Details {
  template<typename O>
  struct ModelTransaction {
    std::vector<O> m_operations;
  };

  template<typename M, typename F>
  void visit(const typename M::Operation& operation, F&& f) {
    static_assert(std::is_invocable_v<F, const typename M::AddOperation&> ||
      std::is_invocable_v<F, const typename M::RemoveOperation&> ||
      std::is_invocable_v<F, const typename M::MoveOperation&> ||
      std::is_invocable_v<F, const typename M::UpdateOperation&>);
    if(auto transaction = boost::get<typename M::Transaction>(&operation)) {
      for(auto& transaction_operation : transaction->m_operations) {
        visit<M>(transaction_operation, std::forward<F>(f));
      }
    } else {
      boost::apply_visitor([&] (const auto& operation) {
        using Parameter = std::decay_t<decltype(operation)>;
        if constexpr(std::is_invocable_v<F, Parameter>) {
          std::forward<F>(f)(operation);
        }
      }, operation);
    }
  }

  template<typename M, typename F, typename... G>
  typename std::enable_if_t<(sizeof...(G) != 0)>
  visit(const typename M::Operation& operation, F&& f, G&&... g) {
    static_assert(std::is_invocable_v<F, const typename M::AddOperation&> ||
      std::is_invocable_v<F, const typename M::RemoveOperation&> ||
      std::is_invocable_v<F, const typename M::MoveOperation&> ||
      std::is_invocable_v<F, const typename M::UpdateOperation&>);
    if(auto transaction = boost::get<typename M::Transaction>(&operation)) {
      for(auto& transaction_operation : transaction->m_operations) {
        visit<M>(
          transaction_operation, std::forward<F>(f), std::forward<G>(g)...);
      }
    } else {
      auto is_visited = boost::apply_visitor([&] (const auto& operation) {
        using Parameter = std::decay_t<decltype(operation)>;
        if constexpr(std::is_invocable_v<F, Parameter>) {
          std::forward<F>(f)(operation);
          return true;
        }
        return false;
      }, operation);
      if(!is_visited) {
        visit<M>(operation, std::forward<G>(g)...);
      }
    }
  }
}

  /** Base class used to model a list of values. */
  class AnyListModel {
    public:

      /** Indicates a value was added to the model. */
      struct AddOperation {

        /** The index where the value was inserted. */
        int m_index;
      };

      /** Indicates a value was removed from the model. */
      struct RemoveOperation {

        /** The index of the value removed. */
        int m_index;
      };

      /** Indicates a value was moved from one index to another. */
      struct MoveOperation {

        /** The index of the value that was moved. */
        int m_source;

        /** The index that the value was moved to. */
        int m_destination;
      };

      /** Indicates a value was updated. */
      struct UpdateOperation {

        /** The index of the updated value. */
        int m_index;
      };

      /** Consolidates all basic operations. */
      using Operation = typename boost::make_recursive_variant<AddOperation,
        RemoveOperation, MoveOperation, UpdateOperation,
        Details::ModelTransaction<boost::recursive_variant_>>::type;

      /**
       * An operation consisting of a list of sub-operations performed as single
       * transaction.
       */
      using Transaction = typename boost::mpl::deref<
        typename boost::mpl::advance<
          typename boost::mpl::begin<Operation::types>::type,
          boost::mpl::int_<
            boost::mpl::size<Operation::types>::value - 1>>::type>::type;

      /**
       * Signals an operation was applied to this model.
       * @param operation The operation that was applied.
       */
      using OperationSignal = Signal<void (const Operation&)>;

      virtual ~AnyListModel() = default;

      /** Returns the number of items in the model. */
      virtual int get_size() const = 0;

      /**
       * Returns the value at a specified index.
       * @throws <code>std::out_of_range</code> iff index is out of range.
       */
      std::any get(int index) const;

      /**
       * Sets the value at a specified index.
       * @param index - The index to set.
       * @param value - The value to set at the specified index.
       * @return The state of the value at the <i>index</i>, or
       *         <code>QValidator::State::Invalid</code> iff row or column is
       *         out of range.
       */
      virtual QValidator::State set(int index, const std::any& value) = 0;

      /**
       * Appends a value.
       * @param value The value to append to this model.
       * @return <code>QValidator::State::Acceptable</code> if the model
       *         supports the operation, <code>QValidator::State::Invalid</code>
       *         otherwise.
       */
      virtual QValidator::State push(const std::any& value);

      /**
       * Inserts a value at a specified index.
       * @param value The value to insert.
       * @param index The index to insert the value at.
       * @return <code>QValidator::State::Acceptable</code> if the model
       *         supports the operation, <code>QValidator::State::Invalid</code>
       *         otherwise.
       */
      virtual QValidator::State insert(const std::any& value, int index) = 0;

      /**
       * Moves a value.
       * @param source - The index of the value to move.
       * @param destination - The index to move the value to.
       * @return <code>QValidator::State::Acceptable</code> if the model
       *         supports the operation, <code>QValidator::State::Invalid</code>
       *         otherwise.
       */
      virtual QValidator::State move(int source, int destination) = 0;

      /**
       * Removes a value from the model.
       * @param index - The index of the value to remove.
       * @return <code>QValidator::State::Acceptable</code> if the model
       *         supports the operation, <code>QValidator::State::Invalid</code>
       *         otherwise.
       */
      virtual QValidator::State remove(int index) = 0;

      /** Connects a slot to the OperationSignal. */
      virtual boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const = 0;

    protected:

      /** Constructs an empty model. */
      AnyListModel() = default;

      /**
       * Returns the value at a specified index.
       * @throws <code>std::out_of_range</code> iff index is out of range.
       */
      virtual std::any at(int index) const = 0;

    private:
      AnyListModel(const AnyListModel&) = delete;
      AnyListModel& operator =(const AnyListModel&) = delete;
  };

  /**
   * Base class for a model over a list of values.
   * @param <T> The type of value being listed.
   */
  template<typename T>
  class ListModel : public AnyListModel {
    public:

      /** The type of value being listed. */
      using Type = T;

      /**
       * Returns the value at a specified index.
       * @throws <code>std::out_of_range</code> iff index is out of range.
       */
      virtual const Type& get(int index) const = 0;

      /**
       * Sets the value at a specified index.
       * @param index - The index to set.
       * @param value - The value to set at the specified index.
       * @return The state of the value at the <i>index</i>, or
       *         <code>QValidator::State::Invalid</code> iff row or column is
       *         out of range.
       */
      virtual QValidator::State set(int index, const Type& value);

      /**
       * Appends a value.
       * @param value The value to append to this model.
       * @return <code>QValidator::State::Acceptable</code> if the model
       *         supports the operation, <code>QValidator::State::Invalid</code>
       *         otherwise.
       */
      virtual QValidator::State push(const Type& value);

      /**
       * Inserts a value at a specified index.
       * @param value The value to insert.
       * @param index The index to insert the value at.
       * @return <code>QValidator::State::Acceptable</code> if the model
       *         supports the operation, <code>QValidator::State::Invalid</code>
       *         otherwise.
       */
      virtual QValidator::State insert(const Type& value, int index);

      /**
       * Moves a value.
       * @param source - The index of the value to move.
       * @param destination - The index to move the value to.
       * @return <code>QValidator::State::Acceptable</code> if the model
       *         supports the operation, <code>QValidator::State::Invalid</code>
       *         otherwise.
       */
      virtual QValidator::State move(int source, int destination) override;

      /**
       * Removes a value from the model.
       * @param index - The index of the value to remove.
       * @return <code>QValidator::State::Acceptable</code> if the model
       *         supports the operation, <code>QValidator::State::Invalid</code>
       *         otherwise.
       */
      virtual QValidator::State remove(int index) override;

    protected:

      /** Constructs an empty model. */
      ListModel() = default;

    private:
      ListModel(const ListModel&) = delete;
      ListModel& operator =(const ListModel&) = delete;
      QValidator::State set(int index, const std::any& value) override;
      std::any at(int index) const override;
      QValidator::State insert(const std::any& value, int index) override;
  };

  template<>
  class ListModel<std::any> : public AnyListModel {
    public:
      using Type = std::any;

      virtual const Type& get(int index) const = 0;

      virtual QValidator::State set(int index, const Type& value);

      virtual QValidator::State push(const Type& value);

      virtual QValidator::State insert(const Type& value, int index);

      virtual QValidator::State move(int source, int destination) override;

      virtual QValidator::State remove(int index) override;

    protected:
      ListModel() = default;

    private:
      ListModel(const ListModel&) = delete;
      ListModel& operator =(const ListModel&) = delete;
      std::any at(int index) const override;
  };

  /**
   * Applies a callable to an Operation.
   * @param operation The operation to visit.
   * @param f The callable to apply to the <i>operation</i>.
   */
  template<typename... F>
  void visit(const AnyListModel::Operation& operation, F&&... f) {
    return Details::visit<AnyListModel>(operation, std::forward<F>(f)...);
  }

  template<typename T>
  QValidator::State ListModel<T>::set(int index, const Type& value) {
    return QValidator::State::Invalid;
  }

  template<typename T>
  QValidator::State ListModel<T>::set(int index, const std::any& value) {
    return set(index, std::any_cast<const Type&>(value));
  }

  template<typename T>
  std::any ListModel<T>::at(int index) const {
    return get(index);
  }

  template<typename T>
  QValidator::State ListModel<T>::push(const Type& value) {
    return insert(value, get_size());
  }

  template<typename T>
  QValidator::State ListModel<T>::insert(const Type& value, int index) {
    return QValidator::State::Invalid;
  }

  template<typename T>
  QValidator::State ListModel<T>::move(int source, int destination) {
    return QValidator::State::Invalid;
  }

  template<typename T>
  QValidator::State ListModel<T>::remove(int index) {
    return QValidator::State::Invalid;
  }

  template<typename T>
  QValidator::State ListModel<T>::insert(const std::any& value, int index) {
    return insert(std::any_cast<const Type&>(value), index);
  }
}

#endif
