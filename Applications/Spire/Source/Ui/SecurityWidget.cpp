#include "Spire/Ui/SecurityWidget.hpp"
#include <QKeyEvent>
#include "Spire/SecurityInput/SecurityInputDialog.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

SecurityWidget::SecurityWidget(Ref<SecurityInputModel> input_model,
    Theme theme, QWidget* parent)
    : QWidget(parent),
      m_input_model(input_model.Get()),
      m_empty_window_label(nullptr),
      m_overlay_widget(nullptr) {
  setFocusPolicy(Qt::ClickFocus);
  auto empty_label_font_color = QColor();
  if(theme == Theme::DARK) {
    setStyleSheet("background-color: #25212E;");
    empty_label_font_color = Qt::white;
  } else {
    setStyleSheet("background-color: #FFFFFF;");
    empty_label_font_color = Qt::black;
  }
  m_layout = new QVBoxLayout(this);
  m_layout->setContentsMargins({});
  m_empty_window_label = new QLabel(tr("Enter a ticker symbol."), this);
  m_empty_window_label->setAlignment(Qt::AlignCenter);
  m_empty_window_label->setStyleSheet(QString(R"(
    color: %3;
    font-family: Roboto;
    font-size: %1px;
    padding-top: %2px;)").arg(scale_height(12)).arg(scale_height(16))
    .arg(empty_label_font_color.name()));
  m_layout->addWidget(m_empty_window_label);
}

void SecurityWidget::set_widget(QWidget* widget) {
  delete_later(m_empty_window_label);
  m_widget = widget;
  m_layout->addWidget(m_widget);
  m_widget->show();
  m_widget->installEventFilter(this);
}

connection SecurityWidget::connect_change_security_signal(
    const ChangeSecuritySignal::slot_type& slot) const {
  return m_change_security_signal.connect(slot);
}

void SecurityWidget::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_PageUp) {
    if(m_current_security != Security()) {
      auto s = m_securities.push_front(m_current_security);
      if(s != Security()) {
        m_current_security = s;
        m_change_security_signal(s);
      }
    }
    event->accept();
  } else if(event->key() == Qt::Key_PageDown) {
    if(m_current_security != Security()) {
      auto s = m_securities.push_back(m_current_security);
      if(s != Security()) {
        m_current_security = s;
        m_change_security_signal(s);
      }
    }
    event->accept();
  }
  auto pressed_key = event->text();
  if(!pressed_key.isEmpty() && pressed_key[0].isLetterOrNumber()) {
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
  }
}

void SecurityWidget::show_overlay_widget() {
  auto p = static_cast<QWidget*>(parent());
  m_overlay_widget = new QLabel(p);
  m_overlay_widget->setStyleSheet(
    "background-color: rgba(245, 245, 245, 153);");
  m_overlay_widget->resize(p->size());
  m_overlay_widget->move(0, 0);
  m_overlay_widget->show();
}

void SecurityWidget::hide_overlay_widget() {
  delete_later(m_overlay_widget);
}

void SecurityWidget::on_security_input_accept(SecurityInputDialog* dialog) {
  auto& security = dialog->get_security();
  if(security != Security() && security != m_current_security) {
    delete_later(m_empty_window_label);
    m_securities.push(m_current_security);
    m_current_security = security;
    m_change_security_signal(security);
  }
  dialog->close();
  hide_overlay_widget();
}

void SecurityWidget::on_security_input_reject(SecurityInputDialog* dialog) {
  dialog->close();
  hide_overlay_widget();
}
