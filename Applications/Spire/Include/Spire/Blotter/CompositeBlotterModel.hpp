#ifndef SPIRE_COMPOSITE_BLOTTER_MODEL_HPP
#define SPIRE_COMPOSITE_BLOTTER_MODEL_HPP
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterModel.hpp"

namespace Spire {

  /** Implements the BlotterModel by composing constituent models together. */
  class CompositeBlotterModel : public BlotterModel {
    public:

      /**
       * Constructs a CompositeBlotterModel.
       * @param name Represents the name of this blotter.
       * @param is_active Whether this blotter is the active blotter.
       * @param is_pinned Whether this blotter is pinned.
       * @param markets The market database to use.
       * @param currency The blotter's primary currency.
       * @param exchange_rate The table of exchange rates used to convert
       *        currencies to the blotter's primary currency.
       * @param buying_power The account's buying power.
       * @param net_loss The account's allowable net loss.
       * @param tasks The list of tasks belonging to this blotter.
       * @param task_selection The list of selected tasks.
       * @param valuation The valuation used by this blotter.
       * @param orders Represents this blotter's orders.
       * @param position_selection The list of selected positions.
       */
      CompositeBlotterModel(std::shared_ptr<TextModel> name,
        std::shared_ptr<BooleanModel> is_active,
        std::shared_ptr<BooleanModel> is_pinned, Nexus::MarketDatabase markets,
        Nexus::CurrencyId currency, Nexus::ExchangeRateTable exchange_rates,
        std::shared_ptr<MoneyModel> buying_power,
        std::shared_ptr<MoneyModel> net_loss,
        std::shared_ptr<BlotterTaskListModel> tasks,
        std::shared_ptr<ListModel<int>> task_selection,
        std::shared_ptr<ValuationModel> valuation,
        std::shared_ptr<ListModel<int>> position_selection);

      std::shared_ptr<TextModel> get_name() override;

      std::shared_ptr<BooleanModel> is_active() override;

      std::shared_ptr<BooleanModel> is_pinned() override;

      const Nexus::MarketDatabase& get_markets() const override;

      Nexus::CurrencyId get_currency() const override;

      const Nexus::ExchangeRateTable& get_exchange_rates() const override;

      std::shared_ptr<MoneyModel> get_buying_power() override;

      std::shared_ptr<MoneyModel> get_net_loss() override;

      std::shared_ptr<BlotterTaskListModel> get_tasks() override;

      std::shared_ptr<ListModel<int>> get_task_selection() const override;

      std::shared_ptr<ValuationModel> get_valuation() override;

      std::shared_ptr<ListModel<int>> get_position_selection() const override;

    private:
      std::shared_ptr<TextModel> m_name;
      std::shared_ptr<BooleanModel> m_is_active;
      std::shared_ptr<BooleanModel> m_is_pinned;
      Nexus::MarketDatabase m_markets;
      Nexus::CurrencyId m_currency;
      Nexus::ExchangeRateTable m_exchange_rates;
      std::shared_ptr<MoneyModel> m_buying_power;
      std::shared_ptr<MoneyModel> m_net_loss;
      std::shared_ptr<BlotterTaskListModel> m_tasks;
      std::shared_ptr<ListModel<int>> m_task_selection;
      std::shared_ptr<ValuationModel> m_valuation;
      std::shared_ptr<ListModel<int>> m_position_selection;
  };

  /**
   * Makes a CompositeBlotterModel using local models as its constituents.
   * @param markets The market database to use.
   * @param currency The blotter's primary currency.
   * @param exchange_rate The table of exchange rates used to convert currencies
   *        to the blotter's primary currency.
   */
  std::shared_ptr<CompositeBlotterModel> make_local_blotter_model(
    Nexus::MarketDatabase markets, Nexus::CurrencyId currency,
    Nexus::ExchangeRateTable exchange_rates);

  /**
   * Makes a CompositeBlotterModel whose positions, orders, profit and loss,
   * and status is derived from its tasks model.
   * @param name Represents the name of this blotter.
   * @param is_active Whether this blotter is the active blotter.
   * @param is_pinned Whether this blotter is pinned.
   * @param markets The market database to use.
   * @param currency The blotter's primary currency.
   * @param exchange_rate The table of exchange rates used to convert currencies
   *        to the blotter's primary currency.
   * @param buying_power The account's buying power.
   * @param net_loss The account's allowable net loss.
   * @param tasks The list of tasks belonging to this blotter.
   * @param valuation The valuation to use.
   */
  std::shared_ptr<CompositeBlotterModel> make_derived_blotter_model(
    std::shared_ptr<TextModel> name, std::shared_ptr<BooleanModel> is_active,
    std::shared_ptr<BooleanModel> is_pinned, Nexus::MarketDatabase markets,
    Nexus::CurrencyId currency, Nexus::ExchangeRateTable exchange_rates,
    std::shared_ptr<MoneyModel> buying_power,
    std::shared_ptr<MoneyModel> net_loss,
    std::shared_ptr<BlotterTaskListModel> tasks,
    std::shared_ptr<ValuationModel> valuation);
}

#endif