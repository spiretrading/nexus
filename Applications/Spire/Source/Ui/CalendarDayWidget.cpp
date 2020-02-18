#include "Spire/Ui/CalendarDayWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::gregorian;
using namespace boost::signals2;
using namespace Spire;

CalendarDayWidget::CalendarDayWidget(date displayed_date,
    const QColor& text_color, QWidget* parent)
    : QLabel(parent),
      m_date(displayed_date),
      m_text_color(text_color) {
  setAlignment(Qt::AlignCenter);
  setText(QString::number(displayed_date.day()));
  set_default_style();
}

void CalendarDayWidget::set_highlight() {
  set_selected_style();
}

void CalendarDayWidget::remove_highlight() {
  set_default_style();
}

connection CalendarDayWidget::connect_clicked_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_clicked_signal.connect(slot);
}

void CalendarDayWidget::mousePressEvent(QMouseEvent* event) {
  m_clicked_signal(m_date);
}

void CalendarDayWidget::set_default_style() {
  setStyleSheet(QString(R"(
    QLabel {
      background-color: #FFFFFF;
      border-radius: %2px;
      color: %3;
      font-family: Roboto;
      font-size: %1px;
    }

    QLabel:hover {
      background-color: #F2F2FF;
    }
  )").arg(scale_height(12)).arg(scale_width(2))
      .arg(m_text_color.name()));
}

void CalendarDayWidget::set_selected_style() {
  setStyleSheet(QString(R"(
    QWidget {
      background-color: #4B23A0;
      color: #FFFFFF;
      border-radius: %2px;
      font-family: Roboto;
      font-size: %1px;
    }
  )").arg(scale_height(12)).arg(scale_width(2)));
}
