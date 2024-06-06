#ifndef SPIRE_TIME_AND_SALES_PROPERTIES_WINDOW_HPP
#define SPIRE_TIME_AND_SALES_PROPERTIES_WINDOW_HPP
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** A ValueModel over a TimeAndSalesProperties. */
  using TimeAndSalesPropertiesModel = ValueModel<TimeAndSalesProperties>;

  /** A LocalValueModel over a TimeAndSalesProperties. */
  using LocalTimeAndSalesPropertiesModel =
    LocalValueModel<TimeAndSalesProperties>;

  /** Displays the properties for the time and sales. */
  class TimeAndSalesPropertiesWindow : public Window {
    public:

      /**
       * Constructs a TimeAndSalesPropertiesWindow.
       * @param current The initial current properties.
       * @param parent The parent widget.
       */
      explicit TimeAndSalesPropertiesWindow(
        std::shared_ptr<TimeAndSalesPropertiesModel> current,
        QWidget* parent = nullptr);

      /** Returns the current properties. */
      const std::shared_ptr<TimeAndSalesPropertiesModel>&
        get_current() const;

    protected:
      void hideEvent(QHideEvent* event) override;

    private:
      struct PropertiesWindowModel;
      std::unique_ptr<PropertiesWindowModel> m_model;
      TimeAndSalesProperties m_initial_properties;

      void on_font(const QFont& font);
      void on_cancel();
      void on_done();
  };
}

#endif
