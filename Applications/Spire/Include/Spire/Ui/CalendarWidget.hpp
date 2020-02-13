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
      class MonthSpinBox : public QWidget {
        public:

          using MonthSignal = Signal<void (const boost::gregorian::date&)>;

          explicit MonthSpinBox(const boost::gregorian::date& initial_date,
              QWidget* parent = nullptr)
              : QWidget(parent) {
            auto layout = new QHBoxLayout(this);
            layout->setSpacing(0);
            layout->setContentsMargins({});
            m_left_label = new QLabel(this);
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
            m_right_label->installEventFilter(this);
            m_right_label->setFixedSize(scale(16, 26));
            m_right_label->setPixmap(QPixmap::fromImage(
              imageFromSvg(":/Icons/arrow-right.svg", scale(16, 26),
              QRect(translate(6, 10), scale(4, 6)))));
            layout->addWidget(m_right_label);
            set_date(initial_date);
            update_label();
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

      mutable DateSignal m_date_signal;
      MonthSpinBox* m_month_spin_box;
      boost::gregorian::date m_selected_date;
      CalendarDayWidget* m_selected_date_widget;
      QGridLayout* m_calendar_layout;
      std::array<boost::gregorian::date, 42> m_dates;

      void on_date_selected(const boost::gregorian::date& date);
      void on_month_changed(const boost::gregorian::date& date);
      void add_day_label(QLayout* layout, const QString& text);
      CalendarDayWidget* get_day_widget(const boost::gregorian::date& date);
      void set_highlight();
      void update_calendar(const boost::gregorian::date& displayed_date);
  };
}

#endif
