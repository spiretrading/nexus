#include "Spire/Ui/TimeInputWidget.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QRegularExpressionValidator>
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/DropDownMenu.hpp"

using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;

TimeInputWidget::TimeInputWidget(QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins({});
  m_hour_line_edit = new QLineEdit("12", this);
  m_hour_line_edit->setValidator(new QRegularExpressionValidator(
    QRegularExpression("^\\d\\d$"), this));
  m_hour_line_edit->setFixedSize(scale(23, 26));
  m_hour_line_edit->setAlignment(Qt::AlignRight);
  m_hour_line_edit->installEventFilter(this);
  layout->addWidget(m_hour_line_edit);
  m_colon_widget = new ColonWidget(this);
  m_colon_widget->setFixedSize(scale(3, 26));
  layout->addWidget(m_colon_widget);
  m_minute_line_edit = new QLineEdit("00", this);
  m_minute_line_edit->setValidator(new QRegularExpressionValidator(
    QRegularExpression("^\\d\\d$"), this));
  m_minute_line_edit->setFixedSize(scale(24, 26));
  m_minute_line_edit->installEventFilter(this);
  layout->addWidget(m_minute_line_edit);
  layout->addSpacing(scale_width(8));
  m_drop_down_menu = new DropDownMenu({tr("AM"), tr("PM")}, this);
  m_drop_down_menu->setFixedSize(scale(54, 26));
  layout->addWidget(m_drop_down_menu);
  m_drop_down_menu->connect_selected_signal([=] (auto item) {
    on_drop_down_changed(item);
  });
  set_unfocused_style();
}

void TimeInputWidget::set_time(Scalar time) {
  auto duration = static_cast<time_duration>(time);
  auto hour = duration.hours();
  auto minute = duration.minutes();
  if(hour < 12) {
    if(hour == 0) {
      hour = 12;
    }
    m_drop_down_menu->set_current_text(tr("AM"));
  } else {
    if(hour != 12) {
      hour -= 12;
    }
    m_drop_down_menu->set_current_text(tr("PM"));
  }
  m_hour_line_edit->setText(clamped_value(QString::number(hour), 1, 12));
  m_minute_line_edit->setText(clamped_value(QString::number(minute), 0, 59));
}

connection TimeInputWidget::connect_time_signal(
    const TimeChangeSignal::slot_type& slot) const {
  return m_time_signal.connect(slot);
}

bool TimeInputWidget::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_hour_line_edit) {
    if(event->type() == QEvent::FocusIn) {
      set_focus_style();
    } else if(event->type() == QEvent::FocusOut) {
      set_unfocused_style();
    } else if(event->type() == QEvent::KeyPress) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) {
        if(m_hour_line_edit->text().isEmpty()) {
          m_hour_line_edit->setText(QString::number(0));
        }
        m_hour_line_edit->setText(get_line_edit_value(m_hour_line_edit->text(),
          e->key(), 1, 12));
        on_time_changed();
      } else if(e->key() == Qt::Key_Right &&
          m_hour_line_edit->cursorPosition() ==
          m_hour_line_edit->text().count()) {
        m_minute_line_edit->setFocus();
        m_minute_line_edit->setCursorPosition(0);
      }
    } else if(event->type() == QEvent::KeyRelease) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() >= Qt::Key_0 && e->key() <= Qt::Key_9) {
        if(m_hour_line_edit->text().length() == 2) {
          m_hour_line_edit->setText(clamped_value(m_hour_line_edit->text(),
            1, 12));
        }
        on_time_changed();
      }
    }
  } else if(watched == m_minute_line_edit) {
    if(event->type() == QEvent::FocusIn) {
      set_focus_style();
    } else if(event->type() == QEvent::FocusOut) {
      set_unfocused_style();
    } else if(event->type() == QEvent::KeyPress) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) {
        if(m_minute_line_edit->text().isEmpty()) {
          m_minute_line_edit->setText(QString::number(-1));
        }
        m_minute_line_edit->setText(get_line_edit_value(
          m_minute_line_edit->text(), e->key(), 0, 59));
        on_time_changed();
      } else if(e->key() == Qt::Key_Left &&
          m_minute_line_edit->cursorPosition() == 0) {
        m_hour_line_edit->setFocus();
        m_hour_line_edit->setCursorPosition(2);
      }
    } else if(event->type() == QEvent::KeyRelease) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() >= Qt::Key_0 && e->key() <= Qt::Key_9) {
        if(m_minute_line_edit->text().length() == 2) {
          m_minute_line_edit->setText(clamped_value(m_minute_line_edit->text(),
            0, 59));
        }
        on_time_changed();
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

QString TimeInputWidget::clamped_value(const QString& text, int min_value,
    int max_value) {
  return clamped_value(text, min_value, max_value, 0);
}

QString TimeInputWidget::clamped_value(const QString& text, int min_value,
    int max_value, int addend) {
  auto ok = false;
  auto value = text.toInt(&ok);
  if(ok) {
    value += addend;
    value = min(max_value, max(min_value, value));
    if(value < 10) {
      return QString("0" + QString::number(value));
    }
    return QString::number(value);
  }
  return text;
}

QString TimeInputWidget::get_line_edit_value(const QString& text, int key,
    int min_value, int max_value) {
  if(key == Qt::Key_Up) {
    return clamped_value(text, min_value, max_value, 1);
  }
  return clamped_value(text, min_value, max_value, -1);
}

void TimeInputWidget::set_focus_style() {
  set_style("#4B23A0");
  m_colon_widget->set_style("#4B23A0");
}

void TimeInputWidget::set_unfocused_style() {
  set_style("#C8C8C8");
  m_colon_widget->set_style("#C8C8C8");
}

void TimeInputWidget::set_style(const QString& border_hex) {
  m_hour_line_edit->setStyleSheet(QString(R"(
    QLineEdit {
      border-bottom: %2px solid %3;
      border-left: %1px solid %3;
      border-right: none;
      border-top: %2px solid %3;
      font-family: Roboto;
      font-size: %4px;
    })").arg(scale_width(1)).arg(scale_height(1)).arg(border_hex)
        .arg(scale_height(12)));
  m_minute_line_edit->setStyleSheet(QString(R"(
    QLineEdit {
      border-bottom: %2px solid %3;
      border-left: none;
      border-right: %1px solid %3;
      border-top: %2px solid %3;
      font-family: Roboto;
      font-size: %4px;
    })").arg(scale_width(1)).arg(scale_height(1)).arg(border_hex)
        .arg(scale_height(12)));
}

void TimeInputWidget::on_drop_down_changed(const QString& item) {
  on_time_changed();
}

void TimeInputWidget::on_time_changed() {
  auto hour_ok = false;
  auto minute_ok = false;
  auto hour = m_hour_line_edit->text().toInt(&hour_ok);
  auto minute = m_minute_line_edit->text().toInt(&minute_ok);
  if(hour_ok && minute_ok && hour > 0) {
    if(m_drop_down_menu->get_text() == tr("AM") && hour == 12) {
      hour = 0;
    } else if(m_drop_down_menu->get_text() == tr("PM")) {
      if(hour != 12) {
        hour += 12;
      }
    }
    m_time_signal(Scalar(hours(hour) + minutes(minute)));
  }
}
