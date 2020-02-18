#ifndef SPIRE_CALENDAR_WIDGET_HPP
#define SPIRE_CALENDAR_WIDGET_HPP
#include <array>
#include <boost/date_time/gregorian/greg_date.hpp>
#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QWidget>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Scalar.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  class CalendarWidget : public QWidget {
    public:

      using DateSignal = Signal<void (const boost::gregorian::date&)>;

      explicit CalendarWidget(const boost::gregorian::date& selected_date,
        QWidget* parent = nullptr);

      void set_date(const boost::gregorian::date& date);

      boost::signals2::connection connect_date_signal(
          const DateSignal::slot_type& slot) const {
        return m_date_signal.connect(slot);
      }

    private:
      class MonthAndYearSpinBox : public QWidget {
        public:

          using MonthSignal = Signal<void (const boost::gregorian::date&)>;

          explicit MonthAndYearSpinBox(const boost::gregorian::date& initial_date,
              QWidget* parent = nullptr)
              : QWidget(parent) {
            setFocusPolicy(Qt::NoFocus);
            setCursor(Qt::ArrowCursor);
            auto layout = new QHBoxLayout(this);
            layout->setSpacing(0);
            layout->setContentsMargins({});
            m_left_label = new QLabel(this);
            m_left_label->setStyleSheet("background-color: #FFFFFF;");
            m_left_label->setFocusPolicy(Qt::NoFocus);
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
                background-color: #FFFFFF;
                color: #000000;
                font-family: Roboto;
                font-size: %1px;
              )").arg(scale_height(12)));
            m_month_label->setFixedHeight(scale_height(26));
            layout->addWidget(m_month_label);
            m_right_label = new QLabel(this);
            m_right_label->setStyleSheet("background-color: #FFFFFF;");
            m_right_label->setFocusPolicy(Qt::NoFocus);
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

          const boost::gregorian::date& get_date() const {
            return m_date;
          }

          void set_date(const boost::gregorian::date& date) {
            m_date = date;
            update_label();
          }

          boost::signals2::connection connect_month_signal(
              const MonthSignal::slot_type& slot) const {
            return m_month_signal.connect(slot);
          }

        protected:
          bool eventFilter(QObject* watched, QEvent* event) override {
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

        private:
          mutable MonthSignal m_month_signal;
          boost::gregorian::date m_date;
          QLabel* m_month_label;
          QLabel* m_left_label;
          QLabel* m_right_label;

          void update_label() {
            auto date = boost::posix_time::ptime(m_date);
            // need to be dynamically allocated?
            auto facet = new boost::posix_time::time_facet();
            facet->format("%B");
            std::stringstream stream;
            stream.imbue(std::locale(std::locale::classic(), facet));
            stream << date;
            m_month_label->setText(QString("%1 %2")
              .arg(stream.str().c_str())
              .arg(QString::number(date.date().year())));
          }
      };

      class CalendarDayWidget : public QLabel {
        public:

          using ClickedSignal = Signal<void (const boost::gregorian::date&)>;

          CalendarDayWidget(const boost::gregorian::date& date,
              const QString& text_color_hex, QWidget* parent = nullptr)
              : QLabel(parent),
                m_date(date),
                m_text_color_hex(text_color_hex) {
            setAlignment(Qt::AlignCenter);
            setText(QString::number(date.day()));
            set_default_style();
          }

          void set_highlight() {
            set_selected_style();
          }

          void remove_highlight() {
            set_default_style();
          }

          boost::signals2::connection connect_clicked_signal(
              const ClickedSignal::slot_type& slot) const {
            return m_clicked_signal.connect(slot);
          }

        protected:
          void mousePressEvent(QMouseEvent* event) override {
            m_clicked_signal(m_date);
          }

        private:
          mutable ClickedSignal m_clicked_signal;
          boost::gregorian::date m_date;
          QString m_text_color_hex;

          void set_default_style() {
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
               .arg(m_text_color_hex));
          }

          void set_selected_style() {
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
      };

      class CalendarModel {
        public:

          CalendarModel() = default;

          // indexed at 0
          boost::gregorian::date get_date(int row, int column) {
            return m_dates[row * 7 + column];
          }

          std::tuple<int, int> get_pos(const boost::gregorian::date& date) {
            if(date.month() < m_reference_date.month()) {
              return {date.day_of_week(), 0};
            }
            // TODO: definitely possible to calculate this in constant time.
            auto day_index = std::distance(m_dates.begin(),
              std::find(m_dates.begin(), m_dates.end(), date));
            return {day_index % 7, static_cast<int>(std::floor(day_index / 7))};
          }

          void set_month(int month, int year) {
            m_reference_date = boost::gregorian::date(year, month, 1);
            auto day_count = m_reference_date.end_of_month().day();
            auto first_day_of_week = m_reference_date.day_of_week().as_number();
            for(auto i = 0; i < 42; ++i) {
              if(i < first_day_of_week) {
                m_dates[i] = m_reference_date +
                  boost::gregorian::days(i - first_day_of_week);
              } else if(i > day_count + first_day_of_week - 1) {
                m_dates[i] = boost::gregorian::date(m_reference_date.year(),
                  m_reference_date.month(),
                  i - day_count - first_day_of_week + 1) +
                  boost::gregorian::months(1);
              } else {
                m_dates[i] = boost::gregorian::date(m_reference_date.year(),
                  m_reference_date.month(),
                  i + 1 - first_day_of_week);
              }
            }
          }

        private:
          boost::gregorian::date m_reference_date;
          std::array<boost::gregorian::date, 42> m_dates;
      };

      mutable DateSignal m_date_signal;
      MonthAndYearSpinBox* m_month_spin_box;
      boost::gregorian::date m_selected_date;
      CalendarDayWidget* m_selected_date_widget;
      CalendarModel m_calendar_model;
      QGridLayout* m_calendar_layout;

      void on_date_selected(const boost::gregorian::date& date);
      void on_month_changed(const boost::gregorian::date& date);
      void add_day_label(QLayout* layout, const QString& text);
      CalendarDayWidget* get_day_widget(const boost::gregorian::date& date);
      void set_highlight();
      void update_calendar(const boost::gregorian::date& displayed_date);
  };
}

#endif
