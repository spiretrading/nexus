#include "spire/security_input/security_input_dialog.hpp"
#include <QLabel>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"
#include "spire/security_input/security_info_list_view.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

namespace {
  auto make_drop_shadow_effect(QWidget* w) {
    auto drop_shadow = new QGraphicsDropShadowEffect(w);
    drop_shadow->setBlurRadius(scale_width(12));
    drop_shadow->setXOffset(0);
    drop_shadow->setYOffset(0);
    drop_shadow->setColor(QColor(0, 0, 0, 100));
    return drop_shadow;
  }
}

security_input_dialog::security_input_dialog(security_input_model& model)
    : m_is_dragging(false) {
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  setWindowModality(Qt::WindowModal);
  setGraphicsEffect(make_drop_shadow_effect(this));
  auto layout = new QVBoxLayout(this);
  auto margin = static_cast<QGraphicsDropShadowEffect*>(
    graphicsEffect())->blurRadius();
  layout->setMargin(margin);
  m_dialog = new QWidget(this);
  m_dialog->installEventFilter(this);
  m_dialog->setFixedSize(scale(196, 68));
  setFixedSize(m_dialog->size().width() + (margin * 2),
    m_dialog->size().height() + (margin * 2));
  m_dialog->setContentsMargins(scale_width(8), scale_height(6), scale_width(8),
    scale_height(8));
  m_dialog->setObjectName("security_input_dialog");
  m_dialog->setStyleSheet(QString(R"(
    #security_input_dialog {
      background-color: #F5F5F5;
      border: %1px solid #A0A0A0;
    }
  )").arg(scale_width(1)));
  auto dialog_layout = new QVBoxLayout(m_dialog);
  dialog_layout->setMargin(0);
  dialog_layout->setSpacing(scale_height(10));
  auto text_label = new QLabel(tr("Security"), this);
  text_label->setStyleSheet(QString(R"(
    border: none;
    font-family: Roboto;
    font-size: %1px;
  )").arg(scale_height(12)));
  dialog_layout->addWidget(text_label);
  m_security_input_box = new security_input_box(model, this);
  m_security_input_box->connect_commit_signal(
    [&] (const Security& s) { set_security(s); });
  dialog_layout->addWidget(m_security_input_box);
  layout->addWidget(m_dialog);
}

const Security& security_input_dialog::get_security() const noexcept {
  return m_security;
}

void security_input_dialog::closeEvent(QCloseEvent* event) {
  reject();
}

bool security_input_dialog::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_dialog) {
    if(event->type() == QEvent::MouseButtonPress) {
      auto e = static_cast<QMouseEvent*>(event);
      if(m_is_dragging || e->button() != Qt::LeftButton) {
        return false;
      }
      m_is_dragging = true;
      m_last_mouse_pos = e->globalPos();
    } else if(event->type() == QEvent::MouseButtonRelease) {
      auto e = static_cast<QMouseEvent*>(event);
      if(e->button() != Qt::LeftButton) {
        return false;
      }
      m_is_dragging = false;
    } else if(event->type() == QEvent::MouseMove) {
      auto e = static_cast<QMouseEvent*>(event);
      if(!m_is_dragging) {
        return false;
      }
      auto delta = e->globalPos();
      delta -= m_last_mouse_pos;
      auto window_pos = window()->pos();
      window_pos += delta;
      m_last_mouse_pos = e->globalPos();
      window()->move(window_pos);
    }
  }
  return QDialog::eventFilter(watched, event);
}

void security_input_dialog::set_security(const Security& security) {
  m_security = security;
  accept();
}
