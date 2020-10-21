#include "Spire/UiViewer/TimeOfDayTestWidget.hpp"
#include <QGridLayout>
#include "Spire/Ui/FlatButton.hpp"

using namespace Spire;

namespace {
  auto CONTROL_SIZE() {
    static auto size = scale(50, 26);
    return size;
  }
}

TimeOfDayTestWidget::TimeOfDayTestWidget(QWidget* parent)
    : QWidget(parent) {
  auto container = new QWidget(this);
  auto layout = new QGridLayout(container);
  auto status_label = new QLabel(this);
  status_label->setMinimumWidth(scale_width(100));
  layout->addWidget(status_label, 0, 0, 1, 2);
  m_time_input = new TimeOfDayInputWidget(this);
  m_time_input->connect_time_signal([=] (const auto& time) {
    auto minute_text = [&] {
      if(time.minutes() < 10) {
        return QString("0%1").arg(QString::number(time.minutes()));
      }
      return QString::number(time.minutes());
    }();
    auto period_text = [&] {
      if(time.hours() > 11) {
        return QString(tr("PM"));
      }
      return QString(tr("AM"));
    }();
    status_label->setText(QString("%1 : %2 %3").arg(time.hours()).arg(
      minute_text).arg(period_text));
  });
  layout->addWidget(m_time_input, 1, 0, 1, 2);
  m_hour_input = new TextInputWidget(this);
  m_hour_input->setFixedSize(CONTROL_SIZE());
  layout->addWidget(m_hour_input, 2, 0);
  m_minute_input = new TextInputWidget(this);
  m_minute_input->setFixedSize(CONTROL_SIZE());
  layout->addWidget(m_minute_input, 2, 1);
  auto set_button = make_flat_button(tr("Set 24H Time"), this);
  set_button->setFixedHeight(scale_height(26));
  set_button->connect_clicked_signal([=] { on_set_button(); });
  layout->addWidget(set_button, 3, 0, 1, 2);
}

void TimeOfDayTestWidget::on_set_button() {
  auto hour_ok = false;
  auto hour = m_hour_input->text().toInt(&hour_ok);
  auto minute_ok = false;
  auto minute = m_minute_input->text().toInt(&minute_ok);
  if(hour_ok && minute_ok) {
    m_time_input->set_time({hour, minute, 0});
  }
}
