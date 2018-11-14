#include "spire/ui/dropdown_menu_item.hpp"
#include <QMouseEvent>
#include <QPainter>
#include "spire/spire/dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

DropdownMenuItem::DropdownMenuItem(const QString& text, QWidget* parent)
    : QLabel(text, parent),
      m_is_highlighted(false) {
  setFixedHeight(scale_height(20));
  setStyleSheet(QString(R"(
    QLabel {
      background-color: #FFFFFF;
      font-family: Roboto;
      font-size: %1pt;
      padding-left: %2px;
    }

    QLabel:hover {
      background-color: #F2F2FF;
    })").arg(scale_height(7)).arg(scale_width(5)));
}

void DropdownMenuItem::set_highlight() {
  m_is_highlighted = true;
}

void DropdownMenuItem::remove_highlight() {
  m_is_highlighted = false;
}

connection DropdownMenuItem::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

void DropdownMenuItem::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    m_selected_signal(text());
  }
}

void DropdownMenuItem::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    m_selected_signal(text());
  }
}

void DropdownMenuItem::paintEvent(QPaintEvent* event) {
  if(m_is_highlighted) {
    auto painter = QPainter(this);
    painter.fillRect(event->rect(), QColor("#F2F2FF"));
  }
  QLabel::paintEvent(event);
}
