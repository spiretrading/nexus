#include "Spire/Ui/DurationInputWidget.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QRegularExpressionValidator>
#include "Spire/Spire/Utility.hpp"

using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;

DurationInputWidget::DurationInputWidget(QWidget* parent)
    : QWidget(parent),
      m_last_valid_hour(12),
      m_last_valid_minute(0) {
  auto layout = new QHBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins({});
  m_hour_line_edit = new QLineEdit("12", this);
  m_hour_line_edit->setValidator(new QRegularExpressionValidator(
    QRegularExpression("^\\d\\d$"), this));
  m_hour_line_edit->setFixedSize(scale(31, 26));
  m_hour_line_edit->setAlignment(Qt::AlignRight);
  m_hour_line_edit->installEventFilter(this);
  layout->addWidget(m_hour_line_edit);
  m_colon_widget1 = new ColonWidget(this);
  m_colon_widget1->setFixedSize(scale(3, 26));
  layout->addWidget(m_colon_widget1);
  m_minute_line_edit = new QLineEdit("00", this);
  m_minute_line_edit->setAlignment(Qt::AlignCenter);
  m_minute_line_edit->setValidator(new QRegularExpressionValidator(
    QRegularExpression("^\\d\\d$"), this));
  m_minute_line_edit->setFixedSize(scale(20, 26));
  m_minute_line_edit->installEventFilter(this);
  layout->addWidget(m_minute_line_edit);
  m_colon_widget2 = new ColonWidget(this);
  m_colon_widget2->setFixedSize(scale(3, 26));
  layout->addWidget(m_colon_widget2);
  m_second_line_edit = new QLineEdit("00", this);
  m_second_line_edit->setAlignment(Qt::AlignLeft);
  m_second_line_edit->setValidator(new QRegularExpressionValidator(
    QRegularExpression("^\\d\\d$"), this));
  m_second_line_edit->setFixedSize(scale(31, 26));
  m_second_line_edit->installEventFilter(this);
  layout->addWidget(m_second_line_edit);
  set_unfocused_style();
}

void DurationInputWidget::set_time(const time_duration& duration) {
  auto hour = duration.hours();
  auto minute = duration.minutes();
  if(hour < 12) {
    if(hour == 0) {
      hour = 12;
    }
  } else {
    if(hour != 12) {
      hour -= 12;
    }
  }
  m_hour_line_edit->setText(clamped_value(QString::number(hour), 1, 12));
  m_minute_line_edit->setText(clamped_value(QString::number(minute), 0, 59));
}

connection DurationInputWidget::connect_time_signal(
    const DurationSignal::slot_type& slot) const {
  return m_duration_signal.connect(slot);
}

bool DurationInputWidget::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_hour_line_edit) {
    if(event->type() == QEvent::FocusIn) {
      set_focus_style();
    } else if(event->type() == QEvent::FocusOut) {
      m_hour_line_edit->setText(clamped_value(
        QString::number(m_last_valid_hour), 1, 12));
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
        if(m_hour_line_edit->text() != "0") {
          on_time_changed();
        }
      }
    }
  } else if(watched == m_minute_line_edit) {
    if(event->type() == QEvent::FocusIn) {
      set_focus_style();
    } else if(event->type() == QEvent::FocusOut) {
      m_minute_line_edit->setText(clamped_value(
        QString::number(m_last_valid_minute), 0, 59));
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

QString DurationInputWidget::clamped_value(const QString& text, int min_value,
    int max_value) {
  return clamped_value(text, min_value, max_value, 0);
}

QString DurationInputWidget::clamped_value(const QString& text, int min_value,
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
  return QString::number(min_value);
}

QString DurationInputWidget::get_line_edit_value(const QString& text, int key,
    int min_value, int max_value) {
  if(key == Qt::Key_Up) {
    return clamped_value(text, min_value, max_value, 1);
  }
  return clamped_value(text, min_value, max_value, -1);
}

void DurationInputWidget::set_focus_style() {
  set_style("#4B23A0");
  m_colon_widget1->set_style("#4B23A0");
  m_colon_widget2->set_style("#4B23A0");
}

void DurationInputWidget::set_unfocused_style() {
  set_style("#C8C8C8");
  m_colon_widget1->set_style("#C8C8C8");
  m_colon_widget2->set_style("#C8C8C8");
}

void DurationInputWidget::set_style(const QString& border_hex) {
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
      border-bottom: %1px solid %2;
      border-left: none;
      border-right: none;
      border-top: %1px solid %2;
      font-family: Roboto;
      font-size: %3px;
      padding-left: %1px;
    })").arg(scale_height(1)).arg(border_hex)
        .arg(scale_height(12)));
  m_second_line_edit->setStyleSheet(QString(R"(
    QLineEdit {
      border-bottom: %2px solid %3;
      border-left: none;
      border-right: %1px solid %3;
      border-top: %2px solid %3;
      font-family: Roboto;
      font-size: %4px;
      padding-left: %5px;
    })").arg(scale_width(1)).arg(scale_height(1)).arg(border_hex)
        .arg(scale_height(12)).arg(scale_width(2)));
}

void DurationInputWidget::on_drop_down_changed(const QString& item) {
  m_hour_line_edit->setText(clamped_value(
    QString::number(m_last_valid_hour), 1, 12));
  m_minute_line_edit->setText(clamped_value(
    QString::number(m_last_valid_minute), 0, 59));
  on_time_changed();
}

void DurationInputWidget::on_time_changed() {
  auto hour_ok = false;
  auto minute_ok = false;
  auto hour = m_hour_line_edit->text().toInt(&hour_ok);
  auto minute = m_minute_line_edit->text().toInt(&minute_ok);
  if(hour_ok && minute_ok) {
    m_last_valid_hour = hour;
    m_last_valid_minute = minute;
    //if(m_drop_down_menu->get_text() == tr("AM") && hour == 12) {
    //  hour = 0;
    //} else if(m_drop_down_menu->get_text() == tr("PM")) {
    //  if(hour != 12) {
    //    hour += 12;
    //  }
    //}
    m_duration_signal(hours(hour) + minutes(minute));
  }
}
