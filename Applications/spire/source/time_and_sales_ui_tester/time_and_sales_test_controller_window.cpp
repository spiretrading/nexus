#include "spire/time_and_sales_ui_tester/time_and_sales_test_controller_window.hpp"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include "spire/spire/dimensions.hpp"
#include "spire/time_and_sales/time_and_sales_window.hpp"

using namespace Beam;
using namespace Beam::Threading;
using namespace Nexus;
using namespace spire;
using price_range = time_and_sales_properties::price_range;

time_and_sales_test_controller_window::time_and_sales_test_controller_window(
    time_and_sales_window* window, TimerThreadPool& timer_thread_pool)
    : m_window(window),
      m_timer_thread_pool(&timer_thread_pool) {
  m_window->connect_security_change_signal(
    [=] (const auto& s) { security_changed(s); });
  m_model = std::make_shared<periodic_time_and_sales_model>(Security(),
    *m_timer_thread_pool);
  m_model->set_price(Money(Quantity(20)));
  m_model->set_price_range(price_range::AT_ASK);
  m_model->set_period(boost::posix_time::milliseconds(1000));
  m_model->set_load_duration(boost::posix_time::milliseconds(1000));
  setFixedSize(scale(400, 200));
  auto layout = new QGridLayout(this);
  auto price_label = new QLabel("Price:", this);
  layout->addWidget(price_label, 0, 0);
  auto price_spin_box = new QDoubleSpinBox(this);
  price_spin_box->setMaximum(1000000000);
  price_spin_box->setValue(20.00);
  connect(price_spin_box, &QDoubleSpinBox::editingFinished,
    [=] { update_price(price_spin_box->value()); });
  layout->addWidget(price_spin_box, 0, 1);
  auto price_range_label = new QLabel("Price Range:", this);
  layout->addWidget(price_range_label, 1, 0);
  auto price_range_combo_box = new QComboBox(this);
  price_range_combo_box->addItem("Unknown");
  price_range_combo_box->addItem("Above Ask");
  price_range_combo_box->addItem("At Ask");
  price_range_combo_box->addItem("Inside");
  price_range_combo_box->addItem("At Bid");
  price_range_combo_box->addItem("Below Bid");
  price_range_combo_box->setCurrentIndex(2);
  connect(price_range_combo_box,
    static_cast<void (QComboBox::*)(const QString&)>(
    &QComboBox::currentIndexChanged),
    [=] (auto i) { update_price_range(get_price_range(i)); });
  layout->addWidget(price_range_combo_box, 1, 1);
  auto period_label = new QLabel("Period (ms):", this);
  layout->addWidget(period_label, 2, 0);
  auto period_line_edit = new QSpinBox(this);
  period_line_edit->setMaximum(100000);
  period_line_edit->setValue(1000);
  connect(period_line_edit, &QSpinBox::editingFinished,
    [=] { update_period(period_line_edit->value()); });
  layout->addWidget(period_line_edit, 2, 1);
  auto loading_time_label = new QLabel("Loading Time (ms):", this);
  layout->addWidget(loading_time_label, 3, 0);
  m_loading_time_spin_box = new QSpinBox(this);
  m_loading_time_spin_box->setMaximum(100000);
  m_loading_time_spin_box->setValue(1000);
  connect(m_loading_time_spin_box, &QSpinBox::editingFinished,
    this, &time_and_sales_test_controller_window::update_loading_time);
  layout->addWidget(m_loading_time_spin_box, 3, 1);
  auto data_loaded_check_box_label = new QLabel("All Data Loaded", this);
  layout->addWidget(data_loaded_check_box_label, 4, 0);
  m_all_data_loaded_check_box = new QCheckBox(this);
  connect(m_all_data_loaded_check_box, &QCheckBox::toggled, this,
    &time_and_sales_test_controller_window::update_data_loaded_check_box);
  layout->addWidget(m_all_data_loaded_check_box, 4, 1);
}

void time_and_sales_test_controller_window::security_changed(
    const Security& security) {
  auto price = m_model->get_price();
  auto price_range = m_model->get_price_range();
  auto period = m_model->get_period();
  auto load_duration = m_model->get_load_duration();
  m_model = std::make_shared<periodic_time_and_sales_model>(security,
    *m_timer_thread_pool);
  m_model->set_price(price);
  m_model->set_price_range(price_range);
  m_model->set_period(period);
  m_model->set_load_duration(load_duration);
  m_window->set_model(m_model);
}

void time_and_sales_test_controller_window::update_data_loaded_check_box() {
  if(m_all_data_loaded_check_box->isChecked()) {
    m_model->set_load_duration(boost::posix_time::pos_infin);
  } else {
    m_model->set_load_duration(boost::posix_time::milliseconds(
      m_loading_time_spin_box->value()));
  }
}

void time_and_sales_test_controller_window::update_loading_time() {
  m_model->set_load_duration(boost::posix_time::milliseconds(
    m_loading_time_spin_box->value()));
}

void time_and_sales_test_controller_window::update_price(double price) {
  m_model->set_price(Money(Quantity(price)));
}

void time_and_sales_test_controller_window::update_price_range(
    price_range range) {
  m_model->set_price_range(range);
}

void time_and_sales_test_controller_window::update_period(int ms) {
  m_model->set_period(boost::posix_time::milliseconds(ms));
}

price_range time_and_sales_test_controller_window::get_price_range(
    const QString& range) {
  if(range == "Unknown") {
    return price_range::UNKNOWN;
  }
  if(range == "Above Ask") {
    return price_range::ABOVE_ASK;
  }
  if(range == "At Ask") {
    return price_range::AT_ASK;
  }
  if(range == "Inside") {
    return price_range::INSIDE;
  }
  if(range == "At Bid") {
    return price_range::AT_BID;
  }
  if(range == "Below Bid") {
    return price_range::BELOW_BID;
  }
  return price_range::UNKNOWN;
}
