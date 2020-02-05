#include "Spire/Ui/TimeInputWidget.hpp"
#include <boost/date_time/posix_time/ptime.hpp>
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/DropDownMenu.hpp"

using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;

TimeInputWidget::TimeInputWidget(QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setSpacing(scale_width(8));
  layout->setContentsMargins({});
  m_time_edit = new QTimeEdit({12, 0}, this);
  m_time_edit->setFixedHeight(scale_height(26));
  m_time_edit->setDisplayFormat("hh:mm");
  m_time_edit->setButtonSymbols(QAbstractSpinBox::NoButtons);
  m_time_edit->setMaximumTime({12, 59});
  m_time_edit->setMinimumTime({1, 00});
  m_time_edit->setStyleSheet(QString(R"(
    QTimeEdit {
      background-color: #FFFFFF;
      border: %1px solid #C8C8C8 %2px solid #C8C8C8;
      color: #000000;
      font-family: Roboto;
      font-size: %3px;
      padding-left: %4px;
    }

    QTimeEdit:focus {
      border: %1px solid #4B23A0 %2px solid #4B23A0;
    })").arg(scale_height(1)).arg(scale_width(1)).arg(scale_height(12))
        .arg(scale_width(8)));
  layout->addWidget(m_time_edit);
  m_drop_down_menu = new DropDownMenu({tr("PM"), tr("AM")}, this);
  m_drop_down_menu->setFixedHeight(scale_height(26));
  layout->addWidget(m_drop_down_menu);
}

void TimeInputWidget::set_time(Scalar time) {
  auto timestamp = static_cast<ptime>(time);
  auto hours = [&] {
    auto hour = static_cast<int>(timestamp.time_of_day().hours());
    if(hour >= 12 && hour < 24) {
      m_drop_down_menu->set_current_text(tr("PM"));
      return hour - 12;
    }
    m_drop_down_menu->set_current_text(tr("AM"));
    if(hour == 0) {
      return 12;
    }
    return hour;
  }();
  m_time_edit->setTime({hours,
    static_cast<int>(timestamp.time_of_day().minutes())});
}

connection TimeInputWidget::connect_time_signal(
    const TimeSignal::slot_type& slot) const {
  return m_time_signal.connect(slot);
}
