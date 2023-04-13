#ifndef SPIRE_TIME_AND_SALES_WINDOW_HPP
#define SPIRE_TIME_AND_SALES_WINDOW_HPP
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/TimeAndSales/TimeAndSales.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindowProperties.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Display the time and sales window for a security. */
  class TimeAndSalesWindow : public Window {
    public:

      /**
       * Constructs a TimeAndSalesWindow.
       * @param query_model The model used to query security.
       * @param properties The display properties of the window.
       * @param parent The parent widget.
       */
      explicit TimeAndSalesWindow(
        std::shared_ptr<ComboBox::QueryModel> query_model,
        std::shared_ptr<TimeAndSalesWindowProperties> properties,
        QWidget* parent = nullptr);

      /* Sets the time and sales model when the security has been updated. */
      void set_model(std::shared_ptr<TimeAndSalesModel> model);

      /* Returns the query model. */
      const std::shared_ptr<ComboBox::QueryModel>& get_query_model() const;

      /* Returns the properties. */
      const std::shared_ptr<TimeAndSalesWindowProperties>& get_properties() const;

      /* Returns the security that the window represents. */
      const std::shared_ptr<ValueModel<Nexus::Security>>& get_security() const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      std::shared_ptr<TimeAndSalesModel> m_model;
      std::shared_ptr<TimeAndSalesWindowProperties> m_properties;
      TimeAndSalesTableView* m_table_view;
      TransitionView* m_transition_view;
      SecurityView* m_security_view;
      ResponsiveLabel* m_title_label;
      QtPromise<void> m_promise;
      boost::signals2::scoped_connection m_current_connection;

      void on_current(const Nexus::Security& security);
      void on_table_operation(const TableModel::Operation& operation);
  };
}

#endif
