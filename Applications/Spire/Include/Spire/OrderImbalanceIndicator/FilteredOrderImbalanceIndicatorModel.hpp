#ifndef SPIRE_FILTERED_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#define SPIRE_FILTERED_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"

namespace Spire {

  //! Represents an OrderImbalanceModel that applies custom filters to the
  //! data it provides.
  class FilteredOrderImbalanceIndicatorModel :
      public OrderImbalanceIndicatorModel {
    public:

      //! Defines the filter function return type and signature.
      //! Filter functions return true if an imbalance should not be filtered
      //! out.
      using Filter = std::function<bool (const Nexus::OrderImbalance&)>;

      //! Constructs a FilteredOrderImbalanceIndicatorModel with a given
      //! source model and specific filters.
      /*
        \param source_model The model supplying data to the filtered model.
        \param filters The functions applied to order imbalances as filters.
      */
      FilteredOrderImbalanceIndicatorModel(
        std::shared_ptr<OrderImbalanceIndicatorModel> source_model,
        std::vector<Filter> filters);

      QtPromise<std::vector<Nexus::OrderImbalance>> load(
        const TimeInterval& interval) override;

      QtPromise<std::vector<Nexus::OrderImbalance>> load(
        const Nexus::Security& security,
        const TimeInterval& interval) override;

      SubscriptionResult<boost::optional<Nexus::OrderImbalance>>
        subscribe(const OrderImbalanceSignal::slot_type& slot) override;

    private:
      std::shared_ptr<OrderImbalanceIndicatorModel> m_source_model;
      std::shared_ptr<std::vector<Filter>> m_filters;
  };

  //! Returns true if the imbalance is filtered out.
  /*
    \param filters The filters to apply to the imbalance.
    \param imbalance The imbalance to apply the filters to.
  */
  bool is_imbalance_accepted(
    const std::vector<FilteredOrderImbalanceIndicatorModel::Filter>& filters,
    const Nexus::OrderImbalance& imbalance);

  //! Returns the accepted imbalances from the provided imbalances, after
  //! applying the given filters.
  /*
    \param imbalances The imbalances to have filters applied to.
    \param filters The filters to apply.
  */
  std::vector<Nexus::OrderImbalance> filter_imbalances(
    const std::vector<FilteredOrderImbalanceIndicatorModel::Filter>& filters,
    const std::vector<Nexus::OrderImbalance>& imbalances);

  //! Creates a filter that filters out securities that do not appear in the
  //! given list.
  /*
    \param security_list The list of securities to preserve after filtering.
  */
  FilteredOrderImbalanceIndicatorModel::Filter make_security_list_filter(
    const std::set<Nexus::Security>& security_list);

  //! Creates a filter that filters out symbols that do not start with
  //! or match the given string.
  /*
    \param prefix The string to compare the order imbalances symbols to.
  */
  FilteredOrderImbalanceIndicatorModel::Filter make_symbol_filter(
    const std::string& prefix);

  //! Creates a filter that filters out markets that do not appear in the given
  //! list.
  /*
    \param market_list The list of markets to preserve after filtering.
    \param market_database Market database used for converting display names
                           to market codes.
  */
  FilteredOrderImbalanceIndicatorModel::Filter make_market_list_filter(
    const std::set<std::string>& market_list,
    const Nexus::MarketDatabase& market_database);

  //! Creates a filter that filters out markets that do not start with
  //! or match the given string.
  /*
    \param prefix The string to compare the order imbalance markets to.
    \param market_database Market database used for converting display names
                           to market codes.
  */
  FilteredOrderImbalanceIndicatorModel::Filter make_market_filter(
    const std::string& prefix,
    const Nexus::MarketDatabase& market_database);

  //! Creates a filter that filters out sides that do not match the given
  //! order imbalance side.
  /*
    \param side The given side to preserve after filtering.
  */
  FilteredOrderImbalanceIndicatorModel::Filter make_side_filter(
    Nexus::Side side);

  //! Creates a filter that filters out sizes that are not in the interval
  //! [min, max].
  /*
    \param min The lowest size that will not be filtered out.
    \param max The largest size that will not be filtered out.
  */
  FilteredOrderImbalanceIndicatorModel::Filter make_size_filter(
    Nexus::Quantity min, Nexus::Quantity max);

  //! Creates a filter that filters out reference prices that are not in the
  //! interval [min, max].
  /*
    \param min The lowest reference price that will not be filtered out.
    \param max The highest reference price that will not be filtered out.
  */
  FilteredOrderImbalanceIndicatorModel::Filter make_reference_price_filter(
    Nexus::Money min, Nexus::Money max);

  //! Creates a filter that filters out notional values that are not in the
  //! interval [min, max].
  /*
    \param min The lowest notional value that will not be filtered out.
    \param max The high notional value that willnot be filtered out.
  */
  FilteredOrderImbalanceIndicatorModel::Filter make_notional_value_filter(
    Nexus::Money min, Nexus::Money max);
}

#endif
