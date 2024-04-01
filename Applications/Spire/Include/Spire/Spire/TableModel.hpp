#ifndef SPIRE_TABLE_MODEL_HPP
#define SPIRE_TABLE_MODEL_HPP
#include "Spire/Spire/AnyRef.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Base class for a model over a table of values. */
  class TableModel {
    public:

      /** Indicates a row was added to the model. */
      struct AddOperation {

        /** The index where the row was inserted. */
        int m_index;

        /** A list representation of the added row. */
        std::shared_ptr<const AnyListModel> m_row;

        /**
         * Constructs an AddOperation.
         * @param index The index where the row was inserted.
         * @param row The list representation of the added row.
         */
        AddOperation(int index, std::shared_ptr<const AnyListModel> row);
      };

      /** Indicates a row was removed from the model. */
      struct RemoveOperation {

        /** The index of the row removed. */
        int m_index;

        /** A list representation of the removed row. */
        std::shared_ptr<const AnyListModel> m_row;

        /**
         * Constructs a RemoveOperation.
         * @param index The index where the row removed.
         * @param row The list representation of the removed row.
         */
        RemoveOperation(int index, std::shared_ptr<const AnyListModel> row);
      };

      /** Indicates a row was moved from one index to another. */
      struct MoveOperation {

        /** The index of the row that was moved. */
        int m_source;

        /** The index that the row was moved to. */
        int m_destination;

        /**
         * Constructs a MoveOperation.
         * @param source The index of the row that was moved.
         * @param destination The index that the row was moved to.
         */
        MoveOperation(int source, int destination);
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

        /**
         * Constructs an UpdateOperation.
         * @param row The row of the updated value.
         * @param column The column of the updated value.
         * @param previous The previous value.
         * @param value The updated value.
         */
        UpdateOperation(int row, int column, std::any previous, std::any value);
      };

      /** Indicates the beginning of a transaction. */
      struct StartTransaction {};

      /** Indicates the end of a transaction. */
      struct EndTransaction {};

      /** Consolidates all operations. */
      using Operation = boost::variant<AddOperation, RemoveOperation,
        MoveOperation, UpdateOperation, StartTransaction, EndTransaction>;

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

      /**
       * Removes a row from the model.
       * @param row - The index of the row to remove.
       * @return <code>QValidator::State::Acceptable</code> if the model
       *         supports the operation, <code>QValidator::State::Invalid</code>
       *         otherwise.
       * @throws <code>std::out_of_range</code> - The index is not within this
       *         table's range.
       */
      virtual QValidator::State remove(int row);

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
}

#endif
