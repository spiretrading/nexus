#include "spire/security_input/security_input_dialog.hpp"
#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>
#include "spire/security_input/security_info_list_view.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/drop_shadow.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

security_input_dialog::security_input_dialog(security_input_model& model,
    QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, Qt::Window | Qt::FramelessWindowHint | flags),
      m_is_dragging(false) {
  setWindowModality(Qt::WindowModal);
  m_shadow = std::make_unique<drop_shadow>(this);
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(scale_width(8), scale_height(6), scale_width(8),
    scale_height(8));
  layout->setSpacing(0);
  setFixedSize(scale(196, 68));
  setObjectName("security_input_dialog");
  setStyleSheet(QString(R"(
    #security_input_dialog {
      background-color: #F5F5F5;
      border: %1px solid #A0A0A0;
    }
  )").arg(scale_width(1)));
  auto text_label = new QLabel(tr("Security"), this);
  text_label->setStyleSheet(QString(R"(
    border: none;
    font-family: Roboto;
    font-size: %1px;
  )").arg(scale_height(12)));
  layout->addWidget(text_label);
  layout->setStretchFactor(text_label, 14);
  layout->addStretch(10);
  m_security_input_box = new security_input_box(model, this);
  m_security_input_box->connect_commit_signal(
    [=] (const Security& s) { set_security(s); });
  layout->addWidget(m_security_input_box);
  layout->setStretchFactor(m_security_input_box, 30);
}

const Security& security_input_dialog::get_security() const noexcept {
  return m_security;
}

void security_input_dialog::closeEvent(QCloseEvent* event) {
  reject();
}

void security_input_dialog::mouseMoveEvent(QMouseEvent* event) {
  if(!m_is_dragging) {
    return;
  }
  auto delta = event->globalPos();
  delta -= m_last_mouse_pos;
  auto window_pos = window()->pos();
  window_pos += delta;
  m_last_mouse_pos = event->globalPos();
  window()->move(window_pos);
}

void security_input_dialog::mousePressEvent(QMouseEvent* event) {
  if(m_is_dragging || event->button() != Qt::LeftButton) {
    return;
  }
  m_is_dragging = true;
  m_last_mouse_pos = event->globalPos();
}

void security_input_dialog::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() != Qt::LeftButton) {
    return;
  }
  m_is_dragging = false;
}

void security_input_dialog::set_security(const Security& security) {
  m_security = security;
  accept();
}
