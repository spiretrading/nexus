#ifndef SPIRE_TIME_AND_SALES_TABLE_MODEL_HPP
#define SPIRE_TIME_AND_SALES_TABLE_MODEL_HPP
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"

namespace Spire {

  /**
   * Implements a TimeAndSalesTableModel that loads the time and sales from
   * TimeAndSalesModel and converts them to TableModel.
   */
  class TimeAndSalesTableModel : public TableModel {
  public:

    /* Lists out the columns of the table. */
    enum class Column {

      /** The time column. */
      TIME,

      /** The price column. */
      PRICE,

      /** The size column. */
      SIZE,

      /** The market column. */
      MARKET,

      /** The sales condition column. */
      CONDITION
    };

    /** The number of columns in this table. */
    static const auto COLUMN_SIZE = 5;

    /* Signals that time and sales are being loaded. */
    using BeginLoadingSignal = Signal<void()>;

    /* Signals that time and sales have completed loading. */
    using EndLoadingSignal = Signal<void()>;

    /**
     * Constructs an TimeAndSalesTableModel.
     * @param model The time and sales model.
     */
    explicit TimeAndSalesTableModel(std::shared_ptr<TimeAndSalesModel> model);

    /* Returns the model. */
    const std::shared_ptr<TimeAndSalesModel>& get_model() const;

    /* Sets the model. */
    void set_model(std::shared_ptr<TimeAndSalesModel> model);

    /**
     * Load the historical time and sales.
     * @param max_count The maximum number of entries to load.
     */
    void load_history(int max_count);

    /*
     * Returns the bbo indicator at a specified row.
     * @param row The specified row.
     */
    BboIndicator get_bbo_indicator(int row) const;

    int get_row_size() const override;

    int get_column_size() const override;

    AnyRef at(int row, int column) const override;

    /* Connects a slot to the BeginLoadingSignal. */
    boost::signals2::connection connect_begin_loading_signal(
      const BeginLoadingSignal::slot_type& slot) const;

    /* Connects a slot to the EndLoadingSignal. */
    boost::signals2::connection connect_end_loading_signal(
      const EndLoadingSignal::slot_type& slot) const;

    boost::signals2::connection connect_operation_signal(
      const OperationSignal::slot_type& slot) const override;

  private:
    mutable BeginLoadingSignal m_begin_loading_signal;
    mutable EndLoadingSignal m_end_loading_signal;
    std::shared_ptr<TimeAndSalesModel> m_model;
    ArrayListModel<TimeAndSalesModel::Entry> m_entries;
    QtPromise<void> m_promise;
    TableModelTransactionLog m_transaction;
    boost::signals2::scoped_connection m_connection;

    void load_snapshot(Beam::Queries::Sequence last, int count);
    void on_update(const TimeAndSalesModel::Entry& entry);
    void on_operation(
      const ListModel<TimeAndSalesModel::Entry>::Operation& operation);
  };
}

#endif
