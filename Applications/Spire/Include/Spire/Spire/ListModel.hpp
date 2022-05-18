#ifndef SPIRE_LIST_MODEL_HPP
#define SPIRE_LIST_MODEL_HPP
#include <any>
#include <functional>
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

  /** Base class used to model a list of values. */
  class AnyListModel {
    public:

      /** Indicates a value was added to the model. */
      struct AddOperation {

        /** The index where the value was inserted. */
        int m_index;

        /** The value that was added. */
        std::any m_value;
      };

      /** Indicates a value was removed from the model. */
      struct RemoveOperation {

        /** The index of the value removed. */
        int m_index;

        /** The value that was removed. */
        std::any m_value;
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

        /** The previous value. */
        std::any m_previous;

        /** The updated value. */
        std::any m_value;
      };

      /** Consolidates all basic operations. */
      class Operation {
        public:

          /** Constructs an Operation encapsulating an AddOperation. */
          Operation(AddOperation operation);

          /** Constructs an Operation encapsulating a RemoveOperation. */
          Operation(RemoveOperation operation);

          /** Constructs an Operation encapsulating a MoveOperation. */
          Operation(MoveOperation operation);

          /** Constructs an Operation encapsulating an UpdateOperation. */
          Operation(UpdateOperation operation);

          /** Constructs an Operation encapsulating a Transaction. */
          Operation(std::vector<Operation> operation);

          /** Extracts a reference to a specific operation. */
          template<typename T>
          boost::optional<const T&> get() const;

          /**
           * Applies a callable to an Operation.
           * @param f The callable to apply.
           */
          template<typename... F>
          void visit(F&&... f) const;

        private:
          boost::variant<AddOperation, RemoveOperation, MoveOperation,
            UpdateOperation, std::vector<Operation>> m_operation;
      };

      /**
       * An operation consisting of a list of sub-operations performed as single
       * transaction.
       */
      using Transaction = std::vector<Operation>;

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

      /**
       * Takes a callable function and invokes it. All operations performed on
       * this model during the transaction get appended to a
       * <code>Transaction</code> that is signalled at the end of the
       * transaction. If a transaction is already being invoked, then all
       * operations are appened into the parent transaction.
       * @param transaction The transaction to perform.
       * @return The result of the transaction.
       */
      template<typename F>
      decltype(auto) transact(F&& transaction);

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

      /**
       * Takes a callable function and invokes it. All operations performed on
       * this model during the transaction get appended to a
       * <code>Transaction</code> that is signalled at the end of the
       * transaction. If a transaction is already being invoked, then all
       * operations are appened into the parent transaction.
       * @param transaction The transaction to perform.
       * @return The result of the transaction.
       */
      virtual void transact(const std::function<void ()>& transaction) = 0;

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

      /** Indicates a value was added to the model. */
      struct AddOperation : AnyListModel::AddOperation {
        AddOperation(int index, Type value);

        /** Returns the value that was added. */
        const Type& get_value() const;
      };

      /** Indicates a value was removed from the model. */
      struct RemoveOperation : AnyListModel::RemoveOperation {
        RemoveOperation(int index, Type value);

        /** Returns the value that was removed. */
        const Type& get_value() const;
      };

      /** Indicates a value was updated. */
      struct UpdateOperation : AnyListModel::UpdateOperation {
        UpdateOperation(int index, Type previous, Type value);

        /** Returns the previous value. */
        const Type& get_previous() const;

        /** Returns the updated value. */
        const Type& get_value() const;
      };

      /** Consolidates all basic operations. */
      struct Operation : AnyListModel::Operation {

        /** Constructs an Operation encapsulating an AddOperation. */
        Operation(AddOperation operation);

        /** Constructs an Operation encapsulating a RemoveOperation. */
        Operation(RemoveOperation operation);

        /** Constructs an Operation encapsulating a MoveOperation. */
        Operation(MoveOperation operation);

        /** Constructs an Operation encapsulating an UpdateOperation. */
        Operation(UpdateOperation operation);

        /** Constructs an Operation encapsulating a Transaction. */
        Operation(std::vector<Operation> operation);

        template<typename U>
        boost::optional<const U&> get() const;

        template<typename... F>
        void visit(F&&... f) const;
      };

      using Transaction = std::vector<Operation>;

      /**
       * Signals an operation was applied to this model.
       * @param operation The operation that was applied.
       */
      using OperationSignal = Signal<void (const Operation&)>;

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
      QValidator::State move(int source, int destination) override;

      /**
       * Removes a value from the model.
       * @param index - The index of the value to remove.
       * @return <code>QValidator::State::Acceptable</code> if the model
       *         supports the operation, <code>QValidator::State::Invalid</code>
       *         otherwise.
       */
      QValidator::State remove(int index) override;

      /** Connects a slot to the OperationSignal. */
      virtual boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const = 0;

      /** Connects a slot to the OperationSignal. */
      boost::signals2::connection connect_operation_signal(
        const AnyListModel::OperationSignal::slot_type& slot) const override;

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

      QValidator::State move(int source, int destination) override;

      QValidator::State remove(int index) override;

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
  template<typename Operation, typename... F>
  void visit(const Operation& operation, F&&... f) {
    operation.visit(std::forward<F>(f)...);
  }

  /** Removes all values from a ListModel. */
  void clear(AnyListModel& model);

  template<typename T>
  boost::optional<const T&> AnyListModel::Operation::get() const {
    if(auto operation = boost::get<T>(&m_operation)) {
      return *operation;
    }
    return boost::none;
  }

  template<typename... F>
  void AnyListModel::Operation::visit(F&&... f) const {
    if(auto transaction = get<AnyListModel::Transaction>()) {
      for(auto& operation : *transaction) {
        visit(std::forward<F>(f)...);
      }
    } else {
      auto head = [&] (auto&& f) {
        boost::apply_visitor([&] (const auto& operation) {
          using Parameter = std::decay_t<decltype(operation)>;
          if constexpr(std::is_invocable_v<decltype(f), Parameter>) {
            std::forward<decltype(f)>(f)(operation);
          }
        }, m_operation);
      };
      auto tail = [&] (auto&& f, auto&&... g) {
        auto is_visited = boost::apply_visitor([&] (const auto& operation) {
          using Parameter = std::decay_t<decltype(operation)>;
          if constexpr(std::is_invocable_v<decltype(f), Parameter>) {
            std::forward<decltype(f)>(f)(operation);
            return true;
          }
          return false;
        }, m_operation);
        if(!is_visited) {
          visit(std::forward<decltype(g)>(g)...);
        }
      };
      if constexpr(sizeof...(F) == 1) {
        head(std::forward<F>(f)...);
      } else {
        tail(std::forward<F>(f)...);
      }
    }
  }

  template<typename F>
  decltype(auto) AnyListModel::transact(F&& transaction) {
    using Result = decltype(transaction());
    auto t = static_cast<void (AnyListModel::*)(const std::function<void ()>&)>(
      &AnyListModel::transact);
    if constexpr(std::is_same_v<Result, void>) {
      (this->*t)([&] {
        std::forward<F>(transaction)();
      });
    } else {
      auto result = boost::optional<decltype(transaction())>();
      (this->*t)([&] {
        result.emplace(std::forward<F>(transaction)());
      });
      return *result;
    }
  }

  template<typename T>
  ListModel<T>::AddOperation::AddOperation(int index, Type value)
    : AnyListModel::AddOperation(index, std::move(value)) {}

  template<typename T>
  const typename ListModel<T>::Type& ListModel<T>::AddOperation::get_value()
      const {
    return std::any_cast<const Type&>(m_value);
  }

  template<typename T>
  ListModel<T>::RemoveOperation::RemoveOperation(int index, Type value)
    : AnyListModel::RemoveOperation(index, std::move(value)) {}

  template<typename T>
  const typename ListModel<T>::Type& ListModel<T>::RemoveOperation::get_value()
      const {
    return std::any_cast<const Type&>(m_value);
  }

  template<typename T>
  ListModel<T>::UpdateOperation::UpdateOperation(
    int index, Type previous, Type value)
    : AnyListModel::UpdateOperation(
        index, std::move(previous), std::move(value)) {}

  template<typename T>
  const typename ListModel<T>::Type&
      ListModel<T>::UpdateOperation::get_previous() const {
    return std::any_cast<const Type&>(m_previous);
  }

  template<typename T>
  const typename ListModel<T>::Type& ListModel<T>::UpdateOperation::get_value()
      const {
    return std::any_cast<const Type&>(m_value);
  }

  template<typename T>
  ListModel<T>::Operation::Operation(AddOperation operation)
    : AnyListModel::Operation(std::move(operation)) {}

  template<typename T>
  ListModel<T>::Operation::Operation(RemoveOperation operation)
    : AnyListModel::Operation(std::move(operation)) {}

  template<typename T>
  ListModel<T>::Operation::Operation(MoveOperation operation)
    : AnyListModel::Operation(std::move(operation)) {}

  template<typename T>
  ListModel<T>::Operation::Operation(UpdateOperation operation)
    : AnyListModel::Operation(std::move(operation)) {}

  template<typename T>
  ListModel<T>::Operation::Operation(std::vector<Operation> operation)
    : AnyListModel::Operation([&] {
        auto operations = std::vector<AnyListModel::Operation>();
        for(auto& o : operation) {
          operations.push_back(std::move(o));
        }
        return operations;
      }()) {}

  template<typename T>
  template<typename U>
  boost::optional<const U&> ListModel<T>::Operation::get() const {
    if constexpr(std::is_same_v<U, AddOperation>) {
      if(auto operation =
          AnyListModel::Operation::get<AnyListModel::AddOperation>()) {
        return static_cast<const AddOperation&>(*operation);
      }
      return boost::none;
    } else if constexpr(std::is_same_v<U, RemoveOperation>) {
      if(auto operation =
          AnyListModel::Operation::get<AnyListModel::RemoveOperation>()) {
        return static_cast<const RemoveOperation&>(*operation);
      }
      return boost::none;
    } else if constexpr(std::is_same_v<U, MoveOperation>) {
      if(auto operation =
          AnyListModel::Operation::get<AnyListModel::MoveOperation>()) {
        return static_cast<const MoveOperation&>(*operation);
      }
      return boost::none;
    } else if constexpr(std::is_same_v<U, UpdateOperation>) {
      if(auto operation =
          AnyListModel::Operation::get<AnyListModel::UpdateOperation>()) {
        return static_cast<const UpdateOperation&>(*operation);
      }
      return boost::none;
    } else if constexpr(std::is_same_v<U, Transaction>) {
      if(auto operation =
          AnyListModel::Operation::get<AnyListModel::Transaction>()) {
        return reinterpret_cast<const Transaction&>(
          AnyListModel::Operation::get<AnyListModel::Transaction>());
      }
      return boost::none;
    }
    return boost::none;
  }

  template<typename T>
  template<typename... F>
  void ListModel<T>::Operation::visit(F&&... f) const {
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
