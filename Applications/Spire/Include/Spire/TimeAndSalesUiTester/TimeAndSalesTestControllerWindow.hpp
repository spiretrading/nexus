#ifndef SPIRE_TIME_AND_SALES_TEST_CONTROLLER_WINDOW_HPP
#define SPIRE_TIME_AND_SALES_TEST_CONTROLLER_WINDOW_HPP
#include <memory>
#include <Beam/Threading/Threading.hpp>
#include <QCheckBox>
#include <QSpinBox>
#include <QWidget>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/TimeAndSalesUiTester/PeriodicTimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSales.hpp"
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"

namespace Spire {

  //! Displays the window used to control how a time and sales window is tested.
  class TimeAndSalesTestControllerWindow : public QWidget {
    public:
      TimeAndSalesTestControllerWindow(TimeAndSalesWindow* window);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      TimeAndSalesWindow* m_window;
      std::shared_ptr<PeriodicTimeAndSalesModel> m_model;
      QSpinBox* m_loading_time_spin_box;
      QCheckBox* m_all_data_loaded_check_box;

      void security_changed(const Nexus::Security& security);
      void update_data_loaded_check_box();
      void update_loading_time();
      void update_price(double price);
      void update_price_range(TimeAndSalesProperties::PriceRange range);
      void update_period(int ms);
      TimeAndSalesProperties::PriceRange get_price_range(const QString& range);
  };
}

#endif
