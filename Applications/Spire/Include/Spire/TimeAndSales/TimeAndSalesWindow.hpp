#ifndef SPIRE_TIME_AND_SALES_WINDOW_HPP
#define SPIRE_TIME_AND_SALES_WINDOW_HPP
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/TimeAndSales/TimeAndSales.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindowProperties.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Display the time and sales window. */
  class TimeAndSalesWindow : public Window {
    public:

      /**
       * Constructs a TimeAndSalesWindow.
       * @param time_and_sales The time and sale to represent.
       * @param parent The parent widget.
       */
      explicit TimeAndSalesWindow(
        std::shared_ptr<ComboBox::QueryModel> query_model,
        std::shared_ptr<ValueModel<Nexus::Security>> security,
        std::shared_ptr<TimeAndSalesModel> time_and_sales,
        std::shared_ptr<TimeAndSalesWindowProperties> properties,
        QWidget* parent = nullptr);

      /* Returns the query model. */
      const std::shared_ptr<ComboBox::QueryModel>& get_query_model() const;

      /* Returns the current security. */
      const std::shared_ptr<ValueModel<Nexus::Security>>& get_security() const;

      /* Returns the time and sales. */
      const std::shared_ptr<TimeAndSalesModel>& get_time_and_sales() const;

      /* Returns the properties. */
      const std::shared_ptr<TimeAndSalesWindowProperties>& get_properties() const;

      /* Updates the time and sales. */
      void update_time_and_sales(std::shared_ptr<TimeAndSalesModel> model);

    private:
      std::shared_ptr<TimeAndSalesModel> m_time_and_sales;
      std::shared_ptr<ArrayListModel<TimeAndSalesModel::Entry>> m_entries;
         //std::shared_ptr<TimeAndSalesProperties> m_properties,
      TimeAndSalesTableView* m_table_view;
      TransitionView* m_transition_view;
      SecurityView* m_security_view;
      boost::signals2::scoped_connection m_current_connection;

      void on_current(const Nexus::Security& security);
  };
}

#endif
