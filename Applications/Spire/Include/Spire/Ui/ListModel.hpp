#ifndef SPIRE_LIST_MODEL_HPP
#define SPIRE_LIST_MODEL_HPP
#include <any>
#include <type_traits>
#include <utility>
#include <vector>
#include <boost/hana/functional/overload.hpp>
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
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Details {
  template<typename O>
  struct ModelTransaction {
    std::vector<O> m_operations;
  };

  /**
   * Applies a callable to an Operation.
   * @param operation The operation to visit.
   * @param f The callable to apply to the <i>operation</i>.
   */
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
        visit<M>(transaction_operation, std::forward<F>(f), std::forward<G>(g)...);
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

  /** Base class for a model over a list of values. */
  class ListModel {
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
       * An operation consisting of a list of sub-operations performed as
       * single transaction.
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

      virtual ~ListModel() = default;

      /** Returns the number of items in the model. */
      virtual int get_size() const = 0;

      /**
       * Returns the value at a specified index.
       * @throws <code>std::out_of_range</code> iff index is out of range.
       */
      virtual const std::any& at(int index) const = 0;

      /**
       * Returns the value at a specified index.
       * @throws <code>std::out_of_range</code> iff index is out of range.
       */
      template<typename T>
      const T& get(int index) const;

      /**
       * Sets the value at a specified index.
       * @param index - The index to set.
       * @param value - The value to set at the specified index.
       * @return The state of the value at the <i>index</i>, or
       *         <code>QValidator::State::Invalid</code> iff row or column is
       *         out of range.
       */
      virtual QValidator::State set(int index, const std::any& value);

      /** Connects a slot to the OperationSignal. */
      virtual boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const = 0;

    protected:

      /** Constructs an empty model. */
      ListModel() = default;

    private:
      ListModel(const ListModel&) = delete;
      ListModel& operator =(const ListModel&) = delete;
  };

  template<typename T>
  const T& ListModel::get(int index) const {
    return std::any_cast<const T&>(at(index));
  }

  template<typename... F>
  void visit(const ListModel::Operation& operation, F&&... f) {
    return Details::visit<ListModel>(operation, std::forward<F>(f)...);
  }
}

#endif
