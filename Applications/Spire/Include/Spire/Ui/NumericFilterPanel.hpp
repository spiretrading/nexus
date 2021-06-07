#ifndef SPIRE_NUMERIC_FILTER_PANEL_HPP
#define SPIRE_NUMERIC_FILTER_PANEL_HPP
#include <QWidget>
#include <boost/optional/optional.hpp>
#include "Spire/Spire/Decimal.hpp"
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/FilterPanel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Represents a numeric fliter panel.
   */
  class NumericFilterPanel : public QWidget {
    public:

      /** Indicates the numerical range. */
      struct NumericRange {

        /** The minimum value or none if there is no minimum. */
        boost::optional<Decimal> m_min;

        /** The maximum value or none if there is no maximum. */
        boost::optional<Decimal> m_max;
      };

      /** Type of model used by the NumericFilterPanel. */
      using NumericFilterModel = ValueModel<NumericRange>;

      /** Signals that the filter will be triggered. */
      using FilterSignal = Signal<void ()>;

      /**
       * Constructs a NumericFilterPanel with a LocalValueModel.
       * @param title The title of the panel.
       * @param default_value The default value that is used to set the default
       *                      minimum and maximum value.
       * @param parent The parent widget that shows the panel.
       */
      NumericFilterPanel(const QString& title,
        const NumericRange& default_value, QWidget* parent);

      /**
       * Constructs a NumericFilterPanel.
       * @param model The model used for the minimum and maximum value.
       * @param title The title of the panel.
       * @param default_value The default value that is used to set the default
       *                      minimum and maximum value.
       * @param parent The parent widget that shows the panel.
       */
      NumericFilterPanel(std::shared_ptr<NumericFilterModel> model,
        const QString& title, const NumericRange& default_value,
        QWidget* parent);

      /** Returns the model of the minimum and maximum value. */
      const std::shared_ptr<NumericFilterModel>& get_model() const;

      /** Returns the default minimum and maximum value. */
      NumericRange get_default_value() const;

      /** Sets the default minimum and maximum value. */
      void set_default_value(const NumericRange& default_value);

      /** Connects a slot to the filter signal. */
      boost::signals2::connection connect_filter_signal(
        const FilterSignal::slot_type& slot) const;

    protected:
      bool event(QEvent* event) override;

    private:
      mutable FilterSignal m_filter_signal;
      std::shared_ptr<NumericFilterModel> m_model;
      NumericRange m_default_value;
      FilterPanel* m_filter_panel;
      boost::signals2::scoped_connection m_current_connection;

      void on_style(DecimalBox* field);
  };
}

#endif
