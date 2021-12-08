#ifndef SPIRE_DATE_FILTER_PANEL_HPP
#define SPIRE_DATE_FILTER_PANEL_HPP
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a FilterPanel to filter by a date range. */
  class DateFilterPanel : public QWidget {
    public:

      /** The date unit for the offset. */
      enum class DateUnit {

        /** Day. */
        DAY,

        /** Week. */
        WEEK,

        /** Month. */
        MONTH,

        /** Year. */
        YEAR
      };

      /** The date duration used as an offset from the current date. */
      struct DateOffset {

        /** The date unit. */
        DateUnit m_unit;

        /** The number of units to offset by. */
        int m_value;
      };

      /** Indicates the date range. */
      struct DateRange {

        /** The range's start date. */
        boost::optional<boost::gregorian::date> m_start;

        /** The range's end date. */
        boost::optional<boost::gregorian::date> m_end;

        /** The offset from the current date. */
        boost::optional<DateOffset> m_offset;
      };

      /** A ValueModel over a DateRange. */
      using DateRangeModel = ValueModel<DateRange>;

      /**
       * Signals that the date range for the filter was submitted.
       * @param submission The date range.
       */
      using SubmitSignal = Signal<void(const DateRange& submission)>;

      /**
       * Constructs a DateFilterPanel using a local Model.
       * @param default_range The default date range.
       * @param parent The parent widget showing the panel.
       */
      DateFilterPanel(DateRange default_range, QWidget& parent);

      /**
       * Constructs a DateFilterPanel.
       * @param model The model used for the date range.
       * @param default_range The default date range.
       * @param parent The parent widget that shows the panel.
       */
      DateFilterPanel(std::shared_ptr<DateRangeModel> model,
        DateRange default_range, QWidget& parent);

      /** Returns the model. */
      const std::shared_ptr<DateRangeModel>& get_model() const;

      /** Returns the default date range. */
      const DateRange& get_default_range() const;

      /** Sets the default date range. */
      void set_default_range(const DateRange& default_range);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const typename SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      enum class DateRangeType {
        OFFSET,
        RANGE,
      };
      class DateRangeTypeButtonGroup;
      struct DateRangeComposerModel;
      mutable SubmitSignal m_submit_signal;
      std::unique_ptr<DateRangeComposerModel> m_model;
      DateRange m_default_date_range;
      QWidget* m_offset_body;
      QWidget* m_range_body;
      FilterPanel* m_filter_panel;
      std::unique_ptr<DateRangeTypeButtonGroup> m_range_type_button_group;
      boost::signals2::scoped_connection m_date_unit_connection;

      void on_date_range_type_current(DateRangeType type);
      void on_date_unit_current(DateUnit unit);
      void on_end_date_submit(
        const boost::optional<boost::gregorian::date>& submission);
      void on_start_date_submit(
        const boost::optional<boost::gregorian::date>& submission);
      void on_offset_value_submit(const boost::optional<int>& submission);
      void on_reset();
  };
}

#endif
