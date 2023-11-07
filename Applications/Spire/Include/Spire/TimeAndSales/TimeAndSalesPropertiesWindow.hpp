#ifndef SPIRE_TIME_AND_SALES_PROPERTIES_WINDOW_HPP
#define SPIRE_TIME_AND_SALES_PROPERTIES_WINDOW_HPP
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include "Spire/TimeAndSales/TimeAndSales.hpp"
#include "Spire/Spire/Spire.hpp"
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
       * @param properties The properties of the time and sales.
       * @param parent The parent widget.
       */
      explicit TimeAndSalesPropertiesWindow(
        std::shared_ptr<TimeAndSalesPropertiesModel> properties,
        QWidget* parent = nullptr);

      /* Returns the properties. */
      const std::shared_ptr<TimeAndSalesPropertiesModel>&
        get_properties() const;

    private:
      struct PropertiesWindowModel;
      std::unique_ptr<PropertiesWindowModel> m_model;
      QWidget* m_body;

      void on_font(const QFont& font);
  };
}

#endif
