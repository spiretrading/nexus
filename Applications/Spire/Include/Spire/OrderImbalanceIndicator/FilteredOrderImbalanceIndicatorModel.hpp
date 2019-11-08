#ifndef SPIRE_FILTERED_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#define SPIRE_FILTERED_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"

namespace Spire {

  //! Represents an OrderImbalanceModel that applies custom filters to the
  //! data it provides.
  class FilteredOrderImbalanceIndicatorModel :
      public OrderImbalanceIndicatorModel {
    public:

      //! Constructs a FilteredOrderImbalanceIndicatorModel with a given
      //! source model and specific filters.
      /*
        \param source_model The model supplying data to the filtered model.
        \param filters The functions applied to order imbalances as filters.
                       Filter functions return true if an imbalance should not
                       be filtered out.
      */
      FilteredOrderImbalanceIndicatorModel(
        std::shared_ptr<OrderImbalanceIndicatorModel> source_model,
        std::vector<std::function<bool (const Nexus::OrderImbalance&)>>
          filters);

      std::tuple<boost::signals2::connection,
        QtPromise<std::vector<Nexus::OrderImbalance>>>
        subscribe(const boost::posix_time::ptime& start,
          const boost::posix_time::ptime& end,
          const OrderImbalanceSignal::slot_type& slot) override;

    private:

      struct FilteredOrderImbalanceSignalConnection :
          OrderImbalanceSignalConnection {
        
        boost::signals2::scoped_connection m_source_signal_connection;
      };

      std::shared_ptr<OrderImbalanceIndicatorModel> m_source_model;
      std::vector<std::unique_ptr<FilteredOrderImbalanceSignalConnection>>
        m_signals;
      std::vector<std::function<bool (const Nexus::OrderImbalance&)>>
        m_filters;

      bool is_imbalance_accepted(
        const Nexus::OrderImbalance& imbalance);
      std::vector<Nexus::OrderImbalance> filter_imbalances(
        const std::vector<Nexus::OrderImbalance>& imbalances);
  };

  //! Creates a filter that filters out symbols that do not appear in the
  //! given list.
  /*
    \param symbol_list The list of securities to preserve after filtering.
  */
  std::function<bool (const Nexus::OrderImbalance&)> make_security_list_filter(
    const std::set<std::string>& symbol_list);

  //! Creates a filter that filters out symbols that do not start with
  //! or match the given string.
  /*
    \param filter_string The string to compare the order imbalances symbols to.
  */
  std::function<bool (const Nexus::OrderImbalance&)> make_security_filter(
    const std::string& filter_string);

  //! Creates a filter that filters out markets that do not appear in the given
  //! list.
  /*
    \param market_list The list of markets to preserve after filtering.
  */
  std::function<bool (const Nexus::OrderImbalance&)> make_market_list_filter(
    const std::set<std::string>& market_list);

  //! Creates a filter that filters out markets that do not start with
  //! or match the given string.
  /*
    \param filter_string The string to compare the order imbalance markets to.
  */
  std::function<bool (const Nexus::OrderImbalance&)> make_market_filter(
    const std::string& filter_string);

  //! Creates a filter that filters out sides that do not match the given
  //! order imbalance side.
  /*
    \param side The given side to preserve after filtering.
  */
  std::function<bool (const Nexus::OrderImbalance&)> make_side_filter(
    Nexus::Side side);

  //! Creates a filter that filters out sizes that are not in the interval
  //! [min, max].
  /*
    \param min The lowest size that will not be filtered out.
    \param max The largest size that will not be filtered out.
  */
  std::function<bool (const Nexus::OrderImbalance&)> make_size_filter(
    const Nexus::Quantity& min, const Nexus::Quantity& max);

  //! Creates a filter that filters out reference prices that are not in the
  //! interval [min, max].
  /*
    \param min The lowest reference price that will not be filtered out.
    \param max The highest reference price that will not be filtered out.
  */
  std::function<bool (const Nexus::OrderImbalance&)>
    make_reference_price_filter(const Nexus::Money& min,
      const Nexus::Money& max);

  //! Creates a filter that filters out notional values that are not in the
  //! interval [min, max].
  /*
    \param min The lowest notional value that will not be filtered out.
    \param max The high notional value that willnot be filtered out.
  */
  std::function<bool (const Nexus::OrderImbalance&)>
    make_notional_value_filter(const Nexus::Money& min,
      const Nexus::Money& max);
}

#endif
