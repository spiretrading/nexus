#ifndef SPIRE_DATE_BOX_HPP
#define SPIRE_DATE_BOX_HPP
#include <QWidget>
#include "Spire/Ui/CalendarDatePicker.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/IntegerBox.hpp"

namespace Spire {
  class Box;
  class Button;
  class OverlayPanel;

namespace Styles {

  /** Sets whether the DateBox displays the year field. */
  using YearField = BasicProperty<bool, struct YearFieldTag>;
}

  /** Displays an input field for a date. */
  class DateBox : public QWidget {
    public:

      /**
       * Signals that the current value is being submitted.
       * @param submission The submitted date.
       */
      using SubmitSignal = Signal<
        void (const boost::optional<boost::gregorian::date>& submission)>;

      /**
       * Signals that the current value was rejected as a submission.
       * @param value The rejected value.
       */
      using RejectSignal = Signal<
        void (const boost::optional<boost::gregorian::date>& value)>;

      /**
       * Constructs a DateBox with a local model set to the current system's
       * date.
       * @param parent The parent widget.
       */
      explicit DateBox(QWidget* parent = nullptr);

      /**
       * Constructs a DateBox using a local model set to an initial value.
       * @param current The initial value to display.
       * @param parent The parent widget.
       */
      explicit DateBox(const boost::optional<boost::gregorian::date>& current,
        QWidget* parent = nullptr);

      /**
       * Constructs a DateBox.
       * @param parent The parent widget.
       */
      explicit DateBox(
        std::shared_ptr<OptionalDateModel> current, QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<OptionalDateModel>& get_current() const;

      /** Returns the last submission. */
      const boost::optional<boost::gregorian::date>& get_submission() const;

      /** Returns <code>true</code> iff the DateBox is read-only. */
      bool is_read_only() const;

      /** Sets whether the DateBox is read-only. */
      void set_read_only(bool read_only);

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      /** Connects a slot to the RejectedSignal. */
      boost::signals2::connection connect_reject_signal(
        const RejectSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;

    private:
      struct DateComposerModel;
      struct Field {
        IntegerBox* m_box;
        QLineEdit* m_editor;
      };
      struct Fields {
        Field m_year;
        Field m_month;
        Field m_day;
      };
      mutable SubmitSignal m_submit_signal;
      mutable RejectSignal m_reject_signal;
      std::shared_ptr<DateComposerModel> m_model;
      boost::optional<boost::gregorian::date> m_submission;
      bool m_is_read_only;
      bool m_is_rejected;
      FocusObserver m_focus_observer;
      Fields m_fields;
      QWidget* m_year_dash;
      QWidget* m_date_components;
      Button* m_calendar_button;
      Box* m_input_box;
      OverlayPanel* m_date_picker_panel;
      bool m_date_picker_showing;
      boost::signals2::scoped_connection m_style_connection;

      void set_rejected(bool rejected);
      void focus_and_select_all(const Field& field);
      void show_date_picker();
      void on_year_edited(const QString& text);
      void on_month_edited(const QString& text);
      void on_day_edited(const QString& text);
      void on_button_click();
      void on_field_reject(boost::optional<int> value);
      void on_current(const boost::optional<boost::gregorian::date>& current);
      void on_submit();
      void on_focus(FocusObserver::State state);
      void on_style();
  };
}

#endif
