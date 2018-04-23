#include "spire/time_and_sales/time_and_sales_window.hpp"
#include <QKeyEvent>
#include <QVBoxLayout>
#include "spire/time_and_sales/empty_time_and_sales_model.hpp"
#include "spire/security_input/security_input_dialog.hpp"
#include "spire/security_input/security_input_model.hpp"

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
  set_model(std::make_shared<empty_time_and_sales_model>(Security()));
  setWindowTitle(tr("New Window"));
}

void time_and_sales_window::set_model(
    std::shared_ptr<time_and_sales_model> model) {
  m_model.emplace(std::move(model), m_properties);
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
    auto s = m_securities.push_front(m_current_security);
    if(s != Security()) {
      set_current(s);
    }
    return;
  } else if(event->key() == Qt::Key_PageDown) {
    auto s = m_securities.push_back(m_current_security);
    if(s != Security()) {
      set_current(s);
    }
    return;
  }
  auto pressed_key = event->text();
  if(pressed_key[0].isLetterOrNumber()) {
    auto dialog = new security_input_dialog(*m_input_model, pressed_key, this);
    if(dialog->exec() == QDialog::Accepted) {
      if(dialog->get_security() != Security()) {
        m_securities.push(m_current_security);
        set_current(dialog->get_security());
        activateWindow();
      }
    }
  }
}

void time_and_sales_window::set_current(const Security& s) {
  m_current_security = s;
  m_change_security_signal(s);
  setWindowTitle(QString::fromStdString(ToString(s)));
}
