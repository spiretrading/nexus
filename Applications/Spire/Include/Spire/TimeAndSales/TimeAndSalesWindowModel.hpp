#ifndef SPIRE_TIME_AND_SALES_WINDOW_MODEL_HPP
#define SPIRE_TIME_AND_SALES_WINDOW_MODEL_HPP
#include <QAbstractItemModel>
#include "Spire/Spire/Signal.hpp"
#include "Spire/TimeAndSales/TimeAndSales.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"

namespace Spire {

  //! Implements the table model for displaying time and sales in a list view.
  class TimeAndSalesWindowModel : public QAbstractTableModel {
    public:

      //! Signals that additional time and sales are being loaded.
      using BeginLoadingSignal = Signal<void ()>;

      //! Signals that additional time and sales have completed loading.
      using EndLoadingSignal = Signal<void ()>;

      //! Constructs a TimeAndSalesWindowModel.
      /*!
        \param model The base model.
        \param properties The display properties to use.
      */
      TimeAndSalesWindowModel(std::shared_ptr<TimeAndSalesModel> model,
        const TimeAndSalesProperties& properties);

      //! Returns the security being displayed.
      const Nexus::Security& get_security() const;

      //! Returns true iff time and sales are currently being loaded.
      bool is_loading() const;

      //! Sets the display properties.
      void set_properties(const TimeAndSalesProperties& properties);

      //! Informs the model that a particular row is being displayed.
      /*!
        \param row The index of the row that is visible.
      */
      void set_row_visible(int row);

      //! Connects a slot to the BeginLoadingSignal.
      boost::signals2::connection connect_begin_loading_signal(
        const BeginLoadingSignal::slot_type& slot) const;

      //! Connects a slot to the EndLoadingSignal.
      boost::signals2::connection connect_end_loading_signal(
        const EndLoadingSignal::slot_type& slot) const;

      int rowCount(const QModelIndex& parent) const override;

      int columnCount(const QModelIndex& parent) const override;

      QVariant data(const QModelIndex& index, int role) const override;

      QVariant headerData(int section, Qt::Orientation orientation,
        int role) const override;

    private:
      mutable BeginLoadingSignal m_begin_loading_signal;
      mutable EndLoadingSignal m_end_loading_signal;
      std::shared_ptr<TimeAndSalesModel> m_model;
      boost::signals2::scoped_connection m_time_and_sale_connection;
      TimeAndSalesProperties m_properties;
      std::vector<TimeAndSalesModel::Entry> m_entries;
      bool m_is_loading;
      bool m_is_fully_loaded;
      QtPromise<std::vector<TimeAndSalesModel::Entry>> m_snapshot_promise;

      void update_data(const TimeAndSalesModel::Entry& e);
      void load_snapshot(Beam::Queries::Sequence last);
  };
}

#endif
