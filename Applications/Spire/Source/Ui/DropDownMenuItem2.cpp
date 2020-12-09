#include "Spire/Ui/DropDownMenuItem2.hpp"
#include <QKeyEvent>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

const QVariant& DropDownMenuItem2::get_value() const {
  return m_value;
}

bool DropDownMenuItem2::is_highlighted() const {
  return m_is_highlighted;
}

void DropDownMenuItem2::set_highlighted() {
  m_is_highlighted = true;
  update();
}

void DropDownMenuItem2::reset_highlighted() {
  m_is_highlighted = false;
  update();
}

connection DropDownMenuItem2::connect_hovered_signal(
    const HoveredSignal::slot_type& slot) const {
  return m_hovered_signal.connect(slot);
}

connection DropDownMenuItem2::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

DropDownMenuItem2::DropDownMenuItem2(QVariant value, QWidget* parent)
    : QWidget(parent),
      m_value(std::move(value)),
      m_is_highlighted(false) {
  setAttribute(Qt::WA_Hover);
}

void DropDownMenuItem2::enterEvent(QEvent* event) {
  event->accept();
  m_hovered_signal();
  QWidget::enterEvent(event);
}

void DropDownMenuItem2::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
      event->accept();
      m_selected_signal();
      break;
  }
  if(!event->isAccepted()) {
    QWidget::keyPressEvent(event);
  }
}

void DropDownMenuItem2::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    event->accept();
    m_selected_signal();
  }
  QWidget::mousePressEvent(event);
}
