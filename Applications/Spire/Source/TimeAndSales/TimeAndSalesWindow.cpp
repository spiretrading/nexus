#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include "Spire/TimeAndSales/TimeAndSalesToTableModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/SecurityView.hpp"
#include "Spire/Ui/TransitionView.hpp"

using namespace Nexus;
using namespace Spire;

TimeAndSalesWindow::TimeAndSalesWindow(std::shared_ptr<ComboBox::QueryModel> query_model,
    std::shared_ptr<ValueModel<Security>> security,
    std::shared_ptr<TimeAndSalesModel> time_and_sales,
    std::shared_ptr<TimeAndSalesWindowProperties> properties,
    QWidget* parent)
    : Window(parent),
      m_time_and_sales(std::move(time_and_sales)) {
  setWindowTitle(tr("Time and Sales"));
  set_svg_icon(":/Icons/time-sales.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/time-sales.png"));
  resize(scale(180, 410));

  m_entries = std::make_shared<ArrayListModel<TimeAndSalesModel::Entry>>();
  auto table = std::make_shared<TimeAndSalesToTableModel>(m_entries);
  m_table_view = new TimeAndSalesTableView(table,
    std::move(properties));
  m_transition_view = new TransitionView(m_table_view);
  m_security_view = new SecurityView(std::move(query_model),
    std::move(security), *m_transition_view);
  m_current_connection = m_security_view->get_current()->connect_update_signal(
    std::bind_front(&TimeAndSalesWindow::on_current, this));
  layout()->addWidget(m_security_view);
}
const std::shared_ptr<ComboBox::QueryModel>& TimeAndSalesWindow::get_query_model() const {
  return m_security_view->get_query_model();
}

const std::shared_ptr<ValueModel<Nexus::Security>>& TimeAndSalesWindow::get_security() const {
  return m_security_view->get_current();
}

const std::shared_ptr<TimeAndSalesModel>& TimeAndSalesWindow::get_time_and_sales() const {
  return m_time_and_sales;
}

const std::shared_ptr<TimeAndSalesWindowProperties>& TimeAndSalesWindow::get_properties() const {
  return m_table_view->get_properties();
}

void TimeAndSalesWindow::update_time_and_sales(std::shared_ptr<TimeAndSalesModel> model) {

}

void TimeAndSalesWindow::on_current(const Security& security) {
  setWindowTitle(displayText(security) + tr(" - Time and Sales"));
  m_transition_view->set_status(TransitionView::Status::LOADING);
  auto promise = m_time_and_sales->query_until(Beam::Queries::Sequence::Present(), 20);
  auto result = wait(std::move(promise));
  for(auto& entry : result) {
    m_entries->push(entry);
  }
  m_transition_view->set_status(TransitionView::Status::READY);
}
