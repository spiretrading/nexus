#ifndef SPIRE_TABLE_MODEL_HPP
#define SPIRE_TABLE_MODEL_HPP
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
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Details {
  template<typename O>
  struct TableModelTransaction {
    std::vector<O> m_operations;
  };
}

  /** Base class for a model over a table of values. */
  class TableModel {
    public:

      /** Indicates a row was added to the model. */
      struct AddOperation {

        /** The index where the row was inserted. */
        int m_index;
      };

      /** Indicates a row was removed from the model. */
      struct RemoveOperation {

        /** The index of the row removed. */
        int m_index;
      };

      /** Indicates a row was moved from one index to another. */
      struct MoveOperation {

        /** The index of the row that was moved. */
        int m_source;

        /** The index that the row was moved to. */
        int m_destination;
      };

      /** Indicates a value was updated. */
      struct UpdateOperation {

        /** The row of the updated value. */
        int m_row;

        /** The column of the updated value. */
        int m_column;
      };

      /** Consolidates all basic operations. */
      using Operation = typename boost::make_recursive_variant<AddOperation,
        RemoveOperation, MoveOperation, UpdateOperation,
        Details::TableModelTransaction<boost::recursive_variant_>>::type;

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

      virtual ~TableModel() = default;

      /** Returns the number of rows in the model. */
      virtual int get_row_size() const = 0;

      /** Returns the number of columns in the model. */
      virtual int get_column_size() const = 0;

      /**
       * Returns the value at a specified row and column.
       * @throws <code>std::out_of_range</code> iff row or column is out of
       *         range.
       */
      virtual const std::any& at(int row, int column) const = 0;

      /**
       * Returns the value at a specified row and column.
       * @throws <code>std::out_of_range</code> iff row or column is out of
       *         range.
       */
      template<typename T>
      const T& get(int row, int column) const;

      /**
       * Sets the value at a specified row and column.
       * @param row - The row to set.
       * @param column - The column to set.
       * @param value - The value to set at the specified row and column.
       * @return The state of the value at the <i>row</i> and <i>column</i>, or
       *         <code>QValidator::State::Invalid</code> iff row or column is
       *         out of range.
       */
      virtual QValidator::State set(int row, int column, const std::any& value);

      /** Connects a slot to the OperationSignal. */
      virtual boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const = 0;

    protected:

      /** Constructs an empty model. */
      TableModel() = default;

    private:
      TableModel(const TableModel&) = delete;
      TableModel& operator =(const TableModel&) = delete;
  };

  template<typename T>
  const T& TableModel::get(int row, int column) const {
    return std::any_cast<const T&>(at(row, column));
  }

  /**
   * Applies a callable to a TableModel::Operation.
   * @param operation The operation to visit.
   * @param f The callable to apply to the <i>operation</i>.
   */
  template<typename F>
  void visit(const TableModel::Operation& operation, F&& f) {
    static_assert(std::is_invocable_v<F, const TableModel::AddOperation&> ||
      std::is_invocable_v<F, const TableModel::RemoveOperation&> ||
      std::is_invocable_v<F, const TableModel::MoveOperation&> ||
      std::is_invocable_v<F, const TableModel::UpdateOperation&>);
    if(auto transaction = boost::get<TableModel::Transaction>(&operation)) {
      for(auto& transaction_operation : transaction->m_operations) {
        visit(transaction_operation, std::forward<F>(f));
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

  template<typename F, typename... G>
  void visit(const TableModel::Operation& operation, F&& f, G&&... g) {
    static_assert(std::is_invocable_v<F, const TableModel::AddOperation&> ||
      std::is_invocable_v<F, const TableModel::RemoveOperation&> ||
      std::is_invocable_v<F, const TableModel::MoveOperation&> ||
      std::is_invocable_v<F, const TableModel::UpdateOperation&>);
    if(auto transaction = boost::get<TableModel::Transaction>(&operation)) {
      for(auto& transaction_operation : transaction->m_operations) {
        visit(transaction_operation, std::forward<F>(f), std::forward<G>(g)...);
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
        visit(operation, std::forward<G>(g)...);
      }
    }
  }
}

#endif
