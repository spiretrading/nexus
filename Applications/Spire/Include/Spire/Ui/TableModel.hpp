#ifndef SPIRE_TABLE_MODEL_HPP
#define SPIRE_TABLE_MODEL_HPP
#include "Spire/Spire/AnyRef.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Base class for a model over a table of values. */
  class TableModel {
    public:

      /** Indicates a row was added to the model. */
      struct AddOperation {

        /** The index where the row was inserted. */
        int m_index;

        /** A list representation of the added row. */
        std::shared_ptr<AnyListModel> m_row;
      };

      /** Indicates a row was removed from the model. */
      struct RemoveOperation {

        /** The index of the row removed. */
        int m_index;

        /** A list representation of the removed row. */
        std::shared_ptr<AnyListModel> m_row;
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
       * An operation consisting of a list of sub-operations performed as
       * single transaction.
       */
      using Transaction = std::vector<Operation>;

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
      virtual AnyRef at(int row, int column) const = 0;

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
        const OperationSignal::slot_type& slot) const = 0;

    protected:

      /** Constructs an empty model. */
      TableModel() = default;

    private:
      TableModel(const TableModel&) = delete;
      TableModel& operator =(const TableModel&) = delete;
  };

  template<typename T>
  const T& TableModel::get(int row, int column) const {
    return any_cast<const T>(at(row, column));
  }

  /**
   * Applies a callable to an Operation.
   * @param operation The operation to visit.
   * @param f The callable to apply to the <i>operation</i>.
   */
  template<typename... F>
  void visit(const TableModel::Operation& operation, F&&... f) {
    operation.visit(std::forward<F>(f)...);
  }

  template<typename T>
  boost::optional<const T&> TableModel::Operation::get() const {
    if(auto operation = boost::get<T>(&m_operation)) {
      return *operation;
    }
    return boost::none;
  }

  template<typename... F>
  void TableModel::Operation::visit(F&&... f) const {
    if(auto transaction = get<TableModel::Transaction>()) {
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
}

#endif
