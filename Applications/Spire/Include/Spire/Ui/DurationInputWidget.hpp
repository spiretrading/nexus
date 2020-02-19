#ifndef SPIRE_DURATION_INPUT_WIDGET_HPP
#define SPIRE_DURATION_INPUT_WIDGET_HPP
#include <boost/date_time/posix_time/posix_time.hpp>
#include <QPainter>
#include <QWidget>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  class DurationInputWidget : public QWidget {
    public:

      using DurationSignal = Signal<void (boost::posix_time::time_duration)>;

      explicit DurationInputWidget(QWidget* parent = nullptr);

      void set_duration(const boost::posix_time::time_duration& duration);

      boost::signals2::connection connect_time_signal(
        const DurationSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      class ColonWidget : public QWidget {
        public:

          explicit ColonWidget(QWidget* parent = nullptr)
            : QWidget(parent),
              m_border_color("#C8C8C8") {}

          void set_style(const QColor& color_hex) {
            m_border_color = QColor(color_hex);
            update();
          }

        protected:
          void paintEvent(QPaintEvent* event) override {
            auto painter = QPainter(this);
            painter.fillRect(0, 0, width(), height(), Qt::white);
            painter.fillRect(0, 0, width(), scale_height(1), m_border_color);
            painter.fillRect(0, height() - scale_height(1), width(),
              scale_height(1), m_border_color);
            auto font = QFont("Roboto");
            font.setPixelSize(scale_height(12));
            painter.setFont(font);
            painter.setPen(Qt::black);
            painter.drawText(QPoint(0, scale_height(17)), ":");
          }

        private:
          QColor m_border_color;
      };

      mutable DurationSignal m_duration_signal;
      QLineEdit* m_hour_line_edit;
      QLineEdit* m_minute_line_edit;
      QLineEdit* m_second_line_edit;
      ColonWidget* m_colon_widget1;
      ColonWidget* m_colon_widget2;
      int m_last_valid_hour;
      int m_last_valid_minute;
      int m_last_valid_second;

      QString clamped_value(const QString& text, int min_value, int max_value);
      QString clamped_value(const QString& text, int min_value, int max_value,
        int addend);
      QString get_line_edit_value(const QString& text, int key, int min_value,
        int max_value);
      void set_focus_style();
      void set_unfocused_style();
      void set_style(const QString& border_hex);
      void on_time_changed();
  };
}

#endif
