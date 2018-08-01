#ifndef SPIRE_TIME_AND_SALES_WINDOW_MODEL_HPP
#define SPIRE_TIME_AND_SALES_WINDOW_MODEL_HPP
#include <QAbstractItemModel>
#include "spire/time_and_sales/time_and_sales.hpp"
#include "spire/time_and_sales/time_and_sales_model.hpp"
#include "spire/time_and_sales/time_and_sales_properties.hpp"

namespace spire {

  //! Implements the table model for displaying time and sales in a list view.
  class time_and_sales_window_model : public QAbstractTableModel {
    public:

      //! Signals that additional time and sales are being loaded.
      using begin_loading_signal = Signal<void ()>;

      //! Signals that additional time and sales have completed loading.
      using end_loading_signal = Signal<void ()>;

      //! Constructs a time and sales window model.
      /*!
        \param model The base model.
        \param properties The display properties to use.
      */
      time_and_sales_window_model(std::shared_ptr<time_and_sales_model> model,
        const time_and_sales_properties& properties);

      //! Returns the security being displayed.
      const Nexus::Security& get_security() const;

      //! Returns true iff time and sales are currently being loaded.
      bool is_loading() const;

      //! Sets the display properties.
      void set_properties(const time_and_sales_properties& properties);

      //! Informs the model that a particular row is being displayed.
      /*!
        \param row The index of the row that is visible.
      */
      void set_row_visible(int row);

      //! Connects a slot to the begin_loading_signal.
      boost::signals2::connection connect_begin_loading_signal(
        const begin_loading_signal::slot_type& slot) const;

      //! Connects a slot to the end_loading_signal.
      boost::signals2::connection connect_end_loading_signal(
        const end_loading_signal::slot_type& slot) const;

      int rowCount(const QModelIndex& parent) const override;

      int columnCount(const QModelIndex& parent) const override;

      QVariant data(const QModelIndex& index, int role) const override;

      QVariant headerData(int section, Qt::Orientation orientation,
        int role) const override;

    private:
      mutable begin_loading_signal m_begin_loading_signal;
      mutable end_loading_signal m_end_loading_signal;
      std::shared_ptr<time_and_sales_model> m_model;
      time_and_sales_properties m_properties;
      std::vector<time_and_sales_model::entry> m_entries;
      bool m_is_loading;
      bool m_is_fully_loaded;
      QtPromise<std::vector<time_and_sales_model::entry>> m_snapshot_promise;

      void update_data(const time_and_sales_model::entry& e);
      void load_snapshot(Beam::Queries::Sequence last);
  };
}

#endif
