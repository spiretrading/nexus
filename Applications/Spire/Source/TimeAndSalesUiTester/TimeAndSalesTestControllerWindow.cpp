#include "Spire/TimeAndSalesUiTester/TimeAndSalesTestControllerWindow.hpp"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"

using namespace Beam;
using namespace Beam::Threading;
using namespace Nexus;
using namespace Spire;
using PriceRange = TimeAndSalesProperties::PriceRange;

TimeAndSalesTestControllerWindow::TimeAndSalesTestControllerWindow(
    TimeAndSalesWindow* window)
    : m_window(window) {
  m_window->installEventFilter(this);
  m_window->connect_change_security_signal(
    [=] (const auto& s) { security_changed(s); });
  m_model = std::make_shared<PeriodicTimeAndSalesModel>(Security());
  m_model->set_price(Money(Quantity(20)));
  m_model->set_price_range(PriceRange::AT_ASK);
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
    qOverload<int>(&QComboBox::currentIndexChanged), [=] (auto i) {
      update_price_range(get_price_range(price_range_combo_box->currentText()));
    });
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
    this, &TimeAndSalesTestControllerWindow::update_loading_time);
  layout->addWidget(m_loading_time_spin_box, 3, 1);
  auto data_loaded_check_box_label = new QLabel("All Data Loaded", this);
  layout->addWidget(data_loaded_check_box_label, 4, 0);
  m_all_data_loaded_check_box = new QCheckBox(this);
  connect(m_all_data_loaded_check_box, &QCheckBox::toggled, this,
    &TimeAndSalesTestControllerWindow::update_data_loaded_check_box);
  layout->addWidget(m_all_data_loaded_check_box, 4, 1);
}

bool TimeAndSalesTestControllerWindow::eventFilter(QObject* watched,
    QEvent* event) {
  if(event->type() == QEvent::Close) {
    close();
  }
  return QWidget::eventFilter(watched, event);
}

void TimeAndSalesTestControllerWindow::security_changed(
    const Security& security) {
  auto price = m_model->get_price();
  auto price_range = m_model->get_price_range();
  auto period = m_model->get_period();
  auto load_duration = m_model->get_load_duration();
  m_model = std::make_shared<PeriodicTimeAndSalesModel>(security);
  m_model->set_price(price);
  m_model->set_price_range(price_range);
  m_model->set_period(period);
  m_model->set_load_duration(load_duration);
  m_window->set_model(m_model);
}

void TimeAndSalesTestControllerWindow::update_data_loaded_check_box() {
  if(m_all_data_loaded_check_box->isChecked()) {
    m_model->set_load_duration(boost::posix_time::pos_infin);
  } else {
    m_model->set_load_duration(boost::posix_time::milliseconds(
      m_loading_time_spin_box->value()));
  }
}

void TimeAndSalesTestControllerWindow::update_loading_time() {
  if(!m_all_data_loaded_check_box->isChecked()) {
    m_model->set_load_duration(boost::posix_time::milliseconds(
      m_loading_time_spin_box->value()));
  }
}

void TimeAndSalesTestControllerWindow::update_price(double price) {
  m_model->set_price(Money(Quantity(price)));
}

void TimeAndSalesTestControllerWindow::update_price_range(
    PriceRange range) {
  m_model->set_price_range(range);
}

void TimeAndSalesTestControllerWindow::update_period(int ms) {
  m_model->set_period(boost::posix_time::milliseconds(ms));
}

PriceRange TimeAndSalesTestControllerWindow::get_price_range(
    const QString& range) {
  if(range == "Unknown") {
    return PriceRange::UNKNOWN;
  }
  if(range == "Above Ask") {
    return PriceRange::ABOVE_ASK;
  }
  if(range == "At Ask") {
    return PriceRange::AT_ASK;
  }
  if(range == "Inside") {
    return PriceRange::INSIDE;
  }
  if(range == "At Bid") {
    return PriceRange::AT_BID;
  }
  if(range == "Below Bid") {
    return PriceRange::BELOW_BID;
  }
  return PriceRange::UNKNOWN;
}
