#include "Spire/TimeAndSales/TimeAndSalesPropertiesWindow.hpp"

using namespace Spire;

TimeAndSalesPropertiesWindow::TimeAndSalesPropertiesWindow(
  std::shared_ptr<TimeAndSalesPropertiesModel> current, QWidget* parent)
  : Window(parent),
    m_current(std::move(current)) {}

const std::shared_ptr<TimeAndSalesPropertiesModel>&
    TimeAndSalesPropertiesWindow::get_current() const {
  return m_current;
}
