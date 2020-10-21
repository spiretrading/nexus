#include "Spire/UiViewer/DurationInputTestWidget.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <QGridLayout>
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/FlatButton.hpp"

using namespace boost::posix_time;
using namespace Spire;

namespace {
  auto INPUT_SIZE() {
    static auto size = scale(50, 26);
    return size;
  }
}

DurationInputTestWidget::DurationInputTestWidget(QWidget* parent)
    : QWidget(parent) {
  auto container = new QWidget(this);
  auto layout = new QGridLayout(container);
  m_duration_input = new DurationInputWidget(this);
  m_duration_input->setFixedSize(scale(88, 26));
  m_duration_input->connect_time_signal([=] (const auto& time) {
    auto minutes_string = [&] {
      if(time.minutes() < 10) {
        return QString("0%1").arg(time.minutes());
      }
      return QString::number(time.minutes());
    }();
    auto seconds_string = [&] {
      if(time.seconds() < 10) {
        return QString("0%1").arg(time.seconds());
      }
      return QString::number(time.seconds());
    }();
    m_status_label->setText(QString("%1 : %2 : %3").arg(time.hours()).arg(
      minutes_string).arg(seconds_string));
  });
  layout->addWidget(m_duration_input, 0, 0);
  m_status_label = new QLabel(this);
  m_status_label->setAlignment(Qt::AlignCenter);
  layout->addWidget(m_status_label, 0, 1);
  auto input_container = new QWidget(this);
  layout->addWidget(input_container, 1, 0, 1, 2);
  auto container_layout = new QHBoxLayout(input_container);
  m_hour_input = new TextInputWidget(this);
  m_hour_input->setFixedSize(INPUT_SIZE());
  container_layout->addWidget(m_hour_input);
  m_minute_input = new TextInputWidget(this);
  m_minute_input->setFixedSize(INPUT_SIZE());
  container_layout->addWidget(m_minute_input);
  m_second_input = new TextInputWidget(this);
  m_second_input->setFixedSize(INPUT_SIZE());
  container_layout->addWidget(m_second_input);
  auto set_duration_button = make_flat_button(tr("Set Duration"), this);
  set_duration_button->setFixedHeight(scale_height(26));
  set_duration_button->connect_clicked_signal([=] { on_set_button(); });
  layout->addWidget(set_duration_button, 2, 0, 1, 2);
}

void DurationInputTestWidget::on_set_button() {
  auto hour_ok = false;
  auto hour = m_hour_input->text().toInt(&hour_ok);
  auto minute_ok = false;
  auto minute = m_minute_input->text().toInt(&minute_ok);
  auto second_ok = false;
  auto second = m_second_input->text().toInt(&second_ok);
  if(hour_ok && minute_ok && second_ok) {
    m_duration_input->set_duration({hour, minute, second});
    m_status_label->setText("");
  } else {
    m_status_label->setText(tr("Invalid Inputs"));
  }
}
