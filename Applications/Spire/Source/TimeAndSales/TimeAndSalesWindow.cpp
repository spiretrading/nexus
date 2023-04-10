#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/SecurityView.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TransitionView.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

TimeAndSalesWindow::TimeAndSalesWindow(std::shared_ptr<ComboBox::QueryModel> query_model,
    std::shared_ptr<TimeAndSalesWindowProperties> properties,
    QWidget* parent)
    : Window(parent),
      m_properties(std::move(properties)) {
  setWindowTitle(tr("Time and Sales"));
  set_svg_icon(":/Icons/time-sales.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/time-sales.png"));
  resize(scale(180, 410));

  m_time_and_sales = std::make_shared<ArrayListModel<TimeAndSale>>();
  auto table = std::make_shared<TimeAndSalesTableModel>(m_time_and_sales);
  m_table_view = new TimeAndSalesTableView(table);
  m_transition_view = new TransitionView(m_table_view);
  m_security_view = new SecurityView(std::move(query_model),
    std::make_shared<LocalValueModel<Security>>(), *m_transition_view);
  m_current_connection = m_security_view->get_current()->connect_update_signal(
    std::bind_front(&TimeAndSalesWindow::on_current, this));
  layout()->addWidget(m_security_view);
}

void TimeAndSalesWindow::set_time_and_sales_model(std::shared_ptr<TimeAndSalesModel> model) {
  m_time_and_sales_model = std::move(model);
  m_transition_view->set_status(TransitionView::Status::LOADING);
  auto promise = m_time_and_sales_model->query_until(Beam::Queries::Sequence::Present(), 20);
  auto result = wait(std::move(promise));
  for(auto& entry : result) {
    m_time_and_sales->insert(entry.m_time_and_sale.GetValue(), 0);
    auto time_and_sale_style = m_properties->get_style(entry.m_indicator);
    for(auto column = 0; column < m_table_view->get_table()->get_column_size(); ++column) {
      update_style(*m_table_view->get_item({0, column}), [&] (auto& style) {
        style.get(Any() > is_a<TextBox>()).
          set(BackgroundColor(time_and_sale_style.m_band_color)).
          set(text_style(time_and_sale_style.m_font, QColor(time_and_sale_style.m_text_color)));
        style.get(Any() > Body()).
          set(BackgroundColor(time_and_sale_style.m_band_color));
      });
    }
  }
  m_transition_view->set_status(TransitionView::Status::READY);
  //m_time_and_sales_model->connect_update_signal([=] (const auto& entry) {
  //  m_time_and_sales->insert(entry.m_time_and_sale.GetValue(), 0);
  //  auto time_and_sale_style = m_properties->get_style(entry.m_indicator);
  //  for(auto column = 0; column < m_table_view->get_table()->get_column_size(); ++column) {
  //    update_style(*m_table_view->get_item({0, column}), [&] (auto& style) {
  //      style.get(Any() > is_a<TextBox>()).
  //        set(BackgroundColor(time_and_sale_style.m_band_color)).
  //        set(text_style(time_and_sale_style.m_font, QColor(time_and_sale_style.m_text_color)));
  //      style.get(Any() > Body()).
  //        set(BackgroundColor(time_and_sale_style.m_band_color));
  //    });
  //  }
  //});
}

const std::shared_ptr<ComboBox::QueryModel>& TimeAndSalesWindow::get_query_model() const {
  return m_security_view->get_query_model();
}

const std::shared_ptr<ValueModel<Nexus::Security>>& TimeAndSalesWindow::get_security() const {
  return m_security_view->get_current();
}

//const std::shared_ptr<TimeAndSalesModel>& TimeAndSalesWindow::get_time_and_sales() const {
//  return m_time_and_sales;
//}

const std::shared_ptr<TimeAndSalesWindowProperties>& TimeAndSalesWindow::get_properties() const {
  return m_properties;
}

void TimeAndSalesWindow::on_current(const Security& security) {
  setWindowTitle(displayText(security) + tr(" - Time and Sales"));
}
