#ifndef SPIRE_TIME_AND_SALES_UI_TESTER_HPP
#define SPIRE_TIME_AND_SALES_UI_TESTER_HPP
#include <memory>
#include <QWidget>
#include "Nexus/Definitions/Security.hpp"
#include "spire/time_and_sales/periodic_time_and_sales_model.hpp"
#include "spire/time_and_sales/time_and_sales.hpp"
#include "spire/time_and_sales/time_and_sales_properties.hpp"

namespace spire {

  //! Displays a time_and_sales_window tester
  class time_and_sales_ui_tester : public QWidget {
    public:
      time_and_sales_ui_tester(time_and_sales_window* window);

    private:
      time_and_sales_window* m_window;
      std::shared_ptr<periodic_time_and_sales_model> m_model;

      void security_changed(const Nexus::Security& security);
      void update_price(double price);
      void update_price_range(
        time_and_sales_properties::price_range range);
      void update_period(int ms);
      time_and_sales_properties::price_range get_price_range(
        const QString& range);
  };
}

#endif
