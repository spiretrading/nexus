#ifndef SPIRE_TIME_AND_SALES_PROPERTIES_WINDOW_FACTORY_HPP
#define SPIRE_TIME_AND_SALES_PROPERTIES_WINDOW_FACTORY_HPP
#include "Spire/TimeAndSales/TimeAndSalesPropertiesWindow.hpp"
#include "Spire/TimeAndSales/TimeAndSales.hpp"

namespace Spire {

  /** A factory used to create the TimeAndSalesPropertiesWindow. */
  class TimeAndSalesPropertiesWindowFactory {
    public:

      /**
       * Constructs a TimeAndSalesPropertiesWindowFactory using the default
       * model.
       */
      TimeAndSalesPropertiesWindowFactory();

      /**
       * Constructs a TimeAndSalesPropertiesWindowFactory.
       * @param properties The properties model of the time and sales.
       */
      explicit TimeAndSalesPropertiesWindowFactory(
        std::shared_ptr<TimeAndSalesPropertiesModel> properties);

      /* Returns the properties model. */
      const std::shared_ptr<TimeAndSalesPropertiesModel>&
        get_properties() const;

      /* Creates a TimeAndSalesPropertiesWindow. */
      TimeAndSalesPropertiesWindow* create();

    private:
      std::shared_ptr<TimeAndSalesPropertiesModel> m_properties;
      std::unique_ptr<TimeAndSalesPropertiesWindow> m_properties_window;
  };
}

#endif