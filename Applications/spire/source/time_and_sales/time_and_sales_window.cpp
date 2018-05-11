#include "spire/time_and_sales/time_and_sales_window.hpp"
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <QDebug>
#include <QKeyEvent>
#include <QTableView>
#include <QVBoxLayout>
#include "spire/security_input/security_input_dialog.hpp"
#include "spire/security_input/security_input_model.hpp"
#include "spire/time_and_sales/empty_time_and_sales_model.hpp"
#include "spire/time_and_sales/periodic_time_and_sales_model.hpp"
#include "spire/time_and_sales/time_and_sales_window_model.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

time_and_sales_window::time_and_sales_window(
    const time_and_sales_properties& properties, 
    security_input_model& input_model, QWidget* parent)
    : QWidget(parent),
      m_input_model(&input_model) {
  set_properties(properties);
  m_change_security_signal.connect(
    [=] (const Security& s) { update_model(s); });
  m_body = new QWidget(this);
  m_body->setMinimumSize(scale(148, 200));
  resize(scale_width(182), scale_height(452));
  m_body->setStyleSheet("background-color: #FFFFFF;");
  auto window_layout = new QHBoxLayout(this);
  window_layout->setContentsMargins({});
  auto window = new spire::window(m_body, this);
  setWindowTitle(tr("Time and Sales"));
  window->set_icon(imageFromSvg(":/icons/time-sale-black.svg", scale(26, 26),
    QRect(translate(8, 8), scale(10, 10))),
    imageFromSvg(":/icons/time-sale-grey.svg", scale(26, 26),
    QRect(translate(8, 8), scale(10, 10))));
  window_layout->addWidget(window);
  auto layout = new QVBoxLayout(m_body);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_table = new QTableView(this);
  m_table->setFocusPolicy(Qt::NoFocus);
  layout->addWidget(m_table);
  m_volume_label = new QLabel(tr("Volume: 0"), this);
  m_volume_label->setFocusPolicy(Qt::NoFocus);
  m_volume_label->setFixedHeight(scale_height(20));
  m_volume_label->setStyleSheet(QString(R"(
    background-color: #F5F5F5;
    color: #8C8C8C;
    font-family: Roboto;
    font-size: %1px;
    font-weight: 550;
    padding-left: %2px;)").arg(scale_height(10)).arg(scale_width(8)));
  layout->addWidget(m_volume_label);
  set_model(std::make_shared<empty_time_and_sales_model>(Security()));
}

void time_and_sales_window::set_model(
    std::shared_ptr<time_and_sales_model> model) {
  model->connect_volume_signal(
    [=] (const Quantity& v) { update_volume(v); });
  model->connect_time_and_sale_signal(
    [=] (const time_and_sales_model::entry& e) { update_row(e); });
  m_model.emplace(std::move(model), m_properties);
  m_table->setModel(&m_model.get());
}

const time_and_sales_properties& time_and_sales_window::get_properties() const {
  return m_properties;
}

void time_and_sales_window::set_properties(
    const time_and_sales_properties& properties) {}

connection time_and_sales_window::connect_closed_signal(
    const closed_signal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

void time_and_sales_window::closeEvent(QCloseEvent* event) {
  m_closed_signal();
}

void time_and_sales_window::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_PageUp) {
    if(m_current_security != Security()) {
      auto s = m_securities.push_front(m_current_security);
      if(s != Security()) {
        set_current(s);
      }
    }
    return;
  } else if(event->key() == Qt::Key_PageDown) {
    if(m_current_security != Security()) {
      auto s = m_securities.push_back(m_current_security);
      if(s != Security()) {
        set_current(s);
      }
    }
    return;
  }
  auto pressed_key = event->text();
  if(pressed_key[0].isLetterOrNumber()) {
    auto dialog = new security_input_dialog(*m_input_model, pressed_key, this);
    dialog->move(geometry().center().x() - dialog->width() / 2,
      geometry().center().y() - dialog->height() / 2);
    if(dialog->exec() == QDialog::Accepted) {
      auto s = dialog->get_security();
      if(s != Security() && s != m_current_security) {
        m_securities.push(m_current_security);
        set_current(s);
        activateWindow();
      }
    }
  }
}

void time_and_sales_window::set_current(const Security& s) {
  if(s == m_current_security) {
    return;
  }
  m_current_security = s;
  m_change_security_signal(s);
  setWindowTitle(QString::fromStdString(ToString(s)) +
    tr(" - Time and Sales"));
}

void time_and_sales_window::update_model(const Security& s) {
  auto model = std::make_shared<periodic_time_and_sales_model>(s);
  model->set_price(Money(Quantity(20)));
  model->set_price_range(time_and_sales_properties::price_range::AT_ASK);
  model->set_period(boost::posix_time::time_duration(0, 0, 1, 0));
  set_model(model);
}

void time_and_sales_window::update_row(
    const time_and_sales_model::entry& entry) {
  
}

void time_and_sales_window::update_volume(const Quantity& volume) {
  m_volume_label->setText(tr("Volume: ") + Beam::ToString(volume).c_str());
}
