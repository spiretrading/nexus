#include "spire/ui/security_widget.hpp"
#include <QKeyEvent>
#include "spire/security_input/security_input_dialog.hpp"
#include "spire/spire/dimensions.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

SecurityWidget::SecurityWidget(Ref<SecurityInputModel> input_model,
    Theme theme, QWidget* parent)
    : QWidget(parent),
      m_input_model(input_model.Get()) {
  if(theme == Theme::DARK) {
    setStyleSheet("background-color: #D0D0D0;");
  } else {
    setStyleSheet("background-color: #FFFFFF;");
  }
  m_layout = new QVBoxLayout(this);
  m_layout->setContentsMargins({});
  m_empty_window_label = std::make_unique<QLabel>(
    tr("Enter a ticker symbol."), this);
  m_empty_window_label->setAlignment(Qt::AlignCenter);
  m_empty_window_label->setStyleSheet(QString(R"(
    font-family: Roboto;
    font-size: %1px;
    padding-top: %2px;)").arg(scale_height(12)).arg(scale_height(16)));
  m_layout->addWidget(m_empty_window_label.get());
  window()->installEventFilter(this);
}

void SecurityWidget::set_widget(QWidget* widget) {
  m_empty_window_label.reset();
  m_widget = widget;
  m_layout->addWidget(m_widget);
  m_widget->show();
  m_widget->installEventFilter(this);
}

connection SecurityWidget::connect_security_change_signal(
    const ChangeSecuritySignal::slot_type& slot) const {
  return m_security_change_signal.connect(slot);
}

bool SecurityWidget::eventFilter(QObject* object, QEvent* event) {
  if(object == window() || object == m_widget) {
    if(event->type() == QEvent::KeyPress) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() == Qt::Key_PageUp) {
        if(m_current_security != Security()) {
          auto s = m_securities.push_front(m_current_security);
          if(s != Security()) {
            m_current_security = s;
            m_security_change_signal(s);
          }
        }
        event->accept();
        return true;
      } else if(e->key() == Qt::Key_PageDown) {
        if(m_current_security != Security()) {
          auto s = m_securities.push_back(m_current_security);
          if(s != Security()) {
            m_current_security = s;
            m_security_change_signal(s);
          }
        }
        event->accept();
        return true;
      }
      auto pressed_key = e->text();
      if(pressed_key[0].isLetterOrNumber()) {
        auto dialog = new SecurityInputDialog(Ref(*m_input_model), pressed_key,
          this);
        dialog->setWindowModality(Qt::NonModal);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(dialog, &QDialog::accepted, this,
          [=] { on_security_input_accept(dialog); });
        connect(dialog, &QDialog::rejected, this,
          [=] { on_security_input_reject(dialog); });
        dialog->move(window()->geometry().center().x() -
          dialog->width() / 2, window()->geometry().center().y() -
          dialog->height() / 2);
        show_overlay_widget();
        dialog->show();
        event->accept();
        return true;
      }
    }
  }
  return QWidget::eventFilter(object, event);
}

void SecurityWidget::show_overlay_widget() {
  auto p = static_cast<QWidget*>(parent());
  m_overlay_widget = std::make_unique<QLabel>(p);
  m_overlay_widget->setStyleSheet(
    "background-color: rgba(245, 245, 245, 153);");
  m_overlay_widget->resize(p->size());
  m_overlay_widget->move(0, 0);
  m_overlay_widget->show();
}

void SecurityWidget::hide_overlay_widget() {
  m_overlay_widget.reset();
}

void SecurityWidget::on_security_input_accept(SecurityInputDialog* dialog) {
  auto s = dialog->get_security();
  if(s != Security() && s != m_current_security) {
    m_empty_window_label.reset();
    m_securities.push(m_current_security);
    m_current_security = s;
    activateWindow();
    m_security_change_signal(s);
  }
  dialog->close();
  m_overlay_widget.reset();
}

void SecurityWidget::on_security_input_reject(SecurityInputDialog* dialog) {
  dialog->close();
  m_overlay_widget.reset();
}
