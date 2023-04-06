#ifndef SPIRE_TIME_AND_SALES_TABLE_VIEW_HPP
#define SPIRE_TIME_AND_SALES_TABLE_VIEW_HPP
#include <QWidget>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesToTableModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindowProperties.hpp"

namespace Spire {

  /** Display the time and sales TableView. */
  class TimeAndSalesTableView : public QWidget {
    public:

      using Column = TimeAndSalesToTableModel::Column;

      /**
       * Constructs a TimeAndSalesTableView.
       * @param time_and_sales The time and sale to represent.
       * @param properties 
       * @param parent The parent widget.
       */
      explicit TimeAndSalesTableView(std::shared_ptr<TableModel> table,
        std::shared_ptr<TimeAndSalesWindowProperties> properties,
        QWidget* parent = nullptr);

      /* Returns the time and sales. */
      const std::shared_ptr<TableModel>& get_table() const;

      /* Returns the properties. */
      const std::shared_ptr<TimeAndSalesWindowProperties>& get_properties() const;

    private:
      std::shared_ptr<TableModel> m_table;
      std::shared_ptr<TimeAndSalesWindowProperties> m_properties;
      std::shared_ptr<ArrayTableModel> m_table_model;
      //boost::signals2::scoped_connection m_update_connection;
      boost::signals2::scoped_connection m_current_connection;

      QWidget* table_view_builder(const std::shared_ptr<TableModel>& table, int row, int column);
      //void query_until(Beam::Queries::Sequence sequence);
      //void on_current(const Nexus::Security& security);
      //void on_update(const TimeAndSalesModel::Entry& entry);
  };
}

#endif
