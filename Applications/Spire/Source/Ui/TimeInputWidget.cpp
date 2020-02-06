#include "Spire/Ui/TimeInputWidget.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QRegularExpressionValidator>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/DropDownMenu.hpp"

using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;

namespace {
  const auto TWELVE_HOUR_SECONDS = 12 * 60 * 60;
}

TimeInputWidget::TimeInputWidget(QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins({});
  m_hour_line_edit = new QLineEdit("12", this);
  m_hour_line_edit->setValidator(new QRegularExpressionValidator(
    QRegularExpression("^[0|1]\\d$"), this));
  m_hour_line_edit->setFixedSize(scale(26, 26));
  m_hour_line_edit->setAlignment(Qt::AlignRight);
  m_hour_line_edit->installEventFilter(this);
  layout->addWidget(m_hour_line_edit);
  m_minute_line_edit = new QLineEdit("00", this);
  m_minute_line_edit->setValidator(new QRegularExpressionValidator(
    QRegularExpression("^[0-5]\\d$"), this));
  m_minute_line_edit->setFixedSize(scale(24, 26));
  m_minute_line_edit->installEventFilter(this);
  layout->addWidget(m_minute_line_edit);
  layout->addSpacing(scale_width(8));
  m_drop_down_menu = new DropDownMenu({tr("PM"), tr("AM")}, this);
  m_drop_down_menu->setFixedSize(scale(54, 26));
  layout->addWidget(m_drop_down_menu);
  m_drop_down_menu->connect_selected_signal([=] (auto item) {
    on_drop_down_changed(item);
  });
  set_unfocused_style();
}

bool TimeInputWidget::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_hour_line_edit) {
    if(event->type() == QEvent::FocusIn) {
      set_focus_style();
    } else if(event->type() == QEvent::FocusOut) {
      set_unfocused_style();
    } else if(event->type() == QEvent::KeyPress) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() == Qt::Key_Right &&
          m_hour_line_edit->cursorPosition() == 2) {
        m_minute_line_edit->setFocus();
        m_minute_line_edit->setCursorPosition(0);
      }
    }
  }
  if(watched == m_minute_line_edit) {
    if(event->type() == QEvent::FocusIn) {
      set_focus_style();
    } else if(event->type() == QEvent::FocusOut) {
      set_unfocused_style();
    } else if(event->type() == QEvent::KeyPress) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() == Qt::Key_Left &&
          m_minute_line_edit->cursorPosition() == 0) {
        m_hour_line_edit->setFocus();
        m_hour_line_edit->setCursorPosition(2);
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void TimeInputWidget::set_focus_style() {
  set_style("#4B23A0");
}

void TimeInputWidget::set_unfocused_style() {
  set_style("#C8C8C8");
}

void TimeInputWidget::set_style(const QString& border_hex) {
  m_hour_line_edit->setStyleSheet(QString(R"(
      QLineEdit {
        border-bottom: %2px solid %3;
        border-left: %1px solid %3;
        border-right: none;
        border-top: %2px solid %3;
      })").arg(scale_width(1)).arg(scale_height(1)).arg(border_hex));
  m_minute_line_edit->setStyleSheet(QString(R"(
      QLineEdit {
        border-bottom: %2px solid %3;
        border-left: none;
        border-right: %1px solid %3;
        border-top: %2px solid %3;
      })").arg(scale_width(1)).arg(scale_height(1)).arg(border_hex));
}

void TimeInputWidget::on_drop_down_changed(const QString& item) {
  on_time_changed();
}

void TimeInputWidget::on_time_changed() {
  // parse time from line edits and drop down box.
  //m_time_signal(time);
}
