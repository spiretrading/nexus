#include "spire/security_input/security_input_dialog.hpp"
#include <QApplication>
#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>
#include "spire/security_input/security_info_list_view.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/drop_shadow.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

SecurityInputDialog::SecurityInputDialog(SecurityInputModel& model,
    QWidget* parent, Qt::WindowFlags flags)
    : SecurityInputDialog(model, "", parent, flags) {}

SecurityInputDialog::SecurityInputDialog(SecurityInputModel& model,
    const QString& initial_text, QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, Qt::FramelessWindowHint | flags),
      m_is_dragging(false) {
  setWindowModality(Qt::WindowModal);
  m_shadow = std::make_unique<DropShadow>(this);
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(scale_width(8), scale_height(6), scale_width(8),
    scale_height(8));
  layout->setSpacing(0);
  setFixedSize(scale(196, 68));
  setObjectName("SecurityInputDialog");
  setStyleSheet(QString(R"(
    #SecurityInputDialog {
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
  m_security_input_box = new SecurityInputBox(model, initial_text, this);
  m_security_input_box->connect_commit_signal(
    [=] (const Security& s) { set_security(s); });
  layout->addWidget(m_security_input_box);
  layout->setStretchFactor(m_security_input_box, 30);
}

SecurityInputDialog::~SecurityInputDialog() = default;

const Security& SecurityInputDialog::get_security() const noexcept {
  return m_security;
}

void SecurityInputDialog::changeEvent(QEvent* event) {
  if(event->type() == QEvent::ActivationChange) {
    if(QApplication::activeWindow() != this) {
      for(auto& child : m_security_input_box->children()) {
        auto c = qobject_cast<QWidget*>(child);
        if(c != nullptr && c->isActiveWindow()) {
          c->installEventFilter(this);
          return;
        }
      }
      reject();
    }
  }
}

void SecurityInputDialog::closeEvent(QCloseEvent* event) {
  reject();
}

bool SecurityInputDialog::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::ActivationChange) {
    auto c = static_cast<QWidget*>(watched);
    if(QApplication::activeWindow() != c) {
      if(QApplication::activeWindow() == this) {
        c->removeEventFilter(this);
      } else {
        reject();
      }
    }
  }
  return QDialog::eventFilter(watched, event);
}

void SecurityInputDialog::mouseMoveEvent(QMouseEvent* event) {
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

void SecurityInputDialog::mousePressEvent(QMouseEvent* event) {
  if(m_is_dragging || event->button() != Qt::LeftButton) {
    return;
  }
  m_is_dragging = true;
  m_last_mouse_pos = event->globalPos();
}

void SecurityInputDialog::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() != Qt::LeftButton) {
    return;
  }
  m_is_dragging = false;
}

void SecurityInputDialog::set_security(const Security& security) {
  m_security = security;
  accept();
}
