#include "Spire/Ui/MonthAndYearSpinBox.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <QEvent>
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;

MonthAndYearSpinBox::MonthAndYearSpinBox(date initial_date, QWidget* parent)
    : QWidget(parent) {
  setFocusPolicy(Qt::NoFocus);
  setCursor(Qt::ArrowCursor);
  auto layout = new QHBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins({});
  m_left_label = new QLabel(this);
  m_left_label->setStyleSheet("QLabel { background-color: #FFFFFF; }");
  m_left_label->setFocusProxy(parent);
  m_left_label->setCursor({QPixmap::fromImage(
    imageFromSvg(":/Icons/finger-cursor.svg", scale(18, 18))), 0, 0});
  m_left_label->installEventFilter(this);
  m_left_label->setFixedSize(scale(16, 26));
  m_left_label->setPixmap(QPixmap::fromImage(
    imageFromSvg(":/Icons/arrow-left.svg", scale(16, 26),
    QRect(translate(6, 10), scale(4, 6)))));
  layout->addWidget(m_left_label);
  m_month_label = new QLabel(this);
  m_month_label->setAlignment(Qt::AlignCenter);
  m_month_label->setStyleSheet(QString(R"(
    QLabel {
      background-color: #FFFFFF;
      color: #000000;
      font-family: Roboto;
      font-size: %1px;
    })").arg(scale_height(12)));
  m_month_label->setFixedHeight(scale_height(26));
  layout->addWidget(m_month_label);
  m_right_label = new QLabel(this);
  m_right_label->setStyleSheet("QLabel { background-color: #FFFFFF; }");
  m_right_label->setCursor({QPixmap::fromImage(
    imageFromSvg(":/Icons/finger-cursor.svg", scale(18, 18))), 0, 0});
  m_right_label->installEventFilter(this);
  m_right_label->setFixedSize(scale(16, 26));
  m_right_label->setPixmap(QPixmap::fromImage(
    imageFromSvg(":/Icons/arrow-right.svg", scale(16, 26),
    QRect(translate(6, 10), scale(4, 6)))));
  layout->addWidget(m_right_label);
  set_date(initial_date);
  update_label();
}

date MonthAndYearSpinBox::get_date() const {
  return m_date;
}

void MonthAndYearSpinBox::set_date(date current_date) {
  m_date = current_date;
  update_label();
}

connection MonthAndYearSpinBox::connect_month_signal(
    const MonthSignal::slot_type& slot) const {
  return m_month_signal.connect(slot);
}

bool MonthAndYearSpinBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_left_label) {
    if(event->type() == QEvent::MouseButtonRelease) {
      m_date -= boost::gregorian::months(1);
      update_label();
      m_month_signal(m_date);
    }
  } else if(watched == m_right_label) {
    if(event->type() == QEvent::MouseButtonRelease) {
      m_date += boost::gregorian::months(1);
      update_label();
      m_month_signal(m_date);
    }
  }
  return QWidget::eventFilter(watched, event);
}

void MonthAndYearSpinBox::update_label() {
  auto date = ptime(m_date);
  auto facet = new time_facet();
  facet->format("%B");
  std::stringstream stream;
  stream.imbue(std::locale(std::locale::classic(), facet));
  stream << date;
  m_month_label->setText(QString("%1 %2")
    .arg(stream.str().c_str())
    .arg(QString::number(date.date().year())));
}
