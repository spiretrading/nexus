#ifndef SPIRE_TIME_AND_SALES_PROPERTIES_WINDOW_FACTORY_HPP
#define SPIRE_TIME_AND_SALES_PROPERTIES_WINDOW_FACTORY_HPP
#include "Spire/Spire/ProxyValueModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesPropertiesWindow.hpp"

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

      /** Returns the properties displayed. */
      const std::shared_ptr<TimeAndSalesPropertiesModel>&
        get_properties() const;

      /**
       * Returns a TimeAndSalesPropertiesWindow that preview edits live through
       * the given proxy.
       * @param live_preview The proxy that receives live edits.
       */
      TimeAndSalesPropertiesWindow* make(
        std::shared_ptr<ProxyValueModel<TimeAndSalesProperties>> live_preview);

    private:
      std::shared_ptr<TimeAndSalesPropertiesModel> m_properties;
      std::unique_ptr<TimeAndSalesPropertiesWindow> m_properties_window;
      std::shared_ptr<LocalTimeAndSalesPropertiesModel> m_preview;
      std::shared_ptr<ProxyValueModel<TimeAndSalesProperties>> m_live_preview;
      boost::signals2::scoped_connection m_commit_connection;
      boost::signals2::scoped_connection m_cancel_connection;

      void on_commit();
      void on_cancel();
  };
}

#endif
