#include "Spire/Ui/DropDownMenuItem2.hpp"
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

DropDownMenuItem2::DropDownMenuItem2(QVariant value, QWidget* parent)
    : QWidget(parent),
      m_value(std::move(value)),
      m_is_highlighted(false) {
  setAttribute(Qt::WA_Hover);
}

void DropDownMenuItem2::enterEvent(QEvent* event) {
  m_hovered_signal();
}

void DropDownMenuItem2::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    m_selected_signal();
  }
}

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
