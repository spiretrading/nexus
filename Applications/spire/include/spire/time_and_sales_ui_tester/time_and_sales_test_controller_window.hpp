#ifndef SPIRE_TIME_AND_SALES_TEST_CONTROLLER_WINDOW_HPP
#define SPIRE_TIME_AND_SALES_TEST_CONTROLLER_WINDOW_HPP
#include <memory>
#include <Beam/Threading/Threading.hpp>
#include <QCheckBox>
#include <QSpinBox>
#include <QWidget>
#include "Nexus/Definitions/Security.hpp"
#include "spire/time_and_sales_ui_tester/periodic_time_and_sales_model.hpp"
#include "spire/time_and_sales/time_and_sales.hpp"
#include "spire/time_and_sales/time_and_sales_properties.hpp"

namespace spire {

  //! Displays the window used to control how a time and sales window is tested.
  class TimeAndSalesTestControllerWindow : public QWidget {
    public:
      TimeAndSalesTestControllerWindow(TimeAndSalesWindow* window,
        Beam::Threading::TimerThreadPool& timer_thread_pool);

    private:
      TimeAndSalesWindow* m_window;
      Beam::Threading::TimerThreadPool* m_timer_thread_pool;
      std::shared_ptr<PeriodicTimeAndSalesModel> m_model;
      QSpinBox* m_loading_time_spin_box;
      QCheckBox* m_all_data_loaded_check_box;

      void security_changed(const Nexus::Security& security);
      void update_data_loaded_check_box();
      void update_loading_time();
      void update_price(double price);
      void update_price_range(TimeAndSalesProperties::PriceRange range);
      void update_period(int ms);
      TimeAndSalesProperties::PriceRange get_price_range(
        const QString& range);
  };
}

#endif
